/**
 * @file    App_State.h
 * @brief   应用层共享状态声明
 * @details 集中声明密码输入缓冲、UI 状态机与计时相关的全局变量。
 *          变量定义位于 App_State.c，UI 层与应用层通过本头文件访问。
 */

#ifndef __APP_STATE_H
#define __APP_STATE_H

#include "Hardware_Profile.h"
#include "clock.h"

/* ==================== 密码输入缓冲 ==================== */
extern u8 g_pwd_buf[6];        /**< 当前输入的密码缓冲区（最多 6 位） */
extern u8 g_pwd_cnt;           /**< 当前已输入的密码位数 */

/* ==================== 系统密码 ==================== */
extern u8 g_system_pwd[6];     /**< 当前生效的系统密码 */
extern u8 g_system_pwd_len;    /**< 系统密码有效长度（4~6 位） */

/* ==================== 新密码暂存 ==================== */
extern u8 g_new_pwd[6];        /**< 修改密码时暂存的新密码 */
extern u8 g_new_pwd_len;       /**< 新密码有效长度 */

/* ==================== UI 状态机 ==================== */
/**
 * @brief 界面状态编号
 * @note  数值与原状态机保持一致，新增状态时只在此处扩展。
 */
typedef enum {
    APP_STATE_MENU = 0,            /**< 主菜单 */
    APP_STATE_MODE_SELECT = 1,     /**< 密码显示模式选择 */
    APP_STATE_UNLOCK = 2,          /**< 开锁密码输入 */
    APP_STATE_MODIFY_NEW = 3,      /**< 输入新密码 */
    APP_STATE_MODIFY_CONFIRM = 4,  /**< 确认新密码 */
    APP_STATE_UNLOCK_SUCCESS = 5,  /**< 开锁成功/计时 */
    APP_STATE_VERIFY_OLD = 7,      /**< 修改密码前验证旧密码 */
    APP_STATE_VERIFY_VIEW = 8,     /**< 查看记录前验证密码 */
    APP_STATE_RECORD_MENU = 10,    /**< 记录查询菜单 */
    APP_STATE_PWD_HISTORY = 11,    /**< 密码修改历史 */
    APP_STATE_UNLOCK_HISTORY = 12  /**< 开锁历史 */
} App_UiState;

/** 当前状态是否处于密码输入页 */
#define APP_STATE_IS_PASSWORD_INPUT(s) \
    ((s) == APP_STATE_UNLOCK || (s) == APP_STATE_MODIFY_NEW || \
     (s) == APP_STATE_MODIFY_CONFIRM || (s) == APP_STATE_VERIFY_OLD || \
     (s) == APP_STATE_VERIFY_VIEW)

extern App_UiState g_ui_state;      /**< 当前界面状态 */
extern App_UiState g_target_state;  /**< 选择通信端口后要进入的目标状态 */
extern u8 g_mask_mode;              /**< 密码显示模式：1 掩码显示，0 明文显示 */
extern u8 g_error_count;            /**< 连续输错次数 */
extern u8 g_is_locked;              /**< 系统锁定标志：1 已锁定 */

/* ==================== 计时与开锁记录 ==================== */
extern volatile unsigned int g_sys_sec;      /**< 系统秒计数（1 秒中断累加） */
extern unsigned int g_open_time;             /**< 本次开锁时刻（系统秒） */
extern Clock_DateTime g_open_datetime;       /**< 本次开锁时刻（日历时间） */
extern unsigned int g_last_open_time;        /**< 上次开锁时刻（系统秒） */
extern unsigned int g_last_duration;         /**< 上次开锁持续时长（秒） */

#endif /* __APP_STATE_H */
