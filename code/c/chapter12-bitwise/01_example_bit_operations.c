/**
 * @file 01_example_bit_operations.c
 * @brief 位操作基础
 * @description 对应文档: 12-位操作实战
 */
#include <stdio.h>
#include <stdint.h>

static void print_bin8(uint8_t val) {
    for (int i = 7; i >= 0; i--) {
        putchar((val >> i) & 1 ? '1' : '0');
    }
}

void demo_bitwise_and(void) {
    printf("=== 按位与 (&) ===\n");
    uint8_t a = 0xCC;
    uint8_t b = 0xAA;
    uint8_t result = a & b;
    printf("  a     = 0b"); print_bin8(a); printf(" (0x%02X)\n", a);
    printf("  b     = 0b"); print_bin8(b); printf(" (0x%02X)\n", b);
    printf("  a & b = 0b"); print_bin8(result); printf(" (0x%02X)\n", result);
    printf("  用途: 清零特定位、取某几位、掩码操作\n\n");
}

void demo_bitwise_or(void) {
    printf("=== 按位或 (|) ===\n");
    uint8_t a = 0xCC;
    uint8_t b = 0xAA;
    uint8_t result = a | b;
    printf("  a     = 0b"); print_bin8(a); printf(" (0x%02X)\n", a);
    printf("  b     = 0b"); print_bin8(b); printf(" (0x%02X)\n", b);
    printf("  a | b = 0b"); print_bin8(result); printf(" (0x%02X)\n", result);
    printf("  用途: 置位(设为1)特定位、合并标志位\n\n");
}

void demo_bitwise_xor(void) {
    printf("=== 按位异或 (^) ===\n");
    uint8_t a = 0xCC;
    uint8_t b = 0xAA;
    uint8_t result = a ^ b;
    printf("  a     = 0b"); print_bin8(a); printf(" (0x%02X)\n", a);
    printf("  b     = 0b"); print_bin8(b); printf(" (0x%02X)\n", b);
    printf("  a ^ b = 0b"); print_bin8(result); printf(" (0x%02X)\n", result);
    printf("  用途: 翻转位、不用临时变量交换、简单加密\n\n");
}

void demo_bitwise_not(void) {
    printf("=== 按位取反 (~) ===\n");
    uint8_t a = 0xCC;
    uint8_t result = ~a;
    printf("  a  = 0b"); print_bin8(a); printf(" (0x%02X)\n", a);
    printf("  ~a = 0b"); print_bin8(result); printf(" (0x%02X)\n", result);
    printf("  注意: 结果依赖数据类型宽度, uint8_t 取反后提升为 int 再截断\n\n");
}

void demo_left_shift(void) {
    printf("=== 左移 (<<) ===\n");
    uint8_t a = 0x0D;
    printf("  a      = 0b"); print_bin8(a); printf(" (0x%02X) = %d\n", a, a);
    uint8_t r1 = a << 1;
    printf("  a << 1 = 0b"); print_bin8(r1); printf(" (0x%02X) = %d (乘以2)\n", r1, r1);
    uint8_t r2 = a << 3;
    printf("  a << 3 = 0b"); print_bin8(r2); printf(" (0x%02X) = %d (乘以8)\n", r2, r2);
    printf("  用途: 快速乘以2的幂, 移位填充0\n\n");
}

void demo_right_shift(void) {
    printf("=== 右移 (>>) ===\n");
    uint8_t a = 0xD0;
    printf("  a      = 0b"); print_bin8(a); printf(" (0x%02X) = %d\n", a, a);
    uint8_t r1 = a >> 1;
    printf("  a >> 1 = 0b"); print_bin8(r1); printf(" (0x%02X) = %d (除以2)\n", r1, r1);
    uint8_t r2 = a >> 4;
    printf("  a >> 4 = 0b"); print_bin8(r2); printf(" (0x%02X) = %d (除以16)\n", r2, r2);

    printf("  --- 有符号右移 vs 无符号右移 ---\n");
    int8_t signed_val = -8;
    uint8_t unsigned_val = 248;
    printf("  int8_t  -8 >> 1 = %d (算术右移, 高位补符号位)\n", signed_val >> 1);
    printf("  uint8_t 248 >> 1 = %d (逻辑右移, 高位补0)\n\n", unsigned_val >> 1);
}

void demo_truth_table(void) {
    printf("=== 位运算真值表 ===\n");
    printf("  a | b | a&b | a|b | a^b | ~a\n");
    printf("  ---+---+-----+-----+-----+----\n");
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            printf("  %d | %d |  %d  |  %d  |  %d  |  %d\n",
                   a, b, a & b, a | b, a ^ b, !a);
        }
    }
    printf("\n");
}

void demo_shift_overflow(void) {
    printf("=== 移位溢出与未定义行为 ===\n");
    uint32_t val = 1;
    printf("  1 << 31 = 0x%08X (符号位被置位, 但无符号类型合法)\n", val << 31);
    printf("  注意: 移位位数 >= 类型宽度是未定义行为!\n");
    printf("  注意: 对有符号负数左移是未定义行为!\n\n");
}

int main(void) {
    printf("========== 位操作基础示例 ==========\n\n");

    demo_bitwise_and();
    demo_bitwise_or();
    demo_bitwise_xor();
    demo_bitwise_not();
    demo_left_shift();
    demo_right_shift();
    demo_truth_table();
    demo_shift_overflow();

    printf("========== 所有演示完成 ==========\n");
    return 0;
}
