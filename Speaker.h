#ifndef __SPEAKER_H
#define __SPEAKER_H

#include "Hardware_Profile.h"
#include <stdint.h>

// ================== 外部全局变量声明 ==================
extern volatile uint32_t voice_index;
extern volatile uint8_t g_voice_on; 
extern volatile u8 g_bgm_on; 
extern u8 sin_index;
extern volatile unsigned int current_period; 
extern u8 g_volume_level; 

// ================== 外部函数声明 ==================
void Speaker_Init(void);
void Speaker_Play_BGM(u8 on_off);

// 【新增】三种状态语音播报接口
void Speaker_Play_Success(void); // 开锁成功
void Speaker_Play_Error(void);   // 密码错误
void Speaker_Play_Locked(void);  // 系统锁定

#endif