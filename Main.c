/**
 * @file    Main.c
 * @brief   程序入口
 * @details 仅保留最小调度逻辑：初始化一次，然后循环执行应用层调度。
 *          具体功能实现见 App.c（业务逻辑）与 App_UI.c（界面绘制）。
 */

#include "App.h"

/**
 * @brief  程序入口
 * @return 不会返回（主循环为死循环）
 */
int main(void)
{
    App_Init();

    while (1)
    {
        App_Run();
    }
}
