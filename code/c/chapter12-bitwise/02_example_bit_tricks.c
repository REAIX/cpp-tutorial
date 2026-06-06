/**
 * @file 02_example_bit_tricks.c
 * @brief 位操作技巧
 * @description 对应文档: 12-位操作实战
 */
#include <stdio.h>
#include <stdint.h>

static void print_bin8(uint8_t val) {
    for (int i = 7; i >= 0; i--) putchar((val >> i) & 1 ? '1' : '0');
}

void demo_set_bit(void) {
    printf("=== 置位(设置某位为1) ===\n");
    uint8_t reg = 0x00;
    int bit = 3;
    reg |= (1 << bit);
    printf("  设置第%d位: 0b", bit); print_bin8(reg); printf(" (0x%02X)\n", reg);

    reg = 0x00;
    reg |= (1 << 1) | (1 << 3) | (1 << 5);
    printf("  同时设置第1,3,5位: 0b"); print_bin8(reg); printf(" (0x%02X)\n\n", reg);
}

void demo_clear_bit(void) {
    printf("=== 清位(设置某位为0) ===\n");
    uint8_t reg = 0xFF;
    int bit = 4;
    reg &= ~(1 << bit);
    printf("  清除第%d位: 0b", bit); print_bin8(reg); printf(" (0x%02X)\n", reg);

    reg = 0xFF;
    reg &= ~((1 << 0) | (1 << 2) | (1 << 7));
    printf("  同时清除第0,2,7位: 0b"); print_bin8(reg); printf(" (0x%02X)\n\n", reg);
}

void demo_toggle_bit(void) {
    printf("=== 翻转位(0变1, 1变0) ===\n");
    uint8_t reg = 0xAA;
    int bit = 1;
    uint8_t before = reg;
    reg ^= (1 << bit);
    printf("  翻转第%d位: 0b", bit); print_bin8(before); printf(" -> 0b"); print_bin8(reg); printf("\n");

    reg = 0xAA;
    before = reg;
    reg ^= 0xFF;
    printf("  翻转所有位: 0b"); print_bin8(before); printf(" -> 0b"); print_bin8(reg); printf("\n\n");
}

void demo_check_bit(void) {
    printf("=== 检查某位是否为1 ===\n");
    uint8_t reg = 0xB4;
    for (int i = 7; i >= 0; i--) {
        int is_set = (reg >> i) & 1;
        printf("  第%d位 = %d\n", i, is_set);
    }
    printf("\n");
}

void demo_bit_mask(void) {
    printf("=== 位掩码操作 ===\n");
    uint32_t color = 0xFF8040;

    uint8_t red   = (color >> 16) & 0xFF;
    uint8_t green = (color >> 8)  & 0xFF;
    uint8_t blue  = color & 0xFF;
    printf("  颜色 0x%06X: R=%d, G=%d, B=%d\n", color, red, green, blue);

    uint32_t rebuilt = ((uint32_t)red << 16) | ((uint32_t)green << 8) | blue;
    printf("  重建颜色: 0x%06X\n\n", rebuilt);
}

void demo_extract_field(void) {
    printf("=== 提取与设置位域 ===\n");
    uint32_t instr = 0xE3A01005;

    uint32_t cond = (instr >> 28) & 0xF;
    uint32_t opcode = (instr >> 20) & 0xFF;
    uint32_t rn = (instr >> 16) & 0xF;
    uint32_t rd = (instr >> 12) & 0xF;
    uint32_t operand2 = instr & 0xFFF;

    printf("  指令: 0x%08X\n", instr);
    printf("  cond=%X, opcode=%02X, Rn=%X, Rd=%X, operand2=%03X\n\n",
           cond, opcode, rn, rd, operand2);
}

void demo_multi_bit_ops(void) {
    printf("=== 多位同时操作 ===\n");
    uint8_t port = 0xA5;

    uint8_t mask = 0x0F;
    uint8_t cleared = port & ~mask;
    printf("  port=0b"); print_bin8(port); printf(", 清除低4位: 0b"); print_bin8(cleared); printf("\n");

    uint8_t new_low = 0x0A;
    uint8_t combined = cleared | new_low;
    printf("  写入新低4位 0x%02X: 0b", new_low); print_bin8(combined); printf(" (0x%02X)\n\n", combined);
}

int main(void) {
    printf("========== 位操作技巧示例 ==========\n\n");

    demo_set_bit();
    demo_clear_bit();
    demo_toggle_bit();
    demo_check_bit();
    demo_bit_mask();
    demo_extract_field();
    demo_multi_bit_ops();

    printf("========== 所有演示完成 ==========\n");
    return 0;
}
