/**
 * @file number_utils.h
 * @brief 数值转换工具 (C 版本)
 *
 * 提供字符串与数值之间的类型转换功能。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#ifndef CU_NUMBER_UTILS_H
#define CU_NUMBER_UTILS_H

#include "cu/export.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 字符串转整数
 *
 * @param s 要转换的字符串
 * @param default_value 转换失败时的默认值
 * @return 转换后的整数值
 */
CU_API int to_int(const char* s, int default_value);

/**
 * @brief 字符串转双精度浮点数
 *
 * @param s 要转换的字符串
 * @param default_value 转换失败时的默认值
 * @return 转换后的双精度浮点数
 */
CU_API double to_double(const char* s, double default_value);

/**
 * @brief 字符串转布尔值
 *
 * 支持的 true 值: "true", "yes", "1", "on"（不区分大小写）
 *
 * @param s 要转换的字符串
 * @param default_value 转换失败时的默认值
 * @return 转换后的布尔值
 */
CU_API int to_bool(const char* s, int default_value);

/**
 * @brief 整数转字符串
 *
 * @param value 要转换的整数值
 * @param buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @return 指向 buffer 的指针
 */
CU_API char* int_to_string(int value, char* buffer, size_t buffer_size);

/**
 * @brief 双精度浮点数转字符串
 *
 * @param value 要转换的浮点数值
 * @param decimals 小数位数（-1 表示默认格式）
 * @param buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @return 指向 buffer 的指针
 */
CU_API char* double_to_string(double value, int decimals, char* buffer, size_t buffer_size);

/**
 * @brief 判断字符串是否为有效数字
 *
 * 支持整数和小数格式，支持正负号。
 *
 * @param s 要检查的字符串
 * @return 是有效数字返回1，否则返回0
 */
CU_API int is_number(const char* s);

#ifdef __cplusplus
}
#endif

#endif
