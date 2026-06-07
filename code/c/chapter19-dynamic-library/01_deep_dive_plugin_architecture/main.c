/**
 * @file main.c
 * @brief 插件架构深入 - 插件管理器主程序
 * @description 对应文档: 20-dynamic-loading
 *              实现一个完整的插件管理器，演示插件架构的设计模式，
 *              包括插件加载、注册、执行、卸载的完整生命周期
 *
 * 编译:
 *   先编译插件:
 *     gcc -fPIC -shared -o plugin_hello.so plugin_hello.c
 *     gcc -fPIC -shared -o plugin_calc.so plugin_calc.c
 *   再编译主程序:
 *     gcc main.c -ldl -o main
 * 运行:
 *   ./main
 *
 * 插件架构设计要点:
 *   1. 接口统一: 所有插件实现相同的 plugin_descriptor_t 接口
 *   2. 版本检查: 加载时验证 API 版本兼容性
 *   3. 热插拔: 支持运行时加载和卸载插件
 *   4. 错误隔离: 单个插件的错误不影响整个系统
 *   5. 能力查询: 通过 get_capabilities 查询插件支持的功能
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#include "plugin_api.h"

#define MAX_PLUGINS 16

typedef struct {
    void *handle;
    const plugin_descriptor_t *descriptor;
    int active;
} loaded_plugin_t;

static loaded_plugin_t plugins[MAX_PLUGINS];
static int plugin_count = 0;

static void on_plugin_error(const char *message) {
    fprintf(stderr, "  [插件错误] %s\n", message);
}

static plugin_context_t g_plugin_ctx = {
    .api_version = PLUGIN_API_VERSION,
    .on_error = on_plugin_error,
    .user_data = NULL
};

int plugin_load(const char *path) {
    if (plugin_count >= MAX_PLUGINS) {
        printf("  插件数量已达上限 (%d)\n", MAX_PLUGINS);
        return -1;
    }

    void *handle = dlopen(path, RTLD_NOW);
    if (!handle) {
        printf("  加载失败: %s\n", dlerror());
        return -1;
    }

    dlerror();
    plugin_entry_fn entry = (plugin_entry_fn)dlsym(handle, "plugin_entry");
    char *error = dlerror();
    if (error != NULL) {
        printf("  查找 plugin_entry 失败: %s\n", error);
        dlclose(handle);
        return -1;
    }

    const plugin_descriptor_t *desc = entry();
    if (!desc) {
        printf("  plugin_entry 返回 NULL\n");
        dlclose(handle);
        return -1;
    }

    if (desc->api_version != PLUGIN_API_VERSION) {
        printf("  API 版本不兼容: 插件=%d, 主程序=%d\n",
               desc->api_version, PLUGIN_API_VERSION);
        dlclose(handle);
        return -1;
    }

    if (desc->init) {
        int ret = desc->init(&g_plugin_ctx);
        if (ret != 0) {
            printf("  插件初始化失败: %d\n", ret);
            dlclose(handle);
            return -1;
        }
    }

    plugins[plugin_count].handle = handle;
    plugins[plugin_count].descriptor = desc;
    plugins[plugin_count].active = 1;
    plugin_count++;

    printf("  插件加载成功: %s v%d (能力: 0x%x)\n",
           desc->get_name(), desc->get_version(), desc->get_capabilities());
    return 0;
}

void plugin_unload_all(void) {
    for (int i = 0; i < plugin_count; i++) {
        if (plugins[i].active && plugins[i].descriptor->destroy) {
            plugins[i].descriptor->destroy();
        }
        if (plugins[i].handle) {
            dlclose(plugins[i].handle);
        }
        plugins[i].active = 0;
    }
    plugin_count = 0;
    printf("  所有插件已卸载\n");
}

int plugin_execute(const char *plugin_name, const char *operation,
                   int argc, int *argv, int *result) {
    for (int i = 0; i < plugin_count; i++) {
        if (!plugins[i].active) continue;
        if (strcmp(plugins[i].descriptor->get_name(), plugin_name) == 0) {
            return plugins[i].descriptor->execute(operation, argc, argv, result);
        }
    }
    printf("  未找到插件: %s\n", plugin_name);
    return -1;
}

void plugin_list(void) {
    printf("  已加载插件列表 (%d 个):\n", plugin_count);
    for (int i = 0; i < plugin_count; i++) {
        if (!plugins[i].active) continue;
        const plugin_descriptor_t *d = plugins[i].descriptor;
        printf("    [%d] %s v%d, 能力=0x%x\n",
               i, d->get_name(), d->get_version(), d->get_capabilities());
    }
}

void demo_plugin_manager(void) {
    printf("===== 插件管理器演示 =====\n\n");

    printf("--- 加载插件 ---\n");
    plugin_load("./plugin_hello.so");
    plugin_load("./plugin_calc.so");
    printf("\n");

    printf("--- 列出插件 ---\n");
    plugin_list();
    printf("\n");

    printf("--- 执行 hello 插件 ---\n");
    int dummy = 0;
    plugin_execute("hello", "greet", 0, &dummy, NULL);
    int echo_args[] = {10, 20, 30};
    plugin_execute("hello", "echo", 3, echo_args, NULL);
    printf("\n");

    printf("--- 执行 calc 插件 ---\n");
    int result = 0;
    int args[] = {15, 4};
    plugin_execute("calc", "add", 2, args, &result);
    plugin_execute("calc", "sub", 2, args, &result);
    plugin_execute("calc", "mul", 2, args, &result);
    plugin_execute("calc", "div", 2, args, &result);
    plugin_execute("calc", "pow", 2, args, &result);
    int sum_args[] = {1, 2, 3, 4, 5};
    plugin_execute("calc", "sum", 5, sum_args, &result);
    printf("\n");

    printf("--- 测试错误处理 ---\n");
    plugin_execute("calc", "div", 2, (int[]){10, 0}, &result);
    plugin_execute("calc", "unknown_op", 2, args, &result);
    plugin_execute("nonexistent", "test", 0, &dummy, NULL);
    printf("\n");

    printf("--- 卸载所有插件 ---\n");
    plugin_unload_all();
    printf("\n");
}

void demo_plugin_architecture_patterns(void) {
    printf("===== 插件架构设计模式 =====\n\n");

    printf("1. 接口隔离模式 (Interface Segregation)\n");
    printf("   插件只暴露必要的接口，隐藏内部实现\n");
    printf("   通过 plugin_descriptor_t 结构体函数指针表实现\n\n");

    printf("2. 注册表模式 (Registry Pattern)\n");
    printf("   主程序维护一个插件注册表 (loaded_plugin_t plugins[])\n");
    printf("   插件加载时注册，卸载时注销\n\n");

    printf("3. 观察者模式 (Observer)\n");
    printf("   通过 plugin_context_t 中的 on_error 回调\n");
    printf("   插件可以向主程序报告事件\n\n");

    printf("4. 策略模式 (Strategy)\n");
    printf("   不同插件实现相同接口但不同算法\n");
    printf("   主程序根据需要选择插件执行\n\n");

    printf("5. 热重载概念 (Hot Reload)\n");
    printf("   步骤:\n");
    printf("   a. dlclose 卸载旧版本插件\n");
    printf("   b. 编译新版本插件 .so\n");
    printf("   c. dlopen 加载新版本插件\n");
    printf("   注意: 需要确保没有指向旧插件的悬空指针\n\n");

    printf("6. 插件发现机制\n");
    printf("   a. 约定目录: 扫描 ./plugins/ 目录下的 .so 文件\n");
    printf("   b. 配置文件: 从 plugins.conf 读取插件列表\n");
    printf("   c. 环境变量: PLUGIN_PATH=/path/to/plugins\n");
    printf("   d. 命令行参数: ./main --plugin ./plugin_foo.so\n\n");
}

void demo_plugin_pitfalls(void) {
    printf("===== 插件开发常见陷阱 =====\n\n");

    printf("陷阱1: ABI 不兼容\n");
    printf("  插件和主程序使用不同编译器或不同编译选项\n");
    printf("  结构体布局、对齐方式可能不同\n");
    printf("  解决: 严格定义ABI，使用固定大小类型(int32_t等)\n\n");

    printf("陷阱2: 内存所有权不清\n");
    printf("  插件分配的内存由谁释放？主程序还是插件？\n");
    printf("  解决: 谁分配谁释放，或提供统一的分配/释放接口\n\n");

    printf("陷阱3: 悬空指针\n");
    printf("  dlclose 后仍使用从插件获取的函数指针\n");
    printf("  解决: dlclose 前确保所有引用已清除\n\n");

    printf("陷阱4: 符号冲突\n");
    printf("  多个插件定义了相同的全局符号\n");
    printf("  解决: 使用 -fvisibility=hidden，只导出 plugin_entry\n\n");

    printf("陷阱5: 初始化顺序\n");
    printf("  插件A依赖插件B，但A先加载\n");
    printf("  解决: 延迟初始化或使用依赖声明\n\n");

    printf("举一反三 - 生产级插件系统还需要:\n");
    printf("  - 插件沙箱: 限制插件的权限和资源\n");
    printf("  - 版本协商: 运行时协商兼容的接口版本\n");
    printf("  - 依赖管理: 自动加载插件依赖的其他插件\n");
    printf("  - 配置系统: 每个插件独立的配置管理\n");
    printf("  - 日志系统: 统一的日志记录接口\n");
    printf("\n");
}

int main(void) {
    printf("================================================\n");
    printf("  插件架构深入 - 基于dlopen的插件系统设计\n");
    printf("================================================\n\n");

    demo_plugin_manager();
    demo_plugin_architecture_patterns();
    demo_plugin_pitfalls();

    printf("================================================\n");
    printf("  演示结束\n");
    printf("================================================\n");
    return 0;
}
