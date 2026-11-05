#ifndef __TM1638_H
#define __TM1638_H

#include <stdint.h>
#include "Hardware_Profile.h" // 这里面已经包含了 DIO, STB, CLK 的引脚宏定义，千万不要在这里重复写

// ================== TM1638 指令宏 ==================
#define CMD_FixedAddr_TM1638    0x44  // 固定地址模式 (基本不用)
#define CMD_IncreaseAddr_TM1638 0x40  // 自增地址模式 (常用)
#define CMD_DispAddr_TM1638     0xC0  // 显示寄存器首地址

// ================== 显示模式与亮度控制 ==================
#define Disp_Mode_Off     0x80  // 显示关闭
#define Disp_Mode_1_16    0x88  // 显示开 + 1/16 脉宽 (最暗)
#define Disp_Mode_2_16    0x89  // 显示开 + 2/16 脉宽
#define Disp_Mode_4_16    0x8A  // 显示开 + 4/16 脉宽
#define Disp_Mode_10_16   0x8B  // 显示开 + 10/16 脉宽
#define Disp_Mode_11_16   0x8C  // 显示开 + 11/16 脉宽
#define Disp_Mode_12_16   0x8D  // 显示开 + 12/16 脉宽
#define Disp_Mode_13_16   0x8E  // 显示开 + 13/16 脉宽
#define Disp_Mode_14_16   0x8F  // 显示开 + 14/16 脉宽 (最亮)

// 全灭/全亮字符索引
#define DISP_OFF_LedSeg   21    // 对应 LedSeg_Tab 的 0x00
#define DISP_ON_LedSeg    20    // 对应 LedSeg_Tab 的 0xFF

// ================== 函数声明 ==================
// 这些是精简版 TM1638.c 中仅保留的对外接口
void TM1638_Disp_Mode(unsigned char Disp_Mode);
void Open_Display_TM1638(void);

void Write_DispData_TM1638(unsigned char pointNum1, unsigned char pointNum2, 
                           unsigned char NO_8, unsigned char NO_7, unsigned char NO_6, unsigned char NO_5, 
                           unsigned char NO_4, unsigned char NO_3, unsigned char NO_2, unsigned char NO_1);
unsigned char Read_MatrixKey_TM1638(void); // <-- 加上这一行
#endif