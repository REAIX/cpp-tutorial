/**
 * @file 04_example_binary_search.c
 * @brief 二分查找: 迭代/递归, lower_bound, upper_bound
 * @description 对应文档: 27-排序与查找算法
 */

#include <stdio.h>
#include <stdlib.h>

void print_array(const int *arr, int n) {
    printf("[");
    for (int i = 0; i < n; i++) {
        printf("%d", arr[i]);
        if (i < n - 1) printf(", ");
    }
    printf("]\n");
}

int binary_search_iterative(const int *arr, int n, int target) {
    int left = 0, right = n - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (arr[mid] == target) return mid;
        if (arr[mid] < target) left = mid + 1;
        else right = mid - 1;
    }
    return -1;
}

int binary_search_recursive(const int *arr, int left, int right, int target) {
    if (left > right) return -1;
    int mid = left + (right - left) / 2;
    if (arr[mid] == target) return mid;
    if (arr[mid] < target) return binary_search_recursive(arr, mid + 1, right, target);
    return binary_search_recursive(arr, left, mid - 1, target);
}

int lower_bound(const int *arr, int n, int target) {
    int left = 0, right = n;
    while (left < right) {
        int mid = left + (right - left) / 2;
        if (arr[mid] < target) left = mid + 1;
        else right = mid;
    }
    return left;
}

int upper_bound(const int *arr, int n, int target) {
    int left = 0, right = n;
    while (left < right) {
        int mid = left + (right - left) / 2;
        if (arr[mid] <= target) left = mid + 1;
        else right = mid;
    }
    return left;
}

void demo_binary_search_basic(void) {
    printf("\n=== demo_binary_search_basic ===\n");

    int arr[] = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19};
    int n = sizeof(arr) / sizeof(arr[0]);

    printf("有序数组: ");
    print_array(arr, n);

    int targets[] = {7, 4, 1, 19, 20};
    for (int i = 0; i < 5; i++) {
        int idx = binary_search_iterative(arr, n, targets[i]);
        if (idx >= 0)
            printf("查找 %d: 找到, 索引=%d\n", targets[i], idx);
        else
            printf("查找 %d: 未找到\n", targets[i]);
    }
}

void demo_binary_search_recursive(void) {
    printf("\n=== demo_binary_search_recursive ===\n");

    int arr[] = {2, 4, 6, 8, 10, 12, 14, 16};
    int n = sizeof(arr) / sizeof(arr[0]);

    printf("有序数组: ");
    print_array(arr, n);

    int targets[] = {8, 5, 16, 1};
    for (int i = 0; i < 4; i++) {
        int idx = binary_search_recursive(arr, 0, n - 1, targets[i]);
        printf("递归查找 %d: %s\n", targets[i],
               idx >= 0 ? "找到" : "未找到");
    }

    printf("\n迭代 vs 递归:\n");
    printf("  迭代: 无栈开销, 空间O(1), 推荐\n");
    printf("  递归: 代码简洁, 空间O(log n)\n");
}

void demo_lower_upper_bound(void) {
    printf("\n=== demo_lower_upper_bound ===\n");

    int arr[] = {1, 2, 2, 2, 3, 4, 4, 5, 6};
    int n = sizeof(arr) / sizeof(arr[0]);

    printf("数组(有重复): ");
    print_array(arr, n);

    printf("\n查找值2:\n");
    int lb = lower_bound(arr, n, 2);
    int ub = upper_bound(arr, n, 2);
    printf("  lower_bound(2) = 索引%d (第一个>=2的位置)\n", lb);
    printf("  upper_bound(2) = 索引%d (第一个>2的位置)\n", ub);
    printf("  值2的范围: [%d, %d), 共%d个\n", lb, ub, ub - lb);

    printf("\n查找值4:\n");
    lb = lower_bound(arr, n, 4);
    ub = upper_bound(arr, n, 4);
    printf("  lower_bound(4) = 索引%d\n", lb);
    printf("  upper_bound(4) = 索引%d\n", ub);
    printf("  值4的范围: [%d, %d), 共%d个\n", lb, ub, ub - lb);

    printf("\n查找不存在的值7:\n");
    lb = lower_bound(arr, n, 7);
    ub = upper_bound(arr, n, 7);
    printf("  lower_bound(7) = 索引%d (=n, 不存在)\n", lb);
    printf("  upper_bound(7) = 索引%d\n", ub);

    printf("\n举一反三:\n");
    printf("  lower_bound: 第一个>=target的位置\n");
    printf("  upper_bound: 第一个>target的位置\n");
    printf("  等值范围: [lower_bound, upper_bound)\n");
    printf("  插入位置: lower_bound给出有序插入点\n");
}

void demo_binary_search_pitfalls(void) {
    printf("\n=== demo_binary_search_pitfalls ===\n");
    printf("二分查找常见陷阱:\n\n");

    printf("1. 中点计算溢出:\n");
    printf("   错误: mid = (left + right) / 2  (left+right可能溢出)\n");
    printf("   正确: mid = left + (right - left) / 2\n\n");

    printf("2. 循环条件:\n");
    printf("   标准二分: left <= right (搜索精确值)\n");
    printf("   lower/upper_bound: left < right (搜索边界)\n\n");

    printf("3. 边界更新:\n");
    printf("   标准二分: left = mid + 1, right = mid - 1\n");
    printf("   lower_bound: left = mid + 1, right = mid\n");
    printf("   upper_bound: left = mid + 1, right = mid\n\n");

    printf("4. 前提条件:\n");
    printf("   数组必须有序! 无序数组二分查找结果无意义\n");
    printf("   适合静态/很少修改的数据, 频繁插入用BST\n");
}

int main(void) {
    printf("二分查找: 迭代/递归, lower_bound, upper_bound\n");

    demo_binary_search_basic();
    demo_binary_search_recursive();
    demo_lower_upper_bound();
    demo_binary_search_pitfalls();

    printf("\n所有演示完成!\n");
    return 0;
}
