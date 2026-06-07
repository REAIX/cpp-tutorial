/**
 * @file 03_example_merge_sort.c
 * @brief 归并排序: 递归实现与临时数组
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

void merge(int *arr, int *temp, int left, int mid, int right) {
    int i = left;
    int j = mid + 1;
    int k = left;

    while (i <= mid && j <= right) {
        if (arr[i] <= arr[j]) {
            temp[k++] = arr[i++];
        } else {
            temp[k++] = arr[j++];
        }
    }

    while (i <= mid) temp[k++] = arr[i++];
    while (j <= right) temp[k++] = arr[j++];

    for (int x = left; x <= right; x++) {
        arr[x] = temp[x];
    }
}

void merge_sort_recursive(int *arr, int *temp, int left, int right) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    merge_sort_recursive(arr, temp, left, mid);
    merge_sort_recursive(arr, temp, mid + 1, right);
    merge(arr, temp, left, mid, right);
}

void merge_sort(int *arr, int n) {
    int *temp = (int *)malloc(n * sizeof(int));
    if (!temp) return;
    merge_sort_recursive(arr, temp, 0, n - 1);
    free(temp);
}

void merge_sort_iterative(int *arr, int n) {
    int *temp = (int *)malloc(n * sizeof(int));
    if (!temp) return;

    for (int width = 1; width < n; width *= 2) {
        for (int left = 0; left < n; left += 2 * width) {
            int mid = left + width - 1;
            int right = (left + 2 * width - 1 < n - 1) ? left + 2 * width - 1 : n - 1;
            if (mid < right) {
                merge(arr, temp, left, mid, right);
            }
        }
    }

    free(temp);
}

void demo_merge_sort_basic(void) {
    printf("\n=== demo_merge_sort_basic ===\n");

    int arr[] = {38, 27, 43, 3, 9, 82, 10};
    int n = sizeof(arr) / sizeof(arr[0]);

    printf("排序前: ");
    print_array(arr, n);

    merge_sort(arr, n);

    printf("排序后: ");
    print_array(arr, n);
}

void demo_merge_sort_iterative(void) {
    printf("\n=== demo_merge_sort_iterative ===\n");

    int arr[] = {12, 11, 13, 5, 6, 7, 3, 1, 9};
    int n = sizeof(arr) / sizeof(arr[0]);

    printf("排序前: ");
    print_array(arr, n);

    merge_sort_iterative(arr, n);

    printf("迭代归并排序后: ");
    print_array(arr, n);
}

void demo_merge_sort_stable(void) {
    printf("\n=== demo_merge_sort_stable ===\n");
    printf("归并排序是稳定排序: 相等元素保持原始顺序\n\n");

    typedef struct { int key; char label; } Item;
    Item items[] = {{3,'a'}, {1,'b'}, {3,'c'}, {2,'d'}, {1,'e'}, {2,'f'}};
    int n = sizeof(items) / sizeof(items[0]);

    printf("排序前: ");
    for (int i = 0; i < n; i++) printf("(%d,%c) ", items[i].key, items[i].label);
    printf("\n");

    Item *temp = (Item *)malloc(n * sizeof(Item));
    for (int width = 1; width < n; width *= 2) {
        for (int left = 0; left < n; left += 2 * width) {
            int mid = left + width - 1;
            int right = (left + 2 * width - 1 < n - 1) ? left + 2 * width - 1 : n - 1;
            if (mid >= right) continue;

            int i = left, j = mid + 1, k = left;
            while (i <= mid && j <= right) {
                if (items[i].key <= items[j].key) temp[k++] = items[i++];
                else temp[k++] = items[j++];
            }
            while (i <= mid) temp[k++] = items[i++];
            while (j <= right) temp[k++] = items[j++];
            for (int x = left; x <= right; x++) items[x] = temp[x];
        }
    }
    free(temp);

    printf("排序后: ");
    for (int i = 0; i < n; i++) printf("(%d,%c) ", items[i].key, items[i].label);
    printf("\n");
    printf("注意: (3,a)在(3,c)前, (1,b)在(1,e)前, (2,d)在(2,f)前 -> 稳定!\n");
}

void demo_merge_sort_properties(void) {
    printf("\n=== demo_merge_sort_properties ===\n");
    printf("归并排序特性:\n\n");
    printf("时间复杂度: O(n log n) - 所有情况\n");
    printf("空间复杂度: O(n) - 需要临时数组\n");
    printf("稳定性:     稳定排序\n");
    printf("适用场景:   大数据量, 链表排序, 外部排序\n\n");

    printf("归并排序优势:\n");
    printf("  1. 时间复杂度始终O(n log n), 无最坏情况\n");
    printf("  2. 稳定排序, 保留相等元素原始顺序\n");
    printf("  3. 适合链表排序(不需要随机访问)\n");
    printf("  4. 适合外部排序(数据无法全部放入内存)\n\n");

    printf("归并排序劣势:\n");
    printf("  1. 需要O(n)额外空间\n");
    printf("  2. 对小数组不如插入排序高效\n");
    printf("  3. 递归实现有栈开销\n\n");

    printf("举一反三 - 外部排序:\n");
    printf("  1. 大文件分成小块, 每块排序后写入临时文件\n");
    printf("  2. 多路归并: 同时归并多个有序文件\n");
    printf("  3. 败者树: 高效选择多路中的最小元素\n");
}

int main(void) {
    printf("归并排序: 递归实现与临时数组\n");

    demo_merge_sort_basic();
    demo_merge_sort_iterative();
    demo_merge_sort_stable();
    demo_merge_sort_properties();

    printf("\n所有演示完成!\n");
    return 0;
}
