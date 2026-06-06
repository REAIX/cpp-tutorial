/**
 * @file 01_deep_dive_ds_patterns.c
 * @brief 数据结构模式深入: 泛型数据结构、回调模式、迭代器模式、内存所有权
 * @description 对应文档: 26-链表与数据结构
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct GenericNode {
    void *data;
    struct GenericNode *next;
} GenericNode;

typedef struct {
    GenericNode *head;
    int size;
    size_t element_size;
    void (*free_func)(void *);
} GenericList;

GenericList *glist_create(size_t element_size, void (*free_func)(void *)) {
    GenericList *list = (GenericList *)malloc(sizeof(GenericList));
    if (list) {
        list->head = NULL;
        list->size = 0;
        list->element_size = element_size;
        list->free_func = free_func;
    }
    return list;
}

void glist_push_back(GenericList *list, const void *data) {
    GenericNode *n = (GenericNode *)malloc(sizeof(GenericNode));
    if (!n) return;

    n->data = malloc(list->element_size);
    if (!n->data) { free(n); return; }
    memcpy(n->data, data, list->element_size);
    n->next = NULL;

    if (!list->head) {
        list->head = n;
    } else {
        GenericNode *cur = list->head;
        while (cur->next) cur = cur->next;
        cur->next = n;
    }
    list->size++;
}

void glist_foreach(const GenericList *list, void (*callback)(void *data, void *ctx), void *ctx) {
    GenericNode *cur = list->head;
    while (cur) {
        callback(cur->data, ctx);
        cur = cur->next;
    }
}

void glist_destroy(GenericList *list) {
    GenericNode *cur = list->head;
    while (cur) {
        GenericNode *temp = cur;
        if (list->free_func) {
            list->free_func(cur->data);
        } else {
            free(cur->data);
        }
        cur = cur->next;
        free(temp);
    }
    free(list);
}

typedef struct {
    GenericNode *current;
} ListIterator;

ListIterator glist_iterator_begin(const GenericList *list) {
    ListIterator it;
    it.current = list->head;
    return it;
}

int glist_iterator_has_next(const ListIterator *it) {
    return it->current != NULL;
}

void *glist_iterator_next(ListIterator *it) {
    if (!it->current) return NULL;
    void *data = it->current->data;
    it->current = it->current->next;
    return data;
}

void demo_generic_list(void) {
    printf("\n=== demo_generic_list ===\n");
    printf("泛型链表: 使用void*存储任意类型数据\n\n");

    GenericList *int_list = glist_create(sizeof(int), NULL);
    for (int i = 10; i <= 50; i += 10) {
        glist_push_back(int_list, &i);
    }

    printf("int链表: ");
    GenericNode *cur = int_list->head;
    while (cur) {
        printf("%d ", *(int *)cur->data);
        cur = cur->next;
    }
    printf("\n");

    typedef struct { double x; double y; } Point;
    GenericList *point_list = glist_create(sizeof(Point), NULL);
    Point p1 = {1.0, 2.0}, p2 = {3.0, 4.0}, p3 = {5.0, 6.0};
    glist_push_back(point_list, &p1);
    glist_push_back(point_list, &p2);
    glist_push_back(point_list, &p3);

    printf("Point链表: ");
    cur = point_list->head;
    while (cur) {
        Point *p = (Point *)cur->data;
        printf("(%.1f,%.1f) ", p->x, p->y);
        cur = cur->next;
    }
    printf("\n");

    glist_destroy(int_list);
    glist_destroy(point_list);

    printf("\n泛型数据结构陷阱:\n");
    printf("  1. void*丢失类型信息, 需要用户自己转换\n");
    printf("  2. 深拷贝vs浅拷贝: 嵌套指针需要自定义复制函数\n");
    printf("  3. 内存所有权: 谁负责释放数据?\n");
}

static int sum_result __attribute__((used)) = 0;
void sum_callback(void *data, void *ctx) {
    int val = *(int *)data;
    int *sum = (int *)ctx;
    *sum += val;
}

void print_callback(void *data, void *ctx) {
    int val = *(int *)data;
    int *count = (int *)ctx;
    printf("  [%d] 值=%d\n", (*count)++, val);
}

void demo_callback_pattern(void) {
    printf("\n=== demo_callback_pattern ===\n");
    printf("回调模式: 将操作作为参数传递, 解耦数据与处理逻辑\n\n");

    GenericList *list = glist_create(sizeof(int), NULL);
    int vals[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) glist_push_back(list, &vals[i]);

    printf("遍历打印:\n");
    int count = 0;
    glist_foreach(list, print_callback, &count);

    int sum = 0;
    glist_foreach(list, sum_callback, &sum);
    printf("求和: %d\n", sum);

    glist_destroy(list);

    printf("\n回调模式优势:\n");
    printf("  1. 数据结构不需要知道具体操作\n");
    printf("  2. 同一数据结构支持不同操作\n");
    printf("  3. 类似于C++的std::for_each + lambda\n");
}

void demo_iterator_pattern(void) {
    printf("\n=== demo_iterator_pattern ===\n");
    printf("迭代器模式: 统一遍历接口, 隐藏内部实现\n\n");

    GenericList *list = glist_create(sizeof(int), NULL);
    int vals[] = {100, 200, 300};
    for (int i = 0; i < 3; i++) glist_push_back(list, &vals[i]);

    printf("使用迭代器遍历:\n");
    ListIterator it = glist_iterator_begin(list);
    while (glist_iterator_has_next(&it)) {
        int *val = (int *)glist_iterator_next(&it);
        printf("  值=%d\n", *val);
    }

    glist_destroy(list);

    printf("\n迭代器模式优势:\n");
    printf("  1. 统一遍历接口, 不关心内部实现\n");
    printf("  2. 支持多种数据结构使用相同遍历代码\n");
    printf("  3. 可以实现过滤、变换等组合操作\n");
}

void demo_memory_ownership(void) {
    printf("\n=== demo_memory_ownership ===\n");
    printf("内存所有权: 谁分配, 谁释放\n\n");

    printf("策略1: 容器拥有所有权\n");
    printf("  - 容器负责分配和释放数据内存\n");
    printf("  - glist_push_back时深拷贝数据\n");
    printf("  - glist_destroy时释放所有数据\n\n");

    printf("策略2: 用户拥有所有权\n");
    printf("  - 容器只存储指针, 不负责释放\n");
    printf("  - 用户需要自己管理数据的生命周期\n");
    printf("  - 危险: 悬空指针风险\n\n");

    printf("策略3: 共享所有权(引用计数)\n");
    printf("  - 数据附带引用计数\n");
    printf("  - 容器增加引用计数, 删除时减少\n");
    printf("  - 引用计数为0时释放\n\n");

    printf("最佳实践:\n");
    printf("  1. 明确文档说明所有权策略\n");
    printf("  2. 优先使用深拷贝(容器拥有)\n");
    printf("  3. 提供自定义free函数支持复杂类型\n");
    printf("  4. 避免混合策略, 保持一致性\n");
}

int main(void) {
    printf("数据结构模式深入: 泛型、回调、迭代器、内存所有权\n");

    demo_generic_list();
    demo_callback_pattern();
    demo_iterator_pattern();
    demo_memory_ownership();

    printf("\n所有演示完成!\n");
    return 0;
}
