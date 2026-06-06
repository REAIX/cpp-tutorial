/**
 * @file main.c
 * @brief 头文件守卫示例
 * @description 对应文档: 16-多文件编程 - Include guards, #pragma once, 重复包含问题
 */
#include <stdio.h>
#include "config.h"
#include "module_a.h"

void demo_include_guard(void) {
    printf("=== 头文件守卫(Include Guard) ===\n");
    printf("  #ifndef CONFIG_H\n");
    printf("  #define CONFIG_H\n");
    printf("  // ... 头文件内容 ...\n");
    printf("  #endif\n\n");
    printf("  作用: 防止头文件被重复包含\n");
    printf("  如果没有守卫, 重复包含会导致:\n");
    printf("    - 重复定义编译错误\n");
    printf("    - 类型重定义错误\n\n");
}

void demo_pragma_once(void) {
    printf("=== #pragma once ===\n");
    printf("  #pragma once  // 编译器保证只包含一次\n\n");
    printf("  优点: 简洁, 不需要宏名, 不会宏名冲突\n");
    printf("  缺点: 非标准C, 但主流编译器都支持\n\n");
    printf("  推荐做法:\n");
    printf("    - 新项目可用 #pragma once\n");
    printf("    - 需要最大可移植性时用 #ifndef 守卫\n");
    printf("    - 两者可以同时使用(双保险)\n\n");
}

void demo_repeated_inclusion_problem(void) {
    printf("=== 重复包含问题演示 ===\n");
    printf("  场景: main.c 包含 module_a.h, module_a.h 包含 config.h\n");
    printf("  如果 main.c 也直接包含 config.h:\n");
    printf("    - 没有守卫: config.h被包含2次 -> 编译错误!\n");
    printf("    - 有守卫:   config.h只被包含1次 -> 正常\n\n");
    printf("  更复杂的场景:\n");
    printf("    a.h -> 包含 b.h 和 c.h\n");
    printf("    b.h -> 包含 c.h (重复!)\n");
    printf("    没有守卫: c.h被包含2次\n\n");
}

void demo_guard_naming(void) {
    printf("=== 守卫宏命名规范 ===\n");
    printf("  推荐格式: <项目>_<路径>_<文件名>_H\n");
    printf("  示例:\n");
    printf("    myapp_src_utils_math_h\n");
    printf("    MYAPP_SRC_UTILS_MATH_H\n");
    printf("  避免简单命名如 _H, _H 等(可能冲突)\n");
    printf("  避免以双下划线开头(保留给实现)\n\n");
}

int main(void) {
    printf("========== 头文件守卫示例 ==========\n\n");

    demo_include_guard();
    demo_pragma_once();
    demo_repeated_inclusion_problem();
    demo_guard_naming();

    printf("=== 模块A功能演示 ===\n");
    module_a_init();
    module_a_show_config();
    module_a_process("测试数据");
    module_a_shutdown();

    printf("\n========== 所有演示完成 ==========\n");
    return 0;
}
