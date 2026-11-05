/**
 * @file    App.h
 * @brief   应用层接口
 * @details main() 只负责调用 App_Init() 完成一次性初始化，
 *          随后在死循环中反复调用 App_Run() 执行一轮应用调度。
 */

#ifndef __APP_H
#define __APP_H

/**
 * @brief  系统初始化
 * @details 完成时钟、定时器、GPIO、LCD、串口、Flash、触摸与 USB 初始化，
 *          并执行三步开机确认流程，最后显示主菜单。
 */
void App_Init(void);

/**
 * @brief  应用主循环单次调度
 * @details 依次处理 USB 数据、按键、密码输入、开锁计时与触摸音量，
 *          函数返回即代表完成一轮循环（调用方需持续调用）。
 */
void App_Run(void);

#endif /* __APP_H */
