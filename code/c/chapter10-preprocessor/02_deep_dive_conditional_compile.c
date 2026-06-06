/** @file 02_deep_dive_conditional_compile.c
 *  @brief 条件编译进阶：头文件守卫、平台检测、特性测试宏、预处理器配置、构建变体
 *  @description 对应文档: 10-预处理器 | 举一反三：条件编译的高级应用
 */

#include <stdio.h>
#include <stdlib.h>

void demo_include_guards(void) {
    printf("=== 头文件守卫 ===\n");

    printf("方式1: 传统宏守卫 (可移植)\n");
    printf("  #ifndef MY_HEADER_H\n");
    printf("  #define MY_HEADER_H\n");
    printf("  // 头文件内容\n");
    printf("  #endif\n\n");

    printf("方式2: #pragma once (广泛支持但非标准)\n");
    printf("  #pragma once\n");
    printf("  // 头文件内容\n\n");

    printf("对比:\n");
    printf("  宏守卫: 标准保证, 但宏名可能冲突\n");
    printf("  #pragma once: 简洁, 无命名冲突, 但非标准\n");
    printf("  推荐: 两者都用 (双重保险)\n");

    printf("\n");
}

void demo_platform_detection(void) {
    printf("=== 平台检测 ===\n");

    printf("操作系统检测:\n");
#ifdef _WIN32
    printf("  当前平台: Windows\n");
#elif defined(__linux__)
    printf("  当前平台: Linux\n");
#elif defined(__APPLE__)
    printf("  当前平台: macOS\n");
#elif defined(__unix__)
    printf("  当前平台: Unix\n");
#else
    printf("  当前平台: 未知\n");
#endif

    printf("\n编译器检测:\n");
#if defined(__GNUC__) && !defined(__clang__)
    printf("  编译器: GCC %d.%d.%d\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#elif defined(__clang__)
    printf("  编译器: Clang\n");
#elif defined(_MSC_VER)
    printf("  编译器: MSVC %d\n", _MSC_VER);
#else
    printf("  编译器: 未知\n");
#endif

    printf("\n架构检测:\n");
#if defined(__x86_64__) || defined(_M_X64)
    printf("  架构: x86_64\n");
#elif defined(__i386__) || defined(_M_IX86)
    printf("  架构: x86 (32位)\n");
#elif defined(__arm__)
    printf("  架构: ARM\n");
#elif defined(__aarch64__)
    printf("  架构: ARM64\n");
#else
    printf("  架构: 未知\n");
#endif

    printf("\n");
}

void demo_feature_test_macros(void) {
    printf("=== 特性测试宏 ===\n");

    printf("C 标准特性测试:\n");
#ifdef __STDC_VERSION__
    long ver = (long)__STDC_VERSION__;
    if (ver >= 202311L) printf("  C23 或更新\n");
    else if (ver >= 201112L) printf("  C11 (STDC_VERSION = %ldL)\n", ver);
    else if (ver >= 199901L) printf("  C99 (STDC_VERSION = %ldL)\n", ver);
    else printf("  C89/C90\n");
#else
    printf("  C89/C90 (无 __STDC_VERSION__)\n");
#endif

    printf("\nPOSIX 特性测试:\n");
    printf("  #define _POSIX_C_SOURCE 200809L  // 启用 POSIX.1-2008\n");
    printf("  #define _GNU_SOURCE              // 启用 GNU 扩展\n");
    printf("  #define _BSD_SOURCE               // 启用 BSD 扩展\n");

    printf("\n特性测试宏必须放在所有头文件之前定义\n");

    printf("\n");
}

void demo_build_configuration(void) {
    printf("=== 通过预处理器配置程序 ===\n");

#define CONFIG_MAX_CONNECTIONS 100
#define CONFIG_BUFFER_SIZE 4096
#define CONFIG_USE_SSL 1
#define CONFIG_LOG_LEVEL 2

    printf("配置参数:\n");
    printf("  MAX_CONNECTIONS = %d\n", CONFIG_MAX_CONNECTIONS);
    printf("  BUFFER_SIZE = %d\n", CONFIG_BUFFER_SIZE);

#if CONFIG_USE_SSL
    printf("  SSL: 启用\n");
#else
    printf("  SSL: 禁用\n");
#endif

    printf("  LOG_LEVEL = %d\n", CONFIG_LOG_LEVEL);

#if CONFIG_LOG_LEVEL >= 1
#define LOG_ERROR_MSG(msg) printf("[ERROR] %s\n", msg)
#else
#define LOG_ERROR_MSG(msg) ((void)0)
#endif

#if CONFIG_LOG_LEVEL >= 2
#define LOG_WARN_MSG(msg) printf("[WARN]  %s\n", msg)
#else
#define LOG_WARN_MSG(msg) ((void)0)
#endif

#if CONFIG_LOG_LEVEL >= 3
#define LOG_INFO_MSG(msg) printf("[INFO]  %s\n", msg)
#else
#define LOG_INFO_MSG(msg) ((void)0)
#endif

    printf("\n日志输出 (级别 %d):\n", CONFIG_LOG_LEVEL);
    LOG_ERROR_MSG("这是一个错误");
    LOG_WARN_MSG("这是一个警告");
    LOG_INFO_MSG("这是一条信息");

    printf("\n");
}

void demo_build_variants(void) {
    printf("=== 构建变体 ===\n");

    printf("Debug vs Release:\n\n");

#ifdef NDEBUG
    printf("  Release 构建: 断言被禁用\n");
#define ASSERT_ENABLED 0
#else
    printf("  Debug 构建: 断言已启用\n");
#define ASSERT_ENABLED 1
#endif

    printf("  ASSERT_ENABLED = %d\n", ASSERT_ENABLED);

    printf("\n常见的构建变体控制:\n");
    printf("  -DNDEBUG        禁用断言 (Release)\n");
    printf("  -DDEBUG         启用调试代码\n");
    printf("  -DVERBOSE       详细输出\n");
    printf("  -DTEST          测试模式\n");

    printf("\nMakefile 示例:\n");
    printf("  CFLAGS += -DVERSION=\\\"1.0\\\"\n");
    printf("  CFLAGS += -DMAX_SIZE=$(MAX_SIZE)\n");

    printf("\n");
}

void demo_conditional_compile_patterns(void) {
    printf("=== 条件编译常用模式 ===\n");

    printf("模式1: 调试代码\n");
    printf("  #ifdef DEBUG\n");
    printf("      debug_print_state();\n");
    printf("  #endif\n\n");

    printf("模式2: 平台特定代码\n");
    printf("  #ifdef _WIN32\n");
    printf("      Sleep(ms);\n");
    printf("  #else\n");
    printf("      usleep(ms * 1000);\n");
    printf("  #endif\n\n");

    printf("模式3: 废弃API兼容\n");
    printf("  #ifndef NEW_API\n");
    printf("      // 旧版实现\n");
    printf("  #else\n");
    printf("      // 新版实现\n");
    printf("  #endif\n\n");

    printf("模式4: 编译时断言\n");
    printf("  #define STATIC_ASSERT(cond, msg) \\\n");
    printf("      typedef char static_assert_##msg[(cond) ? 1 : -1]\n\n");

#define STATIC_ASSERT(cond, msg) typedef char static_assert_##msg[(cond) ? 1 : -1] __attribute__((used))

    STATIC_ASSERT(sizeof(int) >= 4, int_must_be_at_least_4_bytes);

    printf("  STATIC_ASSERT(sizeof(int) >= 4, int_must_be_at_least_4_bytes);\n");
    printf("  编译时检查, 条件不满足则编译失败\n");

    printf("\n");
}

void demo_compile_time_compute(void) {
    printf("=== 编译时计算 ===\n");

#define FACT_0 1
#define FACT_1 1
#define FACT_2 (2 * FACT_1)
#define FACT_3 (3 * FACT_2)
#define FACT_4 (4 * FACT_3)
#define FACT_5 (5 * FACT_4)
#define FACT_6 (6 * FACT_5)
#define FACT_7 (7 * FACT_6)
#define FACT_8 (8 * FACT_7)
#define FACT_9 (9 * FACT_8)
#define FACT_10 (10 * FACT_9)

    printf("编译时常量:\n");
    printf("  FACT_5 = %d\n", FACT_5);
    printf("  FACT_10 = %d\n", FACT_10);

    printf("\n注意: C 预处理器不支持递归宏展开\n");
    printf("上面的阶乘通过逐级定义实现, 每级引用上一级\n");
    printf("对于编译时计算, C11 引入了 _Static_assert 和更好的工具\n");

    printf("\n");
}

int main(void) {
    demo_include_guards();
    demo_platform_detection();
    demo_feature_test_macros();
    demo_build_configuration();
    demo_build_variants();
    demo_conditional_compile_patterns();
    demo_compile_time_compute();

    return 0;
}
