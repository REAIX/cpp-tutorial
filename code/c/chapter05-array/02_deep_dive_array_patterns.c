/** @file 02_deep_dive_array_patterns.c
 *  @brief 深入理解数组算法模式：反转、旋转、去重、查重等
 *  @description 对应文档: 05-array | 常用数组算法、数组与指针的深层区别、实战模式
 *  编译命令: gcc -std=c17 02_deep_dive_array_patterns.c -o 02_deep_dive_array_patterns
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_arr(const int arr[], int n) {
    printf("[");
    for (int i = 0; i < n; i++) {
        printf("%d", arr[i]);
        if (i < n - 1) printf(", ");
    }
    printf("]");
}

void demo_reverse(void) {
    printf("═══════════════════════════════════════\n");
    printf("  数组反转(Reverse)\n");
    printf("═══════════════════════════════════════\n\n");

    void reverse(int arr[], int n) {
        for (int i = 0; i < n / 2; i++) {
            int tmp = arr[i];
            arr[i] = arr[n - 1 - i];
            arr[n - 1 - i] = tmp;
        }
    }

    int arr[] = {1, 2, 3, 4, 5, 6, 7};
    int n = 7;

    printf("原始: ");
    print_arr(arr, n);
    printf("\n");

    reverse(arr, n);
    printf("反转: ");
    print_arr(arr, n);
    printf("\n");

    printf("\n举一反三:\n");
    printf("  时间复杂度: O(n/2) = O(n)\n");
    printf("  空间复杂度: O(1) (原地操作)\n");
    printf("  变体: 反转部分数组 → reverse(arr+left, right-left+1)\n");
}

void demo_rotate(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  数组旋转(Rotate)\n");
    printf("═══════════════════════════════════════\n\n");

    void reverse_range(int arr[], int left, int right) {
        while (left < right) {
            int tmp = arr[left];
            arr[left] = arr[right];
            arr[right] = tmp;
            left++;
            right--;
        }
    }

    void rotate_right(int arr[], int n, int k) {
        k = k % n;
        if (k == 0) return;
        reverse_range(arr, 0, n - 1);
        reverse_range(arr, 0, k - 1);
        reverse_range(arr, k, n - 1);
    }

    int arr[] = {1, 2, 3, 4, 5, 6, 7};
    int n = 7;

    printf("原始: ");
    print_arr(arr, n);
    printf("\n");

    rotate_right(arr, n, 3);
    printf("右旋3位: ");
    print_arr(arr, n);
    printf("\n");

    printf("\n三次反转法的原理:\n");
    printf("  原始:   [1 2 3 4 5 6 7]\n");
    printf("  全反转: [7 6 5 4 3 2 1]\n");
    printf("  前k反:  [5 6 7 4 3 2 1]\n");
    printf("  后n-k反:[5 6 7 1 2 3 4]\n");
    printf("  时间: O(n), 空间: O(1)\n");

    printf("\n其他旋转方法:\n");
    printf("  1. 逐次右移: O(n×k), O(1) — 简单但慢\n");
    printf("  2. 辅助数组: O(n), O(k) — 需要额外空间\n");
    printf("  3. 三次反转: O(n), O(1) — 最优\n");
    printf("  4. 循环替换: O(n), O(1) — 数学方法\n");
}

void demo_deduplicate(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  数组去重(Deduplicate)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("方法1: 有序数组去重(双指针法):\n");
    int sorted[] = {1, 1, 2, 2, 2, 3, 4, 4, 5, 5, 5};
    int n = 11;

    printf("  原始: ");
    print_arr(sorted, n);
    printf("\n");

    int new_len = 1;
    for (int i = 1; i < n; i++) {
        if (sorted[i] != sorted[new_len - 1]) {
            sorted[new_len] = sorted[i];
            new_len++;
        }
    }

    printf("  去重: ");
    print_arr(sorted, new_len);
    printf(" (新长度=%d)\n", new_len);

    printf("\n方法2: 无序数组去重(辅助标记):\n");
    int unsorted[] = {3, 1, 2, 3, 2, 4, 1, 5};
    int m = 8;

    printf("  原始: ");
    print_arr(unsorted, m);
    printf("\n");

    int result[8];
    int result_len = 0;
    for (int i = 0; i < m; i++) {
        int is_dup = 0;
        for (int j = 0; j < result_len; j++) {
            if (unsorted[i] == result[j]) {
                is_dup = 1;
                break;
            }
        }
        if (!is_dup) {
            result[result_len++] = unsorted[i];
        }
    }

    printf("  去重: ");
    print_arr(result, result_len);
    printf("\n");
    printf("  时间: O(n²), 空间: O(n) — 可用哈希表优化到O(n)\n");
}

void demo_find_duplicates(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  查找重复元素\n");
    printf("═══════════════════════════════════════\n\n");

    printf("方法1: 排序后相邻比较:\n");
    int arr[] = {4, 2, 5, 2, 3, 5, 1};
    int n = 7;

    int sorted_copy[7];
    memcpy(sorted_copy, arr, sizeof(arr));

    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            if (sorted_copy[j] > sorted_copy[j + 1]) {
                int tmp = sorted_copy[j];
                sorted_copy[j] = sorted_copy[j + 1];
                sorted_copy[j + 1] = tmp;
            }
        }
    }

    printf("  排序后: ");
    print_arr(sorted_copy, n);
    printf("\n  重复元素: ");
    int prev_dup = -1;
    for (int i = 1; i < n; i++) {
        if (sorted_copy[i] == sorted_copy[i - 1] && sorted_copy[i] != prev_dup) {
            printf("%d ", sorted_copy[i]);
            prev_dup = sorted_copy[i];
        }
    }
    printf("\n");

    printf("\n方法2: 利用索引标记(1~n范围内的数):\n");
    int arr2[] = {3, 1, 3, 4, 2, 2};
    int n2 = 6;

    printf("  原始: ");
    print_arr(arr2, n2);
    printf("\n  重复元素: ");

    for (int i = 0; i < n2; i++) {
        int idx = abs(arr2[i]) - 1;
        if (arr2[idx] < 0) {
            printf("%d ", abs(arr2[i]));
        } else {
            arr2[idx] = -arr2[idx];
        }
    }
    printf("\n  (通过取反标记已访问的索引，空间O(1))\n");
}

void demo_two_pointer_patterns(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  双指针模式(Two Pointer)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("1. 对撞指针 —— 两数之和(有序数组):\n");
    int sorted[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    int n = 9;
    int target = 11;

    int left = 0, right = n - 1;
    int found = 0;
    while (left < right) {
        int sum = sorted[left] + sorted[right];
        if (sum == target) {
            printf("  找到: arr[%d]+arr[%d] = %d+%d = %d\n",
                   left, right, sorted[left], sorted[right], target);
            found = 1;
            break;
        } else if (sum < target) {
            left++;
        } else {
            right--;
        }
    }
    if (!found) printf("  未找到和为%d的两数\n", target);

    printf("\n2. 快慢指针 —— 移除指定值:\n");
    int arr[] = {3, 2, 2, 3, 4, 2, 5};
    int m = 7;
    int val_to_remove = 2;

    printf("  原始: ");
    print_arr(arr, m);
    printf(", 移除%d\n", val_to_remove);

    int slow = 0;
    for (int fast = 0; fast < m; fast++) {
        if (arr[fast] != val_to_remove) {
            arr[slow] = arr[fast];
            slow++;
        }
    }

    printf("  结果: ");
    print_arr(arr, slow);
    printf(" (新长度=%d)\n", slow);

    printf("\n3. 滑动窗口 —— 最大子数组和(固定窗口):\n");
    int data[] = {2, 1, 5, 1, 3, 2};
    int len = 6;
    int window_size = 3;

    int window_sum = 0;
    for (int i = 0; i < window_size; i++) window_sum += data[i];
    int max_sum = window_sum;

    for (int i = window_size; i < len; i++) {
        window_sum += data[i] - data[i - window_size];
        if (window_sum > max_sum) max_sum = window_sum;
    }
    printf("  窗口大小%d的最大子数组和: %d\n", window_size, max_sum);
}

void demo_prefix_sum(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  前缀和(Prefix Sum)\n");
    printf("═══════════════════════════════════════\n\n");

    int arr[] = {2, 8, 3, 9, 6, 5, 4};
    int n = 7;

    int prefix[8];
    prefix[0] = 0;
    for (int i = 0; i < n; i++) {
        prefix[i + 1] = prefix[i] + arr[i];
    }

    printf("原始数组:   ");
    print_arr(arr, n);
    printf("\n前缀和数组: ");
    print_arr(prefix, n + 1);
    printf("\n\n");

    printf("区间查询 [left, right] 的和:\n");
    printf("  sum = prefix[right+1] - prefix[left]\n\n");

    int queries[][2] = {{0, 3}, {2, 5}, {1, 6}};
    for (int q = 0; q < 3; q++) {
        int l = queries[q][0], r = queries[q][1];
        int range_sum = prefix[r + 1] - prefix[l];
        printf("  sum[%d..%d] = %d\n", l, r, range_sum);
    }

    printf("\n举一反三 —— 前缀和的应用:\n");
    printf("  1. 区间求和: O(1)查询\n");
    printf("  2. 差分数组: 区间修改O(1)\n");
    printf("  3. 二维前缀和: 矩阵区域求和\n");
    printf("  4. 前缀异或: 区间异或查询\n");
}

int main(void) {
    demo_reverse();
    demo_rotate();
    demo_deduplicate();
    demo_find_duplicates();
    demo_two_pointer_patterns();
    demo_prefix_sum();

    return 0;
}
