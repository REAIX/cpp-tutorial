#ifndef CXXU_EXPORT_H
#define CXXU_EXPORT_H

/*
 * CXXU_API 是一个跨平台的函数/类导出宏，不是 C++ 关键字。
 *
 * 作用：控制动态库中符号的可见性，让编译器知道哪些函数或类需要暴露给外部使用。
 *
 * 展开结果：
 *   - Windows (DLL 导出): __declspec(dllexport)
 *   - Windows (DLL 导入): __declspec(dllimport)
 *   - Linux/GCC:          __attribute__((visibility("default")))
 *   - 静态链接时:          空（无额外修饰）
 *
 * 使用方式：
 *   包含本头文件后，直接在函数或类声明前加上 CXXU_API 即可：
 *     CXXU_API void my_function(void);
 *     class CXXU_API MyClass { ... };
 *
 * 无需手动定义此宏，通过编译选项控制：
 *   - 构建 DLL 时定义 CXXU_BUILDING_DLL
 *   - 使用 DLL 时定义 CXXU_DLL
 *   - 静态链接时不定义上述宏
 */

#ifdef CXXU_BUILDING_DLL
    #ifdef _WIN32
        #define CXXU_API __declspec(dllexport)
    #else
        #define CXXU_API __attribute__((visibility("default")))
    #endif
#elif defined(CXXU_DLL)
    #ifdef _WIN32
        #define CXXU_API __declspec(dllimport)
    #else
        #define CXXU_API __attribute__((visibility("default")))
    #endif
#else
    #define CXXU_API
#endif

#endif
