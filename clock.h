/**
 * @file    clock.h
 * @brief   系统时钟与软件日历时钟接口
 */

#ifndef CLOCK_H_
#define CLOCK_H_

#include "Hardware_Profile.h"

/**
 * @brief 软件日历时间
 * @note  结构体可能在中断中被逐字段更新，读取时建议使用
 *        Clock_GetDateTime() 获取一致快照。
 */
typedef struct {
    u16 year;
    u8  month;
    u8  day;
    u8  hour;
    u8  minute;
    u8  second;
} Clock_DateTime;

extern volatile Clock_DateTime g_clock;

void Clock_Init(void);
void Clock_TimeInit(void);
void Clock_TimeTick1s(void);

/**
 * @brief  获取当前时间的一致快照
 * @param  out 输出缓冲区，不可为 NULL
 * @note   内部短暂关中断，避免 1 秒中断更新到一半时读到撕裂的数据。
 */
void Clock_GetDateTime(Clock_DateTime *out);

/**
 * @brief  将指定时间格式化为 "YYYY-MM-DD HH:MM:SS"
 * @param  time 待格式化的时间
 * @param  buf  输出缓冲区，至少 20 字节（19 字符 + '\0'）
 */
void Clock_DateTimeFormat(const Clock_DateTime *time, char *buf);

/**
 * @brief  将当前系统时间格式化为 "YYYY-MM-DD HH:MM:SS"
 * @param  buf 输出缓冲区，至少 20 字节
 */
void Clock_TimeFormat(char *buf);

/**
 * @brief  设置系统时间
 * @param  year/month/day/hour/minute/second 待设置的日期时间
 * @return 1 = 设置成功；0 = 参数非法，时间保持不变
 */
u8 Clock_TimeSet(u16 year, u8 month, u8 day, u8 hour, u8 minute, u8 second);

#endif
