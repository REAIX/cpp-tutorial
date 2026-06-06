/** @file 03_example_type_conversion.c
 *  @brief 类型转换：隐式转换与显式转换(强制类型转换)
 *  @description 对应文档: 01-data-types | 演示C语言中的自动类型提升、隐式转换规则和显式强制转换
 *  编译命令: gcc -std=c17 03_example_type_conversion.c -o 03_example_type_conversion
 */

#include <stdio.h>
#include <stdlib.h>

void demo_implicit_conversion(void) {
    printf("═══════════════════════════════════════\n");
    printf("  隐式类型转换(自动转换)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("1. 整数提升 (Integer Promotion):\n");
    printf("   char/short 在运算时自动提升为 int\n");
    char c = 100;
    short s = 200;
    printf("   char(%d) + short(%d) → int(%d)\n", c, s, c + s);
    printf("   sizeof(c+s) = %zu (不是1或2，而是int的大小)\n", sizeof(c + s));

    printf("\n2. 算术转换 (Usual Arithmetic Conversions):\n");
    printf("   不同类型运算时，低精度向高精度转换\n\n");

    int i = 10;
    double d = 3.14;
    printf("   int(%d) + double(%.2f) → double(%.2f)\n", i, d, i + d);
    printf("   sizeof(i+d) = %zu\n", sizeof(i + d));

    printf("\n   转换优先级(从低到高):\n");
    printf("   int → unsigned int → long → unsigned long → long long → float → double → long double\n");

    printf("\n3. 赋值转换:\n");
    double pi = 3.14159;
    int truncated = pi;
    printf("   double(%.5f) → int(%d) (截断小数部分)\n", pi, truncated);

    int big = 300;
    char small = big;
    printf("   int(%d) → char(%d) (溢出截断: 300-256=44)\n", big, small);
}

void demo_explicit_cast(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  显式类型转换(强制转换)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("语法: (目标类型)表达式\n\n");

    int a = 5, b = 2;

    printf("1. 整数除法 → 浮点除法:\n");
    printf("   %d / %d = %d (整数除法，小数丢失)\n", a, b, a / b);
    printf("   (double)%d / %d = %.2f (强制转换后得到浮点结果)\n", a, b, (double)a / b);
    printf("   %d / (double)%d = %.2f (也可以只转一个)\n", a, b, a / (double)b);

    printf("\n2. 指针类型转换:\n");
    int value = 0x41424344;
    char *byte_ptr = (char *)&value;
    printf("   int 0x%X 的字节序(本机):\n", value);
    printf("   字节0: 0x%02X ('%c')\n", (unsigned char)byte_ptr[0], byte_ptr[0]);
    printf("   字节1: 0x%02X ('%c')\n", (unsigned char)byte_ptr[1], byte_ptr[1]);
    printf("   字节2: 0x%02X ('%c')\n", (unsigned char)byte_ptr[2], byte_ptr[2]);
    printf("   字节3: 0x%02X ('%c')\n", (unsigned char)byte_ptr[3], byte_ptr[3]);
    printf("   (小端序: 低位字节在前)\n");

    printf("\n3. void* 通用指针:\n");
    int num = 42;
    void *vp = &num;
    int *ip = (int *)vp;
    printf("   int* → void* → int*: 值 = %d\n", *ip);
}

void demo_conversion_pitfalls(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  类型转换的常见陷阱\n");
    printf("═══════════════════════════════════════\n\n");

    printf("陷阱1: 有符号与无符号比较\n");
    int signed_val = -1;
    unsigned int unsigned_val = 1;
    if ((unsigned int)signed_val < unsigned_val) {
        printf("   -1 < 1: 正确\n");
    } else {
        printf("   -1 < 1: 错误! -1被转为无符号后变成%u\n", (unsigned int)signed_val);
    }

    printf("\n陷阱2: 整数溢出后赋值\n");
    int large = 2147483647;
    printf("   INT_MAX = %d\n", large);
    printf("   INT_MAX + 1 = %d (溢出! 未定义行为)\n", large + 1);

    printf("\n陷阱3: 精度丢失\n");
    float f = 16777217;
    printf("   16777217 存入float后变为: %.0f\n", f);
    printf("   (float只有约7位有效数字，无法精确表示)\n");

    printf("\n陷阱4: 缩窄转换\n");
    double d = 1e20;
    float f2 = (float)d;
    printf("   1e20 强转为float: %f (可能溢出为无穷)\n", f2);
}

void demo_safe_conversion(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  安全的类型转换实践\n");
    printf("═══════════════════════════════════════\n\n");

    printf("1. 整数除法先转类型:\n");
    int total = 17, count = 5;
    double average = (double)total / count;
    printf("   %d / %d = %.2f (先转再除)\n", total, count, average);

    printf("\n2. 避免有符号/无符号混用:\n");
    int len = -1;
    size_t ulen = (size_t)len;
    printf("   负数转size_t: %zu (变成极大值!)\n", ulen);
    printf("   建议: 比较前确保类型一致\n");

    printf("\n3. 使用显式转换而非让编译器隐式转换:\n");
    printf("   int i = (int)3.14;  // 明确表示截断意图\n");
    printf("   而非: int i = 3.14; // 编译器可能警告\n");

    printf("\n4. 检查范围再转换:\n");
    long big_val = (long)3000000000LL;
    if (big_val >= INT_MIN && big_val <= INT_MAX) {
        int safe = (int)big_val;
        printf("   转换安全: %ld → %d\n", big_val, safe);
    } else {
        printf("   %ld 超出int范围，不能安全转换!\n", big_val);
    }
}

int main(void) {
    demo_implicit_conversion();
    demo_explicit_cast();
    demo_conversion_pitfalls();
    demo_safe_conversion();

    return 0;
}
