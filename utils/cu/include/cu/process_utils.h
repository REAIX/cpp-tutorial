/**
 * @file process_utils.h
 * @brief 进程工具 (C 版本)
 *
 * 提供延时、命令执行、进程信息获取等功能，支持跨平台。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#ifndef CU_PROCESS_UTILS_H
#define CU_PROCESS_UTILS_H

#include "cu/export.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 阻塞式延时
 *
 * 使当前线程阻塞指定的毫秒数。
 *
 * @param milliseconds 延时的毫秒数
 */
CU_API void cu_sleep(unsigned int milliseconds);

/**
 * @brief 执行系统命令
 *
 * 执行指定的系统命令并捕获输出。
 *
 * @param command 要执行的命令字符串
 * @param output 输出缓冲区
 * @param output_size 输出缓冲区大小
 * @return 命令退出码，失败返回-1
 */
CU_API int execute_command(const char* command, char* output, size_t output_size);

/**
 * @brief 获取当前进程 ID
 *
 * @return 当前进程的 ID
 */
CU_API int get_process_id(void);

/**
 * @brief 获取环境变量值
 *
 * @param name 环境变量名
 * @param buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @return 指向 buffer 的指针，失败返回 NULL
 */
CU_API char* get_environment_variable(const char* name, char* buffer, size_t buffer_size);

/**
 * @brief 设置环境变量
 *
 * @param name 环境变量名
 * @param value 环境变量值（为 NULL 则删除）
 * @return 成功返回1，失败返回0
 */
CU_API int set_environment_variable(const char* name, const char* value);

#ifdef __cplusplus
}
#endif

#endif
