/** @file 02_example_pointer_arithmetic.c
 *  @brief 指针算术：指针加减整数、指针差值、指针遍历
 *  @description 对应文档: 06-指针
 */

#include <stdio.h>
#include <stddef.h>

void demo_pointer_add_sub(void) {
    printf("=== 指针加减整数 ===\n");

    int arr[5] = {10, 20, 30, 40, 50};
    int *p = arr;

    printf("数组: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n\n");

    printf("p 指向 arr[0]: *p = %d, 地址 = %p\n", *p, (void *)p);
    printf("p + 1 指向 arr[1]: *(p+1) = %d, 地址 = %p\n", *(p + 1), (void *)(p + 1));
    printf("p + 2 指向 arr[2]: *(p+2) = %d, 地址 = %p\n", *(p + 2), (void *)(p + 2));

    printf("\n指针 +1 实际移动的字节数 = sizeof(int) = %zu\n", sizeof(int));
    printf("地址差: %td 字节 (等于 sizeof(int))\n", (ptrdiff_t)((char *)(p + 1) - (char *)p));

    printf("\n");
}

void demo_pointer_different_types(void) {
    printf("=== 不同类型指针的步长 ===\n");

    char carr[4] = {'A', 'B', 'C', 'D'};
    int iarr[4] = {100, 200, 300, 400};
    double darr[4] = {1.1, 2.2, 3.3, 4.4};

    char *cp = carr;
    int *ip = iarr;
    double *dp = darr;

    printf("char*   +1 移动 %td 字节\n", (ptrdiff_t)((char *)(cp + 1) - (char *)cp));
    printf("int*    +1 移动 %td 字节\n", (ptrdiff_t)((char *)(ip + 1) - (char *)ip));
    printf("double* +1 移动 %td 字节\n", (ptrdiff_t)((char *)(dp + 1) - (char *)dp));

    printf("\n核心规则: 指针 +N 实际移动 N * sizeof(所指类型) 字节\n");
    printf("\n");
}

void demo_pointer_difference(void) {
    printf("=== 指针差值 ===\n");

    int arr[6] = {1, 2, 3, 4, 5, 6};
    int *p1 = &arr[1];
    int *p2 = &arr[4];

    printf("p1 指向 arr[1] = %d\n", *p1);
    printf("p2 指向 arr[4] = %d\n", *p2);
    printf("p2 - p1 = %td (元素个数差)\n", p2 - p1);
    printf("p1 - p2 = %td (可以为负数)\n", p1 - p2);

    printf("\n指针差值的类型是 ptrdiff_t, 结果以元素为单位, 不是字节\n");
    printf("\n");
}

void demo_pointer_comparison(void) {
    printf("=== 指针比较 ===\n");

    int arr[5] = {5, 10, 15, 20, 25};
    int *p1 = &arr[1];
    int *p2 = &arr[3];

    printf("p1 指向 arr[1], p2 指向 arr[3]\n");
    printf("p1 < p2: %s\n", p1 < p2 ? "true" : "false");
    printf("p1 > p2: %s\n", p1 > p2 ? "true" : "false");
    printf("p1 == p2: %s\n", p1 == p2 ? "true" : "false");
    printf("p1 != p2: %s\n", p1 != p2 ? "true" : "false");

    printf("\n同一数组内的指针可以比较大小, 不同数组的指针比较是未定义行为\n");
    printf("\n");
}

void demo_pointer_traversal(void) {
    printf("=== 指针遍历数组 ===\n");

    int arr[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int *begin = arr;
    int *end = arr + 8;

    printf("方式1: 指针从 begin 遍历到 end\n");
    for (int *p = begin; p != end; p++) {
        printf("%d ", *p);
    }
    printf("\n");

    printf("方式2: 指针 + 偏移量\n");
    for (int i = 0; i < 8; i++) {
        printf("%d ", *(begin + i));
    }
    printf("\n");

    printf("方式3: 指针下标运算\n");
    for (int i = 0; i < 8; i++) {
        printf("%d ", begin[i]);
    }
    printf("\n");

    printf("\n三种方式等价, 方式1最常用且高效\n");
    printf("\n");
}

void demo_pointer_increment_decrement(void) {
    printf("=== 指针自增自减 ===\n");

    int arr[5] = {100, 200, 300, 400, 500};
    int *p = arr;

    printf("初始: p 指向 arr[0] = %d\n", *p);

    p++;
    printf("p++ 后: p 指向 arr[1] = %d\n", *p);

    p += 2;
    printf("p += 2 后: p 指向 arr[3] = %d\n", *p);

    p--;
    printf("p-- 后: p 指向 arr[2] = %d\n", *p);

    printf("\n注意 *p++ 和 (*p)++ 的区别:\n");
    p = arr;
    int val = *p++;
    printf("*p++ 先取值再移动: val = %d, *p = %d\n", val, *p);

    p = arr;
    (*p)++;
    printf("(*p)++ 先解引用再自增: arr[0] = %d\n", arr[0]);

    printf("\n");
}

int main(void) {
    demo_pointer_add_sub();
    demo_pointer_different_types();
    demo_pointer_difference();
    demo_pointer_comparison();
    demo_pointer_traversal();
    demo_pointer_increment_decrement();

    return 0;
}
