/**
 * @file 01_example_errno.c
 * @brief errno使用与错误报告
 * @description 对应文档: 13-错误处理与信号
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

void demo_errno_basic(void) {
    printf("=== errno基本用法 ===\n");
    errno = 0;
    FILE *f = fopen("nonexistent_file.txt", "r");
    if (f == NULL) {
        printf("  打开文件失败!\n");
        printf("  errno值: %d\n", errno);
        printf("  strerror(errno): %s\n", strerror(errno));
    }

    errno = 0;
    double result = sqrt(-1.0);
    if (errno != 0) {
        printf("  sqrt(-1)失败!\n");
        printf("  errno值: %d\n", errno);
        printf("  strerror(errno): %s\n", strerror(errno));
    } else {
        printf("  sqrt(-1) = %f (某些实现返回NaN但不设errno)\n", result);
    }
    printf("\n");
}

void demo_perror(void) {
    printf("=== perror用法 ===\n");
    errno = ENOENT;
    perror("  自定义前缀");

    errno = EACCES;
    perror("  权限错误");

    errno = ENOMEM;
    perror("  内存不足");
    printf("\n");
}

void demo_common_errno_values(void) {
    printf("=== 常见errno值 ===\n");
    struct { int code; const char *name; } errors[] = {
        {EPERM,  "EPERM"},
        {ENOENT, "ENOENT"},
        {ESRCH,  "ESRCH"},
        {EINTR,  "EINTR"},
        {EIO,    "EIO"},
        {ENXIO,  "ENXIO"},
        {EACCES, "EACCES"},
        {ENOMEM, "ENOMEM"},
        {EINVAL, "EINVAL"},
        {ERANGE, "ERANGE"},
    };
    for (int i = 0; i < (int)(sizeof(errors) / sizeof(errors[0])); i++) {
        printf("  %-10s (%2d): %s\n", errors[i].name, errors[i].code, strerror(errors[i].code));
    }
    printf("\n");
}

void demo_errno_pitfall(void) {
    printf("=== errno使用陷阱 ===\n");

    printf("  陷阱1: errno不会自动清零\n");
    errno = ENOENT;
    printf("    当前errno=%d, 调用成功的函数后...\n", errno);
    int x = (int)fputs("", stdout);
    (void)x;
    printf("    errno=%d (可能被成功函数修改, 也可能不变!)\n", errno);
    printf("    正确做法: 在调用可能失败的函数前手动清零 errno = 0\n\n");

    printf("  陷阱2: 库函数不一定设置errno\n");
    printf("    某些函数返回错误指示但不设errno, 需查阅文档\n\n");

    printf("  陷阱3: perror/strerror不是线程安全的(旧版)\n");
    printf("    C11提供strerror_s(如果可用), 或使用strerror_r\n\n");
}

void demo_errno_with_strtol(void) {
    printf("=== strtol的errno用法 ===\n");
    const char *inputs[] = {"12345", "999999999999999999999", "abc", "42abc"};
    for (int i = 0; i < 4; i++) {
        char *endptr = NULL;
        errno = 0;
        long val = strtol(inputs[i], &endptr, 10);
        if (errno == ERANGE) {
            printf("  \"%s\" -> 溢出! errno=ERANGE\n", inputs[i]);
        } else if (endptr == inputs[i]) {
            printf("  \"%s\" -> 无法转换, 无数字字符\n", inputs[i]);
        } else {
            printf("  \"%s\" -> %ld (未消费: \"%s\")\n", inputs[i], val, endptr);
        }
    }
    printf("\n");
}

int main(void) {
    printf("========== errno使用与错误报告示例 ==========\n\n");

    demo_errno_basic();
    demo_perror();
    demo_common_errno_values();
    demo_errno_pitfall();
    demo_errno_with_strtol();

    printf("========== 所有演示完成 ==========\n");
    return 0;
}
