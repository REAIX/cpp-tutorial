/** @file 02_deep_dive_bitwise_tricks.c
 *  @brief 位运算实战技巧：标志位、掩码、无临时变量交换等
 *  @description 对应文档: 02-operators | 实用位运算技巧、标志位管理、位域、性能优化
 *  编译命令: gcc -std=c17 02_deep_dive_bitwise_tricks.c -o 02_deep_dive_bitwise_tricks
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void demo_bit_flags(void) {
    printf("═══════════════════════════════════════\n");
    printf("  位标志(Flags)模式\n");
    printf("═══════════════════════════════════════\n\n");

    #define FLAG_READ    (1 << 0)
    #define FLAG_WRITE   (1 << 1)
    #define FLAG_EXECUTE (1 << 2)
    #define FLAG_DELETE  (1 << 3)

    unsigned int permissions = 0;

    printf("设置权限标志:\n");
    permissions |= FLAG_READ | FLAG_WRITE;
    printf("  READ | WRITE = 0x%04X\n", permissions);

    printf("\n检查是否有某权限:\n");
    printf("  有READ权限?   %s\n", (permissions & FLAG_READ) ? "是" : "否");
    printf("  有EXECUTE权限? %s\n", (permissions & FLAG_EXECUTE) ? "是" : "否");

    printf("\n添加权限:\n");
    permissions |= FLAG_EXECUTE;
    printf("  添加EXECUTE后 = 0x%04X\n", permissions);

    printf("\n移除权限:\n");
    permissions &= ~FLAG_WRITE;
    printf("  移除WRITE后 = 0x%04X\n", permissions);

    printf("\n切换权限(有则移除，无则添加):\n");
    permissions ^= FLAG_DELETE;
    printf("  切换DELETE后 = 0x%04X (添加了DELETE)\n", permissions);
    permissions ^= FLAG_DELETE;
    printf("  再切换DELETE后 = 0x%04X (移除了DELETE)\n", permissions);

    printf("\n举一反三 —— 位标志的优势:\n");
    printf("  1. 节省内存: 32个标志只需1个int\n");
    printf("  2. 高效: 一次操作可设置/检查多个标志\n");
    printf("  3. 常见于: 文件权限、选项标志、硬件寄存器\n");
}

void demo_bit_masks(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  位掩码(Mask)操作\n");
    printf("═══════════════════════════════════════\n\n");

    uint32_t value = 0xABCDEF12;

    printf("原值: 0x%08X\n\n", value);

    printf("1. 提取低8位(低字节):\n");
    uint32_t low_byte = value & 0xFF;
    printf("   value & 0xFF = 0x%02X\n", low_byte);

    printf("\n2. 提取高8位(高字节):\n");
    uint32_t high_byte = (value >> 24) & 0xFF;
    printf("   (value >> 24) & 0xFF = 0x%02X\n", high_byte);

    printf("\n3. 提取中间16位:\n");
    uint32_t mid_word = (value >> 8) & 0xFFFF;
    printf("   (value >> 8) & 0xFFFF = 0x%04X\n", mid_word);

    printf("\n4. 设置某几位为指定值:\n");
    uint32_t modified = value & ~0xFF00;
    modified |= (0x55 << 8);
    printf("   将第8-15位设为0x55: 0x%08X\n", modified);

    printf("\n5. 构造掩码:\n");
    printf("   低n位掩码: (1 << n) - 1\n");
    printf("   低4位掩码: (1 << 4) - 1 = 0x%X\n", (1 << 4) - 1);
    printf("   低8位掩码: (1 << 8) - 1 = 0x%X\n", (1 << 8) - 1);
}

void demo_swap_without_temp(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  无临时变量交换\n");
    printf("═══════════════════════════════════════\n\n");

    printf("方法1: 异或交换法\n");
    int a = 100, b = 200;
    printf("  交换前: a=%d, b=%d\n", a, b);
    a ^= b;
    b ^= a;
    a ^= b;
    printf("  交换后: a=%d, b=%d\n", a, b);

    printf("\n异或交换的原理:\n");
    printf("  a ^= b → a = a^b\n");
    printf("  b ^= a → b = b^(a^b) = a\n");
    printf("  a ^= b → a = (a^b)^a = b\n");

    printf("\n方法2: 加减交换法\n");
    int x = 10, y = 20;
    printf("  交换前: x=%d, y=%d\n", x, y);
    x = x + y;
    y = x - y;
    x = x - y;
    printf("  交换后: x=%d, y=%d\n", x, y);

    printf("\n⚠️ 评价: 这些技巧在实际编程中不推荐!\n");
    printf("  1. 可读性差\n");
    printf("  2. 异或交换: a和b指向同一地址时结果为0 (bug!)\n");
    printf("  3. 加减交换: 可能溢出\n");
    printf("  4. 现代编译器对临时变量交换有专门优化\n");
    printf("  推荐写法: int tmp=a; a=b; b=tmp;\n");
}

void demo_bit_counting(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  位计数技巧\n");
    printf("═══════════════════════════════════════\n\n");

    printf("1. 统计1的个数(Popcount):\n");
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    unsigned int n = 0b11010110;
#pragma GCC diagnostic pop
    unsigned int temp = n;
    int count = 0;
    while (temp) {
        count += temp & 1;
        temp >>= 1;
    }
    printf("   0b%08X 中有 %d 个1\n", n, count);

    printf("\n   Brian Kernighan算法(更高效):\n");
    temp = n;
    count = 0;
    while (temp) {
        temp &= temp - 1;
        count++;
    }
    printf("   0b%08X 中有 %d 个1 (只循环1的个数次)\n", n, count);

    printf("\n2. 判断2的幂:\n");
    unsigned int vals[] = {1, 2, 4, 8, 16, 3, 6, 12};
    for (int i = 0; i < 8; i++) {
        int is_power2 = vals[i] > 0 && (vals[i] & (vals[i] - 1)) == 0;
        printf("   %2u 是2的幂? %s\n", vals[i], is_power2 ? "是" : "否");
    }
    printf("   原理: 2的幂的二进制只有一个1，n & (n-1) == 0\n");

    printf("\n3. 判断奇偶(比%%2更快):\n");
    for (int i = 0; i <= 5; i++) {
        printf("   %d 是%s数\n", i, (i & 1) ? "奇" : "偶");
    }

    printf("\n4. 快速乘除2的幂:\n");
    int val = 13;
    printf("   %d × 4 = %d (val << 2)\n", val, val << 2);
    printf("   %d / 4 = %d (val >> 2)\n", val, val >> 2);
}

void demo_bit_fields_example(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  位域(Bit Fields)\n");
    printf("═══════════════════════════════════════\n\n");

    struct TCPHeader {
        unsigned int src_port : 16;
        unsigned int dst_port : 16;
        unsigned int seq_num : 32;
        unsigned int data_offset : 4;
        unsigned int reserved : 3;
        unsigned int ns : 1;
        unsigned int cwr : 1;
        unsigned int ece : 1;
        unsigned int urg : 1;
        unsigned int ack : 1;
        unsigned int psh : 1;
        unsigned int rst : 1;
        unsigned int syn : 1;
        unsigned int fin : 1;
    };

    printf("位域允许在结构体中指定成员的位宽:\n");
    printf("  struct Flags {\n");
    printf("      unsigned int read    : 1;  // 1位\n");
    printf("      unsigned int write   : 1;  // 1位\n");
    printf("      unsigned int execute : 1;  // 1位\n");
    printf("      unsigned int reserved: 29; // 填充\n");
    printf("  };\n\n");

    struct Flags {
        unsigned int read : 1;
        unsigned int write : 1;
        unsigned int execute : 1;
        unsigned int reserved : 29;
    };

    struct Flags f = {1, 0, 1, 0};
    printf("  read=%u, write=%u, execute=%u\n", f.read, f.write, f.execute);
    printf("  sizeof(struct Flags) = %zu 字节\n", sizeof(f));

    printf("\n位域的注意事项:\n");
    printf("  ✓ 节省内存，适合协议头、硬件寄存器映射\n");
    printf("  ✗ 位域的内存布局依赖编译器(不可移植)\n");
    printf("  ✗ 不能对位域成员取地址\n");
    printf("  ✗ int位域的符号性由实现定义\n");
    printf("  ✗ 跨平台通信时避免使用位域\n");
}

void demo_practical_patterns(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  实用位运算模式\n");
    printf("═══════════════════════════════════════\n\n");

    printf("1. 对齐到2的幂:\n");
    unsigned int size = 17;
    unsigned int aligned = (size + 3) & ~3;
    printf("   %u 对齐到4字节: %u\n", size, aligned);

    size = 33;
    aligned = (size + 7) & ~7;
    printf("   %u 对齐到8字节: %u\n", size, aligned);

    printf("\n2. 提取RGB颜色分量:\n");
    uint32_t color = 0xFF8040;
    uint8_t red   = (color >> 16) & 0xFF;
    uint8_t green = (color >> 8) & 0xFF;
    uint8_t blue  = color & 0xFF;
    printf("   0x%06X → R=%d, G=%d, B=%d\n", color, red, green, blue);

    printf("\n3. 组合RGB颜色:\n");
    uint32_t new_color = ((uint32_t)128 << 16) | ((uint32_t)200 << 8) | 64;
    printf("   R=128, G=200, B=64 → 0x%06X\n", new_color);

    printf("\n4. 最低有效位(Least Significant Bit):\n");
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    uint32_t v = 0b10110000;
#pragma GCC diagnostic pop
    uint32_t lsb = v & (-v);
    printf("   0b%08X 的最低有效位: 0b%08X\n", v, lsb);
    printf("   原理: -v = ~v + 1 (补码)，与原值取与得到最低1\n");

    printf("\n5. 向上取整到2的幂:\n");
    unsigned int nums[] = {5, 13, 100, 255, 256};
    for (int i = 0; i < 5; i++) {
        unsigned int n = nums[i];
        unsigned int p = 1;
        while (p < n) p <<= 1;
        printf("   %3u → %u\n", n, p);
    }
}

int main(void) {
    demo_bit_flags();
    demo_bit_masks();
    demo_swap_without_temp();
    demo_bit_counting();
    demo_bit_fields_example();
    demo_practical_patterns();

    return 0;
}
