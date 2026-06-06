/**
 * @file greetlib.h
 * @brief 动态库示例 - 问候库头文件
 * @description 对应文档: 19-dynamic-library
 *              声明动态库提供的问候接口
 */

#ifndef GREETLIB_H
#define GREETLIB_H

void greet_hello(const char *name);
void greet_goodbye(const char *name);
void greet_time_of_day(int hour);
const char *greet_get_version(void);

#endif
