/**
 * @file 02_deep_dive_bitwise_applications.c
 * @brief 位操作实际应用
 * @description 对应文档: 12-位操作实战
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static uint32_t g_bloom_bitmap[16];
static int g_bloom_bit_count = 16 * 32;

static uint32_t hash1(const char *s) {
    uint32_t h = 5381;
    while (*s) h = h * 33 + (unsigned char)*s++;
    return h;
}

static uint32_t hash2(const char *s) {
    uint32_t h = 0;
    while (*s) h = h * 31 + (unsigned char)*s++;
    return h;
}

static void bloom_set_bit(int idx) {
    idx %= g_bloom_bit_count;
    g_bloom_bitmap[idx / 32] |= (1u << (idx % 32));
}

static int bloom_get_bit(int idx) {
    idx %= g_bloom_bit_count;
    return (g_bloom_bitmap[idx / 32] >> (idx % 32)) & 1;
}

static uint32_t g_bm_bitmap[4];
static int g_bm_total_bits = 4 * 32;

static void bm_set(int idx) {
    if (idx >= 0 && idx < g_bm_total_bits)
        g_bm_bitmap[idx / 32] |= (1u << (idx % 32));
}

static void __attribute__((used)) bm_clear(int idx) {
    if (idx >= 0 && idx < g_bm_total_bits)
        g_bm_bitmap[idx / 32] &= ~(1u << (idx % 32));
}

static int bm_test(int idx) {
    if (idx >= 0 && idx < g_bm_total_bits)
        return (g_bm_bitmap[idx / 32] >> (idx % 32)) & 1;
    return 0;
}

void demo_permission_flags(void) {
    printf("=== 权限标志系统(Unix风格) ===\n");

    typedef enum {
        PERM_READ    = 1 << 0,
        PERM_WRITE   = 1 << 1,
        PERM_EXECUTE = 1 << 2,
        PERM_DELETE  = 1 << 3,
        PERM_SHARE   = 1 << 4,
        PERM_ADMIN   = 1 << 5,
    } permission_t;

    (void)sizeof(permission_t);

    uint32_t user_perms = PERM_READ | PERM_WRITE;
    uint32_t admin_perms = PERM_READ | PERM_WRITE | PERM_EXECUTE | PERM_DELETE | PERM_SHARE | PERM_ADMIN;

    printf("  普通用户权限: 0x%02X\n", user_perms);
    printf("    读取: %s\n", (user_perms & PERM_READ) ? "是" : "否");
    printf("    写入: %s\n", (user_perms & PERM_WRITE) ? "是" : "否");
    printf("    执行: %s\n", (user_perms & PERM_EXECUTE) ? "是" : "否");

    user_perms |= PERM_EXECUTE;
    printf("  添加执行权限后: 0x%02X\n", user_perms);

    user_perms &= ~PERM_WRITE;
    printf("  移除写入权限后: 0x%02X\n", user_perms);

    user_perms ^= PERM_SHARE;
    printf("  切换分享权限后: 0x%02X\n", user_perms);

    int has_admin = (admin_perms & (PERM_READ | PERM_WRITE)) == (PERM_READ | PERM_WRITE);
    printf("  管理员同时有读写权限: %s\n\n", has_admin ? "是" : "否");
}

void demo_simple_compression(void) {
    printf("=== 位压缩: 多个小值打包到一个整数 ===\n");

    struct packed_date {
        uint32_t day   : 5;
        uint32_t month : 4;
        uint32_t year  : 12;
        uint32_t       : 11;
    };

    struct packed_date d;
    d.day = 29;
    d.month = 5;
    d.year = 2026;

    printf("  日期: %u年%u月%u日\n", d.year, d.month, d.day);
    printf("  sizeof(packed_date) = %zu 字节 (vs 普通struct需12字节)\n\n", sizeof(d));
}

void demo_crc_basics(void) {
    printf("=== CRC校验基础概念 ===\n");

    uint8_t data[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F};
    uint8_t crc = 0;
    uint8_t polynomial = 0x07;

    for (int i = 0; i < (int)sizeof(data); i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x80)
                crc = (crc << 1) ^ polynomial;
            else
                crc <<= 1;
        }
    }
    printf("  数据: \"Hello\" -> CRC-8 = 0x%02X\n", crc);
    printf("  原理: 将数据视为多项式, 对生成多项式做模2除法\n\n");
}

void demo_bloom_filter_concept(void) {
    printf("=== 布隆过滤器概念演示 ===\n");

    memset(g_bloom_bitmap, 0, sizeof(g_bloom_bitmap));

    const char *items[] = {"hello", "world", "test"};
    for (int i = 0; i < 3; i++) {
        uint32_t h1 = hash1(items[i]);
        uint32_t h2 = hash2(items[i]);
        bloom_set_bit((int)h1);
        bloom_set_bit((int)h2);
        printf("  添加 \"%s\": hash1=%u, hash2=%u\n", items[i], h1 % (unsigned)g_bloom_bit_count, h2 % (unsigned)g_bloom_bit_count);
    }

    const char *checks[] = {"hello", "missing", "world", "nope"};
    for (int i = 0; i < 4; i++) {
        uint32_t h1 = hash1(checks[i]);
        uint32_t h2 = hash2(checks[i]);
        int found = bloom_get_bit((int)h1) && bloom_get_bit((int)h2);
        printf("  查询 \"%s\": %s(可能有假阳性)\n", checks[i], found ? "可能存在" : "一定不存在");
    }
    printf("\n");
}

void demo_bitmap(void) {
    printf("=== 位图(Bitmap)集合操作 ===\n");

    memset(g_bm_bitmap, 0, sizeof(g_bm_bitmap));

    int elements[] = {3, 7, 15, 42, 100, 127};
    for (int i = 0; i < (int)(sizeof(elements) / sizeof(elements[0])); i++) {
        bm_set(elements[i]);
    }

    printf("  添加元素: ");
    for (int i = 0; i < (int)(sizeof(elements) / sizeof(elements[0])); i++)
        printf("%d ", elements[i]);
    printf("\n");

    printf("  测试存在: ");
    for (int i = 0; i < 130; i++) {
        if (bm_test(i)) printf("%d ", i);
    }
    printf("\n");

    int count = 0;
    for (int i = 0; i < g_bm_total_bits; i++) {
        if (bm_test(i)) count++;
    }
    printf("  位图中1的个数: %d\n", count);
    printf("  内存占用: %zu 字节 (vs 布尔数组需 %d 字节)\n\n",
           sizeof(g_bm_bitmap), g_bm_total_bits);
}

int main(void) {
    printf("========== 位操作实际应用 ==========\n\n");

    demo_permission_flags();
    demo_simple_compression();
    demo_crc_basics();
    demo_bloom_filter_concept();
    demo_bitmap();

    printf("========== 所有演示完成 ==========\n");
    return 0;
}
