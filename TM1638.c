#include "Generic.h"
#include "Hardware_Profile.h"
#include "TM1638.h"

// 数码管段码表 (0~9, A~F, ., +, -, =, AllOn, AllOff, I, r, U, P, L, n, S, O)
unsigned char LedSeg_Tab[] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, // 0-9
    0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71,                         // A-F
    0x80, 0x60, 0x40, 0x48, 0xff, 0x00, 0x30, 0x50, 0x3E,       // ., +, -, =, AllOn, AllOff, I, r, U
    0x73, 0x38, 0x54, 0x6D, 0x3F                                // P, L, n, S, O
};

/* ================== 底层驱动函数 ================== */

static void TM1638_Write(unsigned char Data)
{
    unsigned char i = 0;
    for(i = 0; i < 8; i++)
    {
        mLow_TM1638_CLK;
        // 【修复】这里加上大括号，完美解决宏展开导致的语法错误
        if(Data & 0x01) {
            mHigh_TM1638_DIO
        } else {
            mLow_TM1638_DIO
        }
        mHigh_TM1638_CLK;
        Data >>= 1;
    }
}

// 恢复读取函数，防止 MatrixKey.c 报错
static unsigned char TM1638_Read(void)
{
    unsigned char i = 0;
    unsigned char Data = 0;
    mSetIn_TM1638_DIO;
    for(i = 0; i < 8; i++)
    {
        Data >>= 1; 
        mLow_TM1638_CLK;
        mDelay_Cycles(M_Delay_2us);
        if(mRead_TM1638_DIO) Data |= 0x80;
        else Data &= 0x7f;
        mHigh_TM1638_CLK;
    }
    return (Data);
}

static void TM1638_Set_CMD(unsigned char Cmd)
{
    mLow_TM1638_STB;
    TM1638_Write(Cmd);
    mHigh_TM1638_STB;
}

/* ================== 业务功能函数 ================== */

void TM1638_Disp_Mode(unsigned char Disp_Mode)
{
    TM1638_Set_CMD(Disp_Mode);
}


void Open_Display_TM1638(void)
{
    Write_DispData_TM1638(0, 0, DISP_OFF_LedSeg, DISP_OFF_LedSeg, DISP_OFF_LedSeg, DISP_OFF_LedSeg, DISP_OFF_LedSeg, DISP_OFF_LedSeg, DISP_OFF_LedSeg, DISP_OFF_LedSeg);
}

// 全屏刷新显示 (保留小数点逻辑)
void Write_DispData_TM1638(unsigned char pointNum1, unsigned char pointNum2, unsigned char NO_8, unsigned char NO_7, unsigned char NO_6, unsigned char NO_5, unsigned char NO_4, unsigned char NO_3, unsigned char NO_2, unsigned char NO_1)
{
    TM1638_Set_CMD(CMD_IncreaseAddr_TM1638); 
    mLow_TM1638_STB;  
    TM1638_Write(CMD_DispAddr_TM1638);  

    if((pointNum1 == 4) || (pointNum2 == 4)) TM1638_Write(LedSeg_Tab[NO_4] | 0x80);  
    else TM1638_Write(LedSeg_Tab[NO_4]);  
    TM1638_Write(0x00); 

    if((pointNum1 == 3) || (pointNum2 == 3)) TM1638_Write(LedSeg_Tab[NO_3] | 0x80);  
    else TM1638_Write(LedSeg_Tab[NO_3]);  
    TM1638_Write(0x00); 

    if((pointNum1 == 2) || (pointNum2 == 2)) TM1638_Write(LedSeg_Tab[NO_2] | 0x80);  
    else TM1638_Write(LedSeg_Tab[NO_2]);  
    TM1638_Write(0x00); 

    if((pointNum1 == 1) || (pointNum2 == 1)) TM1638_Write(LedSeg_Tab[NO_1] | 0x80);  
    else TM1638_Write(LedSeg_Tab[NO_1]);  
    TM1638_Write(0x00); 

    if((pointNum1 == 8) || (pointNum2 == 8)) TM1638_Write(LedSeg_Tab[NO_8] | 0x80);  
    else TM1638_Write(LedSeg_Tab[NO_8]);  
    TM1638_Write(0x00); 

    if((pointNum1 == 7) || (pointNum2 == 7)) TM1638_Write(LedSeg_Tab[NO_7] | 0x80);  
    else TM1638_Write(LedSeg_Tab[NO_7]);  
    TM1638_Write(0x00); 

    if((pointNum1 == 6) || (pointNum2 == 6)) TM1638_Write(LedSeg_Tab[NO_6] | 0x80);  
    else TM1638_Write(LedSeg_Tab[NO_6]);  
    TM1638_Write(0x00); 

    if((pointNum1 == 5) || (pointNum2 == 5)) TM1638_Write(LedSeg_Tab[NO_5] | 0x80);  
    else TM1638_Write(LedSeg_Tab[NO_5]);  
    TM1638_Write(0x00); 

    mHigh_TM1638_STB;
}

// 【恢复】供 MatrixKey.c 调用的按键扫描函数（避免报未定义错误）
unsigned char Read_MatrixKey_TM1638(void)
{
	unsigned char KeyByte[4];
	unsigned char i = 0;

	mLow_TM1638_STB;
	TM1638_Write(0x42);
	for(i = 0; i < 4; i++) {
		KeyByte[i] = TM1638_Read();
	}
	mHigh_TM1638_STB;

	if(KeyByte[0] & BIT_2) return 1;
	else if(KeyByte[0] & BIT_6) return 4;
	else if(KeyByte[0] & BIT_0) return 2;
	else if(KeyByte[0] & BIT_1) return 3;
	else if(KeyByte[0] & BIT_5) return 6;
	else if(KeyByte[0] & BIT_4) return 5;
	else if(KeyByte[1] & BIT_0) return 0xb;
	else if(KeyByte[1] & BIT_4) return 0xc;
	else if(KeyByte[2] & BIT_0) return 0xd;
	else if(KeyByte[2] & BIT_4) return 8;
	else if(KeyByte[1] & BIT_1) return 0xa;
	else if(KeyByte[1] & BIT_5) return 0xb;
	else if(KeyByte[2] & BIT_5) return 9;
	else if(KeyByte[2] & BIT_6) return 7;
	else if(KeyByte[3] & BIT_0) return 0xf;
	else if(KeyByte[3] & BIT_1) return 0xe;
	else if(KeyByte[3] & BIT_2) return 0;
	else if(KeyByte[1] & BIT_2) return 0x13; 
	else if(KeyByte[1] & BIT_6) return 0x12; 
	else if(KeyByte[2] & BIT_2) return 0x11; 
	else if(KeyByte[2] & BIT_1) return 0x10; 

	return (0xFF);
}