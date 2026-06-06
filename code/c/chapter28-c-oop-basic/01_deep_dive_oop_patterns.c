/**
 * @file 01_deep_dive_oop_patterns.c
 * @brief OOP模式深入: 对象生命周期、构造/析构、引用计数
 * @description 对应文档: 28-C语言面向对象实现-基础
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *data;
    size_t length;
    size_t capacity;
} String;

String *string_create(const char *init) {
    String *s = (String *)malloc(sizeof(String));
    if (!s) return NULL;

    s->length = init ? strlen(init) : 0;
    s->capacity = s->length + 16;
    s->data = (char *)malloc(s->capacity);
    if (!s->data) { free(s); return NULL; }

    if (init) {
        memcpy(s->data, init, s->length);
    }
    s->data[s->length] = '\0';
    return s;
}

String *string_create_empty(void) {
    return string_create("");
}

void string_destroy(String *s) {
    if (!s) return;
    free(s->data);
    s->data = NULL;
    s->length = 0;
    s->capacity = 0;
    free(s);
}

int string_append(String *s, const char *str) {
    if (!s || !str) return -1;
    size_t add_len = strlen(str);
    size_t new_len = s->length + add_len;

    if (new_len + 1 > s->capacity) {
        size_t new_cap = new_len + 16;
        char *new_data = (char *)realloc(s->data, new_cap);
        if (!new_data) return -1;
        s->data = new_data;
        s->capacity = new_cap;
    }

    memcpy(s->data + s->length, str, add_len);
    s->length = new_len;
    s->data[s->length] = '\0';
    return 0;
}

const char *string_cstr(const String *s) {
    return s ? s->data : "";
}

size_t string_length(const String *s) {
    return s ? s->length : 0;
}

void demo_object_lifecycle(void) {
    printf("\n=== demo_object_lifecycle ===\n");
    printf("对象生命周期: 创建 -> 使用 -> 销毁\n\n");

    printf("1. 创建(构造):\n");
    String *s = string_create("Hello");
    printf("   string_create(\"Hello\"): \"%s\" (len=%zu)\n", string_cstr(s), string_length(s));

    printf("\n2. 使用:\n");
    string_append(s, ", ");
    string_append(s, "World!");
    printf("   追加后: \"%s\" (len=%zu)\n", string_cstr(s), string_length(s));

    printf("\n3. 销毁(析构):\n");
    string_destroy(s);
    printf("   string_destroy() 完成, 内存已释放\n");

    printf("\n生命周期管理原则:\n");
    printf("  1. 谁创建谁销毁(ownership)\n");
    printf("  2. 每个create对应一个destroy\n");
    printf("  3. destroy后指针置NULL, 防止悬空指针\n");
    printf("  4. 考虑使用宏简化: #define AUTO_DESTROY __attribute__((cleanup(...)))\n");
}

typedef struct {
    int *data;
    size_t size;
    size_t capacity;
} IntArray;

IntArray *int_array_create(size_t initial_cap) {
    IntArray *arr = (IntArray *)malloc(sizeof(IntArray));
    if (!arr) return NULL;

    arr->size = 0;
    arr->capacity = initial_cap > 0 ? initial_cap : 4;
    arr->data = (int *)calloc(arr->capacity, sizeof(int));
    if (!arr->data) { free(arr); return NULL; }
    return arr;
}

IntArray *int_array_copy(const IntArray *src) {
    if (!src) return NULL;
    IntArray *dst = (IntArray *)malloc(sizeof(IntArray));
    if (!dst) return NULL;

    dst->size = src->size;
    dst->capacity = src->capacity;
    dst->data = (int *)calloc(dst->capacity, sizeof(int));
    if (!dst->data) { free(dst); return NULL; }

    memcpy(dst->data, src->data, src->size * sizeof(int));
    return dst;
}

void int_array_destroy(IntArray *arr) {
    if (!arr) return;
    free(arr->data);
    arr->data = NULL;
    arr->size = 0;
    arr->capacity = 0;
    free(arr);
}

int int_array_push(IntArray *arr, int value) {
    if (!arr) return -1;
    if (arr->size >= arr->capacity) {
        size_t new_cap = arr->capacity * 2;
        int *new_data = (int *)realloc(arr->data, new_cap * sizeof(int));
        if (!new_data) return -1;
        arr->data = new_data;
        arr->capacity = new_cap;
    }
    arr->data[arr->size++] = value;
    return 0;
}

void demo_constructor_destructor(void) {
    printf("\n=== demo_constructor_destructor ===\n");
    printf("构造/析构模式: 确保对象始终处于有效状态\n\n");

    printf("1. 默认构造:\n");
    IntArray *arr1 = int_array_create(4);
    printf("   创建空数组, 容量=%zu\n", arr1->capacity);

    printf("\n2. 使用对象:\n");
    for (int i = 1; i <= 5; i++) int_array_push(arr1, i * 10);
    printf("   添加5个元素: ");
    for (size_t i = 0; i < arr1->size; i++) printf("%d ", arr1->data[i]);
    printf("(size=%zu, cap=%zu)\n", arr1->size, arr1->capacity);

    printf("\n3. 拷贝构造:\n");
    IntArray *arr2 = int_array_copy(arr1);
    printf("   拷贝: ");
    for (size_t i = 0; i < arr2->size; i++) printf("%d ", arr2->data[i]);
    printf("\n");

    printf("\n4. 析构:\n");
    int_array_destroy(arr1);
    int_array_destroy(arr2);
    printf("   两个数组已销毁\n");

    printf("\nC语言构造/析构模式:\n");
    printf("  构造: xxx_create() - 分配内存, 初始化成员\n");
    printf("  拷贝: xxx_copy() - 深拷贝, 独立所有权\n");
    printf("  析构: xxx_destroy() - 释放资源, 释放内存\n");
    printf("  注意: C没有自动析构, 必须手动调用destroy!\n");
}

typedef struct {
    void *data;
    int *ref_count;
    void (*deleter)(void *);
} SharedPtr;

SharedPtr *shared_ptr_create(void *data, void (*deleter)(void *)) {
    SharedPtr *sp = (SharedPtr *)malloc(sizeof(SharedPtr));
    if (!sp) return NULL;

    sp->data = data;
    sp->ref_count = (int *)malloc(sizeof(int));
    if (!sp->ref_count) { free(sp); return NULL; }
    *(sp->ref_count) = 1;
    sp->deleter = deleter;
    return sp;
}

SharedPtr *shared_ptr_copy(SharedPtr *sp) {
    if (!sp) return NULL;
    (*sp->ref_count)++;
    SharedPtr *new_sp = (SharedPtr *)malloc(sizeof(SharedPtr));
    if (!new_sp) { (*sp->ref_count)--; return NULL; }
    new_sp->data = sp->data;
    new_sp->ref_count = sp->ref_count;
    new_sp->deleter = sp->deleter;
    return new_sp;
}

void *shared_ptr_get(const SharedPtr *sp) {
    return sp ? sp->data : NULL;
}

int shared_ptr_use_count(const SharedPtr *sp) {
    return sp ? *(sp->ref_count) : 0;
}

void shared_ptr_release(SharedPtr *sp) {
    if (!sp) return;
    (*sp->ref_count)--;
    if (*(sp->ref_count) <= 0) {
        if (sp->deleter) sp->deleter(sp->data);
        free(sp->ref_count);
    }
    free(sp);
}

void buffer_deleter(void *data) {
    printf("    [引用计数归零, 释放缓冲区: %p]\n", data);
    free(data);
}

void demo_reference_counting(void) {
    printf("\n=== demo_reference_counting ===\n");
    printf("引用计数: 多个所有者共享资源, 最后一个释放\n\n");

    int *buffer = (int *)malloc(100 * sizeof(int));
    for (int i = 0; i < 100; i++) buffer[i] = i;

    printf("创建共享指针:\n");
    SharedPtr *sp1 = shared_ptr_create(buffer, buffer_deleter);
    printf("  sp1: 引用计数=%d\n", shared_ptr_use_count(sp1));

    printf("\n拷贝共享指针:\n");
    SharedPtr *sp2 = shared_ptr_copy(sp1);
    printf("  sp1: 引用计数=%d\n", shared_ptr_use_count(sp1));
    printf("  sp2: 引用计数=%d\n", shared_ptr_use_count(sp2));

    printf("\n再拷贝一份:\n");
    SharedPtr *sp3 = shared_ptr_copy(sp1);
    printf("  sp1: 引用计数=%d\n", shared_ptr_use_count(sp1));
    printf("  sp2: 引用计数=%d\n", shared_ptr_use_count(sp2));
    printf("  sp3: 引用计数=%d\n", shared_ptr_use_count(sp3));

    printf("\n释放sp1:\n");
    shared_ptr_release(sp1);
    printf("  sp2: 引用计数=%d\n", shared_ptr_use_count(sp2));

    printf("\n释放sp2:\n");
    shared_ptr_release(sp2);
    printf("  sp3: 引用计数=%d\n", shared_ptr_use_count(sp3));

    printf("\n释放sp3(最后一个):\n");
    shared_ptr_release(sp3);

    printf("\n引用计数陷阱:\n");
    printf("  1. 循环引用: A引用B, B引用A -> 永远不释放\n");
    printf("     解决: 使用弱引用(weak reference)打破循环\n");
    printf("  2. 线程安全: 引用计数增减需要原子操作\n");
    printf("  3. 性能开销: 每次拷贝/释放都要修改计数\n");
    printf("  4. 多次释放: 同一个SharedPtr不能release两次\n");
}

int main(void) {
    printf("OOP模式深入: 对象生命周期、构造/析构、引用计数\n");

    demo_object_lifecycle();
    demo_constructor_destructor();
    demo_reference_counting();

    printf("\n所有演示完成!\n");
    return 0;
}
