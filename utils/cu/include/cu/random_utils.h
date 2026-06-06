/**
 * @file random_utils.h
 * @brief 随机工具 (C 版本)
 *
 * 提供 UUID 生成、随机字符串、随机数生成等功能。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#ifndef CU_RANDOM_UTILS_H
#define CU_RANDOM_UTILS_H

#include "cu/export.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化随机数生成器
 *
 * 使用当前时间作为种子。
 */
CU_API void random_init(void);

/**
 * @brief 生成 UUID 字符串
 *
 * 生成符合 UUID v4 规范的唯一标识符。
 * 格式: "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx"
 *
 * @param buffer 输出缓冲区（至少 37 字节）
 * @param buffer_size 缓冲区大小
 * @return 指向 buffer 的指针
 */
CU_API char* generate_uuid(char* buffer, size_t buffer_size);

/**
 * @brief 生成随机字符串
 *
 * 生成包含大小写字母和数字的随机字符串。
 *
 * @param buffer 输出缓冲区
 * @param length 字符串长度
 * @return 指向 buffer 的指针
 */
CU_API char* random_string(char* buffer, size_t length);

/**
 * @brief 生成指定范围内的随机整数
 *
 * @param min_value 最小值（包含）
 * @param max_value 最大值（包含）
 * @return 范围内的随机整数
 */
CU_API int random_int(int min_value, int max_value);

/**
 * @brief 生成指定范围内的随机浮点数
 *
 * @param min_value 最小值
 * @param max_value 最大值
 * @return 范围内的随机浮点数
 */
CU_API double random_double(double min_value, double max_value);

#ifdef __cplusplus
}
#endif

#endif
