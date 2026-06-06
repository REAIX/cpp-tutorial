/**
 * @file 02_example_quick_sort.c
 * @brief 快速排序: Lomuto和Hoare分区方案
 * @description 对应文档: 27-排序与查找算法
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_array(const int *arr, int n) {
    printf("[");
    for (int i = 0; i < n; i++) {
        printf("%d", arr[i]);
        if (i < n - 1) printf(", ");
    }
    printf("]\n");
}

int lomuto_partition(int *arr, int low, int high) {
    int pivot = arr[high];
    int i = low;
    for (int j = low; j < high; j++) {
        if (arr[j] <= pivot) {
            int temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
            i++;
        }
    }
    int temp = arr[i];
    arr[i] = arr[high];
    arr[high] = temp;
    return i;
}

void quick_sort_lomuto(int *arr, int low, int high) {
    if (low < high) {
        int pi = lomuto_partition(arr, low, high);
        quick_sort_lomuto(arr, low, pi - 1);
        quick_sort_lomuto(arr, pi + 1, high);
    }
}

int hoare_partition(int *arr, int low, int high) {
    int pivot = arr[low + (high - low) / 2];
    int i = low - 1;
    int j = high + 1;
    while (1) {
        do { i++; } while (arr[i] < pivot);
        do { j--; } while (arr[j] > pivot);
        if (i >= j) return j;
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

void quick_sort_hoare(int *arr, int low, int high) {
    if (low < high) {
        int pi = hoare_partition(arr, low, high);
        quick_sort_hoare(arr, low, pi);
        quick_sort_hoare(arr, pi + 1, high);
    }
}

void insertion_sort(int *arr, int low, int high) {
    for (int i = low + 1; i <= high; i++) {
        int key = arr[i];
        int j = i - 1;
        while (j >= low && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

void quick_sort_optimized(int *arr, int low, int high) {
    if (high - low < 16) {
        insertion_sort(arr, low, high);
        return;
    }
    if (low < high) {
        int mid = low + (high - low) / 2;
        if (arr[low] > arr[mid]) { int t = arr[low]; arr[low] = arr[mid]; arr[mid] = t; }
        if (arr[low] > arr[high]) { int t = arr[low]; arr[low] = arr[high]; arr[high] = t; }
        if (arr[mid] > arr[high]) { int t = arr[mid]; arr[mid] = arr[high]; arr[high] = t; }
        int t = arr[mid]; arr[mid] = arr[high - 1]; arr[high - 1] = t;

        int pi = lomuto_partition(arr, low, high - 1);
        quick_sort_optimized(arr, low, pi - 1);
        quick_sort_optimized(arr, pi + 1, high);
    }
}

void demo_quick_sort_lomuto(void) {
    printf("\n=== demo_quick_sort_lomuto ===\n");

    int arr[] = {10, 7, 8, 9, 1, 5};
    int n = sizeof(arr) / sizeof(arr[0]);

    printf("排序前: ");
    print_array(arr, n);

    quick_sort_lomuto(arr, 0, n - 1);

    printf("Lomuto分区排序后: ");
    print_array(arr, n);

    printf("\nLomuto分区特点:\n");
    printf("  - 选最后一个元素为pivot\n");
    printf("  - 单指针i维护小于pivot的区域边界\n");
    printf("  - 交换次数较多, 但实现简单\n");
}

void demo_quick_sort_hoare(void) {
    printf("\n=== demo_quick_sort_hoare ===\n");

    int arr[] = {10, 7, 8, 9, 1, 5};
    int n = sizeof(arr) / sizeof(arr[0]);

    printf("排序前: ");
    print_array(arr, n);

    quick_sort_hoare(arr, 0, n - 1);

    printf("Hoare分区排序后: ");
    print_array(arr, n);

    printf("\nHoare分区特点:\n");
    printf("  - 选中间元素为pivot\n");
    printf("  - 双指针从两端向中间扫描\n");
    printf("  - 交换次数少, 效率更高\n");
}

void demo_quick_sort_optimized(void) {
    printf("\n=== demo_quick_sort_optimized ===\n");

    int arr[] = {3, 7, 8, 5, 2, 1, 9, 5, 4, 10, 6, 12, 11, 15, 13, 14, 16, 20, 18, 17, 19};
    int n = sizeof(arr) / sizeof(arr[0]);

    printf("排序前: ");
    print_array(arr, n);

    quick_sort_optimized(arr, 0, n - 1);

    printf("优化版排序后: ");
    print_array(arr, n);

    printf("\n优化策略:\n");
    printf("  1. 三数取中: 避免最坏情况(已排序数组)\n");
    printf("  2. 小数组用插入排序: 减少递归开销\n");
    printf("  3. 尾递归优化: 减少递归深度\n");
}

void demo_lomuto_vs_hoare(void) {
    printf("\n=== demo_lomuto_vs_hoare ===\n");
    printf("Lomuto vs Hoare 分区对比:\n\n");
    printf("特性         Lomuto          Hoare\n");
    printf("pivot选择    最后一个元素     中间元素\n");
    printf("指针         单指针           双指针\n");
    printf("交换次数     较多             较少\n");
    printf("实现难度     简单             稍复杂\n");
    printf("最坏情况     O(n^2)          O(n^2)\n");
    printf("平均性能     稍慢             稍快\n\n");

    printf("快速排序陷阱:\n");
    printf("  1. 已排序数组 -> O(n^2), 用三数取中避免\n");
    printf("  2. 大量重复元素 -> 退化, 用三路快排\n");
    printf("  3. 递归深度 -> 栈溢出, 用尾递归或迭代\n");
    printf("  4. 不稳定排序 -> 相等元素可能交换\n");
}

int main(void) {
    printf("快速排序: Lomuto和Hoare分区方案\n");

    demo_quick_sort_lomuto();
    demo_quick_sort_hoare();
    demo_quick_sort_optimized();
    demo_lomuto_vs_hoare();

    printf("\n所有演示完成!\n");
    return 0;
}
