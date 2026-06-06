/**
 * @file module_a.h
 * @brief 模块A头文件 - 演示头文件守卫和重复包含
 * @description 对应文档: 16-多文件编程
 */
#ifndef MODULE_A_H
#define MODULE_A_H

#include "config.h"

void module_a_init(void);
void module_a_process(const char *data);
void module_a_shutdown(void);
void module_a_show_config(void);

#endif
