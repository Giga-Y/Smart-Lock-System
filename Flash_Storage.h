#ifndef __FLASH_STORAGE_H
#define __FLASH_STORAGE_H

#include "Hardware_Profile.h"
#include "clock.h"

#define FLASH_START_ADDR 0x1F800          /**< 系统数据保存扇区起始地址 */
#define PWD_HISTORY_MAX  5                /**< 密码历史记录条数 */
#define UNLOCK_HISTORY_MAX 5              /**< 开锁历史记录条数 */

#define SYS_DATA_MAGIC   0xA5             /**< 系统数据有效标志 */
#define SYS_DATA_VERSION 0x5A             /**< 数据布局版本，升级时用于迁移旧数据 */

/**
 * @brief 密码记录
 */
typedef struct {
    u8 pwd[6];      /**< 密码内容，最多 6 位数字 */
    u8 len;         /**< 有效长度（4~6） */
} PwdRecord;

/**
 * @brief 单条开锁记录
 * @note  直接保存开锁时刻的日历时间，便于按 "年-月-日 时:分:秒" 显示。
 */
typedef struct {
    Clock_DateTime time;   /**< 开锁时刻 */
    u16 duration;          /**< 本次开锁持续时长（秒） */
    u8  valid;             /**< 记录有效标志 */
} UnlockRecord;

/**
 * @brief 掉电保存的系统数据
 * @note  data_version 必须是结构体最后一个字段，且结构体总长为偶数，
 *        Flash_SaveData() 按 16 位字写入。
 */
typedef struct {
    u8 magic;                                  /**< 固定为 SYS_DATA_MAGIC */
    PwdRecord current_pwd;                     /**< 当前生效密码 */
    PwdRecord pwd_history[PWD_HISTORY_MAX];    /**< 密码修改历史，索引 0 最新 */
    UnlockRecord unlock_history[UNLOCK_HISTORY_MAX]; /**< 开锁历史，索引 0 最新 */
    u8 data_version;                           /**< 数据布局版本 */
} SystemData;

/* 编译期检查：Flash 按 16 位字写入，结构体长度必须为偶数 */
typedef char SystemData_SizeMustBeEven[(sizeof(SystemData) % 2 == 0) ? 1 : -1];

extern SystemData g_sys_data;

void Flash_LoadData(void);
void Flash_SaveData(void);

/**
 * @brief  追加一条开锁记录
 * @param  time     开锁时刻（非空）
 * @param  duration 开锁持续时长（秒）
 */
void Flash_AddUnlockRecord(const Clock_DateTime *time, u16 duration);

#endif
