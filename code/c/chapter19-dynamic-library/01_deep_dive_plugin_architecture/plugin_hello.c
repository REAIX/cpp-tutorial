/**
 * @file plugin_hello.c
 * @brief 插件架构深入 - Hello 问候插件
 * @description 对应文档: 20-dynamic-loading
 *              实现一个简单的问候插件，演示插件接口的实现
 *
 * 编译: gcc -fPIC -shared -o plugin_hello.so plugin_hello.c
 */

#include <stdio.h>
#include <string.h>
#include "plugin_api.h"

static plugin_context_t g_ctx = {0, NULL, NULL};

static int hello_init(const plugin_context_t *ctx) {
    if (ctx) {
        g_ctx = *ctx;
    }
    printf("  [plugin_hello] 初始化完成, API版本=%d\n",
           ctx ? ctx->api_version : 0);
    return 0;
}

static void hello_destroy(void) {
    printf("  [plugin_hello] 已销毁\n");
}

static const char *hello_get_name(void) {
    return "hello";
}

static int hello_get_version(void) {
    return 1;
}

static plugin_capability_t hello_get_capabilities(void) {
    return PLUGIN_CAP_TEXT_PROCESSING;
}

static int hello_execute(const char *operation, int argc, int *argv, int *result) {
    if (strcmp(operation, "greet") == 0) {
        printf("  [plugin_hello] 你好! 欢迎使用插件系统! (参数数量: %d)\n", argc);
        if (result) *result = 0;
        return 0;
    }
    if (strcmp(operation, "echo") == 0) {
        printf("  [plugin_hello] 回显: ");
        for (int i = 0; i < argc; i++) {
            printf("%d ", argv[i]);
        }
        printf("\n");
        if (result) *result = argc;
        return 0;
    }
    if (g_ctx.on_error) {
        g_ctx.on_error("hello: unknown operation");
    }
    return -1;
}

static const plugin_descriptor_t hello_descriptor = {
    .api_version = PLUGIN_API_VERSION,
    .init = hello_init,
    .destroy = hello_destroy,
    .get_name = hello_get_name,
    .get_version = hello_get_version,
    .get_capabilities = hello_get_capabilities,
    .execute = hello_execute
};

const plugin_descriptor_t *plugin_entry(void) {
    return &hello_descriptor;
}
