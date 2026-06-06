/** @file 01_example_malloc_free.c
 *  @brief 内存管理基础：malloc、free、calloc、realloc
 *  @description 对应文档: 09-内存管理
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void demo_malloc(void) {
    printf("=== malloc 基础 ===\n");

    int *p = (int *)malloc(sizeof(int));
    if (p == NULL) {
        printf("内存分配失败!\n");
        return;
    }
    *p = 42;
    printf("malloc 分配单个 int: *p = %d\n", *p);
    free(p);
    p = NULL;

    int *arr = (int *)malloc(5 * sizeof(int));
    if (arr == NULL) {
        printf("内存分配失败!\n");
        return;
    }
    for (int i = 0; i < 5; i++) {
        arr[i] = i * 10;
    }
    printf("malloc 分配数组: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    free(arr);
    arr = NULL;

    printf("\nmalloc 要点:\n");
    printf("1. 参数是字节数, 返回 void* 指针\n");
    printf("2. 分配的内存未初始化, 内容是不确定的\n");
    printf("3. 必须检查返回值是否为 NULL\n");
    printf("4. 使用完后必须 free\n");

    printf("\n");
}

void demo_calloc(void) {
    printf("=== calloc ===\n");

    int *arr = (int *)calloc(5, sizeof(int));
    if (arr == NULL) {
        printf("内存分配失败!\n");
        return;
    }

    printf("calloc 分配并初始化为零: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    free(arr);
    arr = NULL;

    printf("\ncalloc vs malloc:\n");
    printf("  calloc(count, size)  分配 count*size 字节, 全部初始化为0\n");
    printf("  malloc(size)         分配 size 字节, 不初始化\n");
    printf("  calloc 适合需要零初始化的场景 (如数组、计数器)\n");

    printf("\n");
}

void demo_realloc(void) {
    printf("=== realloc ===\n");

    int *arr = (int *)malloc(3 * sizeof(int));
    if (arr == NULL) return;
    arr[0] = 10; arr[1] = 20; arr[2] = 30;

    printf("初始数组 (3个元素): ");
    for (int i = 0; i < 3; i++) printf("%d ", arr[i]);
    printf("\n");

    int *new_arr = (int *)realloc(arr, 6 * sizeof(int));
    if (new_arr == NULL) {
        free(arr);
        printf("realloc 失败!\n");
        return;
    }
    arr = new_arr;

    for (int i = 3; i < 6; i++) {
        arr[i] = i * 10;
    }

    printf("扩容后 (6个元素): ");
    for (int i = 0; i < 6; i++) printf("%d ", arr[i]);
    printf("\n");

    printf("\nrealloc 行为:\n");
    printf("1. 可能原地扩容 (返回原指针)\n");
    printf("2. 可能分配新内存并复制 (返回新指针, 原内存自动释放)\n");
    printf("3. 必须用新返回值更新指针, 不要只用旧指针\n");
    printf("4. 如果 realloc 失败, 原内存仍然有效\n");

    free(arr);
    arr = NULL;

    printf("\n");
}

static int push(int value, int **arr, size_t *capacity, size_t *size) {
    if (*size >= *capacity) {
        size_t new_cap = *capacity == 0 ? 4 : *capacity * 2;
        int *new_arr = (int *)realloc(*arr, new_cap * sizeof(int));
        if (!new_arr) return 0;
        *arr = new_arr;
        *capacity = new_cap;
    }
    (*arr)[(*size)++] = value;
    return 1;
}

void demo_realloc_patterns(void) {
    printf("=== realloc 常见模式 ===\n");

    int *arr = NULL;
    size_t capacity = 0;
    size_t size = 0;

    push(1, &arr, &capacity, &size);
    push(2, &arr, &capacity, &size);
    push(3, &arr, &capacity, &size);
    push(4, &arr, &capacity, &size);
    push(5, &arr, &capacity, &size);

    printf("动态数组: ");
    for (size_t i = 0; i < size; i++) printf("%d ", arr[i]);
    printf("\n");
    printf("size = %zu, capacity = %zu\n", size, capacity);

    free(arr);
    arr = NULL;

    printf("\n扩容策略: 容量翻倍, 摊还 O(1) 时间复杂度\n");

    printf("\n");
}

void demo_malloc_zero(void) {
    printf("=== malloc(0) 的行为 ===\n");

    void *p = malloc(0);
    printf("malloc(0) 返回: %p\n", p);
    printf("malloc(0) 的行为是实现定义的:\n");
    printf("  可能返回 NULL, 也可能返回一个不可解引用的非空指针\n");
    printf("  无论哪种情况, 都必须 free\n");

    if (p != NULL) {
        free(p);
    }

    printf("\n最佳实践: 避免调用 malloc(0)\n");

    printf("\n");
}

void demo_free_rules(void) {
    printf("=== free 的规则 ===\n");

    printf("1. free(NULL) 是安全的, 什么都不做\n");
    int *p = NULL;
    free(p);
    printf("   free(NULL) 执行成功\n");

    printf("\n2. 只能 free malloc/calloc/realloc 返回的指针\n");
    printf("   int arr[10]; free(arr);  // 错误! 栈数组不能 free\n");

    printf("\n3. 不能 free 同一内存两次 (double free)\n");
    printf("   int *p = malloc(sizeof(int));\n");
    printf("   free(p); free(p);  // 错误!\n");

    printf("\n4. free 后指针变成悬垂指针, 建议立即置 NULL\n");
    printf("   free(p); p = NULL;\n");

    printf("\n");
}

int main(void) {
    demo_malloc();
    demo_calloc();
    demo_realloc();
    demo_realloc_patterns();
    demo_malloc_zero();
    demo_free_rules();

    return 0;
}
