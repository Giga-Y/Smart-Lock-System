/**
 * @file    App_State.c
 * @brief   应用层共享状态定义
 * @details 所有跨模块共享的全局变量在此唯一定义，初始值与重构前完全一致。
 */

#include "App_State.h"

/* ==================== 密码输入缓冲 ==================== */
u8 g_pwd_buf[6] = {0};
u8 g_pwd_cnt = 0;

/* ==================== 系统密码 ==================== */
u8 g_system_pwd[6] = "000000";
u8 g_system_pwd_len = 6;

/* ==================== 新密码暂存 ==================== */
u8 g_new_pwd[6] = {0};
u8 g_new_pwd_len = 0;

/* ==================== UI 状态机 ==================== */
App_UiState g_ui_state = APP_STATE_MENU;
App_UiState g_target_state = APP_STATE_MENU;
u8 g_mask_mode = 1;
u8 g_error_count = 0;
u8 g_is_locked = 0;

/* ==================== 计时与开锁记录 ==================== */
volatile unsigned int g_sys_sec = 0;
unsigned int g_open_time = 0;
Clock_DateTime g_open_datetime = {2026, 1, 1, 0, 0, 0};
unsigned int g_last_open_time = 0;
unsigned int g_last_duration = 0;

