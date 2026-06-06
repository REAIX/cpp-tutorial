/** @file 01_deep_dive_pointer_patterns.c
 *  @brief 指针进阶模式：双重指针、函数指针、const指针组合、restrict关键字
 *  @description 对应文档: 06-指针 | 举一反三：掌握指针的高级用法
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void demo_double_pointer(void) {
    printf("=== 双重指针 (指向指针的指针) ===\n");

    int x = 42;
    int *p = &x;
    int **pp = &p;

    printf("x = %d\n", x);
    printf("*p = %d (通过一级指针访问)\n", *p);
    printf("**pp = %d (通过二级指针访问)\n", **pp);

    printf("\n各变量地址:\n");
    printf("&x  = %p (int的地址)\n", (void *)&x);
    printf("p   = %p (int* 存储的值, 即 &x)\n", (void *)p);
    printf("&p  = %p (int* 自身的地址)\n", (void *)&p);
    printf("pp  = %p (int** 存储的值, 即 &p)\n", (void *)pp);

    printf("\n双重指针的典型用途: 在函数内修改调用者的指针\n");
    printf("\n");
}

void swap_pointers(int **a, int **b) {
    int *temp = *a;
    *a = *b;
    *b = temp;
}

void demo_double_pointer_swap(void) {
    printf("=== 用双重指针交换指针 ===\n");

    int x = 10, y = 20;
    int *pa = &x;
    int *pb = &y;

    printf("交换前: *pa = %d, *pb = %d\n", *pa, *pb);
    swap_pointers(&pa, &pb);
    printf("交换后: *pa = %d, *pb = %d\n", *pa, *pb);
    printf("x 和 y 的值没变, 但 pa 和 pb 指向的对象互换了\n");

    printf("\n");
}

void demo_function_pointer(void) {
    printf("=== 函数指针 ===\n");

    int add(int a, int b) { return a + b; }
    int sub(int a, int b) { return a - b; }
    int mul(int a, int b) { return a * b; }

    int (*op)(int, int) = add;
    printf("op = add; op(10, 3) = %d\n", op(10, 3));

    op = sub;
    printf("op = sub; op(10, 3) = %d\n", op(10, 3));

    op = mul;
    printf("op = mul; op(10, 3) = %d\n", op(10, 3));

    printf("\n函数指针数组:\n");
    int (*ops[])(int, int) = {add, sub, mul};
    const char *names[] = {"add", "sub", "mul"};
    for (int i = 0; i < 3; i++) {
        printf("%s(10, 3) = %d\n", names[i], ops[i](10, 3));
    }

    printf("\n");
}

typedef int (*BinaryOp)(int, int);

int compute(int a, int b, BinaryOp op) {
    return op(a, b);
}

void demo_function_pointer_typedef(void) {
    printf("=== 函数指针与 typedef ===\n");

    int add(int a, int b) { return a + b; }
    int mul(int a, int b) { return a * b; }

    printf("compute(5, 3, add) = %d\n", compute(5, 3, add));
    printf("compute(5, 3, mul) = %d\n", compute(5, 3, mul));

    printf("\ntypedef 让函数指针类型更清晰, 常用于回调函数和策略模式\n");
    printf("\n");
}

void demo_const_pointer_combinations(void) {
    printf("=== const 与指针的所有组合 ===\n");

    int a = 10, b = 20;

    {
        int *p = &a;
        *p = 15;
        p = &b;
        printf("1. int *p: 可改值, 可改指向\n");
    }

    {
        const int *p = &a;
        p = &b;
        (void)p;
        printf("2. const int *p: 不可改值, 可改指向 (指向常量)\n");
    }

    {
        int const *p = &a;
        p = &b;
        (void)p;
        printf("3. int const *p: 同上, const int* 和 int const* 等价\n");
    }

    {
        int *const p = &a;
        *p = 25;
        printf("4. int *const p: 可改值, 不可改指向 (常量指针)\n");
    }

    {
        const int *const p = &a;
        (void)p;
        printf("5. const int *const p: 不可改值, 不可改指向\n");
    }

    printf("\n阅读技巧: 从右往左读\n");
    printf("  const int *p => p is a pointer to int const\n");
    printf("  int *const p => p is a const pointer to int\n");
    printf("\n");
}

void demo_restrict_keyword(void) {
    printf("=== restrict 关键字 (C99) ===\n");

    printf("restrict 告诉编译器: 该指针是访问所指数据的唯一方式\n");
    printf("这允许编译器进行更激进的优化\n\n");

    void vector_add_restrict(int *restrict a, int *restrict b, int *restrict c, int n) {
        for (int i = 0; i < n; i++) {
            c[i] = a[i] + b[i];
        }
    }

    int x[4] = {1, 2, 3, 4};
    int y[4] = {5, 6, 7, 8};
    int z[4];

    vector_add_restrict(x, y, z, 4);

    printf("向量加法结果: ");
    for (int i = 0; i < 4; i++) printf("%d ", z[i]);
    printf("\n");

    printf("\n注意: restrict 是程序员对编译器的承诺\n");
    printf("如果违反承诺(如 a 和 c 指向同一内存), 行为是未定义的\n");
    printf("典型应用: memcpy, strcpy 等标准库函数的参数\n");
    printf("\n");
}

void demo_pointer_to_array_vs_array_of_pointers(void) {
    printf("=== 指向数组的指针 vs 指针数组 ===\n");

    int arr[3] = {10, 20, 30};
    int (*pa)[3] = &arr;
    printf("int (*pa)[3] = &arr;  指向整个数组的指针\n");
    printf("(*pa)[1] = %d\n", (*pa)[1]);

    int a = 1, b = 2, c = 3;
    int *ap[3] = {&a, &b, &c};
    printf("\nint *ap[3] = {&a, &b, &c};  指针数组\n");
    printf("*ap[1] = %d\n", *ap[1]);

    printf("\n区分方法:\n");
    printf("int (*pa)[3]  => pa 是指针, 指向 int[3]\n");
    printf("int *ap[3]   => ap 是数组, 包含 3 个 int*\n");
    printf("\n");
}

int main(void) {
    demo_double_pointer();
    demo_double_pointer_swap();
    demo_function_pointer();
    demo_function_pointer_typedef();
    demo_const_pointer_combinations();
    demo_restrict_keyword();
    demo_pointer_to_array_vs_array_of_pointers();

    return 0;
}
