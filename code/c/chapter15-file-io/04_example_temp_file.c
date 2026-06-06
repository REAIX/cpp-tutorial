/**
 * @file 04_example_temp_file.c
 * @brief 临时文件操作
 * @description 对应文档: 15-文件操作
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void demo_tmpfile(void) {
    printf("=== tmpfile() 创建临时文件 ===\n");
    FILE *tmp = tmpfile();
    if (!tmp) {
        perror("  tmpfile失败");
        return;
    }

    fprintf(tmp, "这是临时文件中的数据\n");
    fprintf(tmp, "第二行数据: %d\n", 42);

    rewind(tmp);

    char line[256];
    while (fgets(line, sizeof(line), tmp) != NULL) {
        printf("  读取: %s", line);
    }

    fclose(tmp);
    printf("  tmpfile特点:\n");
    printf("    - 自动生成唯一文件名\n");
    printf("    - 关闭后自动删除(wb+模式)\n");
    printf("    - 线程安全(每次调用返回不同文件)\n\n");
}

void demo_tmpnam(void) {
    printf("=== tmpnam() 生成临时文件名 ===\n");
    char name[L_tmpnam];
    if (tmpnam(name) != NULL) {
        printf("  生成的临时文件名: %s\n", name);
    }

    char name2[L_tmpnam];
    if (tmpnam(name2) != NULL) {
        printf("  第二个临时文件名: %s\n", name2);
    }

    printf("  tmpnam陷阱:\n");
    printf("    - 存在竞态条件(生成名和创建文件之间可能被占用)\n");
    printf("    - 不如tmpfile安全\n");
    printf("    - 调用次数有限(至少TMP_MAX次)\n\n");
}

void demo_manual_temp_file(void) {
    printf("=== 手动创建临时文件(安全模式) ===\n");
    char filename[256];
    snprintf(filename, sizeof(filename), "temp_%ld.tmp", (long)time(NULL));

    FILE *f = fopen(filename, "wx");
    if (f) {
        fprintf(f, "安全创建的临时文件\n");
        fclose(f);
        printf("  已创建: %s\n", filename);
        remove(filename);
        printf("  已删除: %s\n", filename);
    } else {
        printf("  文件已存在或创建失败(模式\"wx\"排他创建)\n");
    }

    printf("  安全建议:\n");
    printf("    - 使用mkstemp(POSIX)或_mktemp_s(Windows)\n");
    printf("    - 设置合适的文件权限\n");
    printf("    - 使用后立即删除(unlink)\n");
    printf("    - 不要放在共享目录(如/tmp)\n\n");
}

void demo_temp_file_pattern(void) {
    printf("=== 临时文件使用模式 ===\n");
    printf("  模式1: tmpfile() - 推荐, 自动管理生命周期\n");
    printf("    FILE *tmp = tmpfile();\n");
    printf("    // ... 使用 ...\n");
    printf("    fclose(tmp);  // 自动删除\n\n");
    printf("  模式2: 命名临时文件 - 需要其他进程访问时\n");
    printf("    char path[] = \"/tmp/myapp_XXXXXX\";\n");
    printf("    int fd = mkstemp(path);  // POSIX\n");
    printf("    // ... 使用 ...\n");
    printf("    unlink(path);  // 立即删除, fd仍可用\n");
    printf("    close(fd);\n\n");
}

int main(void) {
    printf("========== 临时文件操作示例 ==========\n\n");

    demo_tmpfile();
    demo_tmpnam();
    demo_manual_temp_file();
    demo_temp_file_pattern();

    printf("========== 所有演示完成 ==========\n");
    return 0;
}
