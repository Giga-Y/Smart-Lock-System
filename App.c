/**
 * @file    App.c
 * @brief   应用层：系统初始化、主循环调度与密码/时间同步业务
 */

#include <msp430.h>
#include <string.h>

#include "Hardware_Profile.h"
#include "App.h"
#include "App_State.h"
#include "App_UI.h"
#include "clock.h"
#include "UART.h"
#include "USB_Comm.h"
#include "MatrixKey.h"
#include "TM1638.h"
#include "Flash_Storage.h"
#include "Speaker.h"
#include "Buzzer.h"
#include "Touch_Pad.h"
#include "LCD_TFT_ILI9325.h"

/* ==================== 板载三色灯快捷操作 ==================== */
#define MY_LED_All_Off()   P4OUT &= ~(BIT4 | BIT5 | BIT6)
#define MY_LED_RED_ON()    P4OUT = (P4OUT & ~(BIT5|BIT6)) | BIT4
#define MY_LED_GREEN_ON()  P4OUT = (P4OUT & ~(BIT4|BIT6)) | BIT5
#define MY_LED_YELLOW_ON() P4OUT = (P4OUT & ~(BIT4|BIT5)) | BIT6

/* ==================== 主循环上下文 ==================== */
static u8  s_last_key1_state = 1;      
static u8  s_last_key2_state = 1;      
static u16 s_ui_timer = 0;             
static u8  s_ui_visible = 0;           
static u8  s_last_pwd_cnt = 0;         
static unsigned int s_last_refresh = 0;
static u8  s_touch_cnt = 0;            
static u16 s_last_time_sec = 0xFFFF;   



/* ==================== 时间同步状态 ==================== */
#define TIME_SYNC_TIMEOUT_SEC       5   
#define TIME_SYNC_RESULT_HIDE_SEC   2   

static u8  g_time_sync_req = 0;        
static u16 g_sync_send_timer = 0;      
static u16 s_sync_start_sec = 0;       
static u8  s_sync_result_visible = 0;  
static u16 s_sync_result_shown_sec = 0;

/* ==================== 内部函数声明 ==================== */
static void App_HandleLocked(void);
static void App_HandleKey(void);
static void App_HandlePassword(void);
static void App_HandleOpenTimer(void);
static void App_HandleVolume(void);
static void App_HandleTimeRefresh(void);
static void App_HandleTimeSync(void);
static void App_HandleCommStatus(void);

static void App_StartTimeSync(void);
static void App_ShowTimeSyncResult(u8 success);

static void App_ReturnToMenu(void);
static void App_ResetPwdInput(void);
static u8   App_PasswordMatches(const u8 *expected, u8 len);
static void App_TryUnlock(void);
static void App_TryVerifyOld(void);
static void App_TryViewRecords(void);
static void App_TrySetNewPwd(void);
static void App_TryConfirmNewPwd(void);

static void Init_Timer_A0(void) {
    TA0CTL = TASSEL__SMCLK | MC__UP | TACLR; 
    TA0CCR0 = 12000; 
    TA0CCTL0 = CCIE; 
}

/* ==================== 中断 ==================== */
#pragma vector = USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void) {
    switch(__even_in_range(UCA1IV, 4)) {
        case 2: 
        {
            u8 rx = UCA1RXBUF;
            u8 is_link = UART_Link_Parse(rx);
            u8 is_time_sync = is_link ? 0 : UART_TimeSync_Parse(rx);

            /* 链路命令/时间同步帧中的数字不能再进入密码框 */
            if (!is_link && !is_time_sync &&
                APP_STATE_IS_PASSWORD_INPUT(g_ui_state) &&
                g_is_locked == 0 && rx >= '0' && rx <= '9') {
                if (g_pwd_cnt < 6) {
                    g_pwd_buf[g_pwd_cnt++] = rx; 
                }
            }
            break;
        }
        default: break;
    }
}
 /* 该中断用于实时时间更新*/
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0_ISR(void) {
    static u16 ms_count = 0;
    ms_count++;
    if (ms_count >= 1000) { 
        ms_count = 0; 
        g_sys_sec++; 
        Clock_TimeTick1s(); 
    }
}

void App_Init(void)
{
    u8 boot_stage = 0;
    u8 key_val = 0;

    WDTCTL = WDTPW | WDTHOLD; 
    Clock_Init(); 
    Init_Timer_A0(); 
    Clock_TimeInit(); 
    
    P4DIR |= BIT4 | BIT5 | BIT6; 
    MY_LED_All_Off(); 
    P4DIR &= ~(BIT3 | BIT2); 
    P4REN |= (BIT3 | BIT2);  
    P4OUT |= (BIT3 | BIT2);  
    
    Beep_Init();      
    Speaker_Init();   
    Init_LCD_TFT_ILI9325(); 
    Initial_RS232_UART();   
    Switch_UART_Channel(1); 
    Flash_LoadData(); 
    
    Open_Display_TM1638(); 
    Initial_Touch_Pad();

    Draw_Boot_Page(); 
    __delay_cycles(18000000); 
    __enable_interrupt(); 

    USB_Comm_Init();
    
    while(boot_stage < 3) {
        Scan_MatrixKey();
        key_val = Detect_MatrixKey();
        if (key_val != M_NullMatrixKey && key_val != 0xFF) {
            __delay_cycles(40000); 
            if(!g_bgm_on) Beep_Short();
            if (boot_stage == 0) { boot_stage = 1; Draw_Req_Page1(); } 
            else if (boot_stage == 1) { boot_stage = 2; Draw_Req_Page2(); } 
            else if (boot_stage == 2) { boot_stage = 3; }
            while (key_val != M_NullMatrixKey && key_val != 0xFF) {
                __delay_cycles(10000);
                Scan_MatrixKey();
                key_val = Detect_MatrixKey();
            }
        }
        __delay_cycles(10000);
    }
    
    TM1638_Update_Display(0, 0);
    Draw_Welcome_Page();
}

void App_Run(void)
{
    USB_Comm_Poll();

    if (g_is_locked) {
        App_HandleLocked();
    }

    App_HandleCommStatus();
    App_HandleKey();
    App_HandlePassword();
    App_HandleOpenTimer();
    App_HandleVolume();
    App_HandleTimeRefresh();
    App_HandleTimeSync();

    __delay_cycles(10000); 
}

static void App_HandleLocked(void)
{
    static u16 locked_last_sec = 0xFFFF;
    P4DIR |= BEEP_PIN;
    P4OUT |= BEEP_PIN;
    while (1) {
        if (g_sys_sec != locked_last_sec) {
            locked_last_sec = g_sys_sec;
            Draw_Time_Status();
        }
        __delay_cycles(10000);
    }
}

/**
 * @brief 处理上位机自动探测与端口命令
 * @note  仅保留 PING/PONG；PORT:... 命令仍然解析并丢弃，
 *        避免其数字进入密码框，但不再驱动任何界面或状态。
 */
static void App_HandleCommStatus(void)
{
    u16 gie_state;

    if (g_ping_pending) {
        gie_state = __get_SR_register() & GIE;
        __disable_interrupt();
        g_ping_pending = 0;
        if (gie_state) __enable_interrupt();
        Send_String_UART_A((u8*)"PONG\n");
    }

    if (g_comm_cmd_pending) {
        gie_state = __get_SR_register() & GIE;
        __disable_interrupt();
        g_comm_cmd_pending = 0;
        if (gie_state) __enable_interrupt();
    }
}

static void App_HandleKey(void)
{
    u8 key_val;

    Scan_MatrixKey();
    key_val = Detect_MatrixKey();
    if (key_val == M_NullMatrixKey || key_val == 0xFF) return;

    __delay_cycles(40000);

    /* C 键：COM4(RS232) 与 COM7(XDS/USB) 均允许同步时间 */
    if (key_val == MatrixKey_C) {
        if (!g_bgm_on) Beep_Short();
        App_StartTimeSync();
        return;
    }

    if (key_val == MatrixKey_Equal) {
        if (g_is_locked) return;
        if (!g_bgm_on) Beep_Short();
        if (!g_bgm_on) {
            G_SliderPosition = g_volume_level;
            Draw_Voice_UI(g_volume_level);
            TM1638_Update_Display(1, g_volume_level * 25);
            s_ui_timer = 300;
            s_ui_visible = 1;
        } else {
            Clear_Voice_UI();
            TM1638_Update_Display(0, 0);
            s_ui_timer = 0;
            s_ui_visible = 0;
        }
        Speaker_Play_BGM(!g_bgm_on);
        return;
    }

    if (!g_bgm_on) Beep_Short();
    if (g_is_locked && key_val != MatrixKey_AC) return;

    switch (g_ui_state) {
        case APP_STATE_MENU:
            if (key_val == MatrixKey_1) {
                g_target_state = APP_STATE_UNLOCK;
                g_ui_state = APP_STATE_MODE_SELECT;
                Draw_Mode_Page();
            } else if (key_val == MatrixKey_2) {
                g_target_state = APP_STATE_VERIFY_OLD;
                g_ui_state = APP_STATE_MODE_SELECT;
                Draw_Mode_Page();
            } else if (key_val == MatrixKey_3) {
                g_target_state = APP_STATE_VERIFY_VIEW;
                g_ui_state = APP_STATE_MODE_SELECT;
                Draw_Mode_Page();
            }
            break;

        case APP_STATE_MODE_SELECT:
            if (key_val == MatrixKey_1 || key_val == MatrixKey_2) {
                g_mask_mode = (key_val == MatrixKey_2) ? 1 : 0;
                g_ui_state = g_target_state;

                if (g_ui_state == APP_STATE_UNLOCK) Draw_Unlock_Page();
                else if (g_ui_state == APP_STATE_VERIFY_OLD) Draw_Verify_Old_Pwd_Page();
                else if (g_ui_state == APP_STATE_VERIFY_VIEW) Draw_View_Verify_Page();
                else if (g_ui_state == APP_STATE_MODIFY_NEW) Draw_Modify_Page();
            } else if (key_val == MatrixKey_AC) {
                App_ReturnToMenu();
            }
            break;

        case APP_STATE_RECORD_MENU:
            if (key_val == MatrixKey_1) {
                g_ui_state = APP_STATE_PWD_HISTORY;
                Draw_Pwd_History_Page();
            } else if (key_val == MatrixKey_2) {
                g_ui_state = APP_STATE_UNLOCK_HISTORY;
                Draw_Unlock_History_Page();
            } else if (key_val == MatrixKey_AC) {
                MY_LED_All_Off();
                App_ReturnToMenu();
            }
            break;

        case APP_STATE_PWD_HISTORY:
        case APP_STATE_UNLOCK_HISTORY:
            if (key_val == MatrixKey_AC) {
                g_ui_state = APP_STATE_RECORD_MENU;
                Draw_Record_Menu_Page();
            }
            break;

        default:
            if (key_val == MatrixKey_AC) {
                if (g_ui_state == APP_STATE_UNLOCK_SUCCESS) {
                    g_last_open_time = g_open_time;
                    g_last_duration = g_sys_sec - g_open_time;
                    if (g_last_duration == 0) g_last_duration = 1;
                    Flash_AddUnlockRecord(&g_open_datetime, g_last_duration);
                }
                MY_LED_All_Off();
                App_ReturnToMenu();
            }
            break;
    }
}

/* ==================== 密码输入与校验 ==================== */
/**
 * @brief 处理密码输入页的退格、回显与确认键
 */
static void App_HandlePassword(void)
{
    u8 current_key1_state;
    u8 current_key2_state;

    if (!APP_STATE_IS_PASSWORD_INPUT(g_ui_state)) return;

    /* P4.2：退格 */
    current_key1_state = (P4IN & BIT2) ? 1 : 0;
    if (s_last_key1_state == 1 && current_key1_state == 0) {
        __delay_cycles(40000);
        if (!(P4IN & BIT2)) {
            if (!g_bgm_on) Beep_Short();
            if (g_pwd_cnt > 0 && g_is_locked == 0) {
                g_pwd_cnt--;
                g_pwd_buf[g_pwd_cnt] = 0;
                Restore_Hint_Line();
                MY_LED_All_Off();
                Update_Input_UI(g_pwd_cnt);
                s_last_pwd_cnt = g_pwd_cnt;
            }
        }
    }
    s_last_key1_state = current_key1_state;

    /* 数字键/UART 输入后刷新回显 */
    if (g_pwd_cnt != s_last_pwd_cnt) {
        if (s_last_pwd_cnt == 0 && g_pwd_cnt >= 1) {
            Restore_Hint_Line();
            MY_LED_All_Off();
        }
        s_last_pwd_cnt = g_pwd_cnt;
        Update_Input_UI(g_pwd_cnt);
    }

    /* P4.3：确认 */
    current_key2_state = (P4IN & BIT3) ? 1 : 0;
    if (s_last_key2_state == 1 && current_key2_state == 0) {
        __delay_cycles(40000);
        if (!(P4IN & BIT3)) {
            if (!g_bgm_on) Beep_Short();
            if (g_pwd_cnt > 0 && g_is_locked == 0) {
                switch (g_ui_state) {
                    case APP_STATE_UNLOCK:          App_TryUnlock(); break;
                    case APP_STATE_VERIFY_OLD:      App_TryVerifyOld(); break;
                    case APP_STATE_VERIFY_VIEW:     App_TryViewRecords(); break;
                    case APP_STATE_MODIFY_NEW:      App_TrySetNewPwd(); break;
                    case APP_STATE_MODIFY_CONFIRM:  App_TryConfirmNewPwd(); break;
                    default: break;
                }
            }
        }
    }
    s_last_key2_state = current_key2_state;
}

/**
 * @brief 返回主菜单并关闭指示灯
 */
static void App_ReturnToMenu(void)
{
    g_ui_state = APP_STATE_MENU;
    Draw_Welcome_Page();
}

/**
 * @brief 清空当前密码输入
 */
static void App_ResetPwdInput(void)
{
    g_pwd_cnt = 0;
    memset(g_pwd_buf, 0, sizeof(g_pwd_buf));
    Update_Input_UI(g_pwd_cnt);
    s_last_pwd_cnt = 0;
}

/**
 * @brief 比较当前输入与期望密码
 * @param expected 期望密码
 * @param len      期望长度
 * @return 1 = 匹配；0 = 不匹配
 */
static u8 App_PasswordMatches(const u8 *expected, u8 len)
{
    return (g_pwd_cnt == len &&
            strncmp((char *)g_pwd_buf, (char *)expected, len) == 0) ? 1 : 0;
}

/**
 * @brief 开锁密码校验与错误计数处理
 */
static void App_TryUnlock(void)
{
    if (App_PasswordMatches(g_system_pwd, g_system_pwd_len)) {
        g_error_count = 0;
        MY_LED_GREEN_ON();
        g_open_time = g_sys_sec;
        g_ui_state = APP_STATE_UNLOCK_SUCCESS;
        Clock_GetDateTime(&g_open_datetime);
        Speaker_Play_Success();
        Draw_Time_Record_Page();
        Send_String_UART_A((u8*)"SUCCESS!\r\n");
        s_last_pwd_cnt = 0;
        return;
    }

    g_error_count++;
    if (g_error_count >= 3) {
        g_is_locked = 1;
        MY_LED_RED_ON();

        g_bgm_on = 0;
        Speaker_Play_BGM(0);
        P5OUT |= BIT6;
        TM1638_Update_Display(0, 0);
        Clear_Voice_UI();

        Draw_Locked_Page();
        Send_String_UART_A((u8*)"LOCKED!\r\n");

        Speaker_Play_Locked();
        while (g_voice_on);
        return;
    }

    MY_LED_YELLOW_ON();
    Draw_Password_Error_UI(PWD_ERROR_WRONG, 3 - g_error_count);
    Send_String_UART_A((u8*)"WRONG!\r\n");
    App_ResetPwdInput();
    Speaker_Play_Error();
}

/**
 * @brief 修改密码前验证旧密码
 */
static void App_TryVerifyOld(void)
{
    if (App_PasswordMatches(g_system_pwd, g_system_pwd_len)) {
        MY_LED_GREEN_ON();
        g_ui_state = APP_STATE_MODIFY_NEW;
        Draw_Modify_Page();
        s_last_pwd_cnt = 0;
        return;
    }

    MY_LED_YELLOW_ON();
    Draw_Password_Error_UI(PWD_ERROR_OLD, 0);
    Send_String_UART_A((u8*)"OLD WRONG!\r\n");
    App_ResetPwdInput();
    Speaker_Play_Error();
}

/**
 * @brief 查看记录前验证密码
 */
static void App_TryViewRecords(void)
{
    if (App_PasswordMatches(g_system_pwd, g_system_pwd_len)) {
        MY_LED_GREEN_ON();
        g_ui_state = APP_STATE_RECORD_MENU;
        Draw_Record_Menu_Page();
        s_last_pwd_cnt = 0;
        return;
    }

    MY_LED_YELLOW_ON();
    Draw_Password_Error_UI(PWD_ERROR_VIEW, 0);
    Send_String_UART_A((u8*)"VIEW PWD WRONG!\r\n");
    App_ResetPwdInput();
    Speaker_Play_Error();
}

/**
 * @brief 校验并暂存新密码
 */
static void App_TrySetNewPwd(void)
{
    if (g_pwd_cnt >= 4 && g_pwd_cnt <= 6) {
        memcpy(g_new_pwd, g_pwd_buf, 6);
        g_new_pwd_len = g_pwd_cnt;
        g_ui_state = APP_STATE_MODIFY_CONFIRM;
        Draw_Confirm_Page();
        s_last_pwd_cnt = 0;
        return;
    }

    MY_LED_YELLOW_ON();
    Draw_Password_Error_UI(PWD_ERROR_LENGTH, 0);
    App_ResetPwdInput();
    Speaker_Play_Error();
}

/**
 * @brief 确认新密码并写入 Flash
 */
static void App_TryConfirmNewPwd(void)
{
    u8 i;

    if (!App_PasswordMatches(g_new_pwd, g_new_pwd_len)) {
        MY_LED_YELLOW_ON();
        Draw_Password_Error_UI(PWD_ERROR_MISMATCH, 0);
        Send_String_UART_A((u8*)"MISMATCH!\r\n");
        __delay_cycles(5000000);
        MY_LED_All_Off();
        g_ui_state = APP_STATE_MODIFY_NEW;
        Draw_Modify_Page();
        s_last_pwd_cnt = 0;
        Speaker_Play_Error();
        return;
    }

    memcpy(g_system_pwd, g_new_pwd, 6);
    g_system_pwd_len = g_new_pwd_len;
    memcpy(g_sys_data.current_pwd.pwd, g_new_pwd, 6);
    g_sys_data.current_pwd.len = g_new_pwd_len;

    for (i = PWD_HISTORY_MAX - 1; i > 0; i--) {
        g_sys_data.pwd_history[i] = g_sys_data.pwd_history[i - 1];
    }
    memcpy(g_sys_data.pwd_history[0].pwd, g_new_pwd, 6);
    g_sys_data.pwd_history[0].len = g_new_pwd_len;
    Flash_SaveData();

    MY_LED_GREEN_ON();
    Draw_Pwd_Changed_Page();
    Send_String_UART_A((u8*)"PWD CHANGED!\r\n");
    __delay_cycles(15000000);
    MY_LED_All_Off();
    App_ReturnToMenu();
}

/* ==================== 开锁计时页 ==================== */
/**
 * @brief 每秒刷新开锁成功页的持续时长
 */
static void App_HandleOpenTimer(void)
{
    if (g_ui_state != APP_STATE_UNLOCK_SUCCESS) return;
    if (g_sys_sec == s_last_refresh) return;

    s_last_refresh = g_sys_sec;
    Draw_Open_Duration_UI(g_sys_sec - g_open_time);
}

/* ==================== 触摸音量 ==================== */
/**
 * @brief 处理触摸滑条音量与音量浮层超时
 */
static void App_HandleVolume(void)
{
    if (g_bgm_on) {
        u8 new_vol;

        s_touch_cnt++;
        if (s_touch_cnt >= 5) {
            s_touch_cnt = 0;
            Deal_Touch_Pad();
            new_vol = G_SliderPosition;
            if (new_vol > 4) new_vol = 4;
            if (new_vol != g_volume_level) {
                g_volume_level = new_vol;
                Draw_Voice_UI(g_volume_level);
                TM1638_Update_Display(1, g_volume_level * 25);
                s_ui_timer = 300;
                s_ui_visible = 1;
            }
        }
        if (s_ui_timer > 0) {
            s_ui_timer--;
            if (s_ui_timer == 0 && s_ui_visible) {
                Clear_Voice_UI();
                s_ui_visible = 0;
            }
        }
    } else {
        s_ui_timer = 0;
        if (s_ui_visible) {
            Clear_Voice_UI();
            s_ui_visible = 0;
        }
    }
}

/* ==================== 时间显示与同步 ==================== */
/**
 * @brief 每秒刷新右上角时间
 */
static void App_HandleTimeRefresh(void)
{
    if (g_sys_sec == s_last_time_sec) return;

    s_last_time_sec = g_sys_sec;
    Draw_Time_Status();
}

/**
 * @brief 启动一次时间同步请求
 */
static void App_StartTimeSync(void)
{
    Clear_Time_Sync_Result();
    s_sync_result_visible = 0;
    g_time_sync_req = 1;
    g_sync_send_timer = 999;     /* 下一次循环立即发送 */
    s_sync_start_sec = g_sys_sec;
}

/**
 * @brief 显示时间同步结果，并启动 2 秒自动隐藏计时
 * @param success 1 = 成功；0 = 失败
 */
static void App_ShowTimeSyncResult(u8 success)
{
    Draw_Time_Sync_Result(success);
    s_sync_result_shown_sec = g_sys_sec;
    s_sync_result_visible = 1;
}

/**
 * @brief 处理时间同步完成、请求超时与结果提示自动隐藏
 */
static void App_HandleTimeSync(void)
{
    if (g_time_sync_ready) {
        u8 sync_ok;

        g_time_sync_ready = 0;
        g_time_sync_req = 0;

        sync_ok = Clock_TimeSet(g_sync_time[0], g_sync_time[1], g_sync_time[2],
                                g_sync_time[3], g_sync_time[4], g_sync_time[5]);

        Draw_Time_Status();
        s_last_time_sec = g_sys_sec;
        App_ShowTimeSyncResult(sync_ok);

        Send_String_UART_A((unsigned char *)(sync_ok ? "TIME_OK\n" : "TIME_FAIL\n"));
    }

    if (g_time_sync_req) {
        if ((u16)(g_sys_sec - s_sync_start_sec) >= TIME_SYNC_TIMEOUT_SEC) {
            /* 超时未收到有效时间：停止请求，红色提示失败 */
            g_time_sync_req = 0;
            App_ShowTimeSyncResult(0);
            Send_String_UART_A((unsigned char *)"TIME_FAIL\n");
        } else {
            g_sync_send_timer++;
            if (g_sync_send_timer >= 200) { /* 每 200ms 发送一次 */
                g_sync_send_timer = 0;
                Send_String_UART_A((unsigned char *)"REQ_TIME\n");
            }
        }
    }

    /* 时间同步结果只保留 2 秒，避免长期占用屏幕 */
    if (s_sync_result_visible &&
        (u16)(g_sys_sec - s_sync_result_shown_sec) >= TIME_SYNC_RESULT_HIDE_SEC) {
        Clear_Time_Sync_Result();
        s_sync_result_visible = 0;
    }
}
