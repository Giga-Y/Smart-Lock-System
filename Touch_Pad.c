#include "Touch_Pad.h"
#include "structure.h"
#include "CTS_Layer.h"
#include "CTS_HAL.h"

unsigned int G_SliderPosition = 0;
u8 g_touch_last_pos = 0;

void Initial_Touch_Pad(void)  
{
    P3OUT &= ~0x01;
    P3DIR &= ~0x01;  
    P6OUT &= ~0x1F;
    P6DIR &= ~0x1F;  

    // 初始化并多次校准基线，确保当前环境的基准准确
    TI_CAPT_Init_Baseline(&slider);				
    TI_CAPT_Update_Baseline(&slider, 10); 
    
    G_SliderPosition = 0;
    g_touch_last_pos = 0;
}

void Deal_Touch_Pad(void)  
{
    unsigned int raw_val = TI_CAPT_Slider(&slider);	
    
    // 过滤异常值（如果库返回错误码）
    if (raw_val > 100) return; 
    
    // 【核心修复】0~100 均分为 5 档 (0, 1, 2, 3, 4)
    // 没碰的时候 raw_val 为 0，直接稳在 0 档
    u8 new_pos = 0;
    if (raw_val < 20) new_pos = 0;
    else if (raw_val < 40) new_pos = 1;
    else if (raw_val < 60) new_pos = 2;
    else if (raw_val < 80) new_pos = 3;
    else new_pos = 4;
    
    // 【核心修复】防抖逻辑：连续 3 次检测到的档位一样，才允许改变
    // 防止由于手指微颤或环境噪声导致音量频繁乱跳
    static u8 same_cnt = 0;
    if (new_pos != g_touch_last_pos) {
        same_cnt++;
        if (same_cnt >= 3) { 
            G_SliderPosition = new_pos;
            g_touch_last_pos = new_pos;
            same_cnt = 0;
        }
    } else {
        same_cnt = 0;
    }
}