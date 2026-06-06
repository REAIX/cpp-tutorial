/**
 * @file number_utils.c
 * @brief 数值转换工具实现 (C 版本)
 *
 * 实现字符串与数值之间的类型转换功能。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#include "cu/number_utils.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <limits.h>

/**
 * @brief 字符串转整数
 *
 * @param s 要转换的字符串
 * @param default_value 转换失败时的默认值
 * @return 转换后的整数值
 */
int to_int(const char* s, int default_value) {
    if (!s || strlen(s) == 0) {
        return default_value;
    }

    char* endptr;
    long result = strtol(s, &endptr, 10);

    /* 检查是否有有效转换 */
    if (*endptr != '\0' && !isspace((unsigned char)*endptr)) {
        return default_value;
    }

    if (result < INT_MIN || result > INT_MAX) {
        return default_value;
    }

    return (int)result;
}

/**
 * @brief 字符串转双精度浮点数
 *
 * @param s 要转换的字符串
 * @param default_value 转换失败时的默认值
 * @return 转换后的双精度浮点数
 */
double to_double(const char* s, double default_value) {
    if (!s || strlen(s) == 0) {
        return default_value;
    }

    char* endptr;
    double result = strtod(s, &endptr);

    /* 检查是否有有效转换 */
    if (*endptr != '\0' && !isspace((unsigned char)*endptr)) {
        return default_value;
    }

    return result;
}

/**
 * @brief 字符串转布尔值
 *
 * 支持的 true 值: "true", "yes", "1", "on"（不区分大小写）
 * 支持的 false 值: "false", "no", "0", "off"（不区分大小写）
 *
 * @param s 要转换的字符串
 * @param default_value 转换失败时的默认值
 * @return 转换后的布尔值
 */
int to_bool(const char* s, int default_value) {
    if (!s || strlen(s) == 0) {
        return default_value;
    }

    /* 转换为小写便于比较 */
    char lower[32];
    size_t len = strlen(s);
    /* 确保字符串长度+1（包括'\0'）不超过缓冲区大小 */
    if (len + 1 > sizeof(lower)) {
        return default_value;
    }

    for (size_t i = 0; i <= len; i++) {
        lower[i] = tolower((unsigned char)s[i]);
    }

    if (strcmp(lower, "true") == 0 || strcmp(lower, "yes") == 0 ||
        strcmp(lower, "1") == 0 || strcmp(lower, "on") == 0) {
        return 1;
    }

    if (strcmp(lower, "false") == 0 || strcmp(lower, "no") == 0 ||
        strcmp(lower, "0") == 0 || strcmp(lower, "off") == 0) {
        return 0;
    }

    return default_value;
}

/**
 * @brief 整数转字符串
 *
 * @param value 要转换的整数值
 * @param buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @return 指向 buffer 的指针
 */
char* int_to_string(int value, char* buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) {
        return NULL;
    }
    snprintf(buffer, buffer_size, "%d", value);
    return buffer;
}

/**
 * @brief 双精度浮点数转字符串
 *
 * @param value 要转换的浮点数值
 * @param decimals 小数位数（-1 表示默认格式）
 * @param buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @return 指向 buffer 的指针
 */
char* double_to_string(double value, int decimals, char* buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) {
        return NULL;
    }
    if (decimals < 0) decimals = 6;
    snprintf(buffer, buffer_size, "%.*f", decimals, value);
    return buffer;
}

/**
 * @brief 判断字符串是否为有效数字
 *
 * 支持整数和小数格式，支持正负号。
 *
 * @param s 要检查的字符串
 * @return 是有效数字返回1，否则返回0
 */
int is_number(const char* s) {
    if (!s || strlen(s) == 0) {
        return 0;
    }

    int has_digit = 0;
    int has_dot = 0;

    /* 跳过可选的正负号 */
    size_t i = 0;
    if (s[0] == '-' || s[0] == '+') {
        i = 1;
    }

    /* 检查每个字符 */
    for (; s[i] != '\0'; i++) {
        if (isdigit((unsigned char)s[i])) {
            has_digit = 1;
        } else if (s[i] == '.' && !has_dot) {
            has_dot = 1;
        } else {
            return 0;
        }
    }

    return has_digit;
}