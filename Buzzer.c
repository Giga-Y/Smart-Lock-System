#include "Buzzer.h"

void Beep_Init(void)
{
    P4DIR |= BEEP_PIN;
    P4OUT &= ~BEEP_PIN;
}

/**
 * @brief 短促蜂鸣，阻塞式，适合按键提示
 */
void Beep_Short(void)
{
    P4OUT |= BEEP_PIN;
    _delay_ms(50); // 50ms是比较完美的短促“滴”声
    P4OUT &= ~BEEP_PIN;
}