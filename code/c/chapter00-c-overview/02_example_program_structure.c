/** @file 02_example_program_structure.c
 *  @brief C程序结构详解：函数、变量、注释、预处理指令
 *  @description 对应文档: 00-c-overview | 演示C程序的完整结构组成
 *  编译命令: gcc -std=c17 02_example_program_structure.c -o 02_example_program_structure
 */

#include <stdio.h>
#include <stdlib.h>

#define PI 3.14159265
#define MAX_SIZE 100
#define GREETING "欢迎学习C语言"

int global_counter = 0;

int add(int a, int b);
double circle_area(double radius);
void print_separator(const char *title);

int add(int a, int b) {
    return a + b;
}

double circle_area(double radius) {
    return PI * radius * radius;
}

void print_separator(const char *title) {
    printf("\n");
    printf("═══════════════════════════════════════\n");
    printf("  %s\n", title);
    printf("═══════════════════════════════════════\n");
}

void demo_functions(void) {
    print_separator("函数：C程序的构建块");

    int x = 10, y = 20;
    int sum = add(x, y);
    printf("add(%d, %d) = %d\n", x, y, sum);

    double r = 5.0;
    double area = circle_area(r);
    printf("circle_area(%.1f) = %.6f\n", r, area);

    printf("\n函数的组成要素:\n");
    printf("  返回类型  函数名(参数列表) { 函数体 }\n");
    printf("  int       add    (int a, int b) { return a+b; }\n");
}

void demo_variables(void) {
    print_separator("变量：存储数据的基本单元");

    int age = 25;
    double salary = 8500.50;
    char grade = 'A';
    float temperature = 36.5f;

    printf("整型变量 age = %d\n", age);
    printf("双精度变量 salary = %.2f\n", salary);
    printf("字符变量 grade = %c\n", grade);
    printf("单精度变量 temperature = %.1f\n", temperature);

    printf("\n变量三要素:\n");
    printf("  1. 类型 —— 决定占用的内存大小和存储方式\n");
    printf("  2. 名字 —— 标识变量的标识符\n");
    printf("  3. 值   —— 变量中存储的数据\n");

    global_counter++;
    printf("\n全局变量 global_counter = %d\n", global_counter);
}

void demo_comments(void) {
    print_separator("注释：代码的说明书");

    printf("C语言有三种注释方式:\n\n");

    printf("1. 块注释 (C89起支持):\n");
    printf("   /* 这是块注释，可以跨行 */\n\n");

    printf("2. 行注释 (C99起支持):\n");
    printf("   // 这是行注释，到行尾结束\n\n");

    printf("3. 文档注释 (Doxygen风格):\n");
    printf("   /** @brief 简要描述 */\n");
    printf("   /*! @param 参数说明 */\n\n");

    printf("好的注释原则:\n");
    printf("  - 解释「为什么」而非「是什么」\n");
    printf("  - 避免无意义的注释\n");
    printf("  - 保持注释与代码同步\n");
}

void demo_preprocessor(void) {
    print_separator("预处理指令：编译前的文本替换");

    printf("常用预处理指令:\n\n");

    printf("1. #include —— 文件包含\n");
    printf("   #include <stdio.h>   // 系统头文件，在系统目录搜索\n");
    printf("   #include \"myheader.h\" // 用户头文件，先搜索当前目录\n\n");

    printf("2. #define —— 宏定义\n");
    printf("   PI = %f (通过 #define PI 3.14159265 定义)\n", PI);
    printf("   MAX_SIZE = %d (通过 #define MAX_SIZE 100 定义)\n", MAX_SIZE);
    printf("   GREETING = %s\n", GREETING);
    printf("\n");

    printf("3. 条件编译\n");
#ifdef DEBUG
    printf("   当前为 DEBUG 模式 (通过 #ifdef DEBUG 判断)\n");
#else
    printf("   当前为 RELEASE 模式 (未定义 DEBUG 宏)\n");
#endif

    printf("\n4. #pragma —— 编译器特定指令\n");
    printf("   #pragma once  // 保证头文件只包含一次\n");
}

void demo_printf_format(void) {
    print_separator("printf格式化输出");

    int num = 42;
    double pi = 3.14159265358979;

    printf("常用格式说明符:\n");
    printf("  %%d  整数:     %d\n", num);
    printf("  %%f  浮点数:   %f\n", pi);
    printf("  %%.2f 两位小数: %.2f\n", pi);
    printf("  %%e  科学计数: %e\n", pi);
    printf("  %%c  字符:     %c\n", 'Z');
    printf("  %%s  字符串:   %s\n", "Hello");
    printf("  %%p  指针:     %p\n", (void *)&num);
    printf("  %%x  十六进制: %x\n", num);
    printf("  %%o  八进制:   %o\n", num);
    printf("  %%%%  输出%%号本身\n");

    printf("\n宽度和对齐:\n");
    printf("  [%10d]  右对齐，宽度10\n", num);
    printf("  [%-10d]  左对齐，宽度10\n", num);
    printf("  [%010d]  前导零填充\n", num);
}

int main(void) {
    demo_functions();
    demo_variables();
    demo_comments();
    demo_preprocessor();
    demo_printf_format();

    return 0;
}
