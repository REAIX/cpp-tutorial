/**
 * @file main.c
 * @brief 静态库示例 - 使用静态库的主程序
 * @description 对应文档: 18-static-library
 *              演示如何链接并使用静态库 libmathlib.a
 *
 * 编译与链接步骤:
 *   1. 先编译静态库:
 *      gcc -c mathlib.c -o mathlib.o
 *      ar rcs libmathlib.a mathlib.o
 *
 *   2. 编译主程序并链接静态库:
 *      gcc main.c -L. -lmathlib -o main
 *      或
 *      gcc main.c libmathlib.a -o main
 *
 * 参数说明:
 *   -L.        在当前目录搜索库文件
 *   -lmathlib  链接 libmathlib.a（去掉前缀 lib 和后缀 .a）
 */

#include <stdio.h>
#include "mathlib.h"

void demo_basic_operations(void) {
    printf("===== 基本运算演示 =====\n");
    printf("add(10, 3)  = %d\n", mathlib_add(10, 3));
    printf("sub(10, 3)  = %d\n", mathlib_sub(10, 3));
    printf("mul(10, 3)  = %d\n", mathlib_mul(10, 3));
    printf("div(10, 3)  = %d\n", mathlib_div(10, 3));
    printf("div(10, 0)  = %d (除零保护)\n", mathlib_div(10, 0));
    printf("\n");
}

void demo_factorial(void) {
    printf("===== 阶乘演示 =====\n");
    for (int i = 0; i <= 10; i++) {
        printf("%2d! = %d\n", i, mathlib_factorial(i));
    }
    printf("\n");
}

void demo_fibonacci(void) {
    printf("===== 斐波那契数列演示 =====\n");
    printf("Fibonacci 序列 (前15项): ");
    for (int i = 0; i < 15; i++) {
        printf("%d", mathlib_fibonacci(i));
        if (i < 14) {
            printf(", ");
        }
    }
    printf("\n\n");
}

void demo_static_lib_concept(void) {
    printf("===== 静态库概念说明 =====\n");
    printf("静态库 (.a / .lib) 的特点:\n");
    printf("  1. 编译时链接: 静态库在编译时被完整复制到可执行文件中\n");
    printf("  2. 独立运行: 可执行文件不依赖库文件即可运行\n");
    printf("  3. 体积较大: 每个使用该库的程序都有一份库的副本\n");
    printf("  4. 更新不便: 库更新后需要重新编译所有依赖程序\n");
    printf("  5. 启动较快: 无需运行时加载，符号地址在编译期确定\n");
    printf("\n");
    printf("创建静态库的命令:\n");
    printf("  gcc -c mathlib.c -o mathlib.o\n");
    printf("  ar rcs libmathlib.a mathlib.o\n");
    printf("\n");
    printf("查看静态库内容:\n");
    printf("  ar -t libmathlib.a    # 列出目标文件\n");
    printf("  nm libmathlib.a       # 列出符号表\n");
    printf("\n");
}

int main(void) {
    printf("========== 静态库使用示例 ==========\n\n");

    demo_basic_operations();
    demo_factorial();
    demo_fibonacci();
    demo_static_lib_concept();

    printf("========== 程序结束 ==========\n");
    return 0;
}
