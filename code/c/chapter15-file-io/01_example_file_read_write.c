/**
 * @file 01_example_file_read_write.c
 * @brief 文本文件读写
 * @description 对应文档: 15-文件操作
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TEST_FILE = "test_text.txt";

void demo_fopen_fclose(void) {
    printf("=== fopen/fclose ===\n");
    FILE *f = fopen(TEST_FILE, "w");
    if (!f) {
        perror("  fopen失败");
        return;
    }
    printf("  文件已打开: 模式=\"w\" (写入)\n");
    fclose(f);
    printf("  文件已关闭\n\n");

    printf("  打开模式:\n");
    printf("    \"r\"  只读(文件必须存在)\n");
    printf("    \"w\"  只写(清空或创建)\n");
    printf("    \"a\"  追加(文件不存在则创建)\n");
    printf("    \"r+\" 读写(文件必须存在)\n");
    printf("    \"w+\" 读写(清空或创建)\n");
    printf("    \"a+\" 读写追加\n\n");
}

void demo_fputc_fgetc(void) {
    printf("=== fputc/fgetc (单字符) ===\n");
    FILE *f = fopen(TEST_FILE, "w");
    if (!f) return;
    const char *msg = "Hello, 文件操作!";
    for (int i = 0; msg[i] != '\0'; i++) {
        fputc(msg[i], f);
    }
    fclose(f);
    printf("  已写入: \"%s\"\n", msg);

    f = fopen(TEST_FILE, "r");
    if (!f) return;
    printf("  读取: \"");
    int ch;
    while ((ch = fgetc(f)) != EOF) {
        putchar(ch);
    }
    printf("\"\n");
    fclose(f);
    printf("  注意: fgetc返回int而非char, 以区分EOF(-1)和字符0xFF\n\n");
}

void demo_fputs_fgets(void) {
    printf("=== fputs/fgets (字符串行) ===\n");
    FILE *f = fopen(TEST_FILE, "w");
    if (!f) return;
    fputs("第一行: C语言文件操作\n", f);
    fputs("第二行: fputs写入\n", f);
    fputs("第三行: 最后一行", f);
    fclose(f);
    printf("  已写入3行\n");

    f = fopen(TEST_FILE, "r");
    if (!f) return;
    char line[256];
    int line_num = 0;
    while (fgets(line, sizeof(line), f) != NULL) {
        line_num++;
        line[strcspn(line, "\n")] = '\0';
        printf("  行%d: \"%s\"\n", line_num, line);
    }
    fclose(f);
    printf("  注意: fgets保留换行符(如果缓冲区足够)\n\n");
}

void demo_fprintf_fscanf(void) {
    printf("=== fprintf/fscanf (格式化) ===\n");
    FILE *f = fopen(TEST_FILE, "w");
    if (!f) return;
    fprintf(f, "%s %d %.2f\n", "Alice", 25, 89.5);
    fprintf(f, "%s %d %.2f\n", "Bob", 30, 95.0);
    fprintf(f, "%s %d %.2f\n", "Charlie", 28, 72.3);
    fclose(f);
    printf("  已写入3条格式化记录\n");

    f = fopen(TEST_FILE, "r");
    if (!f) return;
    char name[64];
    int age;
    double score;
    printf("  读取结果:\n");
    while (fscanf(f, "%63s %d %lf", name, &age, &score) == 3) {
        printf("    姓名=%s, 年龄=%d, 分数=%.2f\n", name, age, score);
    }
    fclose(f);
    printf("  注意: fscanf以空白符分隔, 不适合含空格的字段\n\n");
}

void demo_error_handling(void) {
    printf("=== 文件操作错误处理 ===\n");
    FILE *f = fopen("__no_such_file__", "r");
    if (!f) {
        perror("  打开失败");
        printf("  feof/ferror在NULL指针上无意义, 先检查返回值!\n");
    }

    f = fopen(TEST_FILE, "r");
    if (f) {
        while (fgetc(f) != EOF) {}
        if (feof(f)) printf("  到达文件末尾(feof=true)\n");
        if (ferror(f)) printf("  发生读取错误(ferror=true)\n");
        clearerr(f);
        printf("  clearerr后: feof=%d, ferror=%d\n",
               feof(f), ferror(f));
        fclose(f);
    }
    printf("\n");
}

int main(void) {
    printf("========== 文本文件读写示例 ==========\n\n");

    demo_fopen_fclose();
    demo_fputc_fgetc();
    demo_fputs_fgets();
    demo_fprintf_fscanf();
    demo_error_handling();

    remove(TEST_FILE);
    printf("========== 所有演示完成 ==========\n");
    return 0;
}
