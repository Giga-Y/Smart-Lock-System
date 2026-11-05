import sys
import time
import threading
from datetime import datetime

import serial
import serial.tools.list_ports

from PyQt6.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout,
                             QHBoxLayout, QLabel, QTextEdit, QLineEdit,
                             QPushButton, QGroupBox)
from PyQt6.QtCore import pyqtSignal, QObject, Qt

BAUD = 9600
PREFERRED_PORT = "COM7"  # 优先端口
FALLBACK_PORT = "COM4"  # 备选端口
PROBE_TIMEOUT = 1.2  # 每个端口探测总时长（s）
PING_INTERVAL = 0.2  # 探测时每次 PING 间隔（s）
RETRY_DELAY = 2.0  # 未发现设备时重试间隔（s）


class Communicate(QObject):
    log_signal = pyqtSignal(str, str)  # (msg, color)
    status_signal = pyqtSignal(str, bool)  # (port, is_connected)
    active_signal = pyqtSignal(str)  # 当前 active 端口，空串表示未识别


class SerialWorker:
    """单线程：发现 → 监听 → 断线回到发现"""

    def __init__(self, comm):
        self.comm = comm
        self.ser = None
        self.active_port = None
        self.running = True
        self.lock = threading.Lock()  # 保护 self.ser
        self.rediscover_event = threading.Event()  # 手动触发重新扫描

        # 【新增】串口接收缓冲区，用于处理分包/截断问题
        self.rx_buffer = ""

    def log(self, msg, color="black"):
        self.comm.log_signal.emit(msg, color)

    # ---------- 发现阶段 ----------
    def _probe_port(self, port_name):
        """成功返回打开的句柄，失败返回 None；句柄归调用者所有"""
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
                            return s  # 成功，句柄交给调用者
                    time.sleep(0.02)
            s.close()
            return None
        except Exception:
            try:
                s.close()
            except:
                pass
            return None

    def _discover(self):
        existing = {p.device for p in serial.tools.list_ports.comports()}
        candidates = [p for p in (PREFERRED_PORT, FALLBACK_PORT) if p in existing]

        if not candidates:
            return None, None

        self.log(f"🔍 正在扫描: {', '.join(candidates)}（优先 {PREFERRED_PORT}）", "#FFA500")
        for port in candidates:
            s = self._probe_port(port)
            if s is not None:
                return port, s
        return None, None

    def _send_port_status(self, port):
        """通知下位机当前使用的端口；port 为空表示无可用端口"""
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
                    self.rx_buffer = ""  # 连接成功后清空残留缓存
                    self._send_port_status(port)  # 通知下位机当前端口
                    self.comm.status_signal.emit(port, True)
                    self.comm.active_signal.emit(port)
                    self.log(f"✅ 自动识别成功: {port}", "#55FF55")
                else:
                    self.comm.active_signal.emit("")
                    self.log(f"⚠️ 未发现设备，{RETRY_DELAY:.0f} 秒后重试...", "#FF5555")
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
                    # 【修改】接收数据但不直接处理，累加到缓冲区
                    raw_data = self.ser.read(self.ser.in_waiting).decode("utf-8", errors="ignore")
                    self.rx_buffer += raw_data

                    # 只要缓冲区里有换行符，说明凑齐了完整的指令
                    while "\n" in self.rx_buffer:
                        line, self.rx_buffer = self.rx_buffer.split("\n", 1)
                        line = line.strip()  # 剔除首尾的 \r 和空格
                        if line:
                            self._handle_data(line)

                    # 防止异常数据冲爆内存（超长不换行直接清空）
                    if len(self.rx_buffer) > 1024:
                        self.rx_buffer = ""

            except (serial.SerialException, OSError):
                self.log(f"⚠️ {self.active_port} 断开", "#FF5555")
                self.comm.status_signal.emit(self.active_port, False)
                self.comm.active_signal.emit("")
                try:
                    self.ser.close()
                except:
                    pass
                self.ser = None
                self.active_port = None
                continue

            if self.rediscover_event.is_set():
                self.rediscover_event.clear()
                self.log("🔁 收到重新扫描请求", "#FFA500")
                self._send_port_status(None)  # 先通知下位机无可用端口
                if self.active_port:
                    self.comm.status_signal.emit(self.active_port, False)
                try:
                    self.ser.close()
                except:
                    pass
                self.ser = None
                self.active_port = None
                self.comm.active_signal.emit("")
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
                        self.log(f"⏰ [{port}] 校时请求，已下发: {t.strip()}", "#00BFFF")
                    except (serial.SerialException, OSError):
                        pass
        elif "TIME_OK" in data:
            self.log(f"✅ [{port}] 时间同步成功", "#55FF55")
        elif "TIME_FAIL" in data:
            self.log(f"❌ [{port}] 时间同步失败", "#FF5555")
        elif "SUCCESS" in data:
            self.log(f"🔐 [{port} 解锁成功]: {data}", "#55FF55")
        elif "WRONG" in data or "LOCKED" in data:
            self.log(f"🔐 [{port} 验证失败]: {data}", "#FF5555")
        else:
            self.log(f"📩 [{port} 反馈]: {data}", "#AAAAAA")

    # ---------- 对外接口 ----------
    def send_password(self, password):
        if self.ser is None or not self.ser.is_open:
            self.log("❌ 发送失败：设备未连接", "#FF5555")
            return
        msg = f"PASS:{password}\n".encode()
        try:
            with self.lock:
                self.ser.write(msg)
            self.log(f"📤 密码已通过 {self.active_port} 发送！", "#FFFF55")
        except (serial.SerialException, OSError):
            self.log("❌ 发送失败：串口异常", "#FF5555")

    def rediscover(self):
        self.rediscover_event.set()

    def start(self):
        threading.Thread(target=self.run, daemon=True).start()

    def stop(self):
        self.running = False
        if self.ser is not None:
            try:
                self.ser.close()
            except:
                pass
            self.ser = None


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.comm = Communicate()
        self.worker = SerialWorker(self.comm)

        self.init_ui()

        self.comm.log_signal.connect(self.append_log)
        self.comm.status_signal.connect(self.update_status)
        self.comm.active_signal.connect(self.update_active)

        self.worker.start()

    def init_ui(self):
        self.setWindowTitle("智能密码锁 - 上位机控制台")
        self.resize(650, 480)

        self.setStyleSheet("""
            QMainWindow { background-color: #2b2b2b; }
            QLabel { color: #ffffff; font-size: 14px; }
            QGroupBox { color: #ffffff; border: 1px solid #555; border-radius: 5px; margin-top: 10px; }
            QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 3px; }
            QTextEdit { background-color: #1e1e1e; color: #d4d4d4; border: 1px solid #333; font-family: Consolas; font-size: 13px; }
            QLineEdit { background-color: #3c3f41; color: #ffffff; border: 1px solid #555; padding: 5px; border-radius: 3px; font-size: 14px; }
            QPushButton { background-color: #0078D7; color: white; border: none; padding: 8px 15px; border-radius: 3px; font-weight: bold; }
            QPushButton:hover { background-color: #005A9E; }
            QPushButton:pressed { background-color: #004578; }
        """)

        central = QWidget()
        self.setCentralWidget(central)
        layout = QVBoxLayout(central)

        # 顶部状态栏
        status_layout = QHBoxLayout()
        self.lbl_com4 = QLabel("🔴 COM4")
        self.lbl_com7 = QLabel("🔴 COM7")
        self.lbl_active = QLabel("当前: 未识别")
        self.lbl_active.setStyleSheet("color: #FFA500;")
        self.btn_rescan = QPushButton("重新扫描")
        self.btn_rescan.clicked.connect(self.on_rescan)

        status_layout.addWidget(self.lbl_com4)
        status_layout.addWidget(self.lbl_com7)
        status_layout.addStretch()
        status_layout.addWidget(self.lbl_active)
        status_layout.addWidget(self.btn_rescan)
        layout.addLayout(status_layout)

        # 日志
        log_group = QGroupBox("运行日志")
        log_layout = QVBoxLayout()
        self.text_log = QTextEdit()
        self.text_log.setReadOnly(True)
        log_layout.addWidget(self.text_log)
        log_group.setLayout(log_layout)
        layout.addWidget(log_group)

        # 密码发送
        ctrl_group = QGroupBox("密码发送")
        ctrl_layout = QHBoxLayout()
        self.input_pwd = QLineEdit()
        self.input_pwd.setPlaceholderText("输入密码...")
        self.input_pwd.returnPressed.connect(self.on_send_click)
        self.btn_send = QPushButton("发送")
        self.btn_send.clicked.connect(self.on_send_click)
        ctrl_layout.addWidget(QLabel("密码:"))
        ctrl_layout.addWidget(self.input_pwd)
        ctrl_layout.addWidget(self.btn_send)
        ctrl_group.setLayout(ctrl_layout)
        layout.addWidget(ctrl_group)

        # 底部：当前连接端口（自动扫描结果）
        self.lbl_bottom = QLabel("当前连接端口: 扫描中...")
        self.lbl_bottom.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.lbl_bottom.setStyleSheet(
            "font-size: 15px; font-weight: bold; color: #FFA500; padding: 6px;")
        layout.addWidget(self.lbl_bottom)

    def append_log(self, msg, color):
        time_str = datetime.now().strftime("%H:%M:%S")
        html = f'<span style="color:#888;">[{time_str}]</span> <span style="color:{color};">{msg}</span>'
        self.text_log.append(html)
        self.text_log.verticalScrollBar().setValue(self.text_log.verticalScrollBar().maximum())

    def update_status(self, port, is_connected):
        color = "#55FF55" if is_connected else "#FF5555"
        icon = "🟢" if is_connected else "🔴"
        if port == "COM4":
            self.lbl_com4.setText(f'<span style="color:{color};">{icon} COM4</span>')
        elif port == "COM7":
            self.lbl_com7.setText(f'<span style="color:{color};">{icon} COM7</span>')

    def update_active(self, port):
        if port:
            self.lbl_active.setText(f"当前: {port} ✅")
            self.lbl_active.setStyleSheet("color: #55FF55;")
            self.lbl_bottom.setText(f"当前连接端口: {port}")
            self.lbl_bottom.setStyleSheet(
                "font-size: 15px; font-weight: bold; color: #55FF55; padding: 6px;")
        else:
            self.lbl_active.setText("当前: 无可用端口")
            self.lbl_active.setStyleSheet("color: #FF5555;")
            self.lbl_bottom.setText("当前连接端口: 无可用端口")
            self.lbl_bottom.setStyleSheet(
                "font-size: 15px; font-weight: bold; color: #FF5555; padding: 6px;")

    def on_send_click(self):
        pwd = self.input_pwd.text().strip()
        if pwd:
            self.worker.send_password(pwd)
            self.input_pwd.clear()
        else:
            self.append_log("⚠️ 密码不能为空", "#FF5555")

    def on_rescan(self):
        self.append_log("🔁 手动触发重新扫描", "#FFA500")
        self.lbl_bottom.setText("当前连接端口: 扫描中...")
        self.lbl_bottom.setStyleSheet(
            "font-size: 15px; font-weight: bold; color: #FFA500; padding: 6px;")
        self.worker.rediscover()

    def closeEvent(self, event):
        self.worker.stop()
        event.accept()


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())