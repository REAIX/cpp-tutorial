/** @file 02_deep_dive_float_pitfalls.c
 *  @brief 深入理解浮点数陷阱：IEEE 754、精度问题、比较方法
 *  @description 对应文档: 01-data-types | IEEE 754浮点标准详解、精度陷阱、浮点比较、epsilon
 *  编译命令: gcc -std=c17 02_deep_dive_float_pitfalls.c -o 02_deep_dive_float_pitfalls
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <stdint.h>
#include <string.h>

void demo_ieee754_structure(void) {
    printf("═══════════════════════════════════════\n");
    printf("  IEEE 754 浮点数结构\n");
    printf("═══════════════════════════════════════\n\n");

    printf("float (32位 = 1符号 + 8指数 + 23尾数):\n");
    float f = -6.625f;
    uint32_t bits;
    memcpy(&bits, &f, sizeof(bits));
    printf("  %.3f 的二进制表示: 0x%08X\n", f, bits);
    printf("  符号位: %d (1=负数)\n", (bits >> 31) & 1);
    printf("  指数:   0x%02X = %d (实际指数=%d)\n",
           (bits >> 23) & 0xFF, (bits >> 23) & 0xFF,
           (int)((bits >> 23) & 0xFF) - 127);
    printf("  尾数:   0x%06X\n", bits & 0x7FFFFF);

    printf("\ndouble (64位 = 1符号 + 11指数 + 52尾数):\n");
    double d = 0.1;
    uint64_t dbits;
    memcpy(&dbits, &d, sizeof(dbits));
    printf("  %.17f 的二进制表示: 0x%016llX\n", d, (unsigned long long)dbits);
    printf("  符号位: %d\n", (int)(dbits >> 63));
    printf("  指数:   0x%03X = %d (实际指数=%d)\n",
           (unsigned int)((dbits >> 52) & 0x7FF),
           (unsigned int)((dbits >> 52) & 0x7FF),
           (int)((dbits >> 52) & 0x7FF) - 1023);

    printf("\nIEEE 754 编码公式:\n");
    printf("  值 = (-1)^sign × 1.mantissa × 2^(exponent - bias)\n");
    printf("  float bias = 127, double bias = 1023\n");
}

void demo_precision_traps(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  浮点精度陷阱\n");
    printf("═══════════════════════════════════════\n\n");

    printf("陷阱1: 0.1 无法精确表示\n");
    double d1 = 0.1;
    printf("  0.1 的实际存储值: %.17f\n", d1);
    printf("  0.1 + 0.2 的结果: %.17f\n", 0.1 + 0.2);
    printf("  0.1 + 0.2 == 0.3? %s\n",
           (0.1 + 0.2 == 0.3) ? "是" : "否! (经典陷阱)");

    printf("\n陷阱2: 累积误差\n");
    float sum = 0.0f;
    for (int i = 0; i < 10000; i++) {
        sum += 0.1f;
    }
    printf("  10000 × 0.1f = %.6f (期望1000.0)\n", sum);
    printf("  误差: %.6f\n", sum - 1000.0f);

    printf("\n陷阱3: 大数吃小数\n");
    float big = 1.0e10f;
    float small = 1.0f;
    float result = big + small;
    printf("  %.0f + %.0f = %.0f (小数被吞掉了!)\n", big, small, result);
    printf("  原因: float只有约7位有效数字，1e10+1无法区分\n");

    printf("\n陷阱4: 不同运算顺序结果不同\n");
    float a = 1.0e8f;
    float b = 1.0f;
    float c = -1.0e8f;
    printf("  (a + b) + c = %.1f\n", (a + b) + c);
    printf("  a + (b + c) = %.1f\n", a + (b + c));
    printf("  数学上应该相等，但浮点运算不满足结合律!\n");

    printf("\n陷阱5: float vs double 精度差异\n");
    float pi_f = 3.14159265358979323846f;
    double pi_d = 3.14159265358979323846;
    printf("  float:  %.20f\n", pi_f);
    printf("  double: %.20f\n", pi_d);
}

static int nearly_equal(double x, double y, double eps) {
    double d = fabs(x - y);
    double m = fmax(fabs(x), fabs(y));
    return d <= m * eps || d < DBL_MIN;
}

void demo_float_comparison(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  浮点数比较的正确方法\n");
    printf("═══════════════════════════════════════\n\n");

    double a = 0.1 + 0.2;
    double b = 0.3;

    printf("错误方法: 直接用 == 比较\n");
    printf("  0.1+0.2 == 0.3 ? %s\n", a == b ? "true" : "false");

    printf("\n方法1: 绝对误差 (适用于值接近0的情况)\n");
    double abs_epsilon = 1e-9;
    if (fabs(a - b) < abs_epsilon) {
        printf("  |%.17f - %.17f| < 1e-9 → 相等\n", a, b);
    }

    printf("\n方法2: 相对误差 (通用方法)\n");
    double rel_epsilon = 1e-9;
    double diff = fabs(a - b);
    double larger = fmax(fabs(a), fabs(b));
    if (diff <= larger * rel_epsilon) {
        printf("  相对误差足够小 → 相等\n");
    }

    printf("\n方法3: 结合绝对和相对误差 (最推荐)\n");
    if (nearly_equal(a, b, 1e-9)) {
        printf("  nearly_equal(0.1+0.2, 0.3, 1e-9) → true\n");
    }

    printf("\n⚠️ 永远不要用 == 比较浮点数!\n");
}

void demo_epsilon_detail(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  Epsilon 详解\n");
    printf("═══════════════════════════════════════\n\n");

    printf("FLT_EPSILON = %.8e\n", FLT_EPSILON);
    printf("DBL_EPSILON = %.16e\n", DBL_EPSILON);

    printf("\nEpsilon的含义: 1.0与下一个可表示浮点数之间的差\n");
    float one = 1.0f;
    float next_one = one + FLT_EPSILON;
    float between = one + FLT_EPSILON / 2.0f;
    printf("  1.0f = %.8f\n", one);
    printf("  1.0f + FLT_EPSILON = %.8f (与1.0不同)\n", next_one);
    printf("  1.0f + FLT_EPSILON/2 = %.8f (可能等于1.0!)\n", between);
    printf("  1.0f == 1.0f+FLT_EPSILON/2 ? %s\n",
           one == between ? "true (被舍入回1.0)" : "false");

    printf("\nEpsilon随数值大小变化:\n");
    float vals[] = {1.0f, 10.0f, 100.0f, 1000.0f, 10000.0f};
    for (int i = 0; i < 5; i++) {
        float v = vals[i];
        float next = nextafterf(v, INFINITY);
        printf("  %.0f 与下一个可表示数的差: %.8e (≈ %.0f × FLT_EPSILON)\n",
               v, next - v, v);
    }

    printf("\n结论: 浮点数的精度与数值大小成正比\n");
    printf("  数值越大，相邻可表示数之间的间距越大\n");
}

void demo_float_best_practices(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  浮点数最佳实践\n");
    printf("═══════════════════════════════════════\n\n");

    printf("1. 优先使用double而非float\n");
    printf("   float:   ~7位有效数字\n");
    printf("   double:  ~15位有效数字\n");
    printf("   现代CPU上double运算不一定比float慢\n\n");

    printf("2. 避免浮点数做循环计数器\n");
    printf("   ✗ for(float f=0.0; f!=1.0; f+=0.1) // 可能死循环!\n");
    printf("   ✓ for(int i=0; i<=10; i++) { float f=i*0.1f; }\n\n");

    printf("3. 金融计算不要用浮点数\n");
    printf("   ✗ float price = 19.99; // 19.99无法精确表示\n");
    printf("   ✓ 用整数(分): int price_cents = 1999; // 精确\n\n");

    printf("4. 注意运算顺序\n");
    printf("   ✓ 先加小数，后加大数 (减少大数吃小数)\n");
    printf("   ✓ 使用Kahan求和算法减少累积误差\n\n");

    printf("5. Kahan求和算法示例:\n");
    float kahan_sum = 0.0f;
    float compensation = 0.0f;
    for (int i = 0; i < 10000; i++) {
        float y = 0.1f - compensation;
        float t = kahan_sum + y;
        compensation = (t - kahan_sum) - y;
        kahan_sum = t;
    }
    printf("   普通求和: 10000×0.1 = 999.999847 (有误差)\n");
    printf("   Kahan求和: 10000×0.1 = %.6f (精度更高)\n", kahan_sum);
}

int main(void) {
    demo_ieee754_structure();
    demo_precision_traps();
    demo_float_comparison();
    demo_epsilon_detail();
    demo_float_best_practices();

    return 0;
}
