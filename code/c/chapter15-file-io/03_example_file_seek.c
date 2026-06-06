/**
 * @file 03_example_file_seek.c
 * @brief 文件定位与随机访问
 * @description 对应文档: 15-文件操作
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *SEEK_FILE = "test_seek.dat";

void demo_fseek_ftell(void) {
    printf("=== fseek/ftell 文件定位 ===\n");
    FILE *f = fopen(SEEK_FILE, "wb");
    if (!f) return;
    const char *data = "ABCDEFGHIJ";
    fwrite(data, 1, strlen(data), f);
    fclose(f);

    f = fopen(SEEK_FILE, "rb");
    if (!f) return;

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    printf("  文件大小: %ld 字节\n", file_size);

    fseek(f, 3, SEEK_SET);
    int ch = fgetc(f);
    printf("  SEEK_SET+3: '%c' (第4个字节)\n", ch);

    fseek(f, -2, SEEK_CUR);
    ch = fgetc(f);
    printf("  SEEK_CUR-2: '%c' (回退2字节后读取)\n", ch);

    fseek(f, -1, SEEK_END);
    ch = fgetc(f);
    printf("  SEEK_END-1: '%c' (最后一个字节)\n", ch);

    fclose(f);
    printf("\n");
}

void demo_rewind(void) {
    printf("=== rewind 回到文件开头 ===\n");
    FILE *f = fopen(SEEK_FILE, "rb");
    if (!f) return;

    fseek(f, 5, SEEK_SET);
    printf("  当前位置: %ld\n", ftell(f));

    rewind(f);
    printf("  rewind后位置: %ld\n", ftell(f));
    printf("  rewind等价于 fseek(f, 0, SEEK_SET), 但同时清除错误标志\n\n");
    fclose(f);
}

void demo_random_access(void) {
    printf("=== 随机访问(模拟索引文件) ===\n");

    typedef struct {
        int id;
        double value;
    } record_t;

    record_t records[5] = {
        {101, 1.1}, {202, 2.2}, {303, 3.3}, {404, 4.4}, {505, 5.5}
    };

    FILE *f = fopen(SEEK_FILE, "wb");
    if (!f) return;
    fwrite(records, sizeof(record_t), 5, f);
    fclose(f);

    f = fopen(SEEK_FILE, "rb");
    if (!f) return;

    int indices[] = {4, 1, 3};
    for (int i = 0; i < 3; i++) {
        int idx = indices[i];
        fseek(f, (long)idx * sizeof(record_t), SEEK_SET);
        record_t r;
        if (fread(&r, sizeof(record_t), 1, f) == 1) {
            printf("  记录[%d]: id=%d, value=%.1f\n", idx, r.id, r.value);
        }
    }

    fclose(f);
    printf("  应用: 数据库索引、固定长度记录文件\n\n");
}

void demo_file_position_pitfall(void) {
    printf("=== 文件定位陷阱 ===\n");
    printf("  1. fseek使用SEEK_END时, 二进制文件的偏移不保证可移植\n");
    printf("  2. 文本模式下fseek的偏移必须是ftell的返回值或0\n");
    printf("  3. ftell返回long, 32位系统上大文件(>2GB)会溢出\n");
    printf("     大文件应使用fseeko/ftello(off_t类型)\n");
    printf("  4. 读写模式切换时必须插入fseek/fsetpos/rewind\n");
    printf("     (C标准要求在写操作后读之前, 或读操作后写之前定位)\n\n");
}

void demo_fgetpos_fsetpos(void) {
    printf("=== fgetpos/fsetpos (可移植定位) ===\n");
    FILE *f = fopen(SEEK_FILE, "rb");
    if (!f) return;

    fpos_t pos;
    fseek(f, 3, SEEK_SET);
    fgetpos(f, &pos);
    printf("  保存位置3: '%c'\n", fgetc(f));

    fseek(f, 7, SEEK_SET);
    printf("  移动到位置7: '%c'\n", fgetc(f));

    fsetpos(f, &pos);
    printf("  恢复到位置3: '%c'\n", fgetc(f));

    fclose(f);
    printf("  fpos_t比long更通用, 适合大文件和多字节编码文件\n\n");
}

int main(void) {
    printf("========== 文件定位与随机访问示例 ==========\n\n");

    demo_fseek_ftell();
    demo_rewind();
    demo_random_access();
    demo_file_position_pitfall();
    demo_fgetpos_fsetpos();

    remove(SEEK_FILE);
    printf("========== 所有演示完成 ==========\n");
    return 0;
}
