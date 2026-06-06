/**
 * @file 01_deep_dive_sort_comparison.c
 * @brief 排序算法对比: 时间/空间复杂度、稳定性、适用场景
 * @description 对应文档: 27-排序与查找算法
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void print_array(const int *arr, int n) {
    printf("[");
    for (int i = 0; i < n; i++) {
        printf("%d", arr[i]);
        if (i < n - 1) printf(", ");
    }
    printf("]\n");
}

void bubble_sort(int *arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < n - 1 - i; j++) {
            if (arr[j] > arr[j + 1]) {
                int t = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = t;
                swapped = 1;
            }
        }
        if (!swapped) break;
    }
}

void selection_sort(int *arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < n; j++) {
            if (arr[j] < arr[min_idx]) min_idx = j;
        }
        if (min_idx != i) {
            int t = arr[i]; arr[i] = arr[min_idx]; arr[min_idx] = t;
        }
    }
}

void insertion_sort(int *arr, int n) {
    for (int i = 1; i < n; i++) {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

void shell_sort(int *arr, int n) {
    for (int gap = n / 2; gap > 0; gap /= 2) {
        for (int i = gap; i < n; i++) {
            int temp = arr[i];
            int j;
            for (j = i; j >= gap && arr[j - gap] > temp; j -= gap) {
                arr[j] = arr[j - gap];
            }
            arr[j] = temp;
        }
    }
}

int lomuto_partition(int *arr, int low, int high) {
    int pivot = arr[high];
    int i = low;
    for (int j = low; j < high; j++) {
        if (arr[j] <= pivot) {
            int t = arr[i]; arr[i] = arr[j]; arr[j] = t;
            i++;
        }
    }
    int t = arr[i]; arr[i] = arr[high]; arr[high] = t;
    return i;
}

void quick_sort(int *arr, int low, int high) {
    if (low < high) {
        int pi = lomuto_partition(arr, low, high);
        quick_sort(arr, low, pi - 1);
        quick_sort(arr, pi + 1, high);
    }
}

void quick_sort_wrapper(int *arr, int n) {
    quick_sort(arr, 0, n - 1);
}

void merge(int *arr, int *temp, int left, int mid, int right) {
    int i = left, j = mid + 1, k = left;
    while (i <= mid && j <= right) {
        temp[k++] = (arr[i] <= arr[j]) ? arr[i++] : arr[j++];
    }
    while (i <= mid) temp[k++] = arr[i++];
    while (j <= right) temp[k++] = arr[j++];
    for (int x = left; x <= right; x++) arr[x] = temp[x];
}

void merge_sort_impl(int *arr, int *temp, int left, int right) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    merge_sort_impl(arr, temp, left, mid);
    merge_sort_impl(arr, temp, mid + 1, right);
    merge(arr, temp, left, mid, right);
}

void merge_sort(int *arr, int n) {
    int *temp = (int *)malloc(n * sizeof(int));
    merge_sort_impl(arr, temp, 0, n - 1);
    free(temp);
}

void heapify(int *arr, int n, int i) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;
    if (left < n && arr[left] > arr[largest]) largest = left;
    if (right < n && arr[right] > arr[largest]) largest = right;
    if (largest != i) {
        int t = arr[i]; arr[i] = arr[largest]; arr[largest] = t;
        heapify(arr, n, largest);
    }
}

void heap_sort(int *arr, int n) {
    for (int i = n / 2 - 1; i >= 0; i--) heapify(arr, n, i);
    for (int i = n - 1; i > 0; i--) {
        int t = arr[0]; arr[0] = arr[i]; arr[i] = t;
        heapify(arr, i, 0);
    }
}

typedef void (*SortFunc)(int *, int);

void copy_array(const int *src, int *dst, int n) {
    memcpy(dst, src, n * sizeof(int));
}

double benchmark_sort(SortFunc func, const int *src, int n, int runs) {
    int *arr = (int *)malloc(n * sizeof(int));
    clock_t total = 0;

    for (int r = 0; r < runs; r++) {
        copy_array(src, arr, n);
        clock_t start = clock();
        func(arr, n);
        total += clock() - start;
    }

    free(arr);
    return (double)total / CLOCKS_PER_SEC / runs * 1000.0;
}

void demo_sort_comparison_table(void) {
    printf("\n=== demo_sort_comparison_table ===\n");
    printf("排序算法对比表:\n\n");
    printf("算法         最好       平均       最坏       空间      稳定\n");
    printf("冒泡排序     O(n)       O(n^2)     O(n^2)     O(1)      是\n");
    printf("选择排序     O(n^2)     O(n^2)     O(n^2)     O(1)      否\n");
    printf("插入排序     O(n)       O(n^2)     O(n^2)     O(1)      是\n");
    printf("希尔排序     O(nlogn)   O(n^1.3)   O(n^2)     O(1)      否\n");
    printf("快速排序     O(nlogn)   O(nlogn)   O(n^2)     O(logn)   否\n");
    printf("归并排序     O(nlogn)   O(nlogn)   O(nlogn)   O(n)      是\n");
    printf("堆排序       O(nlogn)   O(nlogn)   O(nlogn)   O(1)      否\n\n");
}

void demo_sort_benchmark(void) {
    printf("\n=== demo_sort_benchmark ===\n");
    printf("性能基准测试(随机数据, 平均耗时ms):\n\n");

    int sizes[] = {1000, 5000, 10000};
    int num_sizes = 3;

    for (int s = 0; s < num_sizes; s++) {
        int n = sizes[s];
        int *src = (int *)malloc(n * sizeof(int));
        srand(42);
        for (int i = 0; i < n; i++) src[i] = rand();

        printf("n=%d:\n", n);
        printf("  冒泡排序: %.3f ms\n", benchmark_sort(bubble_sort, src, n, 3));
        printf("  选择排序: %.3f ms\n", benchmark_sort(selection_sort, src, n, 3));
        printf("  插入排序: %.3f ms\n", benchmark_sort(insertion_sort, src, n, 3));
        printf("  希尔排序: %.3f ms\n", benchmark_sort(shell_sort, src, n, 3));
        printf("  快速排序: %.3f ms\n", benchmark_sort(quick_sort_wrapper, src, n, 3));
        printf("  归并排序: %.3f ms\n", benchmark_sort(merge_sort, src, n, 3));
        printf("  堆排序:   %.3f ms\n", benchmark_sort(heap_sort, src, n, 3));
        printf("\n");

        free(src);
    }
}

void demo_sort_selection_guide(void) {
    printf("\n=== demo_sort_selection_guide ===\n");
    printf("何时选择哪种排序?\n\n");

    printf("1. 小数据量(n < 50):\n");
    printf("   -> 插入排序: 简单, 常数因子小, 对近乎有序数据O(n)\n\n");

    printf("2. 通用排序:\n");
    printf("   -> 快速排序: 平均最快, 缓存友好, 标准库通常用它\n\n");

    printf("3. 需要稳定排序:\n");
    printf("   -> 归并排序: 稳定的O(nlogn), 但需要额外空间\n\n");

    printf("4. 内存受限:\n");
    printf("   -> 堆排序: O(1)额外空间, O(nlogn)时间\n\n");

    printf("5. 近乎有序数据:\n");
    printf("   -> 插入排序: 几乎O(n), 最适合\n\n");

    printf("6. 大量重复元素:\n");
    printf("   -> 三路快排: 将数组分为<, =, >三部分\n\n");

    printf("7. 外部排序(数据无法全部放入内存):\n");
    printf("   -> 归并排序: 天然支持分块排序+多路归并\n\n");

    printf("8. 实际标准库实现:\n");
    printf("   -> C qsort(): 通常introsort(快排+堆排序+插入排序)\n");
    printf("   -> C++ std::sort(): introsort\n");
    printf("   -> Java: 基本类型用快排, 对象用归并(稳定)\n");
    printf("   -> Python: Timsort(归并+插入, 稳定)\n");
}

int main(void) {
    printf("排序算法对比: 时间/空间复杂度、稳定性、适用场景\n");

    demo_sort_comparison_table();
    demo_sort_benchmark();
    demo_sort_selection_guide();

    printf("\n所有演示完成!\n");
    return 0;
}
