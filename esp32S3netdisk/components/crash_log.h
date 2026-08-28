#ifndef CRASH_LOG_H
#define CRASH_LOG_H

#include <stdbool.h>

/**
 * @brief 注册崩溃捕获（shutdown handler）
 *        应在 app_main 早期调用，为本次运行可能的崩溃做准备
 */
void crash_log_init(void);

/**
 * @brief 检查并保存上次的崩溃日志到 SD 卡"崩溃日志.txt"
 *        应在 SD 卡挂载成功后调用
 */
void crash_log_check_save(void);

#endif // CRASH_LOG_H
