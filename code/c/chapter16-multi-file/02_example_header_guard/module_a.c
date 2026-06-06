/**
 * @file module_a.c
 * @brief 模块A实现
 * @description 对应文档: 16-多文件编程
 */
#include "module_a.h"
#include <stdio.h>

static int g_initialized = 0;

void module_a_init(void) {
    printf("  [模块A] 初始化 - %s v%s\n", APP_NAME, APP_VERSION);
    g_initialized = 1;
}

void module_a_process(const char *data) {
    if (!g_initialized) {
        printf("  [模块A] 错误: 未初始化!\n");
        return;
    }
    printf("  [模块A] 处理数据: \"%s\" (缓冲区=%d字节)\n",
           data ? data : "NULL", MAX_BUFFER_SIZE);
}

void module_a_shutdown(void) {
    printf("  [模块A] 关闭\n");
    g_initialized = 0;
}

void module_a_show_config(void) {
    printf("  [模块A] 配置: APP=%s, VERSION=%s, PORT=%d, BUFFER=%d\n",
           APP_NAME, APP_VERSION, DEFAULT_PORT, MAX_BUFFER_SIZE);
}
