#ifndef __LCD_TFT_ILI9325_H   
#define __LCD_TFT_ILI9325_H 

#include "stdint.h" 

typedef uint32_t  u32;	 //unsigned long
typedef uint16_t  u16;  //unsigned int
typedef uint8_t   u8;   //unsigned char 

// ================= 颜色定义 =================
#define BLACK	     0x0000  // 黑色
#define WHITE	     0xFFFF  // 白色
#define RED  	     0xF800  // 红色
#define BRED         0XF81F
#define BRRED 	     0XFC07  // 棕色
#define GRED 	     0XFFE0
#define MAROON       0x7800  // 深红色
#define MAGENTA      0xF81F  // 品红
#define BROWN 	     0XBC40  // 棕色
#define GREEN        0x07E0  // 绿色
#define LGREEN       0x07F8  // 浅绿色
#define OLIVE        0x7BE0  // 橄榄色
#define DGREEN       0x03E0  // 深绿色
#define DCYAN        0x03EF  // 深青色
#define BLUE 	     0x001F  // 蓝色
#define GBLUE	     0X07FF  // 浅蓝色
#define LBLUE	     0X07FF  // 浅蓝色
#define NAVY         0x000F  // 海军蓝
#define CYAN         0x07FF  // 青色
#define YELLOW       0xFFE0  // 黄色
#define GRAY0        0xEF7D  // 灰色0
#define GRAY1        0x8410  // 灰色1
#define GRAY2        0x4208  // 灰色2
#define GRAY  		 0X8430  // 灰色
#define DGRAY        0x7BEF  // 深灰色
#define PURPLE       0x780F  // 紫色
#define DARKBLUE     0X01CF	 // 深蓝色
#define LIGHTBLUE    0X7D7C  // 亮蓝色
#define GRAYBLUE     0X5458  // 灰蓝色
#define LIGHTGREEN   0X841F  // 亮绿色
#define LGRAY 	     0XC618  // 浅灰色
#define LGRAYBLUE    0XA651  // 浅灰蓝色
#define LBBLUE       0X2B12  // 浅蓝色

// ================= 显示模式选择 =================
#define USE_HORIZONTAL_LCD_TFT   // 使用"水平(横屏)显示模式"
//#define USE_VERTICAL_LCD_TFT   // 使用"垂直(竖屏)显示模式"

// ================= 字体大小定义 =================
#define FONT1206   (1) // 字体12*6
#define FONT1608   (2) // 字体16*8

// ================= 全局变量声明 =================

// ================= 函数声明 =================

// 底层读写
void ILI9325_WriteRegIndex(u16 index); 
void ILI9325_WriteData(u16 data);  
void LCD_TFT_WriteReg(u8 index, u16 data);  
void LCD_TFT_WriteRAM_CMD(void); 
void LCD_TFT_WriteRAM(u16 RGB_data);  

// 初始化与复位
extern void Init_LCD_TFT_ILI9325(void); 
extern void Init_GPIO_LCD_TFT_ILI9325(void);  
extern void Reset_LCD_TFT_ILI9325(void); 
extern void Init_Register_LCD_TFT_ILI9325(void);  

// 窗口与光标
void LCD_TFT_SetWindow(u16 xSta, u16 ySta, u16 xEnd, u16 yEnd);  
void LCD_TFT_SetCursor(u16 x, u16 y);  

// 基础图形绘制
extern void LCD_TFT_Clear(u16 color);   
extern void LCD_TFT_DrawPoint(u16 x, u16 y, u16 color); 
extern void LCD_TFT_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2,u16 color);  
extern void LCD_TFT_DrawRectangle(u16 xSta, u16 ySta, u16 xEnd, u16 yEnd,u16 color);  
extern void LCD_TFT_FillRectangle(u16 xSta,u16 ySta,u16 xEnd,u16 yEnd,u16 color);  

// 字符与字符串显示
extern void LCD_TFT_ShowChar(u16 x,u16 y,u8 AscNum,u8 Font,u16 pointColor,u16 backColor); 
extern void LCD_TFT_ShowString(u16 line,u16 column,u8 *ArrayPoint,u8 Font,u16 pointColor,u16 backColor); 

#endif