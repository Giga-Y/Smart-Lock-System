#include "Speaker.h"
#include "voice_success.h" // 成功
#include "voice_error.h"   // 错误
#include "voice_locked.h"  // 锁定

// ================== 全局变量 ==================
volatile uint32_t voice_index = 0;
volatile uint8_t g_voice_on = 0; 
volatile u8 g_bgm_on = 0; 
u8 sin_index = 0;
volatile unsigned int current_period = 2301; 
u8 g_volume_level = 0; 

u8 was_bgm_on = 0; // 记录语音播报前是否在放 BGM

// 【新增】当前正在播放的语音数组指针和长度
const uint8_t *p_active_voice = 0;
uint32_t active_voice_len = 0;

// 正弦波表
const unsigned int sin_table[10] = {3251, 3995, 3995, 3251, 2048, 844, 100, 100, 844, 2048}; 
const unsigned int key_table[21] = {261,293,329,349,391,440,493,  533,587,659,698,783,880,987,  1046,1174,1318,1396,1567,1760,1975};
unsigned int period_table[21]; 

// =========================================================
// 【南京邮电大学校歌《信达天下》主旋律】


const u8 song_tone[] = { 
    // 1-8
    2, 7, 7,    7, 4,    5, 4, 3, 2, 1,    0,
    2, 4, 7,    8, 7, 5, 5, 4,    3, 2, 1, 2,    1,
    // 9-16
    2, 7, 7,    7, 4, 4,    6, 7, 8, 7, 6,    5,
    5, 6, 7, 7, 6, 7,       // 鼎山路 上 承 (6. 7 i i 7 i)
    8, 8, 7, 6, 5, 5, 4,    // 国脉担当 (2 2 i 7 6 v 6 5)
    3, 2, 1, 2,             // 承国脉担 (4 3 2. 3)
    0,                      // 当 (1 - - -)
    // 17-24
    2, 3, 4, 7,    7, 7, 6,    5, 6, 7, 5,    4,
    2, 3, 4, 7,    5, 5, 4,    3, 2, 2, 1,    1, 5,
    // 25-32 (根据最新截图重写：信达天下)
    4, 5, 4, 7,            // 达天下 (5. 6 5 i)
    2, 1, 7,               // 自强 (3 - - 2 i)
    6, 7, 8, 7,            // 不息 (7 i 2. i)
    5, 4, 4,               // 南邮 (6 - - 5. 5)
    4, 5, 4, 7,            // 达天下 (5. 6 5 i)
    2, 1, 7,               // 自强 (3 - - 2 i)
    6, 7, 8, 7,            // 不息 (7 i 2. i)
    5, 4, 4,               // 南邮 (6 - - 5. 5)
    // 33-38 (高潮)
    4, 4, 5, 7, 8, 9,    
    7, 255, 255, 255,    
    8, 8, 255, 255, 255,
    4, 4, 5, 7,    
    8, 9,    
    7
}; 

const u8 song_beat[] = { 
    // 1-8
    64, 16, 32,    32, 64,    16, 16, 16, 16, 16,    128,
    64, 16, 32,    24, 16, 32, 16, 16,    16, 16, 24, 16,    128,
    // 9-16
    64, 16, 32,    32, 64, 16,    24, 16, 16, 8, 8,    128,
    48, 16, 16, 16, 16, 16,     // 鼎(1.5拍) 山(0.5) 路(0.5) 上(0.5) 承(0.5)  (共4拍)
    16, 16, 16, 16, 32, 16, 16, // 国(0.5) 脉(0.5) 担(0.5) 当(0.5) 承(1)  (共3拍+连音)
    32, 32, 48, 16,             // 国(1) 脉(1) 担(1.5) 当(0.5)
    128,                        // 当(4拍)
    // 17-24
    16, 16, 24, 32,    64, 8, 8,    16, 16, 24, 16,    64,
    16, 16, 24, 32,    64, 16, 16,    16, 16, 24, 16,    64, 16,
    // 25-32 (截图完美解析)
    48, 16, 16, 48,        // 5.(48) 6(16) 5(16) i(48)
    96, 16, 16,            // 3(96) 2(16) i(16)
    16, 16, 48, 48,        // 7(16) i(16) 2.(48) i(48)
    96, 16, 16,            // 6(96) 5.(16) 5(16)
    48, 16, 16, 48,        // 5.(48) 6(16) 5(16) i(48)
    96, 16, 16,            // 3(96) 2(16) i(16)
    16, 16, 48, 48,        // 7(16) i(16) 2.(48) i(48)
    96, 16, 16,            // 6(96) 5.(16) 5(16)
    // 33-38
    24, 16, 16, 32, 32, 32,
    32, 32, 32, 32,
    128, 32, 32, 32, 32,
    24, 16, 16, 32,
    32, 32,
    128
}; 

// 自动计算音符数量
u8 song_len = sizeof(song_tone) / sizeof(song_tone[0]);
u8 song_idx = 0;
u16 beat_cnt = 0;

// ================== 初始化 ==================
void Speaker_Init(void) {
    u8 j;
    for(j = 0; j < 21; j++) {
        period_table[j] = 300000 / key_table[j]; 
    }

    P7SEL |= BIT6;    
    P5DIR |= BIT6;    
    P5OUT &= ~BIT6;   

    DAC12_1CTL0 = DAC12SREF_0 + DAC12AMP_7 + DAC12ENC + DAC12OPS;
    DAC12_1CTL1 |= 0x0002; 

    TB0CTL = TBSSEL__SMCLK | ID_2 | MC__UP | TBCLR; 
    TB0CCTL0 = CCIE;                  
    TB0CCR0 = current_period;          

    TA2CTL = TASSEL_2 + MC_1 + TACLR; 
    TA2CCR0 = 12000; 
    TA2CCTL0 = CCIE;
}

// ================== 通用语音触发器 ==================
void Speaker_Play_Voice_Generic(const uint8_t *p_voice, uint32_t len) {
    was_bgm_on = g_bgm_on; 
    g_bgm_on = 0; 
    g_voice_on = 1;
    voice_index = 0;
    
    p_active_voice = p_voice;
    active_voice_len = len;
    
    P5OUT &= ~BIT6; 
    TB0CCR0 = 750;  
}

// ================== 具体语音播放接口 ==================
void Speaker_Play_Success(void) {
    Speaker_Play_Voice_Generic(voice_success_samples, VOICE_SUCCESS_NUM_SAMPLES);
}

void Speaker_Play_Error(void) {
    Speaker_Play_Voice_Generic(voice_error_samples, VOICE_ERROR_NUM_SAMPLES);
}

void Speaker_Play_Locked(void) {
    Speaker_Play_Voice_Generic(voice_locked_samples, VOICE_LOCKED_NUM_SAMPLES);
}

// ================== BGM 播放函数 ==================
void Speaker_Play_BGM(u8 on_off) {
    if (g_voice_on) return; 
    
    if(on_off) {
        g_bgm_on = 1;
        song_idx = 0;
        beat_cnt = 0;
        if (song_tone[0] != 255) {
            current_period = period_table[song_tone[0]];
        }
        P5OUT &= ~BIT6; 
    } else {
        g_bgm_on = 0;
        P5OUT |= BIT6;  
    }
}

// ================== Timer B0 中断 (发声核心) ==================
#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer_B0_ISR(void) {
    int16_t temp_dac = 0;
    
    if (g_voice_on) {
        // --- 语音模式 ---
        DAC12_1DAT = p_active_voice[voice_index] * 16; 
        
        voice_index++;
        
        if (voice_index >= active_voice_len) {
            g_voice_on = 0;
            
            if (was_bgm_on) {
                g_bgm_on = 1;             
                TB0CCR0 = current_period; 
            } else {
                P5OUT |= BIT6;            
            }
        }
    } 
    else if (g_bgm_on) {
        // --- BGM 模式 ---
        // 遇到休止符 255 时，输出静音，不发声
        if (song_tone[song_idx] != 255) {
            temp_dac = (int16_t)sin_table[sin_index++] - 2048;
            DAC12_1DAT = 2048 + ((temp_dac * g_volume_level) >> 2);
            
            if(sin_index >= 10) sin_index = 0;
            TB0CCR0 = current_period; 
        } else {
            DAC12_1DAT = 2048; // 静音电平
            sin_index = 0;     // 重置正弦波索引
        }
    } 
    else {
        // --- 静音模式 ---
        DAC12_1DAT = 2048; 
    }
}

// ================== Timer A2 中断 (BGM 节拍) ==================
#pragma vector = TIMER2_A0_VECTOR
__interrupt void Timer_A2_ISR(void) {
    if(g_bgm_on && !g_voice_on) {
        beat_cnt++;
        if(beat_cnt >= (song_beat[song_idx] * 20)) { 
            beat_cnt = 0;
            song_idx++;
            if(song_idx >= song_len) song_idx = 0;
            
            // 只有不是休止符时才更新频率
            if (song_tone[song_idx] != 255) {
                current_period = period_table[song_tone[song_idx]]; 
            }
        }
    }
}