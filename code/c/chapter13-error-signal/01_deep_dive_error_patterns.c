/**
 * @file 01_deep_dive_error_patterns.c
 * @brief 错误处理模式深入
 * @description 对应文档: 13-错误处理与信号
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <setjmp.h>

typedef enum {
    ERR_NONE = 0,
    ERR_INVALID_PARAM,
    ERR_OUT_OF_MEMORY,
    ERR_FILE_NOT_FOUND,
    ERR_PERMISSION_DENIED,
    ERR_IO_ERROR,
    ERR_TIMEOUT,
} error_code_t;

static const char *error_messages[] = {
    "无错误",
    "无效参数",
    "内存不足",
    "文件未找到",
    "权限被拒绝",
    "I/O错误",
    "超时",
};

static const char *error_get_message(error_code_t code) {
    if (code >= 0 && code < (int)(sizeof(error_messages) / sizeof(error_messages[0]))) {
        return error_messages[code];
    }
    return "未知错误";
}

static error_code_t divide(int a, int b, int *result) {
    if (result == NULL) return ERR_INVALID_PARAM;
    if (b == 0) return ERR_INVALID_PARAM;
    *result = a / b;
    return ERR_NONE;
}

typedef struct error_context {
    error_code_t code;
    char message[256];
    char source[64];
    int line;
} error_context_t;

static error_code_t read_config(const char *path, error_context_t *ctx) {
    if (path == NULL || ctx == NULL) {
        if (ctx != NULL) {
            ctx->code = ERR_INVALID_PARAM;
            snprintf(ctx->message, sizeof(ctx->message), "路径或上下文为空");
            snprintf(ctx->source, sizeof(ctx->source), "read_config");
            ctx->line = __LINE__;
        }
        return ERR_INVALID_PARAM;
    }
    FILE *f = fopen(path, "r");
    if (!f) {
        ctx->code = ERR_FILE_NOT_FOUND;
        snprintf(ctx->message, sizeof(ctx->message), "无法打开 %s: %s", path, strerror(errno));
        snprintf(ctx->source, sizeof(ctx->source), "read_config");
        ctx->line = __LINE__;
        return ERR_FILE_NOT_FOUND;
    }
    fclose(f);
    return ERR_NONE;
}

static error_code_t process_file_goto(const char *path) {
    FILE *f = NULL;
    char *buffer = NULL;

    f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "  无法打开文件\n");
        goto cleanup;
    }

    buffer = malloc(1024);
    if (!buffer) {
        fprintf(stderr, "  内存分配失败\n");
        goto cleanup;
    }

    if (fread(buffer, 1, 1024, f) == 0 && ferror(f)) {
        fprintf(stderr, "  读取失败\n");
        goto cleanup;
    }

    printf("  文件处理成功(模拟)\n");

cleanup:
    if (buffer) free(buffer);
    if (f) fclose(f);
    return (f && buffer) ? ERR_NONE : ERR_IO_ERROR;
}

static jmp_buf g_error_jmp;

static void do_something_risky(int depth) {
    if (depth > 3) {
        printf("  深度过大, 执行longjmp跳转!\n");
        longjmp(g_error_jmp, depth);
    }
    printf("  深度 %d 正常执行\n", depth);
}

typedef struct {
    FILE *file;
    char *buffer;
    int buffer_size;
    int is_valid;
} scoped_resource_t;

static void scoped_resource_init(scoped_resource_t *r) {
    r->file = NULL;
    r->buffer = NULL;
    r->buffer_size = 0;
    r->is_valid = 0;
}

static void scoped_resource_cleanup(scoped_resource_t *r) {
    if (r->file) { fclose(r->file); r->file = NULL; }
    if (r->buffer) { free(r->buffer); r->buffer = NULL; }
    r->is_valid = 0;
    printf("  资源已清理\n");
}

void demo_error_code_pattern(void) {
    printf("=== 错误码模式 ===\n");

    int result = 0;
    error_code_t rc = divide(10, 3, &result);
    if (rc != ERR_NONE) {
        printf("  错误: %s\n", error_get_message(rc));
    } else {
        printf("  10 / 3 = %d\n", result);
    }

    rc = divide(10, 0, &result);
    if (rc != ERR_NONE) {
        printf("  10 / 0 错误: %s (码=%d)\n", error_get_message(rc), rc);
    }
    printf("\n");
}

void demo_error_chain(void) {
    printf("=== 错误链模式 ===\n");

    error_context_t ctx = {0};
    error_code_t rc = read_config("/nonexistent/config.ini", &ctx);
    if (rc != ERR_NONE) {
        printf("  错误链:\n");
        printf("    代码: %d (%s)\n", ctx.code, error_get_message(ctx.code));
        printf("    消息: %s\n", ctx.message);
        printf("    来源: %s (行 %d)\n", ctx.source, ctx.line);
    }
    printf("\n");
}

void demo_goto_cleanup(void) {
    printf("=== goto清理模式(C语言惯用法) ===\n");

    process_file_goto("__nonexistent__");
    printf("  优点: 避免深层嵌套, 清理逻辑集中\n");
    printf("  这是Linux内核和很多C项目广泛使用的模式\n\n");
}

void demo_setjmp_longjmp(void) {
    printf("=== setjmp/longjmp错误恢复 ===\n");

    int val = setjmp(g_error_jmp);
    if (val == 0) {
        printf("  首次setjmp, 开始执行\n");
        do_something_risky(1);
        do_something_risky(2);
        do_something_risky(3);
        do_something_risky(4);
    } else {
        printf("  从longjmp返回, 跳转值=%d\n", val);
    }

    printf("  陷阱:\n");
    printf("    1. longjmp跳过栈展开, 局部变量可能不一致\n");
    printf("    2. 不要跳回已返回的函数(未定义行为)\n");
    printf("    3. volatile变量在setjmp后修改的值会被保留\n\n");
}

void demo_raii_like_pattern(void) {
    printf("=== C语言RAII-like模式(模拟) ===\n");

    {
        scoped_resource_t res;
        scoped_resource_init(&res);

        res.buffer = malloc(256);
        if (!res.buffer) {
            scoped_resource_cleanup(&res);
            return;
        }
        res.buffer_size = 256;

        printf("  资源分配成功, buffer_size=%d\n", res.buffer_size);

        scoped_resource_cleanup(&res);
    }

    printf("  模式要点:\n");
    printf("    - 统一的init/cleanup函数对\n");
    printf("    - 每个退出路径都调用cleanup\n");
    printf("    - cleanup函数必须处理部分初始化的情况\n\n");
}

int main(void) {
    printf("========== 错误处理模式深入 ==========\n\n");

    demo_error_code_pattern();
    demo_error_chain();
    demo_goto_cleanup();
    demo_setjmp_longjmp();
    demo_raii_like_pattern();

    printf("========== 所有演示完成 ==========\n");
    return 0;
}
