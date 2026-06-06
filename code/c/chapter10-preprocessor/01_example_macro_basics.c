/** @file 01_example_macro_basics.c
 *  @brief 预处理器基础：#define、#include、条件编译(#ifdef, #ifndef, #if)
 *  @description 对应文档: 10-预处理器
 */

#include <stdio.h>
#include <string.h>

#define PI 3.14159265
#define MAX_SIZE 100
#define GREETING "Hello, Preprocessor!"
#define NEWLINE '\n'

void demo_define_constant(void) {
    printf("=== #define 定义常量 ===\n");

    printf("PI = %f\n", PI);
    printf("MAX_SIZE = %d\n", MAX_SIZE);
    printf("GREETING = %s\n", GREETING);
    printf("NEWLINE = '%c'\n", NEWLINE);

    printf("\n#define 是文本替换, 没有类型检查\n");
    printf("推荐: 整数常量用 enum, 其他用 const\n");

    printf("\n");
}

#define SQUARE(x) ((x) * (x))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(x) ((x) >= 0 ? (x) : -(x))

void demo_define_macro(void) {
    printf("=== #define 定义宏 ===\n");

    printf("SQUARE(5) = %d\n", SQUARE(5));
    printf("MAX(10, 20) = %d\n", MAX(10, 20));
    printf("MIN(10, 20) = %d\n", MIN(10, 20));
    printf("ABS(-7) = %d\n", ABS(-7));

    printf("\n宏的参数没有类型, 可以用于任何类型:\n");
    printf("MAX(3.14, 2.72) = %f\n", MAX(3.14, 2.72));

    printf("\n");
}

void demo_include(void) {
    printf("=== #include 文件包含 ===\n");

    printf("#include <file>   搜索系统头文件路径\n");
    printf("#include \"file\"   先搜索当前目录, 再搜索系统路径\n\n");

    printf("常用标准头文件:\n");
    printf("  <stdio.h>  输入输出\n");
    printf("  <stdlib.h> 通用工具\n");
    printf("  <string.h> 字符串操作\n");
    printf("  <math.h>   数学函数\n");
    printf("  <ctype.h>  字符分类\n");
    printf("  <assert.h> 断言\n");

    printf("\n");
}

void demo_ifdef(void) {
    printf("=== #ifdef / #ifndef 条件编译 ===\n");

#define DEBUG 1

#ifdef DEBUG
    printf("DEBUG 模式已启用 (DEBUG = %d)\n", DEBUG);
#else
    printf("DEBUG 模式未启用\n");
#endif

#ifndef RELEASE
    printf("RELEASE 未定义\n");
#else
    printf("RELEASE 已定义\n");
#endif

    printf("\n#ifdef X: 如果定义了 X 则编译\n");
    printf("#ifndef X: 如果未定义 X 则编译\n");

    printf("\n");
}

void demo_if_elif_else(void) {
    printf("=== #if / #elif / #else 条件编译 ===\n");

#define LOG_LEVEL 2

#if LOG_LEVEL == 0
    printf("日志级别: 无\n");
#elif LOG_LEVEL == 1
    printf("日志级别: 错误\n");
#elif LOG_LEVEL == 2
    printf("日志级别: 警告 (当前级别)\n");
#elif LOG_LEVEL == 3
    printf("日志级别: 信息\n");
#else
    printf("日志级别: 调试\n");
#endif

    printf("\n#if 支持整数表达式, #ifdef 只检查是否定义\n");

    printf("\n");
}

void demo_defined_operator(void) {
    printf("=== defined 运算符 ===\n");

#define FEATURE_A

#if defined(FEATURE_A)
    printf("FEATURE_A 已定义\n");
#else
    printf("FEATURE_A 未定义\n");
#endif

#if defined(FEATURE_A) && !defined(FEATURE_B)
    printf("FEATURE_A 已定义且 FEATURE_B 未定义\n");
#endif

#if defined(FEATURE_A) || defined(FEATURE_B)
    printf("FEATURE_A 或 FEATURE_B 至少一个已定义\n");
#endif

    printf("\ndefined(X) 在 #if 中使用, 返回 0 或 1\n");
    printf("比 #ifdef 更灵活, 可以组合逻辑运算\n");

    printf("\n");
}

void demo_undef(void) {
    printf("=== #undef 取消定义 ===\n");

#define TEMP_VALUE 42
    printf("TEMP_VALUE = %d\n", TEMP_VALUE);

#undef TEMP_VALUE
    printf("#undef TEMP_VALUE 后, TEMP_VALUE 不再有效\n");
    printf("#undef 常用于取消之前的宏定义, 重新定义新值\n");

    printf("\n");
}

int main(void) {
    demo_define_constant();
    demo_define_macro();
    demo_include();
    demo_ifdef();
    demo_if_elif_else();
    demo_defined_operator();
    demo_undef();

    return 0;
}
