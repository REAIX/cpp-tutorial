/**
 * @file math_utils.c
 * @brief 数学工具函数实现
 * @description 对应文档: 16-多文件编程
 */
#include "math_utils.h"
#include <stdio.h>

int math_add(int a, int b) {
    return a + b;
}

int math_sub(int a, int b) {
    return a - b;
}

int math_mul(int a, int b) {
    return a * b;
}

double math_div(int a, int b) {
    if (b == 0) {
        fprintf(stderr, "错误: 除数不能为零\n");
        return 0.0;
    }
    return (double)a / b;
}

int math_factorial(int n) {
    if (n < 0) return -1;
    if (n <= 1) return 1;
    int result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}

int math_gcd(int a, int b) {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}
