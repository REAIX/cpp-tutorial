/**
 * @file mathlib.c
 * @brief 静态库示例 - 数学运算库实现
 * @description 对应文档: 18-static-library
 *              实现数学运算库的所有函数，此文件将被编译为静态库 libmathlib.a
 *
 * 编译为静态库的步骤:
 *   1. gcc -c mathlib.c -o mathlib.o        # 编译为目标文件
 *   2. ar rcs libmathlib.a mathlib.o         # 打包为静态库
 *
 * ar 命令参数说明:
 *   r - 插入或替换目标文件到归档中
 *   c - 创建归档文件（如果不存在）
 *   s - 创建符号索引（等同于 ranlib）
 */

#include "mathlib.h"

int mathlib_add(int a, int b) {
    return a + b;
}

int mathlib_sub(int a, int b) {
    return a - b;
}

int mathlib_mul(int a, int b) {
    return a * b;
}

int mathlib_div(int a, int b) {
    if (b == 0) {
        return 0;
    }
    return a / b;
}

int mathlib_factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * mathlib_factorial(n - 1);
}

int mathlib_fibonacci(int n) {
    if (n <= 0) {
        return 0;
    }
    if (n == 1) {
        return 1;
    }
    int prev = 0;
    int curr = 1;
    for (int i = 2; i <= n; i++) {
        int next = prev + curr;
        prev = curr;
        curr = next;
    }
    return curr;
}
