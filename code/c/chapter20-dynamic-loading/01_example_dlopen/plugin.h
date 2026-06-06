/**
 * @file plugin.h
 * @brief dlopen示例 - 插件接口头文件
 * @description 对应文档: 20-dynamic-loading
 *              定义插件的标准接口，所有插件必须实现这些函数
 */

#ifndef PLUGIN_H
#define PLUGIN_H

typedef struct {
    const char *name;
    const char *version;
    int (*init)(void);
    void (*cleanup)(void);
    int (*execute)(int a, int b);
} plugin_interface_t;

#endif
