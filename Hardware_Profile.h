#ifndef __HARDWARE_PROFILE_H
#define __HARDWARE_PROFILE_H

#include <stdint.h>
#include <msp430f6638.h>
#include "HAL_PMM.h"


typedef uint32_t  u32;
typedef uint16_t  u16;
typedef uint8_t   u8;


////////////////////////////////////////////////////////////////////////////
//==**芯片基础配置**================================//

#define mClose_WDT      (WDTCTL = WDTPW + WDTHOLD)
#define mClr_WDTCNT     (WDTCTL = WDTPW + WDTCNTCL)
#define mNop            (__no_operation())

#define mEnable_GIE     (_EINT())
#define mDisable_GIE    (_DINT())

#define mReset_Soft     (PMMCTL0 |= PMMSWPOR)

#define Fosc_System     (12)


////////////////////////////////////////////////////////////////////////////
//==**系统延时宏 MCLK=12MHz**==============//

#define mDelay_Cycles(n) (__delay_cycles(n))

#define M_Delay_1us     (12)
#define M_Delay_2us     (24)
#define M_Delay_5us     (60)
#define M_Delay_10us    (120)
#define M_Delay_100us   (1200)
#define M_Delay_1ms     (12000)
#define M_Delay_1s      (12000000)

#define CPU_F ((double)12000000)
#define _delay_us(x) __delay_cycles((long)(CPU_F*(double)x/1000000.0))
#define _delay_ms(x) __delay_cycles((long)(CPU_F*(double)x/1000.0))


////////////////////////////////////////////////////////////////////////////
//==**SPI_B IO配置**=============================//

#define SPI_B_CLK_SEL   (P8SEL)
#define SPI_B_CLK_DIR   (P8DIR)
#define SPI_B_CLK_PORT  (P8OUT)
#define SPI_B_CLK_IO    (BIT4)

#define mSelect_SPI_B_CLK_FunctionPin  (SPI_B_CLK_SEL |= SPI_B_CLK_IO)
#define mSetOut_SPI_B_CLK (SPI_B_CLK_DIR |= SPI_B_CLK_IO)
#define mHigh_SPI_B_CLK  {SPI_B_CLK_DIR |= SPI_B_CLK_IO; SPI_B_CLK_PORT |= SPI_B_CLK_IO;}
#define mLow_SPI_B_CLK   {SPI_B_CLK_DIR |= SPI_B_CLK_IO; SPI_B_CLK_PORT &= ~SPI_B_CLK_IO;}


#define SPI_B_MOSI_SEL  (P8SEL)
#define SPI_B_MOSI_DIR  (P8DIR)
#define SPI_B_MOSI_IO   (BIT5)
#define mSelect_SPI_B_MOSI_FunctionPin  (SPI_B_MOSI_SEL |= SPI_B_MOSI_IO)
#define mSetOut_SPI_B_MOSI (SPI_B_MOSI_DIR |= SPI_B_MOSI_IO)


#define SPI_B_MISO_SEL  (P8SEL)
#define SPI_B_MISO_DIR  (P8DIR)
#define SPI_B_MISO_IO   (BIT6)
#define mSelect_SPI_B_MISO_FunctionPin  (SPI_B_MISO_SEL |= SPI_B_MISO_IO)
#define mSetIn_SPI_B_MISO (SPI_B_MISO_DIR &= (~SPI_B_MISO_IO))


////////////////////////////////////////////////////////////////////////////
//==**模拟开关TS3A5017 IO**================//

#define TS3A5017_B_IN1_DIR   (P1DIR)
#define TS3A5017_B_IN1_PORT  (P1OUT)
#define TS3A5017_B_IN1_IO    (BIT7)
#define mSetOut_TS3A5017_B_IN1 (TS3A5017_B_IN1_DIR |= TS3A5017_B_IN1_IO)
#define mHigh_TS3A5017_B_IN1   (TS3A5017_B_IN1_PORT |= TS3A5017_B_IN1_IO)
#define mLow_TS3A5017_B_IN1    (TS3A5017_B_IN1_PORT &= (~TS3A5017_B_IN1_IO))
#define mConfig_TS3A5017_B_IN1(IN1) {mSetOut_TS3A5017_B_IN1; IN1 ? mHigh_TS3A5017_B_IN1 : mLow_TS3A5017_B_IN1;}


#define TS3A5017_B_IN2_DIR   (P1DIR)
#define TS3A5017_B_IN2_PORT  (P1OUT)
#define TS3A5017_B_IN2_IO    (BIT6)
#define mSetOut_TS3A5017_B_IN2 (TS3A5017_B_IN2_DIR |= TS3A5017_B_IN2_IO)
#define mHigh_TS3A5017_B_IN2   (TS3A5017_B_IN2_PORT |= TS3A5017_B_IN2_IO)
#define mLow_TS3A5017_B_IN2    (TS3A5017_B_IN2_PORT &= (~TS3A5017_B_IN2_IO))
#define mConfig_TS3A5017_B_IN2(IN2) {mSetOut_TS3A5017_B_IN2; IN2 ? mHigh_TS3A5017_B_IN2 : mLow_TS3A5017_B_IN2;}


////////////////////////////////////////////////////////////////////////////
//==**段式LCD IO**========//

#define LCD_SEG_BackLight_DIR   (P8DIR)
#define LCD_SEG_BackLight_PORT  (P8OUT)
#define LCD_SEG_BackLight_IO    (BIT0)
#define mOpen_LCD_SEG_BackLight {LCD_SEG_BackLight_DIR |= LCD_SEG_BackLight_IO; LCD_SEG_BackLight_PORT |= LCD_SEG_BackLight_IO;}
#define mClose_LCD_SEG_BackLight {LCD_SEG_BackLight_DIR |= LCD_SEG_BackLight_IO; LCD_SEG_BackLight_PORT &= (~LCD_SEG_BackLight_IO);}


#define LCD_SEG_COM_SEL  (P5SEL)
#define LCD_SEG_COM_IO   (BIT3 + BIT4 + BIT5)
#define mSelect_LCD_SEG_FunctionPin (LCD_SEG_COM_SEL |= LCD_SEG_COM_IO)
#define mSelect_LCD_SEG_IoPin (LCD_SEG_COM_SEL &= ~LCD_SEG_COM_IO)


////////////////////////////////////////////////////////////////////////////
//==**TFT LCD ILI9325 IO**===============//

#define LCD_TFT_BackLight_DIR   (P5DIR)
#define LCD_TFT_BackLight_PORT  (P5OUT)
#define LCD_TFT_BackLight_IO    (BIT2)
#define mOpen_LCD_TFT_BackLight {LCD_TFT_BackLight_DIR |= LCD_TFT_BackLight_IO; LCD_TFT_BackLight_PORT |= LCD_TFT_BackLight_IO;}
#define mClose_LCD_TFT_BackLight {LCD_TFT_BackLight_DIR |= LCD_TFT_BackLight_IO; LCD_TFT_BackLight_PORT &= (~LCD_TFT_BackLight_IO);}


#define LCD_TFT_CS_DIR   (P2DIR)
#define LCD_TFT_CS_PORT  (P2OUT)
#define LCD_TFT_CS_IO    (BIT6)
#define mSetOut_LCD_TFT_CS (LCD_TFT_CS_DIR |= LCD_TFT_CS_IO)
#define mHigh_LCD_TFT_CS (LCD_TFT_CS_PORT |= LCD_TFT_CS_IO)
#define mLow_LCD_TFT_CS  (LCD_TFT_CS_PORT &= ~LCD_TFT_CS_IO)
#define mOpen_LCD_TFT_CS  {LCD_TFT_CS_DIR |= LCD_TFT_CS_IO; LCD_TFT_CS_PORT &= (~LCD_TFT_CS_IO);}
#define mClose_LCD_TFT_CS {LCD_TFT_CS_DIR |= LCD_TFT_CS_IO; LCD_TFT_CS_PORT |= LCD_TFT_CS_IO;}
#define mTrigger_LCD_TFT_CS {LCD_TFT_CS_DIR |= LCD_TFT_CS_IO; LCD_TFT_CS_PORT |= LCD_TFT_CS_IO; LCD_TFT_CS_PORT &= ~LCD_TFT_CS_IO;}


#define LCD_TFT_Reset_DIR   (P2DIR)
#define LCD_TFT_Reset_PORT  (P2OUT)
#define LCD_TFT_Reset_IO    (BIT7)
#define mSetOut_LCD_TFT_Reset (LCD_TFT_Reset_DIR |= LCD_TFT_Reset_IO)
#define mOpen_LCD_TFT_Reset  (LCD_TFT_Reset_PORT &= ~LCD_TFT_Reset_IO)
#define mClose_LCD_TFT_Reset (LCD_TFT_Reset_PORT |= LCD_TFT_Reset_IO)


#define LCD_TFT_CLK_SEL  (P8SEL)
#define LCD_TFT_CLK_DIR  (P8DIR)
#define LCD_TFT_CLK_PORT (P8OUT)
#define LCD_TFT_CLK_IO  (BIT4)
#define mSelect_LCD_TFT_CLK_IoPin  (LCD_TFT_CLK_SEL &= ~LCD_TFT_CLK_IO)
#define mSetOut_LCD_TFT_CLK (LCD_TFT_CLK_DIR |= LCD_TFT_CLK_IO)
#define mHigh_LCD_TFT_CLK (LCD_TFT_CLK_PORT |= LCD_TFT_CLK_IO)
#define mLow_LCD_TFT_CLK  (LCD_TFT_CLK_PORT &= ~LCD_TFT_CLK_IO)


#define LCD_TFT_SDA_SEL  (P8SEL)
#define LCD_TFT_SDA_DIR  (P8DIR)
#define LCD_TFT_SDA_PORT (P8OUT)
#define LCD_TFT_SDA_IO  (BIT5)
#define mSelect_LCD_TFT_SDA_IoPin  (LCD_TFT_SDA_SEL &= ~LCD_TFT_SDA_IO)
#define mSetOut_LCD_TFT_SDA (LCD_TFT_SDA_DIR |= LCD_TFT_SDA_IO)
#define mHigh_LCD_TFT_SDA (LCD_TFT_SDA_PORT |= LCD_TFT_SDA_IO)
#define mLow_LCD_TFT_SDA  (LCD_TFT_SDA_PORT &= ~LCD_TFT_SDA_IO)


#define LCD_TFT_RS_SEL  (P8SEL)
#define LCD_TFT_RS_DIR  (P8DIR)
#define LCD_TFT_RS_PORT (P8OUT)
#define LCD_TFT_RS_IO  (BIT6)
#define mSelect_LCD_TFT_RS_IoPin  (LCD_TFT_RS_SEL &= ~LCD_TFT_RS_IO)
#define mSetOut_LCD_TFT_RS (LCD_TFT_RS_DIR |= LCD_TFT_RS_IO)
#define mHigh_LCD_TFT_RS (LCD_TFT_RS_PORT |= LCD_TFT_RS_IO)
#define mLow_LCD_TFT_RS  (LCD_TFT_RS_PORT &= ~LCD_TFT_RS_IO)


////////////////////////////////////////////////////////////////////////////
//==**CC1101模块 IO**=========================//

#define CC1101_CS_DIR   (PJDIR)
#define CC1101_CS_PORT  (PJOUT)
#define CC1101_CS_IO    (BIT1)
#define mOpen_CC1101_CS  {CC1101_CS_DIR |= CC1101_CS_IO; CC1101_CS_PORT &= (~CC1101_CS_IO);}
#define mClose_CC1101_CS {CC1101_CS_DIR |= CC1101_CS_IO; CC1101_CS_PORT |= CC1101_CS_IO;}


////////////////////////////////////////////////////////////////////////////
//==**SD卡模块 IO**=========================//

#define SDcard_CS_DIR   (PJDIR)
#define SDcard_CS_PORT  (PJOUT)
#define SDcard_CS_IO    (BIT0)
#define mOpen_SDcard_CS  {SDcard_CS_DIR |= SDcard_CS_IO; SDcard_CS_PORT &= (~SDcard_CS_IO);}
#define mClose_SDcard_CS {SDcard_CS_DIR |= SDcard_CS_IO; SDcard_CS_PORT |= SDcard_CS_IO;}


////////////////////////////////////////////////////////////////////////////
//==**SPI片选切换宏**=========================//

#define mSPI_Select_LCD_TFT {mOpen_LCD_TFT_CS; mClose_CC1101_CS; mClose_SDcard_CS;}
#define mSPI_Select_CC1101  {mOpen_CC1101_CS; mClose_LCD_TFT_CS; mClose_SDcard_CS;}
#define mSPI_Select_SDcard  {mOpen_SDcard_CS; mClose_LCD_TFT_CS; mClose_CC1101_CS;}


////////////////////////////////////////////////////////////////////////////
//====【新增：LED、按键、UART、密码参数，全部放在#endif之前！】====

/************************ LED引脚 P4.4/4.5/4.6 ************************/
#define LED_PORT        P4OUT
#define LED_RED_PIN     BIT4
#define LED_GREEN_PIN   BIT5
#define LED_YELLOW_PIN  BIT6

#define LED_RED_ON()    LED_PORT |= LED_RED_PIN
#define LED_RED_OFF()   LED_PORT &= ~LED_RED_PIN
#define LED_GREEN_ON()  LED_PORT |= LED_GREEN_PIN
#define LED_GREEN_OFF() LED_PORT &= ~LED_GREEN_PIN
#define LED_YELLOW_ON() LED_PORT |= LED_YELLOW_PIN
#define LED_YELLOW_OFF()LED_PORT &= ~LED_YELLOW_PIN

/************************ 按键引脚 P4.2 P4.3 输入用P4IN ************************/
#define KEY_PORT        P4IN
#define KEY_CONFIRM_PIN BIT2
#define KEY_FUNC_PIN    BIT3



/************************ 密码锁参数 ************************/
#define PWD_DEFAULT_LEN   6
#define PWD_MIN_LEN       4
#define PWD_MAX_LEN       6
#define PWD_DEFAULT       "000000"
#define MAX_ERROR_TIMES   3
#define LOCK_TIME_SEC     30


	////////////////////////////////////////////////////////////////////////////
//==**"TM1638 数码管驱动器"*IO口使用宏定义**============//

//--**P3.5 引脚: DIO (数据输入/输出) ** ----------------------------//
#define TM1638_DIO_DIR  (P3DIR)
#define TM1638_DIO_PORT (P3OUT)
#define TM1638_DIO_IN   (P3IN)
#define TM1638_DIO_IO   (BIT5)

#define mSetOut_TM1638_DIO      {TM1638_DIO_DIR |= TM1638_DIO_IO;}
#define mHigh_TM1638_DIO        {TM1638_DIO_DIR |= TM1638_DIO_IO; TM1638_DIO_PORT |= TM1638_DIO_IO;}
#define mLow_TM1638_DIO         {TM1638_DIO_DIR |= TM1638_DIO_IO; TM1638_DIO_PORT &= (~TM1638_DIO_IO);}
#define mSetIn_TM1638_DIO       {TM1638_DIO_DIR &= ~TM1638_DIO_IO;}
#define mRead_TM1638_DIO        ((TM1638_DIO_IN & TM1638_DIO_IO)?1U:0U)

//--**P3.2 引脚: STB (片选)  ** -----------------------------------------//
#define TM1638_STB_DIR  (P3DIR)
#define TM1638_STB_PORT (P3OUT)
#define TM1638_STB_IO   (BIT2)

#define mHigh_TM1638_STB        {TM1638_STB_DIR |= TM1638_STB_IO; TM1638_STB_PORT |= TM1638_STB_IO;}
#define mLow_TM1638_STB         {TM1638_STB_DIR |= TM1638_STB_IO; TM1638_STB_PORT &= ~TM1638_STB_IO;}

//--**P3.4 引脚: CLK (时钟输入) ** -----------------------------------//
#define TM1638_CLK_DIR  (P3DIR)
#define TM1638_CLK_PORT (P3OUT)
#define TM1638_CLK_IO   (BIT4)

#define mHigh_TM1638_CLK        {TM1638_CLK_DIR |= TM1638_CLK_IO; TM1638_CLK_PORT |= TM1638_CLK_IO;}
#define mLow_TM1638_CLK         {TM1638_CLK_DIR |= TM1638_CLK_IO; TM1638_CLK_PORT &= ~TM1638_CLK_IO;}

/************************ 蜂鸣器Buzz  P1.1 ************************/
/************************ 蜂鸣器Buzz  P1.1 ************************/
#define BUZZER_DIR      P1DIR
#define BUZZER_OUT      P1OUT
#define BUZZER_PIN      BIT1

#define mOpen_BUZZER  {BUZZER_DIR |= BUZZER_PIN; BUZZER_OUT |= BUZZER_PIN;}
#define mClose_BUZZER {BUZZER_DIR |= BUZZER_PIN; BUZZER_OUT &= ~BUZZER_PIN;}
#define mTurn_BUZZER  {BUZZER_DIR |= BUZZER_PIN; BUZZER_OUT ^= BUZZER_PIN;}




#endif

////////////////////////////////////////////////////////////////////////////
//==**外部API注释，写在#endif外面，仅注释，无代码**===============//
////////////////////////////////////////////////////////////////////////////
