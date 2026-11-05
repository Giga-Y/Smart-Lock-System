#ifndef __BUZZER_H
#define __BUZZER_H

#include "Hardware_Profile.h"

// 蜂鸣器引脚定义 (你的原理图是P4.1)
#define BEEP_PIN    BIT1

void Beep_Init(void);
void Beep_Short(void);

#endif