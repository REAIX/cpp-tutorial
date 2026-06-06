/** @file 02_example_logical_operators.c
 *  @brief 逻辑运算符与位运算符：&&、||、!、&、|、^、~、<<、>>
 *  @description 对应文档: 02-operators | 演示逻辑运算符和位运算符的区别与用法
 *  编译命令: gcc -std=c17 02_example_logical_operators.c -o 02_example_logical_operators
 */

#include <stdio.h>
#include <stdlib.h>

void demo_logical_operators(void) {
    printf("═══════════════════════════════════════\n");
    printf("  逻辑运算符: && || !\n");
    printf("═══════════════════════════════════════\n\n");

    int a = 1, b = 0;

    printf("a = %d (真), b = %d (假)\n\n", a, b);
    printf("a && b = %d (逻辑与: 两者都真才为真)\n", a && b);
    printf("a || b = %d (逻辑或: 有一真即为真)\n", a || b);
    printf("!a     = %d (逻辑非: 取反)\n", !a);
    printf("!b     = %d\n", !b);

    printf("\nC语言真假规则:\n");
    printf("  0 为假，一切非零值为真\n");
    printf("  -1 && 3.14 → %d (负数和浮点数也视为真)\n", -1 && 3.14);
    printf("  0 || 0     → %d\n", 0 || 0);
}

void demo_short_circuit(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  短路求值 (Short-circuit Evaluation)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("&& 的短路: 左操作数为假时，右操作数不求值\n");
    printf("|| 的短路: 左操作数为真时，右操作数不求值\n\n");

    int count = 0;

    count = 0;
    int result1 = (0 && (count++ > 0));
    printf("0 && (count++ > 0) → result=%d, count=%d (右侧未执行!)\n", result1, count);

    count = 0;
    int result2 = (1 || (count++ > 0));
    printf("1 || (count++ > 0) → result=%d, count=%d (右侧未执行!)\n", result2, count);

    printf("\n短路求值的实用场景:\n");

    printf("1. 防止空指针解引用:\n");
    printf("   if (ptr != NULL && *ptr > 0) { ... }\n");
    printf("   ptr为NULL时不会解引用\n\n");

    printf("2. 防止除以零:\n");
    printf("   if (divisor != 0 && result / divisor > threshold) { ... }\n\n");

    printf("3. 防止数组越界:\n");
    printf("   if (index >= 0 && index < size && arr[index] == target) { ... }\n");
}

void demo_bitwise_operators(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  位运算符: & | ^ ~ << >>\n");
    printf("═══════════════════════════════════════\n\n");

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    unsigned char a = 0b11001010;
    unsigned char b = 0b10110101;
#pragma GCC diagnostic pop

    printf("a = 0x%02X (%3u)\n", (unsigned)a, (unsigned)a);
    printf("b = 0x%02X (%3u)\n", (unsigned)b, (unsigned)b);
    printf("\n");

    printf("a & b  = 0x%02X (%3u)  按位与: 两位都为1才为1\n", (unsigned)(a & b), (unsigned)(a & b));
    printf("a | b  = 0x%02X (%3u)  按位或: 有一位为1即为1\n", (unsigned)(a | b), (unsigned)(a | b));
    printf("a ^ b  = 0x%02X (%3u)  按位异或: 不同为1，相同为0\n", (unsigned)(a ^ b), (unsigned)(a ^ b));
    printf("~a     = 0x%02X (%3u)  按位取反\n", (unsigned)(unsigned char)(~a), (unsigned)(unsigned char)(~a));
    printf("a << 2 = 0x%02X (%3u)  左移2位(乘以4)\n", (unsigned)(unsigned char)(a << 2), (unsigned)(unsigned char)(a << 2));
    printf("a >> 3 = 0x%02X (%3u)  右移3位(除以8)\n", (unsigned)(unsigned char)(a >> 3), (unsigned)(unsigned char)(a >> 3));

    printf("\n⚠️ 逻辑运算符 vs 位运算符:\n");
    printf("  && 是逻辑与，结果只有0或1\n");
    printf("  &  是按位与，对每一位分别运算\n");
    printf("  例: 5 && 3 = %d, 5 & 3 = %d\n", 5 && 3, 5 & 3);
    printf("  例: 5 || 3 = %d, 5 | 3 = %d\n", 5 || 3, 5 | 3);
}

void demo_shift_operators(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  移位运算符详解\n");
    printf("═══════════════════════════════════════\n\n");

    unsigned int val = 1;
    printf("左移(<<)相当于乘以2的幂:\n");
    for (int i = 0; i <= 8; i++) {
        printf("  1 << %d = %u (= 2^%d)\n", i, val << i, i);
    }

    printf("\n右移(>>)相当于除以2的幂(向下取整):\n");
    unsigned int v = 1000;
    printf("  %u >> 1 = %u\n", v, v >> 1);
    printf("  %u >> 2 = %u\n", v, v >> 2);
    printf("  %u >> 3 = %u\n", v, v >> 3);

    printf("\n有符号数右移:\n");
    int neg = -8;
    printf("  -8 >> 1 = %d (算术右移: 保留符号位)\n", neg >> 1);
    printf("  注意: 有符号数右移是算术右移还是逻辑右移由实现定义\n");
    printf("  大多数现代编译器采用算术右移(高位补符号位)\n");

    printf("\n移位的未定义行为:\n");
    printf("  ✗ 移位数为负数: x << -1\n");
    printf("  ✗ 移位数>=位宽: int x; x << 32 (32位int)\n");
    printf("  ✗ 对负数左移: -1 << 2 (C99起为UB)\n");
}

void demo_xor_properties(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  异或(^)的特殊性质\n");
    printf("═══════════════════════════════════════\n\n");

    printf("性质1: 任何数与自身异或为0\n");
    int x = 42;
    printf("  %d ^ %d = %d\n", x, x, x ^ x);

    printf("\n性质2: 任何数与0异或不变\n");
    printf("  %d ^ 0 = %d\n", x, x ^ 0);

    printf("\n性质3: 异或满足交换律和结合律\n");
    printf("  a ^ b ^ a = b (可用于简单加密)\n");
    int key = 0x5A;
    int data = 0x41;
    int encrypted = data ^ key;
    int decrypted = encrypted ^ key;
    printf("  原文: 0x%02X, 密钥: 0x%02X\n", data, key);
    printf("  加密: 0x%02X, 解密: 0x%02X\n", encrypted, decrypted);

    printf("\n性质4: 找出唯一不同的数\n");
    int arr[] = {2, 3, 4, 3, 2, 6, 4};
    int n = 7;
    int unique = 0;
    for (int i = 0; i < n; i++) {
        unique ^= arr[i];
    }
    printf("  数组: 2,3,4,3,2,6,4 → 唯一出现一次的数: %d\n", unique);
}

int main(void) {
    demo_logical_operators();
    demo_short_circuit();
    demo_bitwise_operators();
    demo_shift_operators();
    demo_xor_properties();

    return 0;
}
