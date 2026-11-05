#include "UART.h"
#include "USB_Comm.h"
#include "clock.h"  // 新增：用于调用 Clock_TimeSet
#include <string.h>

// 初始化串口（不再写死 P7，只配置引脚和寄存器）
void Initial_RS232_UART(void) {
    // 1. 配置 P8.2(TXD) 和 P8.3(RXD) 为复用引脚
    P8SEL &= ~BIT1; 
    P8SEL |= 0X0C;
    P8REN &= ~BIT3;   

    // 2. 串口寄存器配置 (SMCLK = 12MHz)
    UCA1CTL0 = 0;                     // 8位数据，无校验，1停止位
    UCA1CTL1 = UCSWRST | UCSSEL_2;    // 复位，时钟源 = SMCLK (12MHz)

    // 12,000,000 / 9600 = 1250 = 0x04E2
    UCA1BR0 = 0xE2;                   // 1250 的低8位 (226)
    UCA1BR1 = 0x04;                   // 1250 的高8位 (4)
    UCA1MCTL = 0;                     // 完全整除，无需调制
    
    UCA1CTL1 &= ~UCSWRST;             // 释放复位
    UCA1IE |= UCRXIE;                 // 开启接收中断
}

// 动态切换硬件通道
void Switch_UART_Channel(u8 channel) {
    // 配置 P7.4(IN2) 和 P7.5(IN1) 为输出
    P7DIR |= (BIT4 | BIT5); 
    
    if (channel == 1) {
        // 通道 1：外部 RS232
        P7OUT |= BIT4;          // IN2 = 1
        P7OUT &= ~BIT5;         // IN1 = 0 (选择 s3，即 RS232)
    } else {
        // 通道 2：板载 XDS 虚拟串口
        P7OUT &= ~BIT4;         // IN2 = 0
        P7OUT &= ~BIT5;         // IN1 = 0
    }
}

// 发送一个字节
void Send_OneByte_UART_A(unsigned char byte) {
    unsigned int Cnt = 0;
    while (!(UCA1IFG & UCTXIFG)) {
        Cnt++; 
        if(Cnt > 50000) break; 
    }
    UCA1TXBUF = byte;
}

// 发送字符串
// 保留原有 UART 发送路径，同时把同一份数据从 USB CDC 虚拟串口发出。
void Send_String_UART_A(unsigned char *txArray) {
    unsigned char *p = txArray;
    if (p == 0) return;

    while(*p != 0) {
        Send_OneByte_UART_A(*p);
        p++;
    }

    USB_Comm_SendString(txArray);
}


/* =====================================================================
 * 新增：时间同步协议（超轻量级，绝不卡顿）
 * 数据格式："T:YYYYMMDDHHMMSS\n"
 * ===================================================================== */
static u8 sync_state = 0;
static u8 sync_buf[20];
static u8 sync_idx = 0;

volatile u8 g_time_sync_ready = 0;
u16 g_sync_time[6] = {0};

/* =====================================================================
 * 上位机链路状态协议：PORT:COM4 / PORT:COM7 / PORT:NONE
 * 用于上位机自动扫描后通知下位机当前使用的通信端口。
 * ===================================================================== */
static u8 link_state = 0;
static u8 link_buf[8];
static u8 link_len = 0;

volatile u8 g_comm_cmd_pending = 0;
volatile u8 g_comm_cmd_port = 0;   /* 0 = NONE, 1 = COM4, 2 = COM7 */
volatile u8 g_ping_pending = 0;    /* 1 = 收到 PING，需要回复 PONG */

// 串口/USB 接收函数调用
// 返回值：1 = 该字节属于时间同步帧，不应再作为密码字符处理；0 = 普通数据
u8 UART_TimeSync_Parse(u8 byte) {
    // 任何时候收到 'T'，立刻重置状态机
    if (byte == 'T') { 
        sync_state = 1; 
        sync_idx = 0;
        return 1;
    }
    
    if (sync_state == 1) {
        if (byte == ':') { 
            sync_state = 2; 
            sync_idx = 0; 
            return 1;
        } else {
            sync_state = 0;
            return 0;
        }
    } else if (sync_state == 2) {
        if (byte >= '0' && byte <= '9') {
            sync_buf[sync_idx++] = byte;
            
            // 只要凑齐 14 个数字，立刻同步！不需要等换行符
            if (sync_idx == 14) { 
                g_sync_time[0] = (sync_buf[0]-'0')*1000 + (sync_buf[1]-'0')*100 + (sync_buf[2]-'0')*10 + (sync_buf[3]-'0');
                g_sync_time[1] = (sync_buf[4]-'0')*10 + (sync_buf[5]-'0');
                g_sync_time[2] = (sync_buf[6]-'0')*10 + (sync_buf[7]-'0');
                g_sync_time[3] = (sync_buf[8]-'0')*10 + (sync_buf[9]-'0');
                g_sync_time[4] = (sync_buf[10]-'0')*10 + (sync_buf[11]-'0');
                g_sync_time[5] = (sync_buf[12]-'0')*10 + (sync_buf[13]-'0');
                g_time_sync_ready = 1; 
                sync_state = 0; // 解析完成
            }
            return 1;
        } else {
            // 遇到非数字，直接作废
            sync_state = 0;
            return 0;
        }
    }

    return 0;
}

/* 返回值：1 = 该字节属于 PORT 链路命令，不应再进入时间/密码解析 */
u8 UART_Link_Parse(u8 byte) {
    switch (link_state) {
        case 0:
            if (byte == 'P') { link_state = 1; return 1; }
            return 0;
        case 1:
            if (byte == 'O') { link_state = 2; return 1; }  /* PORT */
            if (byte == 'I') { link_state = 6; return 1; }  /* PING */
            link_state = 0; return 0;
        case 2:
            if (byte == 'R') { link_state = 3; return 1; }
            link_state = 0; return 0;
        case 3:
            if (byte == 'T') { link_state = 4; return 1; }
            link_state = 0; return 0;
        case 4:
            if (byte == ':') { link_state = 5; link_len = 0; return 1; }
            link_state = 0; return 0;
        case 5:
            if (byte == '\n' || byte == '\r') {
                link_buf[link_len] = '\0';
                if (strcmp((char *)link_buf, "COM4") == 0) {
                    g_comm_cmd_port = 1;
                    g_comm_cmd_pending = 1;
                } else if (strcmp((char *)link_buf, "COM7") == 0) {
                    g_comm_cmd_port = 2;
                    g_comm_cmd_pending = 1;
                } else if (strcmp((char *)link_buf, "NONE") == 0) {
                    g_comm_cmd_port = 0;
                    g_comm_cmd_pending = 1;
                }
                link_state = 0;
                return 1;
            }
            if (((byte >= 'A') && (byte <= 'Z')) ||
                ((byte >= '0') && (byte <= '9'))) {
                if (link_len < (sizeof(link_buf) - 1)) {
                    link_buf[link_len++] = byte;
                }
                return 1;
            }
            link_state = 0;
            return 0;
        case 6:
            if (byte == 'N') { link_state = 7; return 1; }
            link_state = 0; return 0;
        case 7:
            if (byte == 'G') { link_state = 8; return 1; }
            link_state = 0; return 0;
        case 8:
            if (byte == '\n' || byte == '\r') {
                g_ping_pending = 1;
                link_state = 0;
                return 1;
            }
            link_state = 0;
            return 0;
        default:
            link_state = 0;
            return 0;
    }
}