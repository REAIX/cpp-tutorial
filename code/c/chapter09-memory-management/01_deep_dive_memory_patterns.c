/** @file 01_deep_dive_memory_patterns.c
 *  @brief 内存管理模式：内存池、Arena分配器、自定义分配器、栈式分配(alloca)
 *  @description 对应文档: 09-内存管理 | 举一反三：高效内存分配策略
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct {
    void *buffer;
    size_t capacity;
    size_t used;
} MemoryPool;

MemoryPool *pool_create(size_t capacity) {
    MemoryPool *pool = (MemoryPool *)malloc(sizeof(MemoryPool));
    if (!pool) return NULL;

    pool->buffer = malloc(capacity);
    if (!pool->buffer) {
        free(pool);
        return NULL;
    }

    pool->capacity = capacity;
    pool->used = 0;
    return pool;
}

void *pool_alloc(MemoryPool *pool, size_t size) {
    size_t aligned_size = (size + 7) & ~(size_t)7;
    if (pool->used + aligned_size > pool->capacity) {
        return NULL;
    }
    void *ptr = (char *)pool->buffer + pool->used;
    pool->used += aligned_size;
    return ptr;
}

void pool_reset(MemoryPool *pool) {
    pool->used = 0;
}

void pool_destroy(MemoryPool *pool) {
    if (pool) {
        free(pool->buffer);
        free(pool);
    }
}

void demo_memory_pool(void) {
    printf("=== 内存池 (Memory Pool) ===\n");

    MemoryPool *pool = pool_create(1024);
    if (!pool) return;

    int *a = (int *)pool_alloc(pool, sizeof(int));
    int *b = (int *)pool_alloc(pool, sizeof(int));
    char *str = (char *)pool_alloc(pool, 32);

    if (a) *a = 42;
    if (b) *b = 99;
    if (str) strcpy(str, "Hello, Pool!");

    printf("pool_alloc: *a = %d, *b = %d, str = \"%s\"\n",
           a ? *a : 0, b ? *b : 0, str ? str : "");
    printf("已使用: %zu / %zu 字节\n", pool->used, pool->capacity);

    printf("\n重置内存池:\n");
    pool_reset(pool);
    printf("已使用: %zu / %zu 字节 (所有分配一次性回收)\n", pool->used, pool->capacity);

    printf("\n注意: 重置后 a, b, str 指向的内存已回收, 不要再使用!\n");

    pool_destroy(pool);

    printf("\n内存池优势:\n");
    printf("1. 分配极快 (只需移动指针, 无需搜索空闲列表)\n");
    printf("2. 释放极快 (重置即可, 无需逐个 free)\n");
    printf("3. 无内存碎片\n");
    printf("4. 适合大量小对象、生命周期相同的场景\n");

    printf("\n");
}

typedef struct {
    char *buffer;
    size_t capacity;
    size_t offset;
} Arena;

Arena *arena_create(size_t capacity) {
    Arena *a = (Arena *)malloc(sizeof(Arena));
    if (!a) return NULL;
    a->buffer = (char *)malloc(capacity);
    if (!a->buffer) { free(a); return NULL; }
    a->capacity = capacity;
    a->offset = 0;
    return a;
}

void *arena_alloc(Arena *a, size_t size) {
    if (a->offset + size > a->capacity) return NULL;
    void *ptr = a->buffer + a->offset;
    a->offset += size;
    return ptr;
}

size_t arena_save(const Arena *a) {
    return a->offset;
}

void arena_restore(Arena *a, size_t saved) {
    a->offset = saved;
}

void arena_destroy(Arena *a) {
    if (a) { free(a->buffer); free(a); }
}

void demo_arena_allocator(void) {
    printf("=== Arena 分配器 ===\n");

    Arena *arena = arena_create(4096);
    if (!arena) return;

    size_t checkpoint = arena_save(arena);

    int *arr = (int *)arena_alloc(arena, 10 * sizeof(int));
    char *msg = (char *)arena_alloc(arena, 64);

    if (arr) for (int i = 0; i < 10; i++) arr[i] = i;
    if (msg) strcpy(msg, "Arena allocated string");

    printf("分配后: offset = %zu\n", arena->offset);
    if (arr) printf("arr[5] = %d\n", arr[5]);
    if (msg) printf("msg = \"%s\"\n", msg);

    printf("\n恢复到检查点:\n");
    arena_restore(arena, checkpoint);
    printf("offset = %zu (所有分配被撤销)\n", arena->offset);

    printf("\nArena 特点:\n");
    printf("1. 支持 save/restore (类似栈帧)\n");
    printf("2. 适合编译器、解析器等有嵌套作用域的场景\n");
    printf("3. 比内存池更灵活, 可以部分回收\n");

    arena_destroy(arena);

    printf("\n");
}

typedef struct {
    void *(*alloc)(size_t);
    void (*dealloc)(void *);
} Allocator;

void *default_alloc(size_t size) { return malloc(size); }
void default_dealloc(void *p) { free(p); }

static size_t total_allocated = 0;

static void *tracking_alloc(size_t size) {
    void *p = malloc(size + sizeof(size_t));
    if (!p) return NULL;
    size_t *header = (size_t *)p;
    *header = size;
    total_allocated += size;
    return (void *)(header + 1);
}

static void tracking_dealloc(void *p) {
    if (!p) return;
    size_t *header = ((size_t *)p) - 1;
    total_allocated -= *header;
    free(header);
}

void demo_custom_allocator(void) {
    printf("=== 自定义分配器模式 ===\n");

    Allocator tracking_allocator = {tracking_alloc, tracking_dealloc};

    int *a = (int *)tracking_allocator.alloc(sizeof(int));
    char *b = (char *)tracking_allocator.alloc(100);
    double *c = (double *)tracking_allocator.alloc(sizeof(double));

    if (a) *a = 42;
    if (b) strcpy(b, "tracked");
    if (c) *c = 3.14;

    printf("当前分配总量: %zu 字节\n", total_allocated);
    printf("a = %d, b = \"%s\", c = %.2f\n", a ? *a : 0, b ? b : "", c ? *c : 0);

    tracking_allocator.dealloc(a);
    printf("释放 a 后: %zu 字节\n", total_allocated);
    tracking_allocator.dealloc(b);
    printf("释放 b 后: %zu 字节\n", total_allocated);
    tracking_allocator.dealloc(c);
    printf("释放 c 后: %zu 字节\n", total_allocated);

    printf("\n自定义分配器可以:\n");
    printf("1. 跟踪内存使用\n");
    printf("2. 添加调试信息\n");
    printf("3. 使用特定内存区域\n");
    printf("4. 实现线程安全的分配\n");

    printf("\n");
}

void demo_stack_allocation(void) {
    printf("=== 栈式分配 (alloca / VLA) ===\n");

    printf("alloca: 在栈上分配内存, 函数返回时自动释放\n\n");

    printf("  void func(int n) {\n");
    printf("      int *arr = alloca(n * sizeof(int));\n");
    printf("      // 使用 arr...\n");
    printf("      // 无需 free, 函数返回时自动释放\n");
    printf("  }\n\n");

    printf("VLA (变长数组, C99):\n");
    printf("  void func(int n) {\n");
    printf("      int arr[n];  // 栈上分配, 自动释放\n");
    printf("  }\n\n");

    printf("注意事项:\n");
    printf("1. 栈空间有限 (通常1-8MB), 不要分配大块内存\n");
    printf("2. alloca 不在 C 标准中, 但广泛支持\n");
    printf("3. VLA 在 C11 中变为可选特性\n");
    printf("4. 分配失败不会返回 NULL, 而是栈溢出\n");

    printf("\n");
}

void demo_allocation_strategy_comparison(void) {
    printf("=== 分配策略对比 ===\n");

    printf("%-15s %-10s %-10s %-15s %-15s\n",
           "策略", "分配速度", "释放速度", "碎片", "适用场景");
    printf("%-15s %-10s %-10s %-15s %-15s\n",
           "-----", "------", "------", "----", "--------");
    printf("%-15s %-10s %-10s %-15s %-15s\n",
           "malloc/free", "中等", "中等", "可能严重", "通用");
    printf("%-15s %-10s %-10s %-15s %-15s\n",
           "内存池", "极快", "极快", "无", "大量小对象");
    printf("%-15s %-10s %-10s %-15s %-15s\n",
           "Arena", "极快", "极快", "无", "编译器/解析器");
    printf("%-15s %-10s %-10s %-15s %-15s\n",
           "栈分配", "极快", "自动", "无", "临时小数据");
    printf("%-15s %-10s %-10s %-15s %-15s\n",
           "对象池", "极快", "极快", "无", "固定大小对象");

    printf("\n");
}

int main(void) {
    demo_memory_pool();
    demo_arena_allocator();
    demo_custom_allocator();
    demo_stack_allocation();
    demo_allocation_strategy_comparison();

    return 0;
}
