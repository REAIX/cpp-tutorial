/** @file 03_example_memory_leak.c
 *  @brief 内存泄漏：常见泄漏模式、检测方法
 *  @description 对应文档: 09-内存管理
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void leaky_function(void) {
    char *data = (char *)malloc(100);
    if (data) {
        strcpy(data, "temporary data");
        printf("  函数内: data = \"%s\"\n", data);
    }
}

static int process_data(int error_code) {
    char *buffer = (char *)malloc(256);
    if (!buffer) return -1;

    if (error_code != 0) {
        printf("  错误! error_code = %d, 但忘记 free(buffer) 就返回了\n", error_code);
        free(buffer);
        return error_code;
    }

    strcpy(buffer, "success");
    printf("  处理成功: %s\n", buffer);
    free(buffer);
    return 0;
}

typedef struct {
    char *name;
    int *scores;
    int count;
} Student;

static Student *create_student(const char *name, int count) {
    Student *s = (Student *)malloc(sizeof(Student));
    if (!s) return NULL;

    s->name = (char *)malloc(strlen(name) + 1);
    if (!s->name) {
        free(s);
        return NULL;
    }
    strcpy(s->name, name);

    s->scores = (int *)malloc((size_t)count * sizeof(int));
    if (!s->scores) {
        free(s->name);
        free(s);
        return NULL;
    }
    s->count = count;
    memset(s->scores, 0, (size_t)count * sizeof(int));

    return s;
}

static void destroy_student(Student *s) {
    if (s) {
        free(s->scores);
        free(s->name);
        free(s);
    }
}

static size_t alloc_count = 0;
static size_t free_count = 0;

static void *tracked_malloc(size_t size) {
    void *p = malloc(size);
    if (p) alloc_count++;
    return p;
}

static void tracked_free(void *p) {
    if (p) {
        free(p);
        free_count++;
    }
}

void demo_leak_forget_free(void) {
    printf("=== 泄漏模式1: 忘记释放 ===\n");

    leaky_function();
    printf("  函数返回后, data 指针丢失, 100字节泄漏\n");

    printf("\n修复: 确保每个 malloc 都有对应的 free\n");

    printf("\n");
}

void demo_leak_overwrite_pointer(void) {
    printf("=== 泄漏模式2: 覆盖指针 ===\n");

    int *p = (int *)malloc(sizeof(int));
    if (p) *p = 42;
    printf("  第一次分配: *p = %d\n", p ? *p : 0);

    p = (int *)malloc(sizeof(int));
    if (p) *p = 99;
    printf("  第二次分配: *p = %d\n", p ? *p : 0);
    printf("  第一次分配的内存地址丢失, 发生泄漏!\n");

    free(p);
    p = NULL;

    printf("\n修复: 在重新赋值前先释放旧内存\n");

    printf("\n");
}

void demo_leak_early_return(void) {
    printf("=== 泄漏模式3: 提前返回 ===\n");

    process_data(0);
    process_data(1);

    printf("\n修复: 所有退出路径都必须释放资源\n");
    printf("推荐: 使用 goto cleanup 模式\n");

    printf("\n");
}

void demo_leak_nested_allocation(void) {
    printf("=== 泄漏模式4: 嵌套分配失败 ===\n");

    Student *stu = create_student("Alice", 5);
    if (stu) {
        stu->scores[0] = 95;
        printf("  学生: %s, 分数[0] = %d\n", stu->name, stu->scores[0]);
        destroy_student(stu);
    }

    printf("\n嵌套分配: 每个内层分配失败都要释放之前已分配的外层\n");

    printf("\n");
}

void demo_leak_realloc_fail(void) {
    printf("=== 泄漏模式5: realloc 失败导致泄漏 ===\n");

    int *arr = (int *)malloc(5 * sizeof(int));
    if (!arr) return;
    for (int i = 0; i < 5; i++) arr[i] = i;

    printf("  错误写法:\n");
    printf("    arr = realloc(arr, 1000000000 * sizeof(int));\n");
    printf("    如果 realloc 失败返回 NULL, 原指针丢失!\n\n");

    printf("  正确写法:\n");
    printf("    int *tmp = realloc(arr, new_size);\n");
    printf("    if (tmp) arr = tmp;\n");
    printf("    else { /* 处理失败, arr 仍有效 */ }\n");

    int *tmp = (int *)realloc(arr, 10 * sizeof(int));
    if (tmp) {
        arr = tmp;
        for (int i = 5; i < 10; i++) arr[i] = i;
        printf("  扩容成功: ");
        for (int i = 0; i < 10; i++) printf("%d ", arr[i]);
        printf("\n");
    } else {
        printf("  扩容失败, 原数组仍有效\n");
    }

    free(arr);

    printf("\n");
}

void demo_detection_methods(void) {
    printf("=== 内存泄漏检测方法 ===\n");

    printf("1. Valgrind (Linux):\n");
    printf("   valgrind --leak-check=full ./program\n");
    printf("   检测泄漏、越界、use-after-free 等\n\n");

    printf("2. AddressSanitizer (ASAN):\n");
    printf("   gcc -fsanitize=address -g program.c\n");
    printf("   编译时插入检测代码, 运行时报告错误\n\n");

    printf("3. 自定义分配器跟踪:\n");
    printf("   封装 malloc/free, 记录每次分配和释放\n\n");

    printf("4. 静态分析工具:\n");
    printf("   clang-tidy, cppcheck, Coverity 等\n\n");

    printf("5. 运行时统计:\n");

    alloc_count = 0;
    free_count = 0;

    int *a = (int *)tracked_malloc(sizeof(int));
    char *b = (char *)tracked_malloc(100);
    tracked_free(a);
    tracked_free(b);

    printf("   分配次数: %zu, 释放次数: %zu\n", alloc_count, free_count);
    if (alloc_count == free_count) {
        printf("   看起来没有泄漏 (但无法检测所有情况)\n");
    }

    printf("\n");
}

int main(void) {
    demo_leak_forget_free();
    demo_leak_overwrite_pointer();
    demo_leak_early_return();
    demo_leak_nested_allocation();
    demo_leak_realloc_fail();
    demo_detection_methods();

    return 0;
}
