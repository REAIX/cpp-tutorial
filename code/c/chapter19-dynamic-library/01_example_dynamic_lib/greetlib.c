/**
 * @file greetlib.c
 * @brief 动态库示例 - 问候库实现
 * @description 对应文档: 19-dynamic-library
 *              实现问候库的所有函数，此文件将被编译为动态库 libgreetlib.so
 *
 * 编译为动态库的步骤:
 *   1. gcc -fPIC -c greetlib.c -o greetlib.o    # 编译为位置无关代码
 *   2. gcc -shared -o libgreetlib.so greetlib.o   # 创建共享库
 *
 * 或一步完成:
 *   gcc -fPIC -shared -o libgreetlib.so greetlib.c
 *
 * 关键编译选项:
 *   -fPIC     生成位置无关代码 (Position Independent Code)
 *             动态库在运行时可能被加载到任意地址，必须使用PIC
 *   -shared   创建共享库
 *
 * 为什么需要 -fPIC?
 *   静态库的代码在链接时地址已确定，而动态库的代码在运行时
 *   才被映射到进程地址空间，加载位置不固定。PIC 通过相对寻址
 *   和 GOT (全局偏移表) 解决了这个问题。
 */

#include <stdio.h>
#include "greetlib.h"

static const char *LIB_VERSION = "1.0.0";

void greet_hello(const char *name) {
    if (name == NULL) {
        name = "World";
    }
    printf("Hello, %s! Welcome!\n", name);
}

void greet_goodbye(const char *name) {
    if (name == NULL) {
        name = "World";
    }
    printf("Goodbye, %s! See you next time!\n", name);
}

void greet_time_of_day(int hour) {
    if (hour < 0 || hour > 23) {
        printf("Invalid hour: %d\n", hour);
        return;
    }
    if (hour < 6) {
        printf("It's late night (%02d:00), get some rest!\n", hour);
    } else if (hour < 12) {
        printf("Good morning! (%02d:00)\n", hour);
    } else if (hour < 18) {
        printf("Good afternoon! (%02d:00)\n", hour);
    } else {
        printf("Good evening! (%02d:00)\n", hour);
    }
}

const char *greet_get_version(void) {
    return LIB_VERSION;
}
