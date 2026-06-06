/**
 * @file 03_example_bitfield.c
 * @brief 位域与硬件寄存器模拟
 * @description 对应文档: 12-位操作实战
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

static void print_bin8(uint8_t val) {
    for (int i = 7; i >= 0; i--) putchar((val >> i) & 1 ? '1' : '0');
}

void demo_basic_bitfield(void) {
    printf("=== 基本位域 ===\n");

    struct flags {
        unsigned int is_active  : 1;
        unsigned int has_error  : 1;
        unsigned int mode       : 3;
        unsigned int priority   : 2;
        unsigned int reserved   : 1;
    };

    struct flags f = {0};
    f.is_active = 1;
    f.has_error = 0;
    f.mode = 5;
    f.priority = 3;

    printf("  is_active = %u\n", f.is_active);
    printf("  has_error = %u\n", f.has_error);
    printf("  mode      = %u\n", f.mode);
    printf("  priority  = %u\n", f.priority);
    printf("  sizeof(struct flags) = %zu 字节\n\n", sizeof(f));
}

void demo_bitfield_layout(void) {
    printf("=== 位域内存布局 ===\n");

#pragma pack(push, 1)
    struct packed_reg {
        uint8_t enable  : 1;
        uint8_t irq     : 1;
        uint8_t mode    : 2;
        uint8_t channel : 4;
    };
#pragma pack(pop)

    struct packed_reg r = {0};
    r.enable  = 1;
    r.irq     = 0;
    r.mode    = 2;
    r.channel = 9;

    uint8_t raw;
    memcpy(&raw, &r, sizeof(raw));
    printf("  enable=%u, irq=%u, mode=%u, channel=%u\n",
           r.enable, r.irq, r.mode, r.channel);
    printf("  原始字节: 0x%02X (0b", (unsigned)raw); print_bin8(raw); printf(")\n\n");
}

void demo_hardware_register(void) {
    printf("=== 硬件寄存器模拟 ===\n");

    typedef union {
        uint32_t value;
        struct {
            uint32_t tx_enable   : 1;
            uint32_t rx_enable   : 1;
            uint32_t parity_en   : 1;
            uint32_t parity_odd  : 1;
            uint32_t stop_bits   : 1;
            uint32_t data_bits   : 2;
            uint32_t reserved1   : 1;
            uint32_t baud_rate   : 4;
            uint32_t irq_enable  : 1;
            uint32_t reserved2   : 19;
        } bits;
    } uart_ctrl_t;

    uart_ctrl_t ctrl = {0};
    ctrl.bits.tx_enable  = 1;
    ctrl.bits.rx_enable  = 1;
    ctrl.bits.parity_en  = 1;
    ctrl.bits.parity_odd = 0;
    ctrl.bits.stop_bits  = 0;
    ctrl.bits.data_bits  = 3;
    ctrl.bits.baud_rate  = 12;
    ctrl.bits.irq_enable = 1;

    printf("  UART控制寄存器值: 0x%08X\n", ctrl.value);
    printf("  发送使能: %u\n", ctrl.bits.tx_enable);
    printf("  接收使能: %u\n", ctrl.bits.rx_enable);
    printf("  校验使能: %u (偶校验)\n", ctrl.bits.parity_en);
    printf("  数据位数: %u (8位)\n", ctrl.bits.data_bits + 5);
    printf("  波特率码: %u\n", ctrl.bits.baud_rate);
    printf("  中断使能: %u\n\n", ctrl.bits.irq_enable);
}

void demo_bitfield_pitfall(void) {
    printf("=== 位域陷阱 ===\n");

    struct signed_bitfield {
        int32_t flag : 1;
    };

    struct signed_bitfield s;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverflow"
    s.flag = 1;
#pragma GCC diagnostic pop
    printf("  有符号1位位域: flag = %d (不是1! 是-1!)\n", s.flag);
    printf("  原因: 1位有符号数的范围是 [-1, 0], 不是 [0, 1]\n");
    printf("  修正: 使用 unsigned int flag : 1\n\n");

    struct overflow_bitfield {
        uint32_t small : 3;
    };

    struct overflow_bitfield o;
    o.small = 7;
    printf("  3位无符号位域最大值: %u\n", o.small);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverflow"
    o.small = 8;
#pragma GCC diagnostic pop
    printf("  赋值8溢出后: %u (只保留低3位)\n\n", o.small);
}

void demo_network_packet(void) {
    printf("=== 网络包头部位域模拟 ===\n");

    typedef union {
        uint16_t value;
        struct {
            uint16_t version  : 4;
            uint16_t ihl      : 4;
            uint16_t tos      : 8;
        } fields;
    } ip_header_first_word_t;

    ip_header_first_word_t hdr = {0};
    hdr.fields.version = 4;
    hdr.fields.ihl = 5;
    hdr.fields.tos = 0;

    printf("  IPv4头第一个字: 0x%04X\n", hdr.value);
    printf("  版本=%u, IHL=%u, TOS=%u\n\n",
           hdr.fields.version, hdr.fields.ihl, hdr.fields.tos);
}

int main(void) {
    printf("========== 位域与硬件寄存器模拟示例 ==========\n\n");

    demo_basic_bitfield();
    demo_bitfield_layout();
    demo_hardware_register();
    demo_bitfield_pitfall();
    demo_network_packet();

    printf("========== 所有演示完成 ==========\n");
    return 0;
}
