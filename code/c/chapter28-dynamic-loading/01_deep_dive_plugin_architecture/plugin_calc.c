/**
 * @file plugin_calc.c
 * @brief 插件架构深入 - 计算器插件
 * @description 对应文档: 20-dynamic-loading
 *              实现一个计算器插件，支持多种运算操作
 *
 * 编译: gcc -fPIC -shared -o plugin_calc.so plugin_calc.c
 */

#include <stdio.h>
#include <string.h>
#include "plugin_api.h"

static plugin_context_t g_ctx = {0, NULL, NULL};

static int calc_init(const plugin_context_t *ctx) {
    if (ctx) {
        g_ctx = *ctx;
    }
    printf("  [plugin_calc] 初始化完成\n");
    return 0;
}

static void calc_destroy(void) {
    printf("  [plugin_calc] 已销毁\n");
}

static const char *calc_get_name(void) {
    return "calc";
}

static int calc_get_version(void) {
    return 2;
}

static plugin_capability_t calc_get_capabilities(void) {
    return PLUGIN_CAP_COMPUTATION | PLUGIN_CAP_DATA_TRANSFORM;
}

static int calc_execute(const char *operation, int argc, int *argv, int *result) {
    if (argc < 2) {
        if (g_ctx.on_error) {
            g_ctx.on_error("calc: need at least 2 arguments");
        }
        return -1;
    }

    int a = argv[0];
    int b = argv[1];

    if (strcmp(operation, "add") == 0) {
        if (result) *result = a + b;
        printf("  [plugin_calc] %d + %d = %d\n", a, b, a + b);
        return 0;
    }
    if (strcmp(operation, "sub") == 0) {
        if (result) *result = a - b;
        printf("  [plugin_calc] %d - %d = %d\n", a, b, a - b);
        return 0;
    }
    if (strcmp(operation, "mul") == 0) {
        if (result) *result = a * b;
        printf("  [plugin_calc] %d * %d = %d\n", a, b, a * b);
        return 0;
    }
    if (strcmp(operation, "div") == 0) {
        if (b == 0) {
            if (g_ctx.on_error) {
                g_ctx.on_error("calc: division by zero");
            }
            return -1;
        }
        if (result) *result = a / b;
        printf("  [plugin_calc] %d / %d = %d\n", a, b, a / b);
        return 0;
    }
    if (strcmp(operation, "pow") == 0) {
        if (b < 0) {
            if (g_ctx.on_error) {
                g_ctx.on_error("calc: negative exponent not supported for integer pow");
            }
            return -1;
        }
        int val = 1;
        for (int i = 0; i < b; i++) {
            val *= a;
        }
        if (result) *result = val;
        printf("  [plugin_calc] %d ^ %d = %d\n", a, b, val);
        return 0;
    }
    if (strcmp(operation, "sum") == 0) {
        int total = 0;
        for (int i = 0; i < argc; i++) {
            total += argv[i];
        }
        if (result) *result = total;
        printf("  [plugin_calc] sum of %d numbers = %d\n", argc, total);
        return 0;
    }

    if (g_ctx.on_error) {
        g_ctx.on_error("calc: unknown operation");
    }
    return -1;
}

static const plugin_descriptor_t calc_descriptor = {
    .api_version = PLUGIN_API_VERSION,
    .init = calc_init,
    .destroy = calc_destroy,
    .get_name = calc_get_name,
    .get_version = calc_get_version,
    .get_capabilities = calc_get_capabilities,
    .execute = calc_execute
};

const plugin_descriptor_t *plugin_entry(void) {
    return &calc_descriptor;
}
