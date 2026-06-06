/**
 * @file main.c
 * @brief 多文件编程基本示例
 * @description 对应文档: 16-多文件编程 - 演示头文件、源文件、分离编译、extern
 */
#include <stdio.h>
#include "math_utils.h"

void demo_basic_operations(void) {
    printf("=== 基本运算 ===\n");
    printf("  add(10, 3) = %d\n", math_add(10, 3));
    printf("  sub(10, 3) = %d\n", math_sub(10, 3));
    printf("  mul(10, 3) = %d\n", math_mul(10, 3));
    printf("  div(10, 3) = %.2f\n", math_div(10, 3));
    printf("  div(10, 0) = %.2f (错误处理)\n\n", math_div(10, 0));
}

void demo_advanced_operations(void) {
    printf("=== 高级运算 ===\n");
    printf("  factorial(5) = %d\n", math_factorial(5));
    printf("  factorial(0) = %d\n", math_factorial(0));
    printf("  factorial(-1) = %d (错误返回-1)\n", math_factorial(-1));
    printf("  gcd(48, 18) = %d\n", math_gcd(48, 18));
    printf("  gcd(100, 75) = %d\n\n", math_gcd(100, 75));
}

void demo_extern_concept(void) {
    printf("=== extern关键字说明 ===\n");
    printf("  extern用于声明在其他源文件中定义的全局变量/函数\n");
    printf("  头文件中的函数声明隐含extern\n");
    printf("  示例:\n");
    printf("    file1.c:  int g_counter = 0;          // 定义\n");
    printf("    file2.c:  extern int g_counter;        // 声明(引用)\n");
    printf("    file2.c:  g_counter++;                 // 使用\n\n");
}

void demo_compilation_flow(void) {
    printf("=== 分离编译流程 ===\n");
    printf("  步骤1: 编译各源文件为目标文件\n");
    printf("    gcc -c math_utils.c -o math_utils.o\n");
    printf("    gcc -c main.c -o main.o\n");
    printf("  步骤2: 链接目标文件为可执行文件\n");
    printf("    gcc main.o math_utils.o -o main\n");
    printf("  一步编译:\n");
    printf("    gcc main.c math_utils.c -o main\n\n");
}

int main(void) {
    printf("========== 多文件编程基本示例 ==========\n\n");

    demo_basic_operations();
    demo_advanced_operations();
    demo_extern_concept();
    demo_compilation_flow();

    printf("========== 所有演示完成 ==========\n");
    return 0;
}
