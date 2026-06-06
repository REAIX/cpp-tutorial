/**
 * @file 03_example_template.c
 * @brief 泛型编程: 宏和void*实现泛型
 * @description 对应文档: 29-C语言面向对象实现-进阶
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CLAMP(x, lo, hi) (MIN(MAX(x, lo), hi))

#define SWAP(type, a, b) do { type _tmp = (a); (a) = (b); (b) = _tmp; } while(0)

#define DEFINE_STACK(TYPE, NAME) \
typedef struct { \
    TYPE *data; \
    int top; \
    int capacity; \
} NAME##_Stack; \
\
static int NAME##_stack_init(NAME##_Stack *s, int cap) { \
    s->data = (TYPE *)malloc(cap * sizeof(TYPE)); \
    if (!s->data) return -1; \
    s->top = -1; \
    s->capacity = cap; \
    return 0; \
} \
\
static void NAME##_stack_destroy(NAME##_Stack *s) { \
    free(s->data); \
    s->data = NULL; \
    s->top = -1; \
} \
\
static int NAME##_stack_push(NAME##_Stack *s, TYPE val) { \
    if (s->top >= s->capacity - 1) return -1; \
    s->data[++s->top] = val; \
    return 0; \
} \
\
static int NAME##_stack_pop(NAME##_Stack *s, TYPE *val) { \
    if (s->top < 0) return -1; \
    *val = s->data[s->top--]; \
    return 0; \
}

DEFINE_STACK(int, Int)
DEFINE_STACK(double, Double)

void demo_macro_generic(void) {
    printf("\n=== demo_macro_generic ===\n");
    printf("宏实现泛型: 通过宏生成类型特定的代码\n\n");

    printf("通用工具宏:\n");
    printf("  MAX(3, 5) = %d\n", MAX(3, 5));
    printf("  MIN(3.14, 2.72) = %.2f\n", MIN(3.14, 2.72));
    printf("  CLAMP(15, 0, 10) = %d\n", CLAMP(15, 0, 10));

    int a = 10, b = 20;
    SWAP(int, a, b);
    printf("  SWAP(int, 10, 20) -> a=%d, b=%d\n", a, b);

    printf("\n宏生成栈:\n");
    Int_Stack is;
    Int_stack_init(&is, 10);
    Int_stack_push(&is, 42);
    Int_stack_push(&is, 99);
    int ival;
    Int_stack_pop(&is, &ival);
    printf("  Int栈弹出: %d\n", ival);
    Int_stack_destroy(&is);

    Double_Stack ds;
    Double_stack_init(&ds, 10);
    Double_stack_push(&ds, 3.14);
    Double_stack_push(&ds, 2.72);
    double dval;
    Double_stack_pop(&ds, &dval);
    printf("  Double栈弹出: %.2f\n", dval);
    Double_stack_destroy(&ds);

    printf("\n宏泛型陷阱:\n");
    printf("  1. MAX(i++, j++) -> i可能自增两次!\n");
    printf("  2. 宏不检查类型, 可能传入不兼容类型\n");
    printf("  3. 调试困难, 宏展开后代码不可见\n");
    printf("  4. 代码膨胀, 每种类型生成一份代码\n");
}

typedef struct {
    void *data;
    size_t element_size;
    size_t size;
    size_t capacity;
} GenericVector;

int gvec_init(GenericVector *v, size_t element_size, size_t initial_cap) {
    v->element_size = element_size;
    v->size = 0;
    v->capacity = initial_cap > 0 ? initial_cap : 4;
    v->data = calloc(v->capacity, element_size);
    return v->data ? 0 : -1;
}

void gvec_destroy(GenericVector *v) {
    free(v->data);
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

int gvec_push_back(GenericVector *v, const void *element) {
    if (v->size >= v->capacity) {
        size_t new_cap = v->capacity * 2;
        void *new_data = realloc(v->data, new_cap * v->element_size);
        if (!new_data) return -1;
        v->data = new_data;
        v->capacity = new_cap;
    }
    memcpy((char *)v->data + v->size * v->element_size, element, v->element_size);
    v->size++;
    return 0;
}

void *gvec_get(const GenericVector *v, size_t index) {
    if (index >= v->size) return NULL;
    return (char *)v->data + index * v->element_size;
}

size_t gvec_size(const GenericVector *v) { return v->size; }

void gvec_sort(GenericVector *v, int (*compare)(const void *, const void *)) {
    qsort(v->data, v->size, v->element_size, compare);
}

static int int_cmp(const void *a, const void *b) {
    return *(const int *)a - *(const int *)b;
}

void demo_void_ptr_generic(void) {
    printf("\n=== demo_void_ptr_generic ===\n");
    printf("void*实现泛型: 运行时多态, 一种实现支持所有类型\n\n");

    printf("int向量:\n");
    GenericVector vi;
    gvec_init(&vi, sizeof(int), 4);
    for (int i = 5; i >= 1; i--) gvec_push_back(&vi, &i);
    for (size_t i = 0; i < gvec_size(&vi); i++) {
        printf("  [%zu] = %d\n", i, *(int *)gvec_get(&vi, i));
    }

    gvec_sort(&vi, int_cmp);
    qsort(vi.data, vi.size, vi.element_size, int_cmp);

    printf("排序后:\n");
    for (size_t i = 0; i < gvec_size(&vi); i++) {
        printf("  [%zu] = %d\n", i, *(int *)gvec_get(&vi, i));
    }
    gvec_destroy(&vi);

    printf("\ndouble向量:\n");
    GenericVector vd;
    gvec_init(&vd, sizeof(double), 4);
    double dvals[] = {3.14, 1.41, 2.72, 0.58};
    for (int i = 0; i < 4; i++) gvec_push_back(&vd, &dvals[i]);
    for (size_t i = 0; i < gvec_size(&vd); i++) {
        printf("  [%zu] = %.2f\n", i, *(double *)gvec_get(&vd, i));
    }
    gvec_destroy(&vd);

    printf("\nvoid*泛型特点:\n");
    printf("  1. 一份代码, 多种类型\n");
    printf("  2. 运行时无类型检查, 需要用户保证类型安全\n");
    printf("  3. 需要memcpy/memmove操作, 无法直接运算\n");
    printf("  4. 类似C++的std::vector<void*>\n");
}

static int double_cmp(const void *a, const void *b) {
    double da = *(const double *)a;
    double db = *(const double *)b;
    return (da > db) - (da < db);
}

void demo_generic_sort(void) {
    printf("\n=== demo_generic_sort ===\n");
    printf("qsort: C标准库的泛型排序\n\n");

    int ints[] = {5, 2, 8, 1, 9, 3};
    size_t n_ints = sizeof(ints) / sizeof(ints[0]);
    printf("排序前: ");
    for (size_t i = 0; i < n_ints; i++) printf("%d ", ints[i]);
    qsort(ints, n_ints, sizeof(int), int_cmp);
    printf("\n排序后: ");
    for (size_t i = 0; i < n_ints; i++) printf("%d ", ints[i]);
    printf("\n\n");

    double doubles[] = {3.14, 1.41, 2.72, 0.58};
    size_t n_doubles = sizeof(doubles) / sizeof(doubles[0]);
    printf("排序前: ");
    for (size_t i = 0; i < n_doubles; i++) printf("%.2f ", doubles[i]);
    qsort(doubles, n_doubles, sizeof(double), double_cmp);
    printf("\n排序后: ");
    for (size_t i = 0; i < n_doubles; i++) printf("%.2f ", doubles[i]);
    printf("\n\n");

    printf("宏泛型 vs void*泛型:\n");
    printf("  宏泛型: 编译时生成, 类型安全, 代码膨胀\n");
    printf("  void*: 运行时多态, 一份代码, 无类型检查\n");
    printf("  C++模板: 编译时生成, 类型安全, 零开销\n");
    printf("  选择: 简单工具用宏, 复杂容器用void*\n");
}

int main(void) {
    printf("泛型编程: 宏和void*实现泛型\n");

    demo_macro_generic();
    demo_void_ptr_generic();
    demo_generic_sort();

    printf("\n所有演示完成!\n");
    return 0;
}
