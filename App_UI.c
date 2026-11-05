/**
 * @file    App_UI.c
 * @brief   人机界面层：LCD / TM1638 显示与页面绘制
 * @details 本文件只负责“怎么显示”，集中实现全部页面绘制、密码输入回显、
 *          音量条与开机/解锁流程用到的界面函数；不包含任何业务流程判断。
 *
 * @note    所有绘制函数均为阻塞式，调用一次即完成一屏刷新。
 */

#include <stdio.h>
#include <string.h>

#include "Hardware_Profile.h"
#include "App_UI.h"
#include "App_State.h"
#include "LCD_TFT_ILI9325.h"
#include "TM1638.h"
#include "Flash_Storage.h"
#include "Font_Chinese.h"
#include "clock.h"
/**
 * @brief  刷新 TM1638 数码管显示（开锁状态与音量）
 */
void TM1638_Update_Display(u8 is_open, u8 vol) {
    u8 h = vol / 100;
    u8 t = (vol % 100) / 10;
    u8 o = vol % 10;
    
    TM1638_Disp_Mode(Disp_Mode_14_16);
    if (is_open) {
        Write_DispData_TM1638(0, 0, 29, 25, 14, 27, 21, h, t, o);
    } else {
        Write_DispData_TM1638(0, 0, 12, 26, 29, 28, 14, 21, 21, 21);
    }
}

/**
 * @brief  绘制音量调节浮层与百分比文字
 */
void Draw_Voice_UI(u8 vol_level) {
    u8 percent = vol_level * 25;
    char buf[20];
    LCD_TFT_FillRectangle(120, 260, 239, 315, BLACK);
    sprintf(buf, "voice:%d%%", percent);
    LCD_TFT_ShowString(118, 255, (u8*)buf, FONT1206, WHITE, BLACK); 
    LCD_TFT_DrawRectangle(130, 270, 230, 285, WHITE); 
    u8 rect_w = vol_level * 25; 
    if (rect_w > 0) {
        LCD_TFT_FillRectangle(230 - rect_w, 271, 229, 284, GREEN); 
    }
}

/**
 * @brief  清除音量调节浮层
 */
void Clear_Voice_UI(void) {
    LCD_TFT_FillRectangle(118, 255, 239, 315, BLACK);
}

/**
 * @brief  在指定坐标显示 16x16 汉字点阵
 */
void LCD_TFT_ShowChinese_16x16(u16 x, u16 y, const u8 *pData, u16 pointColor, u16 backColor)
{
    u8 i, j;
    u16 byte;
    if(x > 304 || y > 224) return; 
    
    LCD_TFT_SetWindow(x, y, x + 15, y + 15);
    LCD_TFT_SetCursor(x, y);
    LCD_TFT_WriteRAM_CMD();
    for(i = 0; i < 16; i++) { 
        byte = ((u16)pData[i * 2] << 8) | pData[i * 2 + 1]; 
        for(j = 0; j < 16; j++) { 
            if(byte & (0x8000 >> j)) LCD_TFT_WriteRAM(pointColor); 
            else LCD_TFT_WriteRAM(backColor);
        }
    }
    LCD_TFT_SetWindow(0, 0, 239, 319);
}

/**
 * @brief  绘制开机信息页
 */
void Draw_Boot_Page(void) {
    LCD_TFT_Clear(BLACK);
    LCD_TFT_ShowString(20, 5, (u8*)"2026", FONT1608, WHITE, BLACK);
    LCD_TFT_ShowChinese_16x16(20, 37, HZ_Nian, WHITE, BLACK);
    LCD_TFT_ShowChinese_16x16(20, 54, HZ_Wei, WHITE, BLACK);
    LCD_TFT_ShowChinese_16x16(20, 71, HZ_Chu, WHITE, BLACK);
    LCD_TFT_ShowChinese_16x16(20, 88, HZ_Li, WHITE, BLACK);
    LCD_TFT_ShowChinese_16x16(20, 105, HZ_Qi, WHITE, BLACK);
    LCD_TFT_ShowChinese_16x16(20, 122, HZ_Ying, WHITE, BLACK);
    LCD_TFT_ShowChinese_16x16(20, 139, HZ_Yong, WHITE, BLACK);
    LCD_TFT_ShowChinese_16x16(20, 156, HZ_Ke, WHITE, BLACK);
    LCD_TFT_ShowChinese_16x16(20, 173, HZ_Cheng, WHITE, BLACK);
    LCD_TFT_ShowChinese_16x16(20, 190, HZ_She, WHITE, BLACK);
    LCD_TFT_ShowChinese_16x16(20, 207, HZ_Ji, WHITE, BLACK);
    LCD_TFT_ShowChinese_16x16(60, 10, HZ_Tong, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(60, 28, HZ_Xin, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(60, 46, HZ_Gong, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(60, 64, HZ_Cheng, GREEN, BLACK);
    LCD_TFT_ShowString(60, 86, (u8*)"B23010526", FONT1608, GREEN, BLACK); 
    LCD_TFT_ShowChinese_16x16(60, 158, HZ_Gu, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(60, 178, HZ_Yang, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(100, 10, HZ_Tong, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(100, 28, HZ_Xin, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(100, 46, HZ_Gong, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(100, 64, HZ_Cheng, GREEN, BLACK);
    LCD_TFT_ShowString(100, 86, (u8*)"B23010528", FONT1608, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(100, 158, HZ_Sheng, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(100, 178, HZ_Xiang, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(100, 198, HZ_Yu, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(140, 10, HZ_Wan, CYAN, BLACK);
    LCD_TFT_ShowChinese_16x16(140, 30, HZ_Cheng2, CYAN, BLACK);
    LCD_TFT_ShowChinese_16x16(140, 50, HZ_Ri, CYAN, BLACK);
    LCD_TFT_ShowChinese_16x16(140, 70, HZ_Qi2, CYAN, BLACK);
    LCD_TFT_ShowString(140, 90, (u8*)":2026", FONT1608, CYAN, BLACK);
    LCD_TFT_ShowChinese_16x16(140, 130, HZ_Nian, CYAN, BLACK);
    LCD_TFT_ShowString(140, 150, (u8*)"10", FONT1608, CYAN, BLACK);
    LCD_TFT_ShowChinese_16x16(140, 166, HZ_Yue, CYAN, BLACK);
    LCD_TFT_ShowString(140, 186, (u8*)"28", FONT1608, CYAN, BLACK);
    LCD_TFT_ShowChinese_16x16(140, 202, HZ_Ri, CYAN, BLACK);
    LCD_TFT_ShowString(210, 80, (u8*)"Press Any Key...", FONT1206, WHITE, BLACK);
}

/**
 * @brief  绘制题目要求第 1 页
 */
void Draw_Req_Page1(void) {
    LCD_TFT_Clear(BLACK);
    LCD_TFT_ShowString(20, 10, (u8*)"17. ", FONT1608, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(20, 42, HZ_Dian, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(20, 58, HZ_Zi, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(20, 74, HZ_Mi, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(20, 90, HZ_Ma, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(20, 106, HZ_Suo, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(50, 10, HZ_JiBen, GREEN, BLACK); 
    LCD_TFT_ShowChinese_16x16(50, 26, HZ_Ben, GREEN, BLACK); 
    LCD_TFT_ShowChinese_16x16(50, 42, HZ_Yao, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(50, 58, HZ_Qiu, GREEN, BLACK);
    LCD_TFT_ShowString(50, 74, (u8*)":", FONT1608, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(80, 10, HZ_Tong, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(80, 26, HZ_Guo, GREEN, BLACK);
    LCD_TFT_ShowString(80, 44, (u8*)"USB", FONT1608, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(80, 70, HZ_Yu2, GREEN, BLACK);
    LCD_TFT_ShowString(80, 86, (u8*)"PC", FONT1608, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(80, 106, HZ_Ji2, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(80, 122, HZ_Tong, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(80, 138, HZ_Xin, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(110, 10, HZ_Jiang, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(110, 26, HZ_Mi, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(110, 42, HZ_Ma, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(110, 58, HZ_You, GREEN, BLACK);
    LCD_TFT_ShowString(110, 76, (u8*)"PC", FONT1608, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(110, 94, HZ_Ji2, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(110, 110, HZ_Shu, GREEN, BLACK); 
    LCD_TFT_ShowChinese_16x16(110, 126, HZ_Ru, GREEN, BLACK);  
    LCD_TFT_ShowChinese_16x16(110, 142, HZ_Gei, GREEN, BLACK); 
    LCD_TFT_ShowChinese_16x16(110, 158, HZ_Dan, GREEN, BLACK); 
    LCD_TFT_ShowChinese_16x16(110, 174, HZ_Pian, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(110, 190, HZ_Ji2, GREEN, BLACK); 
    LCD_TFT_ShowString(110, 206, (u8*)".", FONT1608, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(140, 10, HZ_ChuShi, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(140, 26, HZ_Mi, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(140, 42, HZ_Ma, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(140, 58, HZ_Shi, GREEN, BLACK);
    LCD_TFT_ShowString(140, 74, (u8*)"000000", FONT1608, GREEN, BLACK);
    LCD_TFT_ShowString(210, 80, (u8*)"Press Any Key...", FONT1206, CYAN, BLACK);
}

/**
 * @brief  绘制题目要求第 2 页
 */
void Draw_Req_Page2(void) {
    LCD_TFT_Clear(BLACK);
    LCD_TFT_ShowChinese_16x16(20, 10, HZ_Ruo, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(20, 26, HZ_Mi, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(20, 42, HZ_Ma, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(20, 58, HZ_Bu, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(20, 74, HZ_Zheng, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(20, 90, HZ_Que, GREEN, BLACK);
    LCD_TFT_ShowString(20, 106, (u8*)",", FONT1608, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(20, 114, HZ_Ke2, GREEN, BLACK);   
    LCD_TFT_ShowChinese_16x16(20, 130, HZ_Chong, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(20, 146, HZ_Xin2, GREEN, BLACK); 
    LCD_TFT_ShowChinese_16x16(20, 162, HZ_Shu, GREEN, BLACK);  
    LCD_TFT_ShowChinese_16x16(20, 178, HZ_Ru, GREEN, BLACK);   
    LCD_TFT_ShowChinese_16x16(20, 194, HZ_Mi, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(20, 210, HZ_Ma, GREEN, BLACK);
    LCD_TFT_ShowString(20, 226, (u8*)".", FONT1608, GREEN, BLACK);
    LCD_TFT_ShowString(50, 10, (u8*)"LED", FONT1608, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(50, 34, HZ_Huang, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(50, 50, HZ_Deng, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(50, 66, HZ_Liang, GREEN, BLACK);
    LCD_TFT_ShowString(50, 82, (u8*)".", FONT1608, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(80, 10, HZ_Ru2, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(80, 26, HZ_Lian, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(80, 42, HZ_Xu, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(80, 58, HZ_San, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(80, 74, HZ_Ci, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(80, 90, HZ_Shu, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(80, 106, HZ_Ru, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(80, 122, HZ_Cuo, GREEN, BLACK);
    LCD_TFT_ShowString(80, 138, (u8*)",", FONT1608, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(80, 146, HZ_Ze, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(80, 162, HZ_Jin, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(80, 178, HZ_Zhi, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(80, 194, HZ_Shu, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(80, 210, HZ_Ru, GREEN, BLACK);
    LCD_TFT_ShowString(80, 226, (u8*)".", FONT1608, GREEN, BLACK);
    LCD_TFT_ShowString(110, 10, (u8*)"LED", FONT1608, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(110, 34, HZ_Hong, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(110, 50, HZ_Deng, GREEN, BLACK);
    LCD_TFT_ShowChinese_16x16(110, 66, HZ_Liang, GREEN, BLACK);
    LCD_TFT_ShowString(110, 82, (u8*)".", FONT1608, GREEN, BLACK);
    LCD_TFT_ShowString(210, 80, (u8*)"Press Any Key...", FONT1206, CYAN, BLACK);
}

/**
 * @brief  绘制功能主菜单
 */
void Draw_Welcome_Page(void) {
    LCD_TFT_Clear(BLACK);
    LCD_TFT_ShowString(30, 20, (u8*)"Welcome to E-Lock", FONT1608, WHITE, BLACK);
    LCD_TFT_ShowString(60, 20, (u8*)"1. Unlock", FONT1608, GREEN, BLACK);
    LCD_TFT_ShowString(90, 20, (u8*)"2. Modify Pwd", FONT1608, GREEN, BLACK);
    LCD_TFT_ShowString(120, 20, (u8*)"3. View Records", FONT1608, GREEN, BLACK); 
    LCD_TFT_ShowString(200, 20, (u8*)"\"=\" BGM On/Off", FONT1206, CYAN, BLACK);
    /* 与 BGM 提示同一行，紧接其右侧显示 C 键同步时间提示 */
    LCD_TFT_ShowString(200, 124, (u8*)"\"C\" Sync Time", FONT1206, CYAN, BLACK);

    Draw_Time_Status();
}

/**
 * @brief  绘制密码显示模式选择页
 */
void Draw_Mode_Page(void) {
    LCD_TFT_Clear(BLACK);
    LCD_TFT_ShowString(30, 20, (u8*)"Select Display:", FONT1608, WHITE, BLACK);
    LCD_TFT_ShowString(80, 20, (u8*)"1. Plain Text", FONT1608, GREEN, BLACK);
    LCD_TFT_ShowString(120, 20, (u8*)"2. Mask Text (*)", FONT1608, GREEN, BLACK);
    LCD_TFT_ShowString(200, 20, (u8*)"AC: Back", FONT1206, CYAN, BLACK);
    Draw_Time_Status();
}

/**
 * @brief  刷新密码输入回显区
 */
void Update_Input_UI(u8 cnt) {
    if (g_is_locked) return; 
    char display_str[20] = "Input: "; 
    u8 i;
    for(i = 0; i < cnt; i++) {
        if (g_mask_mode == 1) display_str[7 + i] = '*'; 
        else display_str[7 + i] = g_pwd_buf[i]; 
    }
    display_str[7 + cnt] = '\0'; 
    LCD_TFT_FillRectangle(120, 10, 140, 230, BLACK); 
    LCD_TFT_ShowString(120, 20, (u8*)display_str, FONT1608, GREEN, BLACK);
}

/**
 * @brief  恢复当前页面的操作提示行
 */
void Restore_Hint_Line(void) {
    LCD_TFT_FillRectangle(160, 10, 239, 230, BLACK); 
    if (g_ui_state == APP_STATE_UNLOCK) LCD_TFT_ShowString(70, 20, (u8*)"Press KEY2 Confirm", FONT1608, CYAN, BLACK);
    else if (g_ui_state == APP_STATE_VERIFY_OLD) LCD_TFT_ShowString(70, 20, (u8*)"Enter Old Pwd:", FONT1608, CYAN, BLACK);
    else if (g_ui_state == APP_STATE_VERIFY_VIEW) LCD_TFT_ShowString(70, 20, (u8*)"Verify for View:", FONT1608, CYAN, BLACK); 
    else if (g_ui_state == APP_STATE_MODIFY_NEW) LCD_TFT_ShowString(70, 20, (u8*)"Enter New Pwd:", FONT1608, CYAN, BLACK);
    else if (g_ui_state == APP_STATE_MODIFY_CONFIRM) LCD_TFT_ShowString(70, 20, (u8*)"Confirm New Pwd:", FONT1608, CYAN, BLACK);
}

/**
 * @brief  绘制密码错误提示
 * @param  type         错误类型
 * @param  remain_tries 剩余尝试次数（仅 PWD_ERROR_WRONG 使用）
 */
void Draw_Password_Error_UI(PwdErrorType type, u8 remain_tries) {
    char buf[20];

    LCD_TFT_FillRectangle(160, 10, 239, 230, BLACK);

    switch (type) {
        case PWD_ERROR_WRONG:
            LCD_TFT_ShowString(160, 20, (u8*)"WRONG PWD!", FONT1608, RED, BLACK);
            sprintf(buf, "Remain: %d", remain_tries);
            LCD_TFT_ShowString(185, 20, (u8*)buf, FONT1608, RED, BLACK);
            break;
        case PWD_ERROR_OLD:
            LCD_TFT_ShowString(160, 20, (u8*)"Old Pwd Wrong!", FONT1206, RED, BLACK);
            break;
        case PWD_ERROR_VIEW:
            LCD_TFT_ShowString(160, 20, (u8*)"Pwd Wrong!", FONT1206, RED, BLACK);
            break;
        case PWD_ERROR_LENGTH:
            LCD_TFT_ShowString(160, 20, (u8*)"Length Err!(4-6)", FONT1206, RED, BLACK);
            break;
        case PWD_ERROR_MISMATCH:
            LCD_TFT_ShowString(160, 20, (u8*)"Mismatch! Retry", FONT1206, RED, BLACK);
            break;
        default:
            break;
    }
}

/**
 * @brief  绘制密码修改成功页
 */
void Draw_Pwd_Changed_Page(void) {
    LCD_TFT_Clear(BLACK);
    LCD_TFT_ShowString(40, 20, (u8*)"Pwd Changed!", FONT1608, GREEN, BLACK);
    Draw_Time_Status();
}

/**
 * @brief  刷新开锁成功页的持续时长
 * @param  elapsed 开锁已持续的秒数
 */
void Draw_Open_Duration_UI(unsigned int elapsed) {
    char buf[30];

    sprintf(buf, "Duration : %d s       ", elapsed);
    LCD_TFT_ShowString(100, 20, (u8*)buf, FONT1608, WHITE, BLACK);
}

/**
 * @brief  绘制开锁密码输入页
 */
void Draw_Unlock_Page(void) {
    if (g_is_locked) {
        Draw_Locked_Page();
        return;
    }
    LCD_TFT_Clear(BLACK);
    LCD_TFT_ShowString(30, 20, (u8*)"E-Lock Unlock", FONT1608, WHITE, BLACK);
    LCD_TFT_ShowString(70, 20, (u8*)"Press KEY2 Confirm", FONT1608, CYAN, BLACK);
    g_pwd_cnt = 0; memset(g_pwd_buf, 0, 6);
    Update_Input_UI(g_pwd_cnt); 
    LCD_TFT_ShowString(200, 20, (u8*)"AC: Back", FONT1206, CYAN, BLACK);
    Draw_Time_Status();
}

/**
 * @brief  绘制系统锁定页
 * @note   输入错误密码达到上限后进入，仅允许复位恢复。
 */
void Draw_Locked_Page(void) {
    LCD_TFT_Clear(BLACK);
    LCD_TFT_ShowString(60, 20, (u8*)"SYSTEM LOCKED", FONT1608, RED, BLACK);
    LCD_TFT_ShowString(100, 20, (u8*)"Too many errors.", FONT1608, RED, BLACK);
    LCD_TFT_ShowString(160, 20, (u8*)"Please Reset RST", FONT1206, WHITE, BLACK);
    Draw_Time_Status();
}

/**
 * @brief  绘制修改密码前的旧密码验证页
 */
void Draw_Verify_Old_Pwd_Page(void) {
    LCD_TFT_Clear(BLACK);
    LCD_TFT_ShowString(30, 20, (u8*)"Verify Identity", FONT1608, WHITE, BLACK);
    LCD_TFT_ShowString(70, 20, (u8*)"Enter Old Pwd:", FONT1608, CYAN, BLACK);
    g_pwd_cnt = 0; memset(g_pwd_buf, 0, 6);
    Update_Input_UI(g_pwd_cnt); 
    LCD_TFT_ShowString(200, 20, (u8*)"AC: Back", FONT1206, CYAN, BLACK);
    Draw_Time_Status();
}

/**
 * @brief  绘制查看记录前的密码验证页
 */
void Draw_View_Verify_Page(void) {
    LCD_TFT_Clear(BLACK);
    LCD_TFT_ShowString(30, 20, (u8*)"View Records", FONT1608, WHITE, BLACK);
    LCD_TFT_ShowString(70, 20, (u8*)"Verify for View:", FONT1608, CYAN, BLACK);
    g_pwd_cnt = 0; memset(g_pwd_buf, 0, 6);
    Update_Input_UI(g_pwd_cnt); 
    LCD_TFT_ShowString(200, 20, (u8*)"AC: Back", FONT1206, CYAN, BLACK);
    Draw_Time_Status();
}

/**
 * @brief  绘制记录查询菜单页
 */
void Draw_Record_Menu_Page(void) {
    LCD_TFT_Clear(BLACK);
    LCD_TFT_ShowString(30, 20, (u8*)"Records Menu:", FONT1608, WHITE, BLACK);
    LCD_TFT_ShowString(80, 20, (u8*)"1. Pwd History", FONT1608, GREEN, BLACK);
    LCD_TFT_ShowString(120, 20, (u8*)"2. Unlock History", FONT1608, GREEN, BLACK);
    LCD_TFT_ShowString(200, 20, (u8*)"AC: Back", FONT1206, CYAN, BLACK);
    Draw_Time_Status();
}

/**
 * @brief  绘制密码修改历史页
 */
void Draw_Pwd_History_Page(void) {
    LCD_TFT_Clear(BLACK);
    LCD_TFT_ShowString(20, 20, (u8*)"Pwd History (5)", FONT1608, WHITE, BLACK);
    char buf[30];
    char temp_pwd[10]; 
    u8 i;
    u8 y = 60;
    for(i = 0; i < 5; i++) {
        if(g_sys_data.pwd_history[i].len > 0) {
            memset(temp_pwd, 0, 10);
            memcpy(temp_pwd, g_sys_data.pwd_history[i].pwd, 6);
            temp_pwd[6] = '\0'; 
            sprintf(buf, "%d. %s", i+1, temp_pwd);
            LCD_TFT_ShowString(y, 20, (u8*)buf, FONT1608, GREEN, BLACK);
        } else {
            sprintf(buf, "%d. Empty", i+1);
            LCD_TFT_ShowString(y, 20, (u8*)buf, FONT1608, GRAY, BLACK);
        }
        y += 24;
    }
    LCD_TFT_ShowString(200, 20, (u8*)"AC: Back", FONT1206, CYAN, BLACK);
    Draw_Time_Status();
}

/**
 * @brief  绘制开锁历史页
 * @details 每条记录显示完整日历时间 "年-月-日 时:分:秒"，
 *          右侧保留原有的开锁时长信息（Dur:秒）。
 */
void Draw_Unlock_History_Page(void) {
    LCD_TFT_Clear(BLACK);
    LCD_TFT_ShowString(20, 20, (u8*)"Unlock History", FONT1608, WHITE, BLACK);

    char buf[32];
    char time_buf[20];
    u8 i;
    u16 y;

    for(i = 0; i < UNLOCK_HISTORY_MAX; i++) {
        y = 56 + i * 24;
        if(g_sys_data.unlock_history[i].valid) {
            Clock_DateTimeFormat(&g_sys_data.unlock_history[i].time, time_buf);
            sprintf(buf, "%d. %s", i + 1, time_buf);
            LCD_TFT_ShowString(y, 20, (u8*)buf, FONT1206, CYAN, BLACK);

            /* --printf_support=minimal 不支持 %u，开锁时长按秒级 int 输出 */
            sprintf(buf, "Dur:%ds", (int)g_sys_data.unlock_history[i].duration);
            LCD_TFT_ShowString(y, 176, (u8*)buf, FONT1206, CYAN, BLACK);
        } else {
            sprintf(buf, "%d. Empty", i + 1);
            LCD_TFT_ShowString(y, 20, (u8*)buf, FONT1206, GRAY, BLACK);
        }
    }

    LCD_TFT_ShowString(200, 20, (u8*)"AC: Back", FONT1206, CYAN, BLACK);
    Draw_Time_Status();
}

/**
 * @brief  绘制输入新密码页
 */
void Draw_Modify_Page(void) {
    LCD_TFT_Clear(BLACK);
    LCD_TFT_ShowString(30, 20, (u8*)"Modify Password", FONT1608, WHITE, BLACK);
    LCD_TFT_ShowString(70, 20, (u8*)"Enter New Pwd:", FONT1608, CYAN, BLACK);
    g_pwd_cnt = 0; memset(g_pwd_buf, 0, 6);
    Update_Input_UI(g_pwd_cnt); 
    LCD_TFT_ShowString(200, 20, (u8*)"AC: Back", FONT1206, CYAN, BLACK);
    Draw_Time_Status();
}

/**
 * @brief  绘制确认新密码页
 */
void Draw_Confirm_Page(void) {
    LCD_TFT_Clear(BLACK);
    LCD_TFT_ShowString(30, 20, (u8*)"Modify Password", FONT1608, WHITE, BLACK);
    LCD_TFT_ShowString(70, 20, (u8*)"Confirm New Pwd:", FONT1608, CYAN, BLACK);
    g_pwd_cnt = 0; memset(g_pwd_buf, 0, 6);
    Update_Input_UI(g_pwd_cnt); 
    LCD_TFT_ShowString(200, 20, (u8*)"AC: Back", FONT1206, CYAN, BLACK);
    Draw_Time_Status();
}

/**
 * @brief  绘制开锁成功与计时信息页
 */
void Draw_Time_Record_Page(void) {
    char buf[40];
    char time_buf[20];

    LCD_TFT_Clear(BLACK);
    LCD_TFT_ShowString(20, 20, (u8*)"SUCCESS!", FONT1608, GREEN, BLACK);

    /* 本次开锁时刻：使用开锁成功瞬间抓取的日历时间 */
    Clock_DateTimeFormat(&g_open_datetime, time_buf);
    sprintf(buf, "Open Time: %s", time_buf);
    LCD_TFT_ShowString(60, 20, (u8*)buf, FONT1608, WHITE, BLACK);

    sprintf(buf, "Duration : %d s       ", g_sys_sec - g_open_time);
    LCD_TFT_ShowString(100, 20, (u8*)buf, FONT1608, WHITE, BLACK);

    /* 上一次开锁记录：直接读取 Flash 中索引 0 的最近一条记录 */
    if (g_sys_data.unlock_history[0].valid) {
        Clock_DateTimeFormat(&g_sys_data.unlock_history[0].time, time_buf);
        sprintf(buf, "Last Open: %s", time_buf);
        LCD_TFT_ShowString(150, 20, (u8*)buf, FONT1206, CYAN, BLACK);

        sprintf(buf, "Last Dur : %d s       ",
                g_sys_data.unlock_history[0].duration);
        LCD_TFT_ShowString(180, 20, (u8*)buf, FONT1206, CYAN, BLACK);
    }

    LCD_TFT_ShowString(220, 20, (u8*)"Press AC to back", FONT1206, CYAN, BLACK);
    Draw_Time_Status();
}
/**
 * @brief  在屏幕右上角绘制当前时间
 * @note   屏幕为 320x240 横向显示：ShowString 的第一个参数是行（垂直），
 *         第二个参数是列（水平）。时间串固定 19 字符，FONT1206 下宽 114 像素，
 *         从第 200 列开始可完整落在屏幕右侧。
 */
void Draw_Time_Status(void) {
    char buf[20];

    Clock_TimeFormat(buf);
    LCD_TFT_ShowString(2, 200, (u8*)buf, FONT1206, YELLOW, BLACK);
}

/**
 * @brief  在时间同一行的左侧显示时间同步结果
 * @param  success 1 = 成功（绿色）；0 = 失败（红色）
 * @note   先清空左侧提示区，避免成功/失败文案长度不同留下残影。
 */
void Draw_Time_Sync_Result(u8 success) {
    LCD_TFT_FillRectangle(0, 0, 16, 196, BLACK);

    if (success) {
        LCD_TFT_ShowString(2, 20, (u8*)"TIME SYNC SUCCESS", FONT1206, GREEN, BLACK);
    } else {
        LCD_TFT_ShowString(2, 20, (u8*)"TIME SYNC FAILED", FONT1206, RED, BLACK);
    }
}

/**
 * @brief 清除时间同步结果提示
 */
void Clear_Time_Sync_Result(void) {
    LCD_TFT_FillRectangle(0, 0, 16, 196, BLACK);
}
