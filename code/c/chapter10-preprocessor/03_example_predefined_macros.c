/** @file 03_example_predefined_macros.c
 *  @brief 预定义宏：__FILE__、__LINE__、__DATE__、__TIME__、__func__、pragma
 *  @description 对应文档: 10-预处理器
 */

#include <stdio.h>
#include <stdlib.h>

void demo_standard_predefined(void) {
    printf("=== 标准预定义宏 ===\n");

    printf("__FILE__    = %s\n", __FILE__);
    printf("__LINE__    = %d\n", __LINE__);
    printf("__DATE__    = %s\n", __DATE__);
    printf("__TIME__    = %s\n", __TIME__);
    printf("__func__    = %s\n", __func__);
    printf("__STDC__    = %d\n", __STDC__);

#ifdef __STDC_VERSION__
    printf("__STDC_VERSION__ = %ldL\n", (long)__STDC_VERSION__);
#else
    printf("__STDC_VERSION__ 未定义\n");
#endif

    printf("\n__FILE__: 当前源文件名 (字符串字面量)\n");
    printf("__LINE__: 当前行号 (整数)\n");
    printf("__DATE__: 编译日期 (如 \"May 29 2026\")\n");
    printf("__TIME__: 编译时间 (如 \"19:30:00\")\n");
    printf("__func__: 当前函数名 (C99, 不是宏而是标识符)\n");
    printf("__STDC__: 如果编译器遵循 ANSI C 则为 1\n");

    printf("\n");
}

void demo_logging_with_macros(void) {
    printf("=== 使用预定义宏实现日志 ===\n");

#define LOG_DEBUG(fmt, ...) \
    printf("[DEBUG] %s:%d %s() - " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define LOG_INFO(fmt, ...) \
    printf("[INFO]  %s:%d - " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG_ERROR(fmt, ...) \
    fprintf(stderr, "[ERROR] %s:%d %s() - " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

    LOG_DEBUG("程序启动");
    LOG_INFO("处理数据: count = %d", 42);
    LOG_ERROR("文件打开失败: %s", "data.txt");

    printf("\n##__VA_ARGS__ 是 GCC 扩展: 没有额外参数时删除前面的逗号\n");

    printf("\n");
}

void demo_version_macros(void) {
    printf("=== 编译器和标准版本宏 ===\n");

#ifdef __GNUC__
    printf("__GNUC__ = %d (GCC 主版本号)\n", __GNUC__);
#ifdef __GNUC_MINOR__
    printf("__GNUC_MINOR__ = %d (GCC 次版本号)\n", __GNUC_MINOR__);
#endif
#ifdef __GNUC_PATCHLEVEL__
    printf("__GNUC_PATCHLEVEL__ = %d (GCC 补丁级别)\n", __GNUC_PATCHLEVEL__);
#endif
#else
    printf("非 GCC 编译器\n");
#endif

#ifdef _MSC_VER
    printf("_MSC_VER = %d (MSVC 版本号)\n", _MSC_VER);
#else
    printf("非 MSVC 编译器\n");
#endif

#ifdef _WIN32
    printf("_WIN32 已定义 (Windows 平台)\n");
#endif

#ifdef __linux__
    printf("__linux__ 已定义 (Linux 平台)\n");
#endif

#ifdef __APPLE__
    printf("__APPLE__ 已定义 (macOS 平台)\n");
#endif

    printf("\nC 标准版本:\n");
    printf("  __STDC_VERSION__ = 201112L => C11\n");
    printf("  __STDC_VERSION__ = 199901L => C99\n");
    printf("  __STDC_VERSION__ = 202311L => C23\n");

    printf("\n");
}

void demo_pragma(void) {
    printf("=== #pragma 指令 ===\n");

    printf("#pragma once: 确保头文件只包含一次 (非标准但广泛支持)\n");
    printf("  注意: #pragma once 只应用于头文件(.h), 不用于源文件(.c)\n\n");

#pragma pack(push, 1)
    struct PackedStruct {
        char a;
        int b;
    };
#pragma pack(pop)
    printf("#pragma pack(1): PackedStruct 大小 = %zu\n", sizeof(struct PackedStruct));

    printf("\n常用 pragma:\n");
    printf("  #pragma once           头文件守卫\n");
    printf("  #pragma pack(n)        设置对齐\n");
    printf("  #pragma message(\"x\")   编译时输出消息\n");
    printf("  #pragma warning(...)   控制警告\n");
    printf("  #pragma GCC diagnostic GCC 诊断控制\n");

    printf("\n");
}

void demo_compile_time_info(void) {
    printf("=== 编译时信息 ===\n");

    printf("编译日期: %s\n", __DATE__);
    printf("编译时间: %s\n", __TIME__);

    printf("\n可以用这些宏实现版本信息:\n");

#define VERSION_MAJOR 1
#define VERSION_MINOR 2
#define VERSION_PATCH 3

    printf("版本: %d.%d.%d (编译于 %s %s)\n",
           VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,
           __DATE__, __TIME__);

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

    printf("版本字符串: \"%s.%s.%s\"\n",
           TOSTRING(VERSION_MAJOR),
           TOSTRING(VERSION_MINOR),
           TOSTRING(VERSION_PATCH));

    printf("\n两层宏的原因: TOSTRING 展开参数, STRINGIFY 字符串化\n");

    printf("\n");
}

void demo_counter_macro(void) {
    printf("=== __COUNTER__ 宏 (GCC/Clang 扩展) ===\n");

    printf("__COUNTER__ 每次使用递增: %d\n", __COUNTER__);
    printf("__COUNTER__ 每次使用递增: %d\n", __COUNTER__);
    printf("__COUNTER__ 每次使用递增: %d\n", __COUNTER__);

    printf("\n__COUNTER__ 从 0 开始, 每次展开加 1\n");
    printf("用途: 生成唯一的变量名\n\n");

    printf("示例:\n");
    printf("  #define UNIQUE_NAME(name) name ## _ ## __COUNTER__\n");
    printf("  int UNIQUE_NAME(var);  // 展开为 var_3 (或当前计数器值)\n");
    printf("  int UNIQUE_NAME(var);  // 展开为 var_4 (计数器自动递增)\n");
    printf("  两次生成不同的变量名, 避免命名冲突\n");

    printf("\n");
}

int main(void) {
    demo_standard_predefined();
    demo_logging_with_macros();
    demo_version_macros();
    demo_pragma();
    demo_compile_time_info();
    demo_counter_macro();

    return 0;
}
