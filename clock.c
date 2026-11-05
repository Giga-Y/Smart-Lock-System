/**
 * @file    clock.c
 * @brief   系统时钟初始化与软件日历时钟
 * @details 负责配置 12MHz DCO/SMCLK，并在 1 秒中断驱动下维护一套
 *          可读的日历时间（年月日时分秒）。时间同步入口为 Clock_TimeSet()。
 */

#include "Hardware_Profile.h"
#include "clock.h"
#include "HAL_PMM.h"

void Clock_Init(void)
{
    // 1. 设置内核电压为 3 级 (支持 12MHz)
    SetVCore(PMMCOREV_3);

    // 步骤一：先使用内部 REFO 配置 FLL，产生稳定的 12MHz DCO，让 CPU 先跑起来
    UCSCTL3 = SELREF_2;                     
    __bis_SR_register(SCG0);                
    UCSCTL0 = 0x0000;
    UCSCTL1 = DCORSEL_5;                    
    UCSCTL2 = FLLD_0 + 365;                 
    __bic_SR_register(SCG0);                

    // 步骤二：开启外部晶振 XT1 和 XT2
    P5SEL |= BIT4 | BIT5;                   
    P7SEL |= BIT2 | BIT3;                   
    UCSCTL7 &= ~(XT2OFFG | XT1LFOFFG | XT1HFOFFG | DCOFFG);
    SFRIFG1 &= ~OFIFG;
    UCSCTL6 &= ~(XT1OFF | XT2OFF);          
    UCSCTL6 |= XCAP_3;                      

    // 步骤三：分配时钟源
    UCSCTL4 = SELM_4 | SELS_4 | SELA_0;

    // 步骤四：等待晶振稳定
    unsigned int timeout = 0xFFFF;
    do {
        UCSCTL7 &= ~(XT2OFFG | XT1LFOFFG | XT1HFOFFG | DCOFFG);
        SFRIFG1 &= ~OFIFG;
        timeout--;
    } while ((SFRIFG1 & OFIFG) && timeout > 0);
    UCSCTL6 &= ~XT2DRIVE0;
}

/* ==================== 以下为新增的软件走时 ==================== */
volatile Clock_DateTime g_clock = {2026, 1, 1, 0, 0, 0};
static const u8 s_month_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static u8 Clock_DaysOfMonth(u16 year, u8 month) {
    if (month == 2) {
        if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) return 29;
    }
    return s_month_days[month - 1];
}

void Clock_TimeInit(void) {
    g_clock.year = 2026; g_clock.month = 1; g_clock.day = 1;
    g_clock.hour = 0; g_clock.minute = 0; g_clock.second = 0;
}

void Clock_TimeTick1s(void) {
    g_clock.second++;
    if (g_clock.second < 60) return;
    g_clock.second = 0;
    g_clock.minute++;
    if (g_clock.minute < 60) return;
    g_clock.minute = 0;
    g_clock.hour++;
    if (g_clock.hour < 24) return;
    g_clock.hour = 0;
    g_clock.day++;
    if (g_clock.day <= Clock_DaysOfMonth(g_clock.year, g_clock.month)) return;
    g_clock.day = 1;
    g_clock.month++;
    if (g_clock.month <= 12) return;
    g_clock.month = 1;
    g_clock.year++;
}

void Clock_GetDateTime(Clock_DateTime *out)
{
    u16 gie_state;

    if (out == 0) return;

    /* 1 秒中断会逐字段更新 g_clock，读取期间关中断以保证快照一致 */
    gie_state = __get_SR_register() & GIE;
    __disable_interrupt();
    *out = g_clock;
    if (gie_state) __enable_interrupt();
}

void Clock_DateTimeFormat(const Clock_DateTime *time, char *buf)
{
    u16 year = time->year;
    u8 month = time->month, day = time->day;
    u8 hour = time->hour, minute = time->minute, second = time->second;

    buf[0] = (char)('0' + (year / 1000) % 10); buf[1] = (char)('0' + (year / 100) % 10);
    buf[2] = (char)('0' + (year / 10) % 10);   buf[3] = (char)('0' + year % 10); buf[4] = '-';
    buf[5] = (char)('0' + (month / 10) % 10);  buf[6] = (char)('0' + month % 10); buf[7] = '-';
    buf[8] = (char)('0' + (day / 10) % 10);    buf[9] = (char)('0' + day % 10);   buf[10] = ' ';
    buf[11] = (char)('0' + (hour / 10) % 10);  buf[12] = (char)('0' + hour % 10); buf[13] = ':';
    buf[14] = (char)('0' + (minute / 10) % 10);buf[15] = (char)('0' + minute % 10);buf[16] = ':';
    buf[17] = (char)('0' + (second / 10) % 10);buf[18] = (char)('0' + second % 10);buf[19] = '\0';
}

void Clock_TimeFormat(char *buf)
{
    Clock_DateTime now;

    Clock_GetDateTime(&now);
    Clock_DateTimeFormat(&now, buf);
}
u8 Clock_TimeSet(u16 year, u8 month, u8 day, u8 hour, u8 minute, u8 second)
{
    u16 gie_state;

    /* 先做完整范围校验，非法数据不修改当前时间 */
    if (year < 2000 || year > 2099) return 0;
    if (month < 1 || month > 12) return 0;
    if (day < 1 || day > Clock_DaysOfMonth(year, month)) return 0;
    if (hour > 23 || minute > 59 || second > 59) return 0;

    /* 短暂关中断，保证时间变量同时更新，并恢复调用前的中断状态 */
    gie_state = __get_SR_register() & GIE;
    __disable_interrupt();
    g_clock.year   = year;
    g_clock.month  = month;
    g_clock.day    = day;
    g_clock.hour   = hour;
    g_clock.minute = minute;
    g_clock.second = second;
    if (gie_state) __enable_interrupt();

    return 1;
}
