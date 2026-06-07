/**
 * @file main.c
 * @brief dlopen示例 - 运行时动态加载插件的主程序
 * @description 对应文档: 20-dynamic-loading
 *              演示 dlopen/dlsym/dlclose 的完整使用流程
 *
 * 编译:
 *   先编译插件: gcc -fPIC -shared -o plugin_math.so plugin_math.c
 *   再编译主程序: gcc main.c -ldl -o main
 * 运行:
 *   ./main
 *
 * dlopen/dlsym/dlclose API 说明:
 *   dlopen(path, flags)  - 加载动态库
 *     flags:
 *       RTLD_LAZY   - 延迟绑定（函数首次调用时解析）
 *       RTLD_NOW    - 立即绑定（加载时解析所有符号）
 *       RTLD_GLOBAL - 使库的符号对后续加载的库可见
 *       RTLD_LOCAL  - 库的符号不对其他库可见（默认）
 *
 *   dlsym(handle, symbol) - 查找符号
 *     返回符号对应的函数指针或变量指针
 *     出错返回 NULL
 *
 *   dlclose(handle) - 卸载动态库
 *     减少引用计数，为0时卸载
 *
 *   dlerror() - 获取最近的错误信息
 *     在 dlopen/dlsym/dlclose 之后调用
 */

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
static char g_dl_error[256] = "";
static void *dlopen(const char *path, int flags) {
    (void)flags;
    HMODULE h = LoadLibraryA(path);
    if (!h) {
        snprintf(g_dl_error, sizeof(g_dl_error), "LoadLibrary failed: error %lu", GetLastError());
    } else {
        g_dl_error[0] = '\0';
    }
    return (void *)h;
}
static void *dlsym(void *handle, const char *name) {
    FARPROC p = GetProcAddress((HMODULE)handle, name);
    if (!p) {
        snprintf(g_dl_error, sizeof(g_dl_error), "GetProcAddress failed: error %lu", GetLastError());
    } else {
        g_dl_error[0] = '\0';
    }
    return (void *)p;
}
static int dlclose(void *handle) {
    return FreeLibrary((HMODULE)handle) ? 0 : -1;
}
static char *dlerror(void) {
    return g_dl_error[0] ? g_dl_error : NULL;
}
#define RTLD_LAZY   0
#define RTLD_NOW    1
#define RTLD_GLOBAL 2
#define RTLD_LOCAL  0
#else
#include <dlfcn.h>
#endif

#include "plugin.h"

void demo_dlopen_basic(void) {
    printf("===== dlopen 基本用法 =====\n\n");

    printf("步骤1: dlopen 加载动态库\n");
    void *handle = dlopen("./plugin_math.so", RTLD_LAZY);
    if (!handle) {
        printf("  dlopen 失败: %s\n", dlerror());
        printf("  (这是正常的，如果 plugin_math.so 不在当前目录)\n\n");
        printf("  正确用法: dlopen(\"./plugin_math.so\", RTLD_LAZY)\n");
        printf("  或使用绝对路径: dlopen(\"/path/to/plugin_math.so\", RTLD_LAZY)\n\n");
        return;
    }
    printf("  dlopen 成功! handle = %p\n\n", handle);

    printf("步骤2: dlsym 查找符号\n");
    dlerror();
    plugin_interface_t *(*create_fn)(void) = dlsym(handle, "plugin_create");
    char *error = dlerror();
    if (error != NULL) {
        printf("  dlsym 失败: %s\n", error);
        dlclose(handle);
        return;
    }
    printf("  dlsym 成功! plugin_create 函数地址 = %p\n\n", (void *)create_fn);

    printf("步骤3: 使用插件\n");
    plugin_interface_t *plugin = create_fn();
    printf("  插件名称: %s\n", plugin->name);
    printf("  插件版本: %s\n", plugin->version);

    if (plugin->init) {
        plugin->init();
    }
    if (plugin->execute) {
        plugin->execute(10, 20);
    }
    if (plugin->cleanup) {
        plugin->cleanup();
    }
    printf("\n");

    printf("步骤4: dlclose 卸载动态库\n");
    dlclose(handle);
    printf("  dlclose 成功!\n\n");
}

void demo_dlopen_flags(void) {
    printf("===== dlopen 标志详解 =====\n\n");

    printf("RTLD_LAZY vs RTLD_NOW:\n");
    printf("  RTLD_LAZY - 延迟绑定\n");
    printf("    只在函数首次被调用时解析符号\n");
    printf("    优点: 加载速度快，未使用的符号不会被解析\n");
    printf("    缺点: 可能在运行时才发现未定义符号错误\n\n");

    printf("  RTLD_NOW - 立即绑定\n");
    printf("    加载时立即解析所有符号\n");
    printf("    优点: 启动时就能发现所有链接错误\n");
    printf("    缺点: 加载速度较慢\n\n");

    printf("RTLD_GLOBAL vs RTLD_LOCAL:\n");
    printf("  RTLD_GLOBAL - 全局可见\n");
    printf("    库的符号对后续 dlopen 加载的库可见\n");
    printf("    适用于有依赖关系的插件\n\n");

    printf("  RTLD_LOCAL - 局部可见 (默认)\n");
    printf("    库的符号只对本库内部可见\n");
    printf("    避免符号冲突，更安全\n\n");

    printf("常见组合:\n");
    printf("  RTLD_LAZY | RTLD_LOCAL    # 默认，最常用\n");
    printf("  RTLD_NOW | RTLD_LOCAL     # 启动时检查所有符号\n");
    printf("  RTLD_NOW | RTLD_GLOBAL    # 插件间有依赖时使用\n\n");
}

void demo_dlopen_error_handling(void) {
    printf("===== dlopen 错误处理 =====\n\n");

    printf("加载不存在的库:\n");
    void *h = dlopen("./nonexistent.so", RTLD_LAZY);
    if (!h) {
        printf("  错误信息: %s\n\n", dlerror());
    }

    printf("查找不存在的符号:\n");
    h = dlopen(NULL, RTLD_LAZY);
    if (h) {
        dlerror();
        void *sym = dlsym(h, "nonexistent_symbol_xyz");
        (void)sym;
        char *err = dlerror();
        if (err) {
            printf("  错误信息: %s\n\n", err);
        }
        dlclose(h);
    }

    printf("错误处理最佳实践:\n");
    printf("  1. 每次 dlopen/dlsym/dlclose 后都检查 dlerror()\n");
    printf("  2. 在 dlsym 前调用 dlerror() 清除旧错误\n");
    printf("  3. dlsym 返回 NULL 不一定出错，需配合 dlerror() 判断\n");
    printf("  4. dlclose 后不要再使用从该库获取的任何指针\n\n");
}

void demo_dlopen_special_handles(void) {
    printf("===== dlopen 特殊句柄 =====\n\n");

    printf("dlopen(NULL, ...) - 获取主程序的句柄:\n");
    void *main_handle = dlopen(NULL, RTLD_LAZY);
    if (main_handle) {
        printf("  主程序句柄: %p\n", main_handle);
        dlerror();
        void *sym = dlsym(main_handle, "printf");
        char *err = dlerror();
        if (!err) {
            printf("  在主程序中找到 printf: %p\n", sym);
        }
        dlclose(main_handle);
    }
    printf("\n");

    printf("RTLD_DEFAULT - 在所有已加载库中搜索:\n");
    printf("  dlsym(RTLD_DEFAULT, \"symbol\") 搜索所有已加载的库\n\n");

    printf("RTLD_NEXT - 在后续库中搜索:\n");
    printf("  dlsym(RTLD_NEXT, \"malloc\") 查找下一个 malloc 定义\n");
    printf("  常用于包装(wrap)系统函数，同时调用原始版本\n\n");
}

int main(void) {
    printf("========== dlopen 动态加载示例 ==========\n\n");

    demo_dlopen_basic();
    demo_dlopen_flags();
    demo_dlopen_error_handling();
    demo_dlopen_special_handles();

    printf("========== 程序结束 ==========\n");
    return 0;
}
