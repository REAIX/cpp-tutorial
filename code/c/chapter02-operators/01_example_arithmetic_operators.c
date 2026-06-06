/** @file 01_example_arithmetic_operators.c
 *  @brief 算术运算符：+、-、*、/、%、++、--
 *  @description 对应文档: 02-operators | 演示C语言算术运算符的用法和注意事项
 *  编译命令: gcc -std=c17 01_example_arithmetic_operators.c -o 01_example_arithmetic_operators
 */

#include <stdio.h>
#include <stdlib.h>

void demo_basic_arithmetic(void) {
    printf("═══════════════════════════════════════\n");
    printf("  基本算术运算符\n");
    printf("═══════════════════════════════════════\n\n");

    int a = 17, b = 5;

    printf("a = %d, b = %d\n\n", a, b);
    printf("加法 a + b  = %d\n", a + b);
    printf("减法 a - b  = %d\n", a - b);
    printf("乘法 a * b  = %d\n", a * b);
    printf("除法 a / b  = %d (整数除法，截断小数)\n", a / b);
    printf("取余 a %% b  = %d (只适用于整数)\n", a % b);

    printf("\n负数取余的注意事项:\n");
    printf("  -17 %% 5 = %d (C99规定: 结果与被除数同号)\n", -17 % 5);
    printf("  17 %% -5 = %d\n", 17 % -5);
    printf("  -17 %% -5 = %d\n", -17 % -5);

    printf("\n浮点除法:\n");
    double x = 17.0, y = 5.0;
    printf("  %.1f / %.1f = %.2f\n", x, y, x / y);
}

void demo_division_traps(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  除法运算的陷阱\n");
    printf("═══════════════════════════════════════\n\n");

    printf("陷阱1: 整数除法丢失小数部分\n");
    int result1 = 5 / 3;
    printf("  5 / 3 = %d (不是1.67!)\n", result1);
    printf("  修正: (double)5 / 3 = %.4f\n", (double)5 / 3);

    printf("\n陷阱2: 除以零\n");
    printf("  整数除以0: 未定义行为(通常崩溃)\n");
    printf("  浮点除以0: 结果为±∞或NaN\n");
    printf("  1.0/0.0 = %f\n", 1.0 / 0.0);
    printf("  0.0/0.0 = %f\n", 0.0 / 0.0);

    printf("\n陷阱3: 隐式整数除法\n");
    double half = 1 / 2;
    printf("  double half = 1/2; → %.1f (不是0.5!)\n", half);
    printf("  修正: 1.0/2 或 (double)1/2 → %.1f\n", 1.0 / 2);
}

void demo_increment_decrement(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  自增自减运算符: ++ --\n");
    printf("═══════════════════════════════════════\n\n");

    int a = 5;

    printf("前置 vs 后置:\n");
    a = 5;
    printf("  a = %d\n", a);
    printf("  a++ = %d (后置: 先返回原值，再自增)\n", a++);
    printf("  a = %d (自增后的值)\n", a);

    a = 5;
    printf("\n  a = %d\n", a);
    printf("  ++a = %d (前置: 先自增，再返回新值)\n", ++a);
    printf("  a = %d (自增后的值)\n", a);

    printf("\n在表达式中使用:\n");
    int x = 3, y;
    y = x++ * 2;
    printf("  x=3; y = x++ * 2; → y=%d, x=%d\n", y, x);

    x = 3;
    y = ++x * 2;
    printf("  x=3; y = ++x * 2; → y=%d, x=%d\n", y, x);

    printf("\n⚠️ 严重警告: 不要在同一表达式中对同一变量多次修改\n");
    printf("  a[i] = i++;      // 未定义行为!\n");
    printf("  a = a++ + ++a;   // 未定义行为!\n");
    printf("  原因: 修改顺序未定义\n");
}

void demo_unary_operators(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  一元算术运算符\n");
    printf("═══════════════════════════════════════\n\n");

    int a = 10;
    printf("a = %d\n", a);
    printf("+a = %d (正号，通常无效果)\n", +a);
    printf("-a = %d (负号，取反)\n", -a);

    printf("\n注意: 一元减法可能溢出\n");
    printf("  INT_MIN = %d\n", -2147483647 - 1);
    printf("  -INT_MIN 的值超出int范围(未定义行为)\n");
}

void demo_modulo_applications(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  取余运算的实用场景\n");
    printf("═══════════════════════════════════════\n\n");

    printf("1. 判断奇偶:\n");
    for (int i = 0; i < 5; i++) {
        printf("  %d 是%s数\n", i, i % 2 == 0 ? "偶" : "奇");
    }

    printf("\n2. 循环索引:\n");
    int buffer_size = 5;
    for (int i = 0; i < 12; i++) {
        printf("  写入位置: %d (i %% %d)\n", i % buffer_size, buffer_size);
    }

    printf("\n3. 提取各位数字:\n");
    int num = 12345;
    printf("  %d 的各位: ", num);
    int temp = num;
    while (temp > 0) {
        printf("%d ", temp % 10);
        temp /= 10;
    }
    printf("(从个位开始)\n");

    printf("\n4. 时间转换:\n");
    int total_seconds = 3725;
    int hours = total_seconds / 3600;
    int minutes = (total_seconds % 3600) / 60;
    int seconds = total_seconds % 60;
    printf("  %d秒 = %d时%d分%d秒\n", total_seconds, hours, minutes, seconds);
}

int main(void) {
    demo_basic_arithmetic();
    demo_division_traps();
    demo_increment_decrement();
    demo_unary_operators();
    demo_modulo_applications();

    return 0;
}
