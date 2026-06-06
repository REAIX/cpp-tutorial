/**
 * @file 02_example_binary_file.c
 * @brief 二进制文件读写
 * @description 对应文档: 15-文件操作
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *BIN_FILE = "test_binary.dat";

typedef struct {
    int id;
    char name[32];
    double score;
} student_t;

void demo_fwrite_fread(void) {
    printf("=== fwrite/fread 二进制读写 ===\n");
    student_t students[] = {
        {1, "Alice",   89.5},
        {2, "Bob",     95.0},
        {3, "Charlie", 72.3},
    };
    int count = 3;

    FILE *f = fopen(BIN_FILE, "wb");
    if (!f) { perror("  fopen"); return; }
    size_t written = fwrite(students, sizeof(student_t), count, f);
    printf("  写入 %zu 条记录 (每条 %zu 字节)\n", written, sizeof(student_t));
    fclose(f);

    student_t read_back[3] = {0};
    f = fopen(BIN_FILE, "rb");
    if (!f) { perror("  fopen"); return; }
    size_t read_count = fread(read_back, sizeof(student_t), count, f);
    printf("  读取 %zu 条记录:\n", read_count);
    for (size_t i = 0; i < read_count; i++) {
        printf("    id=%d, name=\"%s\", score=%.1f\n",
               read_back[i].id, read_back[i].name, read_back[i].score);
    }
    fclose(f);
    printf("\n");
}

void demo_binary_mode(void) {
    printf("=== 文本模式 vs 二进制模式 ===\n");
    const char *TEXT_FILE = "test_mode.txt";

    unsigned char data[] = {0x0A, 0x0D, 0x41, 0x42, 0x00, 0x43};
    int len = 6;

    FILE *ft = fopen(TEXT_FILE, "w");
    if (ft) {
        size_t w = fwrite(data, 1, len, ft);
        fclose(ft);
        printf("  文本模式写入 %zu 字节\n", w);
    }

    FILE *fb = fopen(BIN_FILE, "wb");
    if (fb) {
        size_t w = fwrite(data, 1, len, fb);
        fclose(fb);
        printf("  二进制模式写入 %zu 字节\n", w);
    }

    ft = fopen(TEXT_FILE, "r");
    if (ft) {
        fseek(ft, 0, SEEK_END);
        long sz = ftell(ft);
        fclose(ft);
        printf("  文本模式文件大小: %ld 字节\n", sz);
    }

    fb = fopen(BIN_FILE, "rb");
    if (fb) {
        fseek(fb, 0, SEEK_END);
        long sz = ftell(fb);
        fclose(fb);
        printf("  二进制模式文件大小: %ld 字节\n", sz);
    }

    printf("  差异原因: Windows文本模式中 \\n(0x0A) 被扩展为 \\r\\n\n");
    printf("  建议: 处理非文本数据时始终用 \"b\" 模式\n\n");
    remove(TEXT_FILE);
}

void demo_struct_serialization(void) {
    printf("=== 结构体序列化注意事项 ===\n");
    printf("  直接fwrite结构体的陷阱:\n");
    printf("    1. 内存对齐填充字节不确定(不同编译器可能不同)\n");
    printf("    2. 字节序(大小端)在不同平台可能不同\n");
    printf("    3. 指针成员不能直接序列化\n");
    printf("    4. 浮点数表示可能不同\n\n");

    printf("  安全的序列化方法:\n");
    printf("    - 逐字段写入, 明确字节序\n");
    printf("    - 使用固定大小的整数类型(uint32_t等)\n");
    printf("    - 使用JSON/Protocol Buffers等格式\n\n");

    student_t s = {42, "Test", 100.0};
    FILE *f = fopen(BIN_FILE, "wb");
    if (f) {
        fwrite(&s.id, sizeof(int), 1, f);
        fwrite(s.name, 1, sizeof(s.name), f);
        fwrite(&s.score, sizeof(double), 1, f);
        fclose(f);
        printf("  逐字段序列化写入完成 (更可移植)\n");
    }
    printf("\n");
}

void demo_array_serialization(void) {
    printf("=== 数组批量序列化 ===\n");
    int data[10];
    for (int i = 0; i < 10; i++) data[i] = i * 100;

    FILE *f = fopen(BIN_FILE, "wb");
    if (f) {
        fwrite(data, sizeof(int), 10, f);
        fclose(f);
    }

    int read_data[10] = {0};
    f = fopen(BIN_FILE, "rb");
    if (f) {
        size_t n = fread(read_data, sizeof(int), 10, f);
        fclose(f);
        printf("  读取 %zu 个int: ", n);
        for (size_t i = 0; i < n; i++) printf("%d ", read_data[i]);
        printf("\n\n");
    }
}

int main(void) {
    printf("========== 二进制文件读写示例 ==========\n\n");

    demo_fwrite_fread();
    demo_binary_mode();
    demo_struct_serialization();
    demo_array_serialization();

    remove(BIN_FILE);
    printf("========== 所有演示完成 ==========\n");
    return 0;
}
