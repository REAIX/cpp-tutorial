/**
 * @file random_utils.c
 * @brief 随机工具实现 (C 版本)
 *
 * 实现 UUID 生成、随机字符串、随机数生成等功能。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#include "cu/random_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif

/**
 * @brief 初始化随机数生成器
 *
 * 使用当前时间作为种子。
 */
void random_init(void) {
    srand((unsigned int)time(NULL));
}

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
char* generate_uuid(char* buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 37) {
        return NULL;
    }

    static const char hex[] = "0123456789abcdef";

    /* 生成标准 UUID 格式 */
    for (int i = 0; i < 36; i++) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            buffer[i] = '-';
        } else {
            buffer[i] = hex[rand() % 16];
        }
    }

    /* 设置版本号 4 和变体 */
    buffer[8] = '-';
    buffer[13] = '-';
    buffer[14] = '4';  /* 版本4 */
    buffer[18] = hex[(rand() % 4) + 8];  /* 变体高位 */
    buffer[23] = '-';
    buffer[36] = '\0';

    return buffer;
}

/**
 * @brief 生成随机字符串
 *
 * 生成包含大小写字母和数字的随机字符串。
 *
 * @param buffer 输出缓冲区
 * @param length 缓冲区长度（生成 length-1 个字符，末尾补 '\0'）
 * @return 指向 buffer 的指针
 */
char* random_string(char* buffer, size_t length) {
    if (!buffer || length == 0) {
        return NULL;
    }

    static const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    for (size_t i = 0; i < length - 1; i++) {
        buffer[i] = chars[rand() % (sizeof(chars) - 1)];
    }
    buffer[length - 1] = '\0';

    return buffer;
}

/**
 * @brief 生成指定范围内的随机整数
 *
 * @param min_value 最小值（包含）
 * @param max_value 最大值（包含）
 * @return 范围内的随机整数
 */
int random_int(int min_value, int max_value) {
    if (min_value >= max_value) {
        return min_value;
    }
    return min_value + rand() % (max_value - min_value + 1);
}

/**
 * @brief 生成指定范围内的随机浮点数
 *
 * @param min_value 最小值
 * @param max_value 最大值
 * @return 范围内的随机浮点数
 */
double random_double(double min_value, double max_value) {
    if (min_value >= max_value) {
        return min_value;
    }
    double scale = (double)rand() / RAND_MAX;
    return min_value + scale * (max_value - min_value);
}