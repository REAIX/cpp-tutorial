/**
 * @file 01_example_singleton.c
 * @brief 单例模式: 静态变量与线程安全版本
 * @description 对应文档: 30-C语言设计模式实现
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

typedef struct {
    char config_path[256];
    int log_level;
    int max_connections;
} AppConfig;

static AppConfig *app_config_instance = NULL;

AppConfig *app_config_get_instance(void) {
    if (!app_config_instance) {
        app_config_instance = (AppConfig *)calloc(1, sizeof(AppConfig));
        if (app_config_instance) {
            strcpy(app_config_instance->config_path, "/etc/app/config.ini");
            app_config_instance->log_level = 2;
            app_config_instance->max_connections = 100;
        }
    }
    return app_config_instance;
}

void app_config_set(const char *path, int log_level, int max_conn) {
    AppConfig *cfg = app_config_get_instance();
    if (cfg) {
        strncpy(cfg->config_path, path, sizeof(cfg->config_path) - 1);
        cfg->log_level = log_level;
        cfg->max_connections = max_conn;
    }
}

void app_config_print(const AppConfig *cfg) {
    if (!cfg) return;
    printf("  配置: path=%s, log_level=%d, max_conn=%d\n",
           cfg->config_path, cfg->log_level, cfg->max_connections);
}

void demo_basic_singleton(void) {
    printf("\n=== demo_basic_singleton ===\n");
    printf("基本单例: 静态指针 + 懒初始化\n\n");

    AppConfig *cfg1 = app_config_get_instance();
    AppConfig *cfg2 = app_config_get_instance();

    printf("cfg1 == cfg2 ? %s (同一对象)\n", cfg1 == cfg2 ? "是" : "否");
    app_config_print(cfg1);

    app_config_set("/custom/path", 3, 200);
    printf("修改后:\n");
    app_config_print(cfg2);

    printf("\n基本单例问题:\n");
    printf("  非线程安全! 多线程同时调用可能创建多个实例\n");
}

typedef struct {
    char name[64];
    int version;
} ThreadSafeSingleton;

static ThreadSafeSingleton *ts_instance = NULL;

#ifdef _WIN32
static CRITICAL_SECTION ts_mutex;
static int ts_mutex_initialized = 0;

static void ts_ensure_mutex(void) {
    if (!ts_mutex_initialized) {
        InitializeCriticalSection(&ts_mutex);
        ts_mutex_initialized = 1;
    }
}
#else
static pthread_mutex_t ts_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

static volatile int ts_initialized = 0;

ThreadSafeSingleton *ts_singleton_get_instance(void) {
    if (!ts_initialized) {
#ifdef _WIN32
        ts_ensure_mutex();
        EnterCriticalSection(&ts_mutex);
#else
        pthread_mutex_lock(&ts_mutex);
#endif

        if (!ts_instance) {
            ts_instance = (ThreadSafeSingleton *)calloc(1, sizeof(ThreadSafeSingleton));
            if (ts_instance) {
                strcpy(ts_instance->name, "ThreadSafeSingleton");
                ts_instance->version = 1;
            }
            ts_initialized = 1;
        }

#ifdef _WIN32
        LeaveCriticalSection(&ts_mutex);
#else
        pthread_mutex_unlock(&ts_mutex);
#endif
    }
    return ts_instance;
}

void demo_thread_safe_singleton(void) {
    printf("\n=== demo_thread_safe_singleton ===\n");
    printf("线程安全单例: 双重检查锁定(DCL)\n\n");

    ThreadSafeSingleton *s1 = ts_singleton_get_instance();
    ThreadSafeSingleton *s2 = ts_singleton_get_instance();

    printf("s1 == s2 ? %s\n", s1 == s2 ? "是" : "否");
    printf("  name=%s, version=%d\n", s1->name, s1->version);

    printf("\n双重检查锁定(DCL):\n");
    printf("  1. 第一次检查(无锁): 已初始化则直接返回\n");
    printf("  2. 加锁: 防止多线程同时创建\n");
    printf("  3. 第二次检查(有锁): 确保只创建一次\n");
    printf("  4. volatile: 防止编译器优化重排序\n\n");

    printf("DCL陷阱:\n");
    printf("  C11之前没有内存模型保证, DCL可能不安全\n");
    printf("  C11: 使用atomic操作保证\n");
    printf("  替代方案: 程序启动时初始化(无并发问题)\n");
}

typedef struct {
    int counter;
} EagerSingleton;

static EagerSingleton eager_instance = {0};

EagerSingleton *eager_singleton_get(void) {
    return &eager_instance;
}

void demo_eager_singleton(void) {
    printf("\n=== demo_eager_singleton ===\n");
    printf("饿汉式单例: 编译时初始化, 天然线程安全\n\n");

    EagerSingleton *e1 = eager_singleton_get();
    EagerSingleton *e2 = eager_singleton_get();

    printf("e1 == e2 ? %s\n", e1 == e2 ? "是" : "否");
    e1->counter = 42;
    printf("e2->counter = %d (共享状态)\n", e2->counter);

    printf("\n三种单例实现对比:\n");
    printf("  懒汉式: 首次使用时创建, 非线程安全\n");
    printf("  DCL:    双重检查锁定, 线程安全, 但C中需谨慎\n");
    printf("  饿汉式: 编译时初始化, 天然线程安全, 但无法延迟\n\n");

    printf("单例模式陷阱:\n");
    printf("  1. 全局状态: 隐式依赖, 测试困难\n");
    printf("  2. 内存泄漏: 谁负责释放? 通常不释放\n");
    printf("  3. 多线程: 需要同步机制\n");
    printf("  4. 过度使用: 不是所有全局变量都需要单例\n");
}

int main(void) {
    printf("单例模式: 静态变量与线程安全版本\n");

    demo_basic_singleton();
    demo_thread_safe_singleton();
    demo_eager_singleton();

    printf("\n所有演示完成!\n");
    return 0;
}
