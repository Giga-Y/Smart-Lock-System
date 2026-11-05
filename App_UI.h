/**
 * @file    App_UI.h
 * @brief   人机界面层接口
 * @details 提供 LCD 页面绘制、密码回显、音量浮层与 TM1638 刷新接口。
 */

#ifndef __APP_UI_H
#define __APP_UI_H

#include "Hardware_Profile.h"

/* ==================== 数码管与音量 UI ==================== */
void TM1638_Update_Display(u8 is_open, u8 vol);
void Draw_Voice_UI(u8 vol_level);
void Clear_Voice_UI(void);

/* ==================== LCD 基础绘制 ==================== */
void LCD_TFT_ShowChinese_16x16(u16 x, u16 y, const u8 *pData, u16 pointColor, u16 backColor);

/* ==================== 密码操作反馈 ==================== */
/**
 * @brief 密码错误提示类型
 */
typedef enum {
    PWD_ERROR_WRONG = 0,   /**< 开锁密码错误（显示剩余次数） */
    PWD_ERROR_OLD,         /**< 旧密码错误 */
    PWD_ERROR_VIEW,        /**< 查看记录验证失败 */
    PWD_ERROR_LENGTH,      /**< 新密码长度非法 */
    PWD_ERROR_MISMATCH     /**< 两次新密码不一致 */
} PwdErrorType;

void Draw_Password_Error_UI(PwdErrorType type, u8 remain_tries);
void Draw_Pwd_Changed_Page(void);
void Draw_Open_Duration_UI(unsigned int elapsed);

/* ==================== 页面绘制 ==================== */
void Draw_Boot_Page(void);
void Draw_Req_Page1(void);
void Draw_Req_Page2(void);
void Draw_Welcome_Page(void);
void Draw_Mode_Page(void);
void Update_Input_UI(u8 cnt);
void Restore_Hint_Line(void);
void Draw_Unlock_Page(void);
void Draw_Locked_Page(void);
void Draw_Verify_Old_Pwd_Page(void);
void Draw_View_Verify_Page(void);
void Draw_Record_Menu_Page(void);
void Draw_Pwd_History_Page(void);
void Draw_Unlock_History_Page(void);
void Draw_Modify_Page(void);
void Draw_Confirm_Page(void);
void Draw_Time_Record_Page(void);
void Draw_Time_Status(void);
void Draw_Time_Sync_Result(u8 success);
void Clear_Time_Sync_Result(void);
#endif /* __APP_UI_H */
