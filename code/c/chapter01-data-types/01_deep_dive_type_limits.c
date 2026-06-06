/** @file 01_deep_dive_type_limits.c
 *  @brief 深入理解类型极限与整数溢出
 *  @description 对应文档: 01-data-types | limits.h、float.h详解，整数溢出/下溢，未定义行为
 *  编译命令: gcc -std=c17 01_deep_dive_type_limits.c -o 01_deep_dive_type_limits
 */

#include <stdio.h>
#include <limits.h>
#include <float.h>
#include <stdint.h>

void demo_limits_h(void) {
    printf("═══════════════════════════════════════\n");
    printf("  limits.h —— 整数类型的极限值\n");
    printf("═══════════════════════════════════════\n\n");

    printf("字符类型:\n");
    printf("  CHAR_BIT    = %d  (每字节位数)\n", CHAR_BIT);
    printf("  CHAR_MIN    = %d\n", CHAR_MIN);
    printf("  CHAR_MAX    = %d\n", CHAR_MAX);
    printf("  SCHAR_MIN   = %d\n", SCHAR_MIN);
    printf("  SCHAR_MAX   = %d\n", SCHAR_MAX);
    printf("  UCHAR_MAX   = %u\n", UCHAR_MAX);

    printf("\n短整型:\n");
    printf("  SHRT_MIN    = %d\n", SHRT_MIN);
    printf("  SHRT_MAX    = %d\n", SHRT_MAX);
    printf("  USHRT_MAX   = %u\n", USHRT_MAX);

    printf("\n整型:\n");
    printf("  INT_MIN     = %d\n", INT_MIN);
    printf("  INT_MAX     = %d\n", INT_MAX);
    printf("  UINT_MAX    = %u\n", UINT_MAX);

    printf("\n长整型:\n");
    printf("  LONG_MIN    = %ld\n", LONG_MIN);
    printf("  LONG_MAX    = %ld\n", LONG_MAX);
    printf("  ULONG_MAX   = %lu\n", ULONG_MAX);

    printf("\n长长整型:\n");
    printf("  LLONG_MIN   = %lld\n", LLONG_MIN);
    printf("  LLONG_MAX   = %lld\n", LLONG_MAX);
    printf("  ULLONG_MAX  = %llu\n", ULLONG_MAX);
}

void demo_float_h(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  float.h —— 浮点类型的极限值\n");
    printf("═══════════════════════════════════════\n\n");

    printf("float:\n");
    printf("  FLT_MIN        = %e  (最小正规格化数)\n", FLT_MIN);
    printf("  FLT_MAX        = %e  (最大值)\n", FLT_MAX);
    printf("  FLT_EPSILON    = %e  (1与下一个可表示数之差)\n", FLT_EPSILON);
    printf("  FLT_DIG        = %d  (十进制有效位数)\n", FLT_DIG);
    printf("  FLT_MANT_DIG   = %d  (尾数位数)\n", FLT_MANT_DIG);
    printf("  FLT_MIN_EXP    = %d  (最小指数)\n", FLT_MIN_EXP);
    printf("  FLT_MAX_EXP    = %d  (最大指数)\n", FLT_MAX_EXP);

    printf("\ndouble:\n");
    printf("  DBL_MIN        = %e\n", DBL_MIN);
    printf("  DBL_MAX        = %e\n", DBL_MAX);
    printf("  DBL_EPSILON    = %e\n", DBL_EPSILON);
    printf("  DBL_DIG        = %d\n", DBL_DIG);
    printf("  DBL_MANT_DIG   = %d\n", DBL_MANT_DIG);

    printf("\nlong double:\n");
    printf("  LDBL_MIN       = %Le\n", LDBL_MIN);
    printf("  LDBL_MAX       = %Le\n", LDBL_MAX);
    printf("  LDBL_EPSILON   = %Le\n", LDBL_EPSILON);
    printf("  LDBL_DIG       = %d\n", LDBL_DIG);
}

void demo_integer_overflow(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  整数溢出 (Integer Overflow)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("有符号整数溢出 —— 未定义行为!\n\n");

    int max_int = INT_MAX;
    printf("INT_MAX = %d\n", max_int);
    printf("INT_MAX + 1 = %d (未定义行为，可能回绕也可能崩溃)\n", max_int + 1);

    int min_int = INT_MIN;
    printf("\nINT_MIN = %d\n", min_int);
    printf("INT_MIN - 1 = %d (未定义行为)\n", min_int - 1);

    printf("\n无符号整数溢出 —— 定义良好的回绕:\n\n");

    unsigned int umax = UINT_MAX;
    printf("UINT_MAX = %u\n", umax);
    printf("UINT_MAX + 1 = %u (回绕到0，这是标准定义的行为)\n", umax + 1);

    unsigned int uzero = 0;
    printf("0u - 1 = %u (回绕到UINT_MAX)\n", uzero - 1);

    printf("\n举一反三 —— 溢出的实际危害:\n");
    printf("  1. 循环条件: for(int i=0; i<=n; i++) 当n=INT_MAX时死循环\n");
    printf("  2. 内存分配: malloc(n*sizeof(int)) 当n很大时溢出，分配过少\n");
    printf("  3. 数组索引: 计算偏移量溢出导致越界访问\n");
    printf("  4. 安全漏洞: 整数溢出是CVE的常见来源\n");
}

void demo_safe_integer_operations(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  安全的整数运算\n");
    printf("═══════════════════════════════════════\n\n");

    printf("1. 加法溢出检测:\n");
    int a = INT_MAX - 10;
    int b = 20;
    if (b > 0 && a > INT_MAX - b) {
        printf("   %d + %d 会溢出! 检测成功\n", a, b);
    } else {
        printf("   %d + %d 安全\n", a, b);
    }

    printf("\n2. 乘法溢出检测:\n");
    int x = 100000;
    int y = 30000;
    if (x != 0 && y > INT_MAX / x) {
        printf("   %d * %d 会溢出! 检测成功\n", x, y);
    } else {
        printf("   %d * %d 安全\n", x, y);
    }

    printf("\n3. 使用stdint.h的固定位宽类型:\n");
    printf("   int32_t  —— 保证32位有符号整数\n");
    printf("   uint64_t —— 保证64位无符号整数\n");
    printf("   int64_t  —— 保证64位有符号整数\n");
    int32_t i32 = 2147483647;
    int64_t i64 = (int64_t)i32 + 1;
    printf("   int32_t: %d, int64_t: %lld (用更大类型避免溢出)\n", i32, i64);

    printf("\n4. 使用size_t表示大小和计数:\n");
    printf("   size_t 是无符号类型，保证能容纳最大对象大小\n");
    printf("   sizeof(size_t) = %zu 字节\n", sizeof(size_t));
}

void demo_underflow_and_special(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  下溢与特殊值\n");
    printf("═══════════════════════════════════════\n\n");

    printf("浮点下溢 (Underflow):\n");
    float tiny = FLT_MIN;
    float smaller = tiny / 2.0f;
    printf("  FLT_MIN = %e\n", tiny);
    printf("  FLT_MIN / 2 = %e (次正规数，精度降低)\n", smaller);
    printf("  FLT_TRUE_MIN = %e (最小正次正规数)\n", FLT_TRUE_MIN);

    printf("\n浮点特殊值:\n");
    float pos_inf = 1.0f / 0.0f;
    float neg_inf = -1.0f / 0.0f;
    float nan_val = 0.0f / 0.0f;

    printf("  +∞ = %f\n", pos_inf);
    printf("  -∞ = %f\n", neg_inf);
    printf("  NaN = %f\n", nan_val);

    printf("\nNaN的特殊性质:\n");
    if (nan_val == nan_val) {
        printf("  NaN == NaN: true\n");
    } else {
        printf("  NaN == NaN: false (NaN不等于任何值，包括自己!)\n");
    }
    if (nan_val != nan_val) {
        printf("  NaN != NaN: true (这是检测NaN的标准方法)\n");
    }
}

void demo_undefined_behavior_examples(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  与类型相关的未定义行为\n");
    printf("═══════════════════════════════════════\n\n");

    printf("以下操作都是未定义行为(UB):\n\n");

    printf("1. 有符号整数溢出\n");
    printf("   int x = INT_MAX; x + 1;  // UB\n\n");

    printf("2. 左移位数为负或大于等于位宽\n");
    printf("   int x = 1 << -1;   // UB\n");
    printf("   int y = 1 << 32;   // UB (int为32位时)\n\n");

    printf("3. 除以零\n");
    printf("   int x = 1 / 0;     // UB\n");
    printf("   int y = 1 %% 0;     // UB\n\n");

    printf("4. 对未初始化的变量取值\n");
    printf("   int x; if(x > 0) {} // UB: x的值不确定\n\n");

    printf("5. 空指针解引用\n");
    printf("   int *p = NULL; *p; // UB\n\n");

    printf("⚠️ 未定义行为意味着编译器可以做任何事:\n");
    printf("   - 可能按你预期工作(最危险的情况!)\n");
    printf("   - 可能产生错误结果\n");
    printf("   - 可能崩溃\n");
    printf("   - 编译器甚至可能优化掉包含UB的代码\n");
    printf("   - 永远不要依赖UB的行为!\n");
}

int main(void) {
    demo_limits_h();
    demo_float_h();
    demo_integer_overflow();
    demo_safe_integer_operations();
    demo_underflow_and_special();
    demo_undefined_behavior_examples();

    return 0;
}
