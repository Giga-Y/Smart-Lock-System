#include "Hardware_Profile.h"
#include "LCD_TFT_FONT.h" 
#include "LCD_TFT_ILI9325.h" 

/****************************************************************************
*函数功能：向ILI9325写"控制命令寄存器地址"
*****************************************************************************/
void ILI9325_WriteRegIndex(u16 index) 
{
	mLow_LCD_TFT_RS;  
	Send_OneByte_SPI_B(index >> 8);  
	Send_OneByte_SPI_B(index);  	 
}

/****************************************************************************
*函数功能：向ILI9325写"16 位数据"
*****************************************************************************/
void ILI9325_WriteData(u16 data)  
{
	mHigh_LCD_TFT_RS; 
	Send_OneByte_SPI_B(data >> 8);	
	Send_OneByte_SPI_B(data);        
}

/****************************************************************************
*函数功能：配置ILI9325寄存器的值(寄存器地址+寄存器值)
*****************************************************************************/
void LCD_TFT_WriteReg(u8 index, u16 data)  
{
	ILI9325_WriteRegIndex(index);
	ILI9325_WriteData(data);
}

/****************************************************************************
*函数功能：向ILI9325写"GRAM命令地址"
*****************************************************************************/
void LCD_TFT_WriteRAM_CMD(void) 
{
	ILI9325_WriteRegIndex(0x22);   
}

/****************************************************************************
*函数功能：向ILI9325写"GRAM数据"
*****************************************************************************/
void LCD_TFT_WriteRAM(u16 RGB_data)  
{
    ILI9325_WriteData(RGB_data); 
}

/****************************************************************************
*函数功能：初始化"彩屏LCD_TFT_ILI9325" 所有相关资源
*****************************************************************************/
void Init_LCD_TFT_ILI9325(void) 
{
	mOpen_LCD_TFT_BackLight; 
	Init_GPIO_LCD_TFT_ILI9325();  
	Reset_LCD_TFT_ILI9325(); 
	Init_Register_LCD_TFT_ILI9325();  
}

/****************************************************************************
*函数功能：初始化"彩屏LCD_TFT_ILI9325" 的IO端口
*****************************************************************************/
void Init_GPIO_LCD_TFT_ILI9325(void)  
{
	mSPI_Select_LCD_TFT; 
	Initial_SPI_B();  
	mConfig_TS3A5017_B_IN2(0);  
	mConfig_TS3A5017_B_IN1(1);  

	mSetOut_LCD_TFT_CS; 
	mLow_LCD_TFT_CS; 
		
	mSetOut_LCD_TFT_Reset; 
	mClose_LCD_TFT_Reset;  

	mSelect_LCD_TFT_RS_IoPin;  
	mSetOut_LCD_TFT_RS;        
	mLow_LCD_TFT_RS;           
}

/****************************************************************************
*函数功能：复位"彩屏LCD_TFT_ILI9325"模块
*****************************************************************************/
void Reset_LCD_TFT_ILI9325(void) 
{
	mClose_LCD_TFT_Reset; 
	_delay_ms(1);
	mOpen_LCD_TFT_Reset; 
	_delay_ms(10);
	mClose_LCD_TFT_Reset; 
	_delay_ms(50);
}

/****************************************************************************
*函数功能：初始化"彩屏LCD_TFT_ILI9325" 控制芯片的寄存器 (原封不动保留)
*****************************************************************************/
void Init_Register_LCD_TFT_ILI9325(void)  
{
	LCD_TFT_WriteReg(0x00E3, 0x3008); // Set internal timing
	LCD_TFT_WriteReg(0x00E7, 0x0012); // Set internal timing
	LCD_TFT_WriteReg(0x00EF, 0x1231); // Set internal timing

#if defined USE_HORIZONTAL_LCD_TFT    
	LCD_TFT_WriteReg(0x0001, 0x0100); 
#elif defined USE_VERTICAL_LCD_TFT       
	LCD_TFT_WriteReg(0x0001, 0x0100); 
#endif

	LCD_TFT_WriteReg(0x0002, 0x0700); // set 1 line inversion

#if defined USE_HORIZONTAL_LCD_TFT    
	LCD_TFT_WriteReg(0x0003, 0x1038); 
#elif defined USE_VERTICAL_LCD_TFT       
	LCD_TFT_WriteReg(0x0003, 0x1030); 
#endif

	LCD_TFT_WriteReg(0x0004, 0x0000); // Resize register
	LCD_TFT_WriteReg(0x0008, 0x0202); // set the back porch and front porch
	LCD_TFT_WriteReg(0x0009, 0x0000); // set non-display area refresh cycle ISC[3:0]
	LCD_TFT_WriteReg(0x000A, 0x0000); // FMARK function
	LCD_TFT_WriteReg(0x000C, 0x0000); // RGB interface setting
	LCD_TFT_WriteReg(0x000D, 0x0000); // Frame marker Position
	LCD_TFT_WriteReg(0x000F, 0x0000); // RGB interface polarity

	LCD_TFT_WriteReg(0x0010, 0x0000); // SAP, BT[3:0], AP, DSTB, SLP, STB
	LCD_TFT_WriteReg(0x0011, 0x0007); // DC1[2:0], DC0[2:0], VC[2:0]
	LCD_TFT_WriteReg(0x0012, 0x0000); // VREG1OUT voltage
	LCD_TFT_WriteReg(0x0013, 0x0000); // VDV[4:0] for VCOM amplitude
	_delay_ms(200); // Dis-charge capacitor power voltage
	
	LCD_TFT_WriteReg(0x0010, 0x1690); // SAP, BT[3:0], AP, DSTB, SLP, STB
	LCD_TFT_WriteReg(0x0011, 0x0227); // R11h=0x0221 at VCI=3.3V, DC1[2:0], DC0[2:0], VC[2:0]
	_delay_ms(50); 
	
	LCD_TFT_WriteReg(0x0012, 0x001C); // External reference voltage= Vci;
	_delay_ms(50); 
	
	LCD_TFT_WriteReg(0x0013, 0x1800); // R13=1200 when R12=009D;VDV[4:0] for VCOM amplitude
	LCD_TFT_WriteReg(0x0029, 0x001C); // R29=000C when R12=009D;VCM[5:0] for VCOMH
	LCD_TFT_WriteReg(0x002B, 0x000D); // Frame Rate = 91Hz
	_delay_ms(50); 

	LCD_TFT_WriteReg(0x0020, 0x0000); // GRAM horizontal Address
	LCD_TFT_WriteReg(0x0021, 0x0000); // GRAM Vertical Address

	LCD_TFT_WriteReg(0x0030, 0x0007);
	LCD_TFT_WriteReg(0x0031, 0x0302);
	LCD_TFT_WriteReg(0x0032, 0x0105);
	LCD_TFT_WriteReg(0x0035, 0x0206);
	LCD_TFT_WriteReg(0x0036, 0x0808);
	LCD_TFT_WriteReg(0x0037, 0x0206);
	LCD_TFT_WriteReg(0x0038, 0x0504);
	LCD_TFT_WriteReg(0x0039, 0x0007);
	LCD_TFT_WriteReg(0x003C, 0x0105);
	LCD_TFT_WriteReg(0x003D, 0x0808);

	LCD_TFT_WriteReg(0x0050, 0x0000); //Horizontal GRAM Start Address
	LCD_TFT_WriteReg(0x0051, 0x00EF); //Horizontal GRAM End Address
	LCD_TFT_WriteReg(0x0052, 0x0000); //Vertical GRAM Start Address
	LCD_TFT_WriteReg(0x0053, 0x013F); //Vertical GRAM End Address

#if defined USE_HORIZONTAL_LCD_TFT    
	LCD_TFT_WriteReg(0x0060, 0x2700); 
#elif defined USE_VERTICAL_LCD_TFT       
	LCD_TFT_WriteReg(0x0060, 0xA700); 
#endif

	LCD_TFT_WriteReg(0x0061, 0x0001); // NDL,VLE, REV
	LCD_TFT_WriteReg(0x006A, 0x0000); // set scrolling line

	LCD_TFT_WriteReg(0x0080, 0x0000);
	LCD_TFT_WriteReg(0x0081, 0x0000);
	LCD_TFT_WriteReg(0x0082, 0x0000);
	LCD_TFT_WriteReg(0x0083, 0x0000);
	LCD_TFT_WriteReg(0x0084, 0x0000);
	LCD_TFT_WriteReg(0x0085, 0x0000);

	LCD_TFT_WriteReg(0x0090, 0x0010);
	LCD_TFT_WriteReg(0x0092, 0x0000);
	LCD_TFT_WriteReg(0x0093, 0x0003);
	LCD_TFT_WriteReg(0x0095, 0x0110);
	LCD_TFT_WriteReg(0x0097, 0x0000);
	LCD_TFT_WriteReg(0x0098, 0x0000);

	LCD_TFT_WriteReg(0x0007, 0x0133); // 262K color and display ON
}

/****************************************************************************
*函数功能：设置显示窗口的起始和终止大小
*****************************************************************************/
void LCD_TFT_SetWindow(u16 xSta, u16 ySta, u16 xEnd, u16 yEnd)  
{
	ILI9325_WriteRegIndex(0x50); //水平开始地址
	ILI9325_WriteData(xSta);

	ILI9325_WriteRegIndex(0x52); //垂直开始地址
	ILI9325_WriteData(ySta);

	ILI9325_WriteRegIndex(0x51);  //水平结束地址
	ILI9325_WriteData(xEnd);

	ILI9325_WriteRegIndex(0x53);  //垂直结束地址
	ILI9325_WriteData(yEnd);
}

/****************************************************************************
*函数功能：设置GRAM的光标位置
*****************************************************************************/
void LCD_TFT_SetCursor(u16 x, u16 y)  
{
	LCD_TFT_WriteReg(0x20, x);  // x轴 坐标
	LCD_TFT_WriteReg(0x21, y);  // y轴 坐标
}

/****************************************************************************
*函数功能：全屏清屏
*****************************************************************************/
void LCD_TFT_Clear(u16 color)   
{
	u32 index = 0;
	LCD_TFT_SetCursor(0x0000, 0x0000);  
	LCD_TFT_WriteRAM_CMD(); 
	for(index = 0; index < 76800; index++)
	{
		LCD_TFT_WriteRAM(color);   
	}
}

/****************************************************************************
*函数功能：画点
*****************************************************************************/
void LCD_TFT_DrawPoint(u16 x, u16 y, u16 color) 
{
	LCD_TFT_SetCursor(x, y);
	LCD_TFT_WriteRAM_CMD(); 
	LCD_TFT_WriteRAM(color);
}

/****************************************************************************
*函数功能：画线
*****************************************************************************/
void LCD_TFT_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2,u16 color)  
{
	u16 t;
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;

	delta_x=x2-x1; 
	delta_y=y2-y1;
	uRow=x1;
	uCol=y1;
	
	if(delta_x>0)incx=1; 
	else if(delta_x==0)incx=0;
	else {incx=-1;delta_x=-delta_x;}
	
	if(delta_y>0)incy=1;
	else if(delta_y==0)incy=0;
	else{incy=-1;delta_y=-delta_y;}
	
	if( delta_x>delta_y)distance=delta_x; 
	else distance=delta_y;
	
	for(t=0;t<=distance+1;t++ )
	{
		LCD_TFT_DrawPoint(uRow,uCol,color);
		xerr+=delta_x ;
		yerr+=delta_y ;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
}

/****************************************************************************
*函数功能：画矩形框
*****************************************************************************/
void LCD_TFT_DrawRectangle(u16 xSta, u16 ySta, u16 xEnd, u16 yEnd,u16 color)  
{
	LCD_TFT_DrawLine(xSta,ySta,xSta,yEnd,color); 
	LCD_TFT_DrawLine(xSta,yEnd,xEnd,yEnd,color); 
	LCD_TFT_DrawLine(xEnd,yEnd,xEnd,ySta,color); 
	LCD_TFT_DrawLine(xEnd,ySta,xSta,ySta,color); 
}

/****************************************************************************
*函数功能：填充一块LCD矩形区域
*****************************************************************************/
void LCD_TFT_FillRectangle(u16 xSta,u16 ySta,u16 xEnd,u16 yEnd,u16 color) 
{
	u16 i = 0;
	u16 j = 0;
	u16 Length = 0;

#if defined USE_HORIZONTAL_LCD_TFT    
	Length = yEnd - ySta + 1;
	for(i = xSta; i <= xEnd; i++)
	{
		LCD_TFT_SetCursor(i,ySta);	 
		LCD_TFT_WriteRAM_CMD(); 	 
		for(j = 0; j < Length; j++) LCD_TFT_WriteRAM(color);
	}
#elif defined USE_VERTICAL_LCD_TFT       
	Length = xEnd - xSta + 1;
	for(i = ySta; i <= yEnd; i++)
	{
		LCD_TFT_SetCursor(xSta,i);	 
		LCD_TFT_WriteRAM_CMD(); 	 
		for(j = 0; j < Length; j++) LCD_TFT_WriteRAM(color);
	}
#endif
}

/****************************************************************************
*函数功能：显示一个字符
*****************************************************************************/
void LCD_TFT_ShowChar(u16 x,u16 y,u8 AscNum,u8 Font,u16 pointColor,u16 backColor) 
{
    u8  AscChar = 0;
    u8  i = 0;
	u8  j = 0;
	u8  VerticalNum = 0;
	u8  horizontalNum = 0;

	AscNum = AscNum - 0x20;

#if defined USE_HORIZONTAL_LCD_TFT 	
	u16 yBuf = 0;
	yBuf = y;  
	switch(Font) 
	{
		case FONT1206: VerticalNum = 12; horizontalNum = 6; break;	
		case FONT1608: VerticalNum = 16; horizontalNum = 8; break;	
		default: break;
	}
	for(i = 0; i < VerticalNum; i++)
	{
		switch(Font) 
		{
			case FONT1206: AscChar = asc2_1206[AscNum][i]; break;	
			case FONT1608: AscChar = asc2_1608[AscNum][i]; break;	
			default: break;
		}
		for(j = 0; j < horizontalNum; j++)
		{
			if(AscChar & 0x01) LCD_TFT_DrawPoint(x, y, pointColor);  
			else  LCD_TFT_DrawPoint(x, y, backColor);  
			AscChar >>= 1;
			y ++;
		}
		y = yBuf;
		x ++;
	}
#elif defined USE_VERTICAL_LCD_TFT  
	u16 xBuf = 0;
	xBuf = x;  
	switch(Font) 
	{
		case FONT1206: VerticalNum = 12; horizontalNum = 6; break;	
		case FONT1608: VerticalNum = 16; horizontalNum = 8; break;	
		default: break;
	}
	for(i = 0; i < VerticalNum; i++)
	{
		switch(Font) 
		{
			case FONT1206: AscChar = asc2_1206[AscNum][i]; break;	
			case FONT1608: AscChar = asc2_1608[AscNum][i]; break;	
			default: break;
		}
		for(j = 0; j < horizontalNum; j++)
		{
			if(AscChar & 0x01) LCD_TFT_DrawPoint(x, y, pointColor);  
			else  LCD_TFT_DrawPoint(x, y, backColor);  
			AscChar >>= 1;
			x ++;
		}
		x = xBuf;
		y ++;
	}
#endif
}

/****************************************************************************
*函数功能：显示字符串
*****************************************************************************/
void LCD_TFT_ShowString(u16 line,u16 column,u8 *ArrayPoint,u8 Font,u16 pointColor,u16 backColor) 
{
	u16 x = 0;
	u16 y = 0;
	u8  VerticalNum = 0;
	u8  horizontalNum = 0;
	u16 max_x = 0;
	u16 max_y = 0;

#if defined USE_HORIZONTAL_LCD_TFT    
	x = line;
	y = column;
	switch(Font) 
	{
		case FONT1206: VerticalNum = 12; horizontalNum = 6; max_x = 234; max_y = 314; break;	
		case FONT1608: VerticalNum = 16; horizontalNum = 8; max_x = 232; max_y = 312; break;	
		default: break;
	}
    while(*ArrayPoint != '\0')
	{
        if(x > max_x) {x = y = 0; LCD_TFT_Clear(backColor);}
        if(y > max_y) {y = 0; x += VerticalNum;}
		LCD_TFT_ShowChar(x,y,*ArrayPoint,Font,pointColor,backColor);
		y += horizontalNum;    
        ArrayPoint ++;  
    }
#elif defined USE_VERTICAL_LCD_TFT       
	x = column;
	y = line;
	switch(Font) 
	{
		case FONT1206: VerticalNum = 12; horizontalNum = 6; break;	
		case FONT1608: VerticalNum = 16; horizontalNum = 8; break;	
		default: break;
	}
	while(*ArrayPoint != '\0')
	{		
        if(x > max_x) {x = 0; y += VerticalNum;}
        if(y > max_y) {x = y = 0; LCD_TFT_Clear(backColor);}
		LCD_TFT_ShowChar(x,y,*ArrayPoint,Font,pointColor,backColor);
		x += horizontalNum;    
        ArrayPoint ++;  
    }
#endif
}