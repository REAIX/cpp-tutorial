/**
 * @file 01_deep_dive_design_by_contract.c
 * @brief 契约式设计: 前置条件、后置条件、不变量
 * @description 对应文档: 29-C语言面向对象实现-进阶
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#ifdef NDEBUG
#define REQUIRE(cond, msg) ((void)0)
#define ENSURE(cond, msg) ((void)0)
#define INVARIANT(cond, msg) ((void)0)
#else
#define REQUIRE(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "前置条件失败: %s (%s:%d)\n", (msg), __FILE__, __LINE__); \
        abort(); \
    } \
} while(0)

#define ENSURE(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "后置条件失败: %s (%s:%d)\n", (msg), __FILE__, __LINE__); \
        abort(); \
    } \
} while(0)

#define INVARIANT(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "不变量失败: %s (%s:%d)\n", (msg), __FILE__, __LINE__); \
        abort(); \
    } \
} while(0)
#endif

typedef struct {
    double *data;
    int size;
    int capacity;
} SafeArray;

static void safe_array_check_invariant(const SafeArray *arr) {
    INVARIANT(arr != NULL, "数组指针不为空");
    INVARIANT(arr->data != NULL, "数据指针不为空");
    INVARIANT(arr->size >= 0, "大小非负");
    INVARIANT(arr->capacity > 0, "容量为正");
    INVARIANT(arr->size <= arr->capacity, "大小不超过容量");
}

SafeArray *safe_array_create(int capacity) {
    REQUIRE(capacity > 0, "容量必须为正数");

    SafeArray *arr = (SafeArray *)malloc(sizeof(SafeArray));
    if (!arr) return NULL;

    arr->data = (double *)calloc(capacity, sizeof(double));
    if (!arr->data) { free(arr); return NULL; }

    arr->size = 0;
    arr->capacity = capacity;

    safe_array_check_invariant(arr);
    ENSURE(arr->size == 0, "新数组大小为0");
    return arr;
}

double safe_array_get(const SafeArray *arr, int index) {
    REQUIRE(arr != NULL, "数组不为空");
    safe_array_check_invariant(arr);
    REQUIRE(index >= 0 && index < arr->size, "索引在范围内");

    double val = arr->data[index];

    ENSURE(!isnan(val), "返回值是有效数字");
    return val;
}

void safe_array_set(SafeArray *arr, int index, double value) {
    REQUIRE(arr != NULL, "数组不为空");
    safe_array_check_invariant(arr);
    REQUIRE(index >= 0 && index < arr->size, "索引在范围内");

    arr->data[index] = value;

    safe_array_check_invariant(arr);
    ENSURE(arr->data[index] == value, "设置后值正确");
}

int safe_array_push(SafeArray *arr, double value) {
    REQUIRE(arr != NULL, "数组不为空");
    safe_array_check_invariant(arr);

    if (arr->size >= arr->capacity) {
        int new_cap = arr->capacity * 2;
        double *new_data = (double *)realloc(arr->data, new_cap * sizeof(double));
        if (!new_data) return -1;
        arr->data = new_data;
        arr->capacity = new_cap;
    }

    arr->data[arr->size++] = value;

    safe_array_check_invariant(arr);
    ENSURE(arr->size > 0, "push后大小>0");
    ENSURE(arr->data[arr->size - 1] == value, "最后元素是刚push的值");
    return 0;
}

double safe_array_pop(SafeArray *arr) {
    REQUIRE(arr != NULL, "数组不为空");
    safe_array_check_invariant(arr);
    REQUIRE(arr->size > 0, "数组非空才能pop");

    double val = arr->data[--arr->size];

    safe_array_check_invariant(arr);
    ENSURE(arr->size >= 0, "pop后大小非负");
    return val;
}

void safe_array_destroy(SafeArray *arr) {
    REQUIRE(arr != NULL, "数组不为空");
    free(arr->data);
    free(arr);
}

void demo_preconditions(void) {
    printf("\n=== demo_preconditions ===\n");
    printf("前置条件(Precondition): 调用方必须满足的条件\n\n");

    SafeArray *arr = safe_array_create(4);
    printf("创建数组, 容量=4\n");

    safe_array_push(arr, 10.0);
    safe_array_push(arr, 20.0);
    safe_array_push(arr, 30.0);
    printf("添加3个元素\n");

    printf("获取 arr[1] = %.1f\n", safe_array_get(arr, 1));

    printf("\n测试前置条件(将触发断言, 注释掉以避免崩溃):\n");
    printf("  safe_array_get(arr, 5)  -> 前置条件: 索引在范围内\n");
    printf("  safe_array_get(arr, -1) -> 前置条件: 索引在范围内\n");
    printf("  safe_array_create(0)    -> 前置条件: 容量必须为正数\n");

    safe_array_destroy(arr);
}

void demo_postconditions(void) {
    printf("\n=== demo_postconditions ===\n");
    printf("后置条件(Postcondition): 函数返回时必须保证的条件\n\n");

    SafeArray *arr = safe_array_create(4);

    printf("push前: size=%d\n", arr->size);
    safe_array_push(arr, 42.0);
    printf("push后: size=%d, data[0]=%.1f (后置条件保证)\n",
           arr->size, arr->data[0]);

    printf("pop前: size=%d\n", arr->size);
    double val = safe_array_pop(arr);
    printf("pop后: size=%d, 弹出值=%.1f (后置条件保证)\n", arr->size, val);

    safe_array_destroy(arr);

    printf("\n后置条件示例:\n");
    printf("  push后: size增加1, 最后元素是push的值\n");
    printf("  pop后:  size减少1, 返回被移除的值\n");
    printf("  create后: size=0, capacity>=初始容量\n");
}

void demo_invariants(void) {
    printf("\n=== demo_invariants ===\n");
    printf("不变量(Invariant): 对象在任何时刻都必须满足的条件\n\n");

    SafeArray *arr = safe_array_create(2);
    printf("创建数组(cap=2), 不变量检查通过\n");

    safe_array_push(arr, 1.0);
    printf("push 1.0, 不变量检查通过\n");

    safe_array_push(arr, 2.0);
    printf("push 2.0, 不变量检查通过\n");

    safe_array_push(arr, 3.0);
    printf("push 3.0 (触发扩容), 不变量检查通过\n");

    printf("当前: size=%d, capacity=%d\n", arr->size, arr->capacity);

    safe_array_destroy(arr);

    printf("\nSafeArray不变量:\n");
    printf("  1. data不为NULL\n");
    printf("  2. size >= 0\n");
    printf("  3. capacity > 0\n");
    printf("  4. size <= capacity\n");
    printf("  不变量在每个公有方法入口和出口检查\n");
}

void demo_contract_summary(void) {
    printf("\n=== demo_contract_summary ===\n");
    printf("契约式设计(Design by Contract)总结:\n\n");

    printf("三方契约:\n");
    printf("  调用方承诺: 满足前置条件\n");
    printf("  被调方承诺: 满足后置条件\n");
    printf("  双方共同: 维护不变量\n\n");

    printf("C语言实现方式:\n");
    printf("  1. assert(): 标准断言, NDEBUG时消失\n");
    printf("  2. 自定义宏: REQUIRE/ENSURE/INVARIANT\n");
    printf("  3. 运行时检查: 始终生效, 不依赖NDEBUG\n\n");

    printf("最佳实践:\n");
    printf("  1. 开发阶段启用所有断言\n");
    printf("  2. 发布版本可选择性关闭(性能考虑)\n");
    printf("  3. 关键不变量始终检查(如安全相关)\n");
    printf("  4. 前置条件失败=调用方bug\n");
    printf("  5. 后置条件失败=实现方bug\n");
    printf("  6. 不变量失败=某处逻辑错误\n");
}

int main(void) {
    printf("契约式设计: 前置条件、后置条件、不变量\n");

    demo_preconditions();
    demo_postconditions();
    demo_invariants();
    demo_contract_summary();

    printf("\n所有演示完成!\n");
    return 0;
}
