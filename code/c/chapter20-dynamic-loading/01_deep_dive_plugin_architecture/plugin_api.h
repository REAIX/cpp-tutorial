/**
 * @file plugin_api.h
 * @brief 插件架构深入 - 统一插件接口定义
 * @description 对应文档: 20-dynamic-loading
 *              定义插件系统的核心接口，所有插件必须遵循此接口
 *
 * 设计原则:
 *   1. 接口稳定: 插件API一旦发布就不再修改，只扩展
 *   2. 版本控制: 通过 api_version 让主程序检查兼容性
 *   3. 最小依赖: 插件接口只使用标准C类型
 *   4. 自描述: 插件能报告自己的名称、版本、能力
 */

#ifndef PLUGIN_API_H
#define PLUGIN_API_H

#include <stddef.h>

#define PLUGIN_API_VERSION 2

typedef enum {
    PLUGIN_CAP_NONE = 0,
    PLUGIN_CAP_COMPUTATION = 1,
    PLUGIN_CAP_TEXT_PROCESSING = 2,
    PLUGIN_CAP_DATA_TRANSFORM = 4,
    PLUGIN_CAP_ALL = 0x7FFFFFFF
} plugin_capability_t;

typedef struct plugin_context plugin_context_t;

typedef int (*plugin_init_fn)(const plugin_context_t *ctx);
typedef void (*plugin_destroy_fn)(void);
typedef const char *(*plugin_get_name_fn)(void);
typedef int (*plugin_get_version_fn)(void);
typedef plugin_capability_t (*plugin_get_capabilities_fn)(void);
typedef int (*plugin_execute_fn)(const char *operation, int argc, int *argv, int *result);
typedef void (*plugin_on_error_fn)(const char *message);

struct plugin_context {
    int api_version;
    plugin_on_error_fn on_error;
    void *user_data;
};

typedef struct {
    int api_version;
    plugin_init_fn init;
    plugin_destroy_fn destroy;
    plugin_get_name_fn get_name;
    plugin_get_version_fn get_version;
    plugin_get_capabilities_fn get_capabilities;
    plugin_execute_fn execute;
} plugin_descriptor_t;

typedef const plugin_descriptor_t *(*plugin_entry_fn)(void);

#endif
