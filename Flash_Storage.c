/**
 * @file    Flash_Storage.c
 * @brief   系统参数与开锁记录的掉电存储
 * @details 使用 FLASH_START_ADDR 起始的一个 Flash 段保存 SystemData。
 *          写入前整段擦除，再按 16 位字依次写入；读取时通过 magic 与
 *          data_version 判断数据是否有效，并兼容旧版本布局。
 */

#include "Flash_Storage.h"
#include <string.h>

/* 引入当前生效的密码缓存（由 App_State.c 定义） */
extern u8 g_system_pwd[6];
extern u8 g_system_pwd_len;

SystemData g_sys_data;

/**
 * @brief 将系统数据恢复为出厂默认值
 * @note  只修改 RAM 中的 g_sys_data，不写 Flash。
 */
static void Flash_SetDefaults(void)
{
    memset(&g_sys_data, 0, sizeof(g_sys_data));

    g_sys_data.magic = SYS_DATA_MAGIC;
    g_sys_data.data_version = SYS_DATA_VERSION;

    memcpy(g_sys_data.current_pwd.pwd, "000000", 6);
    g_sys_data.current_pwd.len = 6;

    memcpy(g_sys_data.pwd_history[0].pwd, "000000", 6);
    g_sys_data.pwd_history[0].len = 6;
}

/**
 * @brief 从 Flash 加载系统数据
 * @details 三种情况：
 *          - magic 无效：视为首次上电，写入默认值；
 *          - magic 有效但 data_version 不匹配：旧版布局，保留密码及
 *            密码历史，开锁记录因格式变化而清空，然后按新布局重写；
 *          - 版本一致：直接复制到 RAM。
 */
void Flash_LoadData(void)
{
    const SystemData *flash_ptr = (const SystemData *)(uintptr_t)FLASH_START_ADDR;

    if (flash_ptr->magic != SYS_DATA_MAGIC) {
        Flash_SetDefaults();
        Flash_SaveData();
    } else if (flash_ptr->data_version != SYS_DATA_VERSION) {
        Flash_SetDefaults();
        memcpy(g_sys_data.current_pwd.pwd, flash_ptr->current_pwd.pwd,
               sizeof(g_sys_data.current_pwd.pwd));
        g_sys_data.current_pwd.len = flash_ptr->current_pwd.len;
        memcpy(g_sys_data.pwd_history, flash_ptr->pwd_history,
               sizeof(g_sys_data.pwd_history));
        Flash_SaveData();
    } else {
        memcpy(&g_sys_data, flash_ptr, sizeof(g_sys_data));
    }

    memcpy(g_system_pwd, g_sys_data.current_pwd.pwd, 6);
    g_system_pwd_len = g_sys_data.current_pwd.len;
}

/**
 * @brief 将 g_sys_data 整体写入 Flash
 * @note  先擦除整个数据段，再按 16 位字写入；写入期间关闭中断。
 */
void Flash_SaveData(void)
{
    u16 *flash_ptr = (u16 *)(uintptr_t)FLASH_START_ADDR;
    u16 *ram_ptr = (u16 *)&g_sys_data;
    u16 i;
    u16 size_in_words = sizeof(SystemData) / 2;
    u16 gie_state = __get_SR_register() & GIE;

    __disable_interrupt();

    FCTL3 = FWKEY;
    FCTL1 = FWKEY + ERASE;
    *(u8 *)flash_ptr = 0; /* 写 0 触发段擦除 */
    while (FCTL3 & BUSY);

    FCTL1 = FWKEY + WRT;
    for (i = 0; i < size_in_words; i++) {
        flash_ptr[i] = ram_ptr[i];
    }
    while (FCTL3 & BUSY);

    FCTL1 = FWKEY;
    FCTL3 = FWKEY + LOCK;

    /* 只恢复调用前的全局中断状态，避免在初始化阶段提前开中断 */
    if (gie_state) __enable_interrupt();
}

/**
 * @brief 追加一条开锁记录
 * @details 记录按时间倒序保存：索引 0 为最新，旧记录依次后移。
 */
void Flash_AddUnlockRecord(const Clock_DateTime *time, u16 duration)
{
    u8 i;

    if (time == 0) return;

    for (i = UNLOCK_HISTORY_MAX - 1; i > 0; i--) {
        g_sys_data.unlock_history[i] = g_sys_data.unlock_history[i - 1];
    }

    g_sys_data.unlock_history[0].time = *time;
    g_sys_data.unlock_history[0].duration = duration;
    g_sys_data.unlock_history[0].valid = 1;

    Flash_SaveData();
}
