# cli_test.py
# 智能密码锁 - 命令行调试控制台（无 UI，与 PyQt 版功能等价）

import sys
import os
import time
import threading
from datetime import datetime

import serial
import serial.tools.list_ports

# ---- Windows 下启用 ANSI 颜色 ----
if os.name == "nt":
    try:
        import ctypes
        _k = ctypes.windll.kernel32
        _k.SetConsoleMode(_k.GetStdHandle(-11), 7)
    except Exception:
        pass

BAUD            = 9600
PREFERRED_PORT  = "COM7"
FALLBACK_PORT   = "COM4"
PROBE_TIMEOUT   = 1.2
PING_INTERVAL   = 0.2
RETRY_DELAY     = 2.0

C = {
    "red":    "\033[91m",
    "green":  "\033[92m",
    "yellow": "\033[93m",
    "cyan":   "\033[96m",
    "orange": "\033[33m",
    "gray":   "\033[90m",
    "reset":  "\033[0m",
}

_print_lock = threading.Lock()


def log(msg, color="reset"):
    ts = datetime.now().strftime("%H:%M:%S")
    with _print_lock:
        # 清掉当前 input 行，避免后台日志和用户输入混在一起
        sys.stdout.write("\r\033[K")
        sys.stdout.write(f"{C['gray']}[{ts}]{C['reset']} {C.get(color, '')}{msg}{C['reset']}\n")
        sys.stdout.flush()


class SerialWorker:
    """单线程：发现 → 监听 → 断线回到发现"""

    def __init__(self):
        self.ser = None
        self.active_port = None
        self.running = True
        self.lock = threading.Lock()
        self.rediscover_event = threading.Event()

    # ---------- 发现阶段 ----------
    def _probe_port(self, port_name):
        try:
            s = serial.Serial(port_name, BAUD, timeout=0.05)
        except (serial.SerialException, OSError):
            return None
        try:
            time.sleep(0.05)
            s.reset_input_buffer()
            t_start = time.time()
            while time.time() - t_start < PROBE_TIMEOUT:
                try:
                    s.write(b"PING\n")
                except (serial.SerialException, OSError):
                    s.close()
                    return None
                t_read = time.time()
                while time.time() - t_read < PING_INTERVAL:
                    if s.in_waiting:
                        buf = s.read(s.in_waiting)
                        if b"PONG" in buf:
                            return s
                    time.sleep(0.02)
            s.close()
            return None
        except Exception:
            try: s.close()
            except: pass
            return None

    def _discover(self):
        existing = {p.device for p in serial.tools.list_ports.comports()}
        candidates = [p for p in (PREFERRED_PORT, FALLBACK_PORT) if p in existing]
        if not candidates:
            return None, None

        log(f"正在扫描: {', '.join(candidates)}（优先 {PREFERRED_PORT}）", "orange")
        for port in candidates:
            s = self._probe_port(port)
            if s is not None:
                return port, s
        return None, None

    def _send_port_status(self, port):
        if self.ser is None or not self.ser.is_open:
            return
        cmd = (f"PORT:{port}\n" if port else "PORT:NONE\n").encode()
        try:
            with self.lock:
                self.ser.write(cmd)
        except (serial.SerialException, OSError):
            pass

    # ---------- 主循环 ----------
    def run(self):
        while self.running:
            if self.ser is None:
                port, ser = self._discover()
                if port:
                    self.ser = ser
                    self.active_port = port
                    self._send_port_status(port)
                    log(f"自动识别成功: {port}", "green")
                else:
                    log(f"未发现设备，{RETRY_DELAY:.0f} 秒后重试...", "red")
                    for _ in range(int(RETRY_DELAY * 10)):
                        if not self.running:
                            break
                        if self.rediscover_event.is_set():
                            self.rediscover_event.clear()
                            break
                        time.sleep(0.1)
                    continue

            # ---- 监听阶段 ----
            try:
                if self.ser.in_waiting > 0:
                    data = self.ser.read(self.ser.in_waiting).decode("utf-8", errors="ignore").strip()
                    self._handle_data(data)
            except (serial.SerialException, OSError):
                log(f"{self.active_port} 断开", "red")
                try: self.ser.close()
                except: pass
                self.ser = None
                self.active_port = None
                continue

            if self.rediscover_event.is_set():
                self.rediscover_event.clear()
                log("收到重新扫描请求", "orange")
                self._send_port_status(None)
                try: self.ser.close()
                except: pass
                self.ser = None
                self.active_port = None
                continue

            time.sleep(0.05)

    # ---------- 数据分发 ----------
    def _handle_data(self, data):
        port = self.active_port
        if not data:
            return
        if "PONG" in data:
            return
        if "REQ_TIME" in data:
            t = datetime.now().strftime("T:%Y%m%d%H%M%S\n")
            with self.lock:
                if self.ser and self.ser.is_open:
                    try:
                        self.ser.write(t.encode())
                        log(f"[{port}] 校时请求，已下发: {t.strip()}", "cyan")
                    except (serial.SerialException, OSError):
                        pass
        elif "TIME_OK" in data:
            log(f"[{port}] 时间同步成功", "green")
        elif "TIME_FAIL" in data:
            log(f"[{port}] 时间同步失败", "red")
        elif "SUCCESS" in data:
            log(f"[{port} 解锁成功]: {data}", "green")
        elif "WRONG" in data or "LOCKED" in data:
            log(f"[{port} 验证失败]: {data}", "red")
        else:
            log(f"[{port} 反馈]: {data}", "gray")

    # ---------- 对外接口 ----------
    def send_password(self, password):
        if self.ser is None or not self.ser.is_open:
            log("发送失败：设备未连接", "red")
            return
        msg = f"PASS:{password}\n".encode()
        try:
            with self.lock:
                self.ser.write(msg)
            log(f"密码已通过 {self.active_port} 发送！", "yellow")
        except (serial.SerialException, OSError):
            log("发送失败：串口异常", "red")

    def rediscover(self):
        self.rediscover_event.set()

    def start(self):
        threading.Thread(target=self.run, daemon=True).start()

    def stop(self):
        self.running = False
        if self.ser is not None:
            try: self.ser.close()
            except: pass
            self.ser = None


def print_help():
    print("命令:")
    print("  直接输入密码 + 回车 → 发送密码")
    print("  r + 回车           → 重新扫描端口")
    print("  s + 回车           → 显示当前状态")
    print("  h + 回车           → 显示帮助")
    print("  q + 回车           → 退出")


def main():
    print("=" * 52)
    print("  智能密码锁 - 命令行调试控制台（无 UI）")
    print("=" * 52)
    print_help()
    print("=" * 52)

    worker = SerialWorker()
    worker.start()

    try:
        while True:
            try:
                line = input("> ")
            except (EOFError, KeyboardInterrupt):
                print()
                break

            cmd = line.strip()
            if not cmd:
                continue
            if cmd == "q":
                break
            elif cmd == "r":
                log("手动触发重新扫描", "orange")
                worker.rediscover()
            elif cmd == "s":
                if worker.active_port:
                    log(f"当前连接端口: {worker.active_port}", "green")
                else:
                    log("当前无可用端口", "red")
            elif cmd == "h":
                print_help()
            else:
                worker.send_password(cmd)
    finally:
        worker.stop()
        print("已退出")


if __name__ == "__main__":
    main()