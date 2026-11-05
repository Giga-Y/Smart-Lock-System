#ifndef __UART_H
#define __UART_H
#include "Hardware_Profile.h"

// 声明官方的底层初始化与发送函数
extern void Initial_RS232_UART(void); 
extern void Send_String_UART_A(unsigned char *txArray);
extern void Send_OneByte_UART_A(unsigned char byte);

// 声明切换通信通道的函数
// channel: 1 = 外部RS232(DB9), 2 = 板载XDS虚拟串口
extern void Switch_UART_Channel(u8 channel);

extern u8 UART_TimeSync_Parse(u8 byte);
extern u8 UART_Link_Parse(u8 byte);
extern volatile u8 g_comm_cmd_pending;
extern volatile u8 g_comm_cmd_port; /* 0 = NONE, 1 = COM4, 2 = COM7 */
extern volatile u8 g_ping_pending;
extern volatile u8 g_time_sync_ready;
extern u16 g_sync_time[6]; // 存放解析出的：年月日时分秒
#endif