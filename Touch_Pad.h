#ifndef __TOUCH_PAD_H
#define __TOUCH_PAD_H

#include "Hardware_Profile.h"

// 声明滑条位置（0~5）
extern unsigned int G_SliderPosition;

void Initial_Touch_Pad(void);
void Deal_Touch_Pad(void);

#endif