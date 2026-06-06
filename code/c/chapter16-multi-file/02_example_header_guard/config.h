/**
 * @file config.h
 * @brief 配置头文件 - 演示头文件守卫
 * @description 对应文档: 16-多文件编程
 */
#ifndef CONFIG_H
#define CONFIG_H

#define APP_NAME "MultiFileDemo"
#define APP_VERSION "1.0.0"
#define MAX_BUFFER_SIZE 1024
#define DEFAULT_PORT 8080

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
} log_level_t;

#endif
