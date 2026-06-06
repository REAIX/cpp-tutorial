/** @file 03_example_pointer_and_array.c
 *  @brief 指针与数组：数组名即指针、指向数组的指针、数组参数退化
 *  @description 对应文档: 06-指针
 */

#include <stdio.h>
#include <string.h>

void demo_array_name_as_pointer(void) {
    printf("=== 数组名作为指针 ===\n");

    int arr[5] = {10, 20, 30, 40, 50};

    printf("arr 的值(地址): %p\n", (void *)arr);
    printf("&arr[0] 的地址: %p\n", (void *)&arr[0]);
    printf("arr == &arr[0]: %s\n", (void *)arr == (void *)&arr[0] ? "true" : "false");

    printf("\n通过数组名指针访问元素:\n");
    for (int i = 0; i < 5; i++) {
        printf("*(arr + %d) = %d, arr[%d] = %d\n", i, *(arr + i), i, arr[i]);
    }

    printf("\narr[i] 等价于 *(arr + i), 这是 C 语言下标运算的本质\n");

    printf("\n注意: sizeof(arr) 是整个数组大小, 不是指针大小\n");
    printf("sizeof(arr) = %zu (5个int)\n", sizeof(arr));
    printf("sizeof(int*) = %zu (指针大小)\n", sizeof(int *));

    printf("\n");
}

void demo_pointer_to_array(void) {
    printf("=== 指向数组的指针 ===\n");

    int arr[5] = {1, 2, 3, 4, 5};

    int *p1 = arr;
    printf("int *p1 = arr; 指向第一个元素\n");
    printf("*p1 = %d\n", *p1);

    int (*p2)[5] = &arr;
    printf("\nint (*p2)[5] = &arr; 指向整个数组的指针\n");
    printf("(*p2)[0] = %d\n", (*p2)[0]);
    printf("(*p2)[4] = %d\n", (*p2)[4]);

    printf("\n区别:\n");
    printf("p1 类型是 int*, 指向单个元素, p1+1 移动 sizeof(int)\n");
    printf("p2 类型是 int(*)[5], 指向整个数组, p2+1 移动 5*sizeof(int)\n");
    printf("p1 + 1 地址偏移: %zu 字节\n", (size_t)((char *)(p1 + 1) - (char *)p1));
    printf("p2 + 1 地址偏移: %zu 字节\n", (size_t)((char *)(p2 + 1) - (char *)p2));

    printf("\n");
}

void demo_array_parameter_decay(int arr[], int size) {
    printf("=== 数组参数退化 ===\n");
    printf("函数参数中 int arr[] 实际退化为 int* 指针\n");
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsizeof-array-argument"
    printf("sizeof(arr) 在函数内 = %zu (指针大小, 不是数组大小!)\n", sizeof(arr));
#pragma GCC diagnostic pop
    printf("sizeof(int*) = %zu\n", sizeof(int *));

    printf("\n数组内容: ");
    for (int i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    printf("\n因此必须额外传递数组大小参数, 无法在函数内通过 sizeof 获取\n");
    printf("\n");
}

void demo_modify_array_via_pointer(void) {
    printf("=== 通过指针修改数组 ===\n");

    int arr[5] = {1, 2, 3, 4, 5};
    printf("修改前: ");
    for (int i = 0; i < 5; i++) printf("%d ", arr[i]);
    printf("\n");

    int *p = arr;
    for (int i = 0; i < 5; i++) {
        *(p + i) *= 10;
    }

    printf("修改后: ");
    for (int i = 0; i < 5; i++) printf("%d ", arr[i]);
    printf("\n");

    printf("\n");
}

void demo_2d_array_and_pointer(void) {
    printf("=== 二维数组与指针 ===\n");

    int matrix[3][4] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12}
    };

    int (*row_ptr)[4] = matrix;
    printf("int (*row_ptr)[4] = matrix; 指向一行的指针\n");
    printf("row_ptr[0][0] = %d\n", row_ptr[0][0]);
    printf("row_ptr[1][2] = %d\n", row_ptr[1][2]);
    printf("row_ptr[2][3] = %d\n", row_ptr[2][3]);

    printf("\n遍历二维数组:\n");
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            printf("%3d ", row_ptr[i][j]);
        }
        printf("\n");
    }

    printf("\n");
}

int main(void) {
    demo_array_name_as_pointer();
    demo_pointer_to_array();
    demo_modify_array_via_pointer();

    int arr[5] = {100, 200, 300, 400, 500};
    demo_array_parameter_decay(arr, 5);

    demo_2d_array_and_pointer();

    return 0;
}
