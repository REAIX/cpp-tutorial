/** @file 01_example_pointer_basics.c
 *  @brief 指针基础：声明、取地址、解引用、指针类型、void指针
 *  @description 对应文档: 06-指针
 */

#include <stdio.h>
#include <stdint.h>

void demo_pointer_declaration(void) {
    printf("=== 指针声明与初始化 ===\n");

    int a = 42;
    int *p = &a;

    printf("变量 a 的值: %d\n", a);
    printf("变量 a 的地址: %p\n", (void *)&a);
    printf("指针 p 存储的地址: %p\n", (void *)p);
    printf("通过 *p 解引用得到的值: %d\n", *p);

    printf("\n通过指针修改变量的值:\n");
    *p = 100;
    printf("执行 *p = 100 后, a = %d\n", a);
    printf("\n");
}

void demo_address_operator(void) {
    printf("=== 取地址运算符 & ===\n");

    char c = 'X';
    double d = 3.14;
    int arr[3] = {10, 20, 30};

    printf("char 变量 c 的地址: %p, 大小: %zu 字节\n", (void *)&c, sizeof(c));
    printf("double 变量 d 的地址: %p, 大小: %zu 字节\n", (void *)&d, sizeof(d));
    printf("数组 arr 的地址: %p\n", (void *)arr);
    printf("arr[0] 的地址: %p\n", (void *)&arr[0]);
    printf("arr[1] 的地址: %p\n", (void *)&arr[1]);
    printf("arr[2] 的地址: %p\n", (void *)&arr[2]);

    printf("\n注意: 数组名本身就是首元素地址, arr == &arr[0]: %s\n",
           (void *)arr == (void *)&arr[0] ? "true" : "false");
    printf("\n");
}

void demo_dereference_operator(void) {
    printf("=== 解引用运算符 * ===\n");

    int x = 55;
    int *px = &x;

    printf("x = %d\n", x);
    printf("*px = %d\n", *px);

    int y = *px;
    printf("int y = *px; => y = %d\n", y);

    *px = 77;
    printf("*px = 77; => x = %d\n", x);

    printf("\n解引用的本质: 通过地址直接访问内存中的数据\n");
    printf("\n");
}

void demo_pointer_types(void) {
    printf("=== 不同类型的指针 ===\n");

    int i = 10;
    double d = 2.718;
    char ch = 'A';

    int *pi = &i;
    double *pd = &d;
    char *pc = &ch;

    printf("int* 指针大小: %zu 字节, 指向 int(%zu 字节)\n", sizeof(pi), sizeof(*pi));
    printf("double* 指针大小: %zu 字节, 指向 double(%zu 字节)\n", sizeof(pd), sizeof(*pd));
    printf("char* 指针大小: %zu 字节, 指向 char(%zu 字节)\n", sizeof(pc), sizeof(*pc));

    printf("\n所有指针大小相同(都是地址), 但类型决定了解引用时读取的字节数\n");
    printf("\n");
}

void demo_void_pointer(void) {
    printf("=== void 指针 (通用指针) ===\n");

    int a = 42;
    double b = 3.14;
    char c = 'Z';

    void *vp;

    vp = &a;
    printf("void* 指向 int: 解引用为 %d\n", *(int *)vp);

    vp = &b;
    printf("void* 指向 double: 解引用为 %.2f\n", *(double *)vp);

    vp = &c;
    printf("void* 指向 char: 解引用为 %c\n", *(char *)vp);

    printf("\nvoid* 的特点:\n");
    printf("1. 可以指向任意类型的数据\n");
    printf("2. 不能直接解引用, 必须先转换为具体类型\n");
    printf("3. 常用于通用函数接口 (如 qsort 的比较函数, malloc 返回值)\n");
    printf("\n");
}

void demo_null_pointer(void) {
    printf("=== NULL 指针 ===\n");

    int *p = NULL;
    printf("NULL 指针的值: %p\n", (void *)p);

    if (p == NULL) {
        printf("p 是空指针, 不指向任何有效地址\n");
    }

    int x = 99;
    p = &x;
    if (p != NULL) {
        printf("赋值后 *p = %d\n", *p);
    }

    printf("\n使用指针前务必检查是否为 NULL, 防止解引用空指针导致崩溃\n");
    printf("\n");
}

void demo_const_pointer(void) {
    printf("=== const 与指针的组合 ===\n");

    int a = 10;
    int b = 20;

    const int *p1 = &a;
    printf("const int *p1: 指向常量int的指针, *p1 不可修改\n");
    printf("*p1 = %d\n", *p1);
    p1 = &b;
    printf("p1 可以指向其他变量, 现在 *p1 = %d\n", *p1);

    int *const p2 = &a;
    printf("\nint *const p2: 常量指针, p2 本身不可修改\n");
    printf("*p2 = %d\n", *p2);
    *p2 = 30;
    printf("*p2 可以修改指向的值, 现在 a = %d\n", a);

    const int *const p3 = &a;
    printf("\nconst int *const p3: 既不能改指向, 也不能改值\n");
    printf("*p3 = %d\n", *p3);

    printf("\n记忆口诀: const 在 * 左边修饰数据, 在 * 右边修饰指针\n");
    printf("\n");
}

int main(void) {
    demo_pointer_declaration();
    demo_address_operator();
    demo_dereference_operator();
    demo_pointer_types();
    demo_void_pointer();
    demo_null_pointer();
    demo_const_pointer();

    return 0;
}
