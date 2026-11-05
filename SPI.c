
#include "Hardware_Profile.h"  //处理器"硬件配置文件" -Hardware specific definitions for Processor

#include "SPI.h"     //"SPI 同步串行外设接口" -驱动程序 -头文件(处理器内部资源)



/****************************************************************************
*函数名-Function:	void Initial_SPI_B(void)
*描述- Description:		SPI_B 模块初始化:端口初始化、配置SPI功能
*输入参数-Input:	None
*输出参数-output:	None
*注意事项-Note
*****************************************************************************/
void Initial_SPI_B(void)  //SPI_B 模块初始化:端口初始化、配置SPI功能
{
////////////////////////////////////////////////////////////////////////////
//==**SPI 端口初始化**===================================//
////////////////////////////////////////////////////////////////////////////
//--care--一定要加上这个语句//"LCD_Segment 端口控制寄存器"相关配置
	mSelect_LCD_SEG_IoPin;  //配置引脚为"通用数字IO"引脚

	LCDBCTL0 &= ~LCDON;   //关闭"段式LCD_Segment"
	LCDBCTL0 &= ~LCDSON;
	LCDBPCTL0 = 0;

	
//==**SPI_B 端口初始化(MSP430F6638)**========================//

//--**P8.4 引脚: SPI_B_CLK (P8.4/UCB1CLK/UCA1STE/S11)** -------------------//
	mSelect_SPI_B_CLK_FunctionPin; //配置引脚为"外围模块功能"引脚
	mSetOut_SPI_B_CLK; //配置"SPI_B_CLK" 引脚为"输出"

//--**P8.5 引脚: SPI_B_MOSI (P8.5/UCB1SIMO/UCB1SDA/S10)** ----------------//
	mSelect_SPI_B_MOSI_FunctionPin; //配置引脚为"外围模块功能"引脚
	mSetOut_SPI_B_MOSI; //配置"SPI_B_MOSI" 引脚为"输出"

//--**P8.6引脚: SPI_B_MISO (P8.6/UCB1SOMI/UCB1SCL/S9)** ----------------//
//	mSelect_SPI_B_MISO_FunctionPin; //配置引脚为"外围模块功能"引脚 
//	mSetIn_SPI_B_MISO; //配置"SPI_B_MISO" 引脚为"输入"  


////////////////////////////////////////////////////////////////////////////
//==**配置SPI_B 功能(MSP430F6638)**==========================//
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//--**先禁用SPI_B模块**----------------------------------------------//
	UCB1CTL1 = UCSWRST;  // Put state machine in reset
		//UCSWRST 使能软件复位。//1 使能，USCI逻辑保持在复位状态。//0 禁止。USCI复位释放。

////////////////////////////////////////////////////////////////////////////
//--**配置SPI_B模块相关参数**------------------------------------//	
	UCB1CTL0 = UCMST+UCSYNC+UCCKPL+UCMSB;    // 3-pin, 8-bit SPI master  // Clock polarity high, MSB
		//UCMST  选择主机模式。//1 主机模式//0 从机模式	                                      
		//UCSYNC  使能同步模式。//1 同步模式//0 异步模式
		//UCCKPL  时钟极性选择//1 不活动状态为高电平//0 不活动状态为低电平
		//UCMSB  选择MSB在前。控制接收或发送移位寄存器的方向。//1 MSB在前//0 LSB在前
	UCB1CTL1 = UCSSEL_2 + UCSWRST;   // 采用SMCLK，保持SW复位										
	UCB1BR0 = 0x01;                 // SMCLK /1 = 4M/1=4M
	//UCB1BR0 = 0x10;                 // SMCLK /16 = 4M/16=250K
	UCB1BR1 = 0; //位时钟预分频器。 (UCxxBR0 + UCxxBR1 × 256)的16位值组成UCBRX分频器的值UCBRx。        
		

	//UCB1MCTL = 0;          //USCI-Ax 模块控制寄存器  // No modulation
	
////////////////////////////////////////////////////////////////////////////
//--**使能SPI_B模块**-------------------------------------------------//
	UCB1CTL1 &= ~UCSWRST;                     //Initialize USCI state machine
		//UCSWRST 使能软件复位。//1 使能，USCI逻辑保持在复位状态。//0 禁止。USCI复位释放。


////////////////////////////////////////////////////////////////////////////
//--**中断配置: SPI_B模块**-----------------------------------------//	   
//UCB1IE = UCRXIE ;                         // Enable USCI_A0 RX,TX interrupt
//UCB1IE = UCTXIE ;    
	//UCB1IE 中断使能寄存器
	//UCTXIE  使能发送中断//UCRXIE 使能接收中断

		//mDelay_Cycles(M_SPI_Delay); //spi的传输的等待延时

}



/****************************************************************************
*函数名-Function:	void Send_OneByte_SPI_B(unsigned char byte) 
*描述- Description:		SPI_B 发送一个字节数据-- "SPI 同步串行外设接口" 
*输入参数-Input:	byte:  要发送的字节数据
*输出参数-output:	None
*注意事项-Note： 
*****************************************************************************/
void Send_OneByte_SPI_B(unsigned char byte)  //SPI_B 发送一个字节数据-- "SPI 同步串行外设接口" 
{
	while (!(UCB1IFG&UCTXIFG));           // USCI_B1 TX buffer ready?

	UCB1TXBUF = byte;                 // 发送命令的标志码
	
	mNop;mNop;mNop;mNop;
}



