/**
 * @file 01_deep_dive_project_structure.c
 * @brief 项目组织与封装模式
 * @description 对应文档: 16-多文件编程 - 项目组织模式、接口与实现、不透明指针、API设计
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct stack {
    int *data;
    int top;
    int capacity;
} stack_t;

static stack_t *stack_create(int cap) {
    stack_t *s = malloc(sizeof(stack_t));
    if (!s) return NULL;
    s->data = malloc(sizeof(int) * (size_t)cap);
    if (!s->data) { free(s); return NULL; }
    s->top = -1;
    s->capacity = cap;
    return s;
}

static void stack_destroy(stack_t *s) {
    if (s) { free(s->data); free(s); }
}

static int stack_push(stack_t *s, int val) {
    if (!s || s->top >= s->capacity - 1) return -1;
    s->data[++s->top] = val;
    return 0;
}

static int stack_pop(stack_t *s, int *val) {
    if (!s || s->top < 0) return -1;
    *val = s->data[s->top--];
    return 0;
}

void demo_module_pattern(void) {
    printf("=== C语言模块化模式 ===\n");
    printf("  C没有class/namespace, 用文件+命名前缀模拟模块:\n\n");
    printf("  logger.h (接口):\n");
    printf("    #ifndef LOGGER_H\n");
    printf("    #define LOGGER_H\n");
    printf("    typedef struct logger logger_t;\n");
    printf("    logger_t *logger_create(const char *name);\n");
    printf("    void logger_log(logger_t *l, const char *msg);\n");
    printf("    void logger_destroy(logger_t *l);\n");
    printf("    #endif\n\n");
    printf("  logger.c (实现):\n");
    printf("    #include \"logger.h\"\n");
    printf("    struct logger { char name[64]; FILE *fp; };\n");
    printf("    // ... 函数实现 ...\n\n");
}

void demo_opaque_pointer_advanced(void) {
    printf("=== 不透明指针进阶用法 ===\n");

    stack_t *stk = stack_create(10);
    if (stk) {
        stack_push(stk, 10);
        stack_push(stk, 20);
        stack_push(stk, 30);

        int val;
        while (stack_pop(stk, &val) == 0) {
            printf("  弹出: %d\n", val);
        }
        stack_destroy(stk);
    }

    printf("  不透明指针的关键:\n");
    printf("    - 头文件只放前向声明: typedef struct stack stack_t;\n");
    printf("    - 调用者无法知道sizeof(stack_t)\n");
    printf("    - 只能通过指针操作, 不能栈上分配\n\n");
}

void demo_api_versioning(void) {
    printf("=== API版本管理 ===\n");
    printf("  保持ABI兼容的方法:\n");
    printf("    1. 不透明指针: 内部可自由修改\n");
    printf("    2. 句柄模式: 用整数ID代替指针\n");
    printf("    3. 虚表模式: 函数指针表(类似C++虚函数)\n\n");
    printf("  版本号策略:\n");
    printf("    #define MYLIB_VERSION_MAJOR 1\n");
    printf("    #define MYLIB_VERSION_MINOR 2\n");
    printf("    #define MYLIB_VERSION_PATCH 3\n");
    printf("    MAJOR变更 = 不兼容的API修改\n");
    printf("    MINOR变更 = 向后兼容的功能新增\n");
    printf("    PATCH变更 = 向后兼容的bug修复\n\n");
}

void demo_header_organization(void) {
    printf("=== 头文件组织原则 ===\n");
    printf("  1. 头文件应自包含(包含所有依赖的头文件)\n");
    printf("  2. 头文件只放声明, 不放定义(inline除外)\n");
    printf("  3. 避免在头文件中使用using/typedef污染全局命名\n");
    printf("  4. 包含顺序: 本项目 -> 第三方 -> 系统头文件\n");
    printf("  5. 前向声明优于包含(减少编译依赖)\n\n");

    printf("  头文件内容顺序:\n");
    printf("    1. 头文件守卫\n");
    printf("    2. 文件级文档注释\n");
    printf("    3. #include依赖\n");
    printf("    4. 宏定义\n");
    printf("    5. 类型定义(typedef/struct/enum)\n");
    printf("    6. 函数声明\n");
    printf("    7. 结束守卫\n\n");
}

int main(void) {
    printf("========== 项目组织与封装模式深入 ==========\n\n");

    demo_module_pattern();
    demo_opaque_pointer_advanced();
    demo_api_versioning();
    demo_header_organization();

    printf("========== 所有演示完成 ==========\n");
    return 0;
}
