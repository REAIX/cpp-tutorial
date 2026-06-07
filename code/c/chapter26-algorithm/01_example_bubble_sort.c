/**
 * @file 01_example_bubble_sort.c
 * @brief 冒泡排序及优化(提前终止)
 * @description 对应文档: 27-排序与查找算法
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void print_array(const int *arr, int n) {
    printf("[");
    for (int i = 0; i < n; i++) {
        printf("%d", arr[i]);
        if (i < n - 1) printf(", ");
    }
    printf("]\n");
}

void bubble_sort_basic(int *arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

void bubble_sort_optimized(int *arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < n - 1 - i; j++) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
                swapped = 1;
            }
        }
        if (!swapped) break;
    }
}

void bubble_sort_with_position(int *arr, int n) {
    int last_swap_pos = n - 1;
    while (last_swap_pos > 0) {
        int current_swap_pos = 0;
        for (int j = 0; j < last_swap_pos; j++) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
                current_swap_pos = j;
            }
        }
        last_swap_pos = current_swap_pos;
    }
}

void copy_array(const int *src, int *dst, int n) {
    for (int i = 0; i < n; i++) dst[i] = src[i];
}

void demo_bubble_basic(void) {
    printf("\n=== demo_bubble_basic ===\n");

    int arr[] = {64, 34, 25, 12, 22, 11, 90};
    int n = sizeof(arr) / sizeof(arr[0]);

    printf("排序前: ");
    print_array(arr, n);

    bubble_sort_basic(arr, n);

    printf("排序后: ");
    print_array(arr, n);
}

void demo_bubble_optimized(void) {
    printf("\n=== demo_bubble_optimized ===\n");

    int arr1[] = {64, 34, 25, 12, 22, 11, 90};
    int arr2[] = {64, 34, 25, 12, 22, 11, 90};
    int arr3[] = {64, 34, 25, 12, 22, 11, 90};
    int n = sizeof(arr1) / sizeof(arr1[0]);

    bubble_sort_basic(arr1, n);
    printf("基础冒泡: ");
    print_array(arr1, n);

    bubble_sort_optimized(arr2, n);
    printf("优化冒泡(提前终止): ");
    print_array(arr2, n);

    bubble_sort_with_position(arr3, n);
    printf("优化冒泡(记录位置): ");
    print_array(arr3, n);
}

void demo_bubble_nearly_sorted(void) {
    printf("\n=== demo_bubble_nearly_sorted ===\n");
    printf("对近乎有序的数据, 优化版冒泡排序效率极高\n\n");

    int arr[] = {1, 2, 3, 5, 4, 6, 7, 8, 9, 10};
    int n = sizeof(arr) / sizeof(arr[0]);

    printf("近乎有序: ");
    print_array(arr, n);

    bubble_sort_optimized(arr, n);

    printf("排序后: ");
    print_array(arr, n);

    printf("\n优化效果:\n");
    printf("  基础冒泡: 总是O(n^2), 即使已有序\n");
    printf("  提前终止: 有序时O(n), 仅需一趟\n");
    printf("  记录位置: 跳过已有序的后部, 减少比较\n");
}

void demo_bubble_stability(void) {
    printf("\n=== demo_bubble_stability ===\n");
    printf("冒泡排序是稳定排序: 相等元素不交换, 保持相对顺序\n\n");

    typedef struct { int key; char label; } Item;
    Item items[] = {{3,'a'}, {1,'b'}, {3,'c'}, {2,'d'}, {1,'e'}};
    int n = sizeof(items) / sizeof(items[0]);

    printf("排序前: ");
    for (int i = 0; i < n; i++) printf("(%d,%c) ", items[i].key, items[i].label);
    printf("\n");

    for (int i = 0; i < n - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < n - 1 - i; j++) {
            if (items[j].key > items[j + 1].key) {
                Item temp = items[j];
                items[j] = items[j + 1];
                items[j + 1] = temp;
                swapped = 1;
            }
        }
        if (!swapped) break;
    }

    printf("排序后: ");
    for (int i = 0; i < n; i++) printf("(%d,%c) ", items[i].key, items[i].label);
    printf("\n");

    printf("注意: (3,a)仍在(3,c)前面 -> 稳定排序\n");
}

int main(void) {
    printf("冒泡排序及优化(提前终止)\n");

    demo_bubble_basic();
    demo_bubble_optimized();
    demo_bubble_nearly_sorted();
    demo_bubble_stability();

    printf("\n所有演示完成!\n");
    return 0;
}
