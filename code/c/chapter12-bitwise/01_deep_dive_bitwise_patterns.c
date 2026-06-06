/**
 * @file 01_deep_dive_bitwise_patterns.c
 * @brief 位操作经典模式与面试题
 * @description 对应文档: 12-位操作实战
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

static void print_bin8(uint8_t val) {
    for (int i = 7; i >= 0; i--) putchar((val >> i) & 1 ? '1' : '0');
}

static void __attribute__((used)) print_bin32(uint32_t val) {
    for (int i = 31; i >= 0; i--) putchar((val >> i) & 1 ? '1' : '0');
}

void demo_xor_swap(void) {
    printf("=== XOR交换(不用临时变量) ===\n");
    int a = 42, b = 99;
    printf("  交换前: a=%d, b=%d\n", a, b);

    a ^= b;
    b ^= a;
    a ^= b;
    printf("  交换后: a=%d, b=%d\n", a, b);

    printf("  陷阱: 如果a和b指向同一地址, XOR交换会清零!\n");
    int arr[] = {10};
    int *p1 = &arr[0], *p2 = &arr[0];
    printf("  同一变量XOR交换前: *p1=%d\n", *p1);
    *p1 ^= *p2;
    *p2 ^= *p1;
    *p1 ^= *p2;
    printf("  同一变量XOR交换后: *p1=%d (被清零了!)\n\n", *p1);
}

void demo_find_single_number(void) {
    printf("=== 找出唯一出现一次的数 ===\n");
    int nums[] = {4, 1, 2, 1, 2};
    int n = sizeof(nums) / sizeof(nums[0]);

    int result = 0;
    for (int i = 0; i < n; i++) {
        result ^= nums[i];
    }
    printf("  数组: ");
    for (int i = 0; i < n; i++) printf("%d ", nums[i]);
    printf("\n  唯一出现一次的数: %d\n", result);
    printf("  原理: a ^ a = 0, 0 ^ b = b, 异或满足交换律\n\n");
}

void demo_find_two_single_numbers(void) {
    printf("=== 找出两个只出现一次的数 ===\n");
    int nums[] = {1, 2, 1, 3, 2, 5};
    int n = sizeof(nums) / sizeof(nums[0]);

    int xor_all = 0;
    for (int i = 0; i < n; i++) xor_all ^= nums[i];

    int rightmost_set_bit = xor_all & (-xor_all);

    int num1 = 0, num2 = 0;
    for (int i = 0; i < n; i++) {
        if (nums[i] & rightmost_set_bit)
            num1 ^= nums[i];
        else
            num2 ^= nums[i];
    }
    printf("  数组: ");
    for (int i = 0; i < n; i++) printf("%d ", nums[i]);
    printf("\n  两个唯一数: %d 和 %d\n\n", num1, num2);
}

void demo_count_set_bits(void) {
    printf("=== 统计1的个数(多种方法) ===\n");
    uint32_t val = 0x1B5;

    printf("  值: 0x%X\n", val);

    int count1 = 0;
    uint32_t v = val;
    while (v) {
        count1 += v & 1;
        v >>= 1;
    }
    printf("  方法1(逐位检查): %d个1\n", count1);

    int count2 = 0;
    v = val;
    while (v) {
        count2++;
        v &= (v - 1);
    }
    printf("  方法2(Brian Kernighan): %d个1\n", count2);

    v = val;
    v = v - ((v >> 1) & 0x55555555);
    v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
    v = (v + (v >> 4)) & 0x0F0F0F0F;
    int count3 = (v * 0x01010101) >> 24;
    printf("  方法3(并行计算/查表): %d个1\n\n", count3);
}

void demo_reverse_bits(void) {
    printf("=== 反转位序 ===\n");
    uint32_t val = 0xD0000000;
    printf("  原始: 0x%08X\n", val);

    uint32_t reversed = val;
    reversed = ((reversed >> 1) & 0x55555555) | ((reversed & 0x55555555) << 1);
    reversed = ((reversed >> 2) & 0x33333333) | ((reversed & 0x33333333) << 2);
    reversed = ((reversed >> 4) & 0x0F0F0F0F) | ((reversed & 0x0F0F0F0F) << 4);
    reversed = ((reversed >> 8) & 0x00FF00FF) | ((reversed & 0x00FF00FF) << 8);
    reversed = (reversed >> 16) | (reversed << 16);
    printf("  反转: 0x%08X\n\n", reversed);
}

void demo_parity_check(void) {
    printf("=== 奇偶校验 ===\n");
    uint32_t vals[] = {0xB, 0xA};
    const char *bin_str[] = {"1011", "1010"};
    for (int i = 0; i < 2; i++) {
        uint32_t v = vals[i];
        int parity = 0;
        while (v) {
            parity ^= 1;
            v &= (v - 1);
        }
        printf("  0b%s 的奇偶性: %s(1的个数%s)\n",
               bin_str[i],
               parity ? "奇" : "偶",
               parity ? "为奇数" : "为偶数");
    }
    printf("\n");
}

void demo_power_of_two(void) {
    printf("=== 判断2的幂 ===\n");
    int test_vals[] = {0, 1, 2, 3, 4, 8, 15, 16, -4};
    for (int i = 0; i < (int)(sizeof(test_vals) / sizeof(test_vals[0])); i++) {
        int v = test_vals[i];
        int is_pow2 = (v > 0) && ((v & (v - 1)) == 0);
        printf("  %3d -> %s2的幂\n", v, is_pow2 ? "是" : "不是");
    }
    printf("  原理: 2的幂的二进制只有一个1, n & (n-1) == 0\n\n");
}

void demo_abs_without_branch(void) {
    printf("=== 无分支求绝对值 ===\n");
    int vals[] = {42, -42, 0, -1, 1};
    for (int i = 0; i < (int)(sizeof(vals) / sizeof(vals[0])); i++) {
        int v = vals[i];
        int mask = v >> (sizeof(int) * 8 - 1);
        int abs_val = (v + mask) ^ mask;
        printf("  |%3d| = %d (mask=0x%X)\n", v, abs_val, mask);
    }
    printf("\n");
}

void demo_lowest_set_bit(void) {
    printf("=== 提取最低位的1 ===\n");
    uint32_t vals[] = {0xB0, 0x01, 0x80, 0xAA};
    for (int i = 0; i < (int)(sizeof(vals) / sizeof(vals[0])); i++) {
        uint32_t v = vals[i];
        uint32_t lowest = v & (-v);
        printf("  0b"); print_bin8((uint8_t)v); printf(" -> 最低位1: 0b"); print_bin8((uint8_t)lowest);
        printf(" (位%d)\n", lowest ? __builtin_ctz(v) : -1);
    }
    printf("  原理: n & (-n) 利用补码特性, -n = ~n + 1\n\n");
}

int main(void) {
    printf("========== 位操作经典模式与面试题 ==========\n\n");

    demo_xor_swap();
    demo_find_single_number();
    demo_find_two_single_numbers();
    demo_count_set_bits();
    demo_reverse_bits();
    demo_parity_check();
    demo_power_of_two();
    demo_abs_without_branch();
    demo_lowest_set_bit();

    printf("========== 所有演示完成 ==========\n");
    return 0;
}
