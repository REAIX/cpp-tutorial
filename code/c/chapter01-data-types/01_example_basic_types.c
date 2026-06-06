/** @file 01_example_basic_types.c
 *  @brief C语言基本数据类型：int, char, float, double
 *  @description 对应文档: 01-data-types | 演示各种基本数据类型的声明、大小和取值范围
 *  编译命令: gcc -std=c17 01_example_basic_types.c -o 01_example_basic_types
 */

#include <stdio.h>
#include <limits.h>
#include <float.h>

void demo_integer_types(void) {
    printf("═══════════════════════════════════════\n");
    printf("  整数类型\n");
    printf("═══════════════════════════════════════\n\n");

    char c = 'A';
    short s = 1000;
    int i = 100000;
    long l = 1000000L;
    long long ll = 1000000000000LL;

    printf("类型              大小    值\n");
    printf("──────────────────────────────────────\n");
    printf("char              %2zu字节  %c (%d)\n", sizeof(char), c, c);
    printf("short             %2zu字节  %d\n", sizeof(short), s);
    printf("int               %2zu字节  %d\n", sizeof(int), i);
    printf("long              %2zu字节  %ld\n", sizeof(long), l);
    printf("long long         %2zu字节  %lld\n", sizeof(long long), ll);

    printf("\n无符号整数类型:\n");
    unsigned char uc = 255;
    unsigned int ui = 4000000000U;
    unsigned long ul = 4000000000UL;

    printf("unsigned char     %2zu字节  %u\n", sizeof(unsigned char), uc);
    printf("unsigned int      %2zu字节  %u\n", sizeof(unsigned int), ui);
    printf("unsigned long     %2zu字节  %lu\n", sizeof(unsigned long), ul);

    printf("\n关键规则:\n");
    printf("  sizeof(char) == 1 是标准保证的\n");
    printf("  sizeof(long) 在Windows(4字节)和Linux64(8字节)上不同\n");
    printf("  整数字面量后缀: L=long, LL=long long, U=unsigned\n");
}

void demo_char_type(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  字符类型 char 详解\n");
    printf("═══════════════════════════════════════\n\n");

    char ch1 = 'A';
    char ch2 = 65;
    char ch3 = '\x41';

    printf("三种方式表示字符'A':\n");
    printf("  'A'   → %c (字符字面量)\n", ch1);
    printf("  65    → %c (ASCII码值)\n", ch2);
    printf("  '\\x41'→ %c (十六进制转义)\n", ch3);

    printf("\nchar本质上是一个小整数:\n");
    printf("  'A' = %d, 'Z' = %d, 'a' = %d, 'z' = %d\n", 'A', 'Z', 'a', 'z');
    printf("  '0' = %d, '9' = %d\n", '0', '9');

    printf("\n大小写转换(利用ASCII差值=32):\n");
    char lower = 'a';
    char upper = lower - 32;
    printf("  '%c' - 32 = '%c'\n", lower, upper);
    printf("  '%c' + 32 = '%c'\n", 'B', 'B' + 32);

    printf("\n注意: char的符号性由实现定义\n");
    printf("  signed char 范围: %d ~ %d\n", SCHAR_MIN, SCHAR_MAX);
    printf("  unsigned char 范围: 0 ~ %u\n", UCHAR_MAX);
}

void demo_float_types(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  浮点类型\n");
    printf("═══════════════════════════════════════\n\n");

    float f = 3.14159f;
    double d = 3.141592653589793;
    long double ld = 3.141592653589793238L;

    printf("类型              大小    值\n");
    printf("──────────────────────────────────────\n");
    printf("float             %2zu字节  %.6f\n", sizeof(float), f);
    printf("double            %2zu字节  %.15f\n", sizeof(double), d);
    printf("long double       %2zu字节  %.18Lf\n", sizeof(long double), ld);

    printf("\n精度对比:\n");
    printf("  float:       %.10f (有效数字约6-7位)\n", f);
    printf("  double:      %.15f (有效数字约15-16位)\n", d);
    printf("  long double: %.18Lf (有效数字约18-19位)\n", ld);

    printf("\n取值范围:\n");
    printf("  float:       %e ~ %e\n", FLT_MIN, FLT_MAX);
    printf("  double:      %e ~ %e\n", DBL_MIN, DBL_MAX);

    printf("\n特殊浮点值:\n");
    printf("  正无穷: %f\n", 1.0 / 0.0);
    printf("  负无穷: %f\n", -1.0 / 0.0);
    printf("  NaN:    %f\n", 0.0 / 0.0);
}

void demo_type_sizes_summary(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  数据类型大小汇总 (本平台)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("类型              sizeof   取值范围\n");
    printf("────────────────────────────────────────────────────\n");
    printf("char              %2zu       %d ~ %d\n", sizeof(char), CHAR_MIN, CHAR_MAX);
    printf("signed char       %2zu       %d ~ %d\n", sizeof(signed char), SCHAR_MIN, SCHAR_MAX);
    printf("unsigned char     %2zu       0 ~ %u\n", sizeof(unsigned char), UCHAR_MAX);
    printf("short             %2zu       %d ~ %d\n", sizeof(short), SHRT_MIN, SHRT_MAX);
    printf("unsigned short    %2zu       0 ~ %u\n", sizeof(unsigned short), USHRT_MAX);
    printf("int               %2zu       %d ~ %d\n", sizeof(int), INT_MIN, INT_MAX);
    printf("unsigned int      %2zu       0 ~ %u\n", sizeof(unsigned int), UINT_MAX);
    printf("long              %2zu       %ld ~ %ld\n", sizeof(long), LONG_MIN, LONG_MAX);
    printf("unsigned long     %2zu       0 ~ %lu\n", sizeof(unsigned long), ULONG_MAX);
    printf("long long         %2zu       %lld ~ %lld\n", sizeof(long long), LLONG_MIN, LLONG_MAX);
    printf("float             %2zu       %e ~ %e\n", sizeof(float), FLT_MIN, FLT_MAX);
    printf("double            %2zu       %e ~ %e\n", sizeof(double), DBL_MIN, DBL_MAX);

    printf("\n标准保证的大小关系:\n");
    printf("  1 = sizeof(char) <= sizeof(short) <= sizeof(int) <= sizeof(long) <= sizeof(long long)\n");
}

void demo_bool_type(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  布尔类型 (_Bool / bool)\n");
    printf("═══════════════════════════════════════\n\n");

    _Bool b1 = 1;
    _Bool b2 = 0;
    _Bool b3 = 42;

    printf("_Bool 是C99引入的原生布尔类型\n");
    printf("  _Bool b1 = 1;   → %d\n", b1);
    printf("  _Bool b2 = 0;   → %d\n", b2);
    printf("  _Bool b3 = 42;  → %d (非零值转为1)\n", b3);
    printf("  sizeof(_Bool) = %zu\n", sizeof(_Bool));

    printf("\n使用 <stdbool.h> 可以写更直观的 bool/true/false:\n");
    printf("  #include <stdbool.h>\n");
    printf("  bool flag = true;   // 等价于 _Bool flag = 1\n");
    printf("  bool check = false; // 等价于 _Bool check = 0\n");

    printf("\nC语言中真假的判断规则:\n");
    printf("  0 为假 (false)\n");
    printf("  一切非零值为真 (true)，包括负数\n");
}

int main(void) {
    demo_integer_types();
    demo_char_type();
    demo_float_types();
    demo_type_sizes_summary();
    demo_bool_type();

    return 0;
}
