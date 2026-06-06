/** @file 01_example_one_dimensional.c
 *  @brief 一维数组：声明、初始化、遍历、边界
 *  @description 对应文档: 05-array | 演示一维数组的声明、初始化方式、遍历方法和越界问题
 *  编译命令: gcc -std=c17 01_example_one_dimensional.c -o 01_example_one_dimensional
 */

#include <stdio.h>
#include <stdlib.h>

void demo_array_declaration(void) {
    printf("═══════════════════════════════════════\n");
    printf("  数组声明与初始化\n");
    printf("═══════════════════════════════════════\n\n");

    int arr1[5];
    (void)arr1;
    int arr2[5] = {1, 2, 3, 4, 5};
    int arr3[5] = {1, 2};
    int arr4[] = {10, 20, 30, 40};
    int arr5[5] = {0};
    int arr6[10] = {[2] = 30, [5] = 60, [8] = 90};

    printf("1. 未初始化(局部数组值不确定):\n");
    printf("   int arr1[5]; → 值不确定(垃圾值)\n\n");

    printf("2. 完全初始化:\n   arr2 = ");
    for (int i = 0; i < 5; i++) printf("%d ", arr2[i]);
    printf("\n\n");

    printf("3. 部分初始化(其余自动为0):\n   arr3 = ");
    for (int i = 0; i < 5; i++) printf("%d ", arr3[i]);
    printf("\n\n");

    printf("4. 省略大小(由初始化列表决定):\n   arr4 = ");
    for (int i = 0; i < 4; i++) printf("%d ", arr4[i]);
    printf(" (大小=%d)\n\n", 4);

    printf("5. 全零初始化:\n   arr5 = ");
    for (int i = 0; i < 5; i++) printf("%d ", arr5[i]);
    printf("\n\n");

    printf("6. C99指定初始化器:\n   arr6 = ");
    for (int i = 0; i < 10; i++) printf("%d ", arr6[i]);
    printf("\n\n");

    printf("⚠️ 数组大小必须是编译期常量(C99之前):\n");
    printf("  int n = 10;\n");
    printf("  int arr[n]; // C99 VLA(变长数组)，C11可选特性\n");
    printf("  #define SIZE 10\n");
    printf("  int arr[SIZE]; // 宏定义常量，始终有效\n");
}

void demo_array_traversal(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  数组遍历\n");
    printf("═══════════════════════════════════════\n\n");

    int arr[] = {5, 12, 8, 23, 1, 17, 9, 14, 6, 20};
    int n = sizeof(arr) / sizeof(arr[0]);

    printf("正序遍历:\n  ");
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    printf("\n逆序遍历:\n  ");
    for (int i = n - 1; i >= 0; i--) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    printf("\n计算数组元素个数:\n");
    printf("  sizeof(arr) = %zu (整个数组字节数)\n", sizeof(arr));
    printf("  sizeof(arr[0]) = %zu (单个元素字节数)\n", sizeof(arr[0]));
    printf("  元素个数 = sizeof(arr)/sizeof(arr[0]) = %zu\n", sizeof(arr) / sizeof(arr[0]));

    printf("\n⚠️ 这个方法只在数组定义的作用域内有效!\n");
    printf("  传给函数后，数组退化为指针，sizeof得到指针大小\n");
}

void demo_array_operations(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  常用数组操作\n");
    printf("═══════════════════════════════════════\n\n");

    int arr[] = {34, 12, 56, 78, 23, 89, 45, 67, 90, 11};
    int n = 10;

    printf("1. 求和与平均值:\n");
    int sum = 0;
    for (int i = 0; i < n; i++) sum += arr[i];
    printf("   总和 = %d, 平均值 = %.1f\n", sum, (double)sum / n);

    printf("\n2. 求最大值和最小值:\n");
    int max = arr[0], min = arr[0];
    int max_idx = 0, min_idx = 0;
    for (int i = 1; i < n; i++) {
        if (arr[i] > max) { max = arr[i]; max_idx = i; }
        if (arr[i] < min) { min = arr[i]; min_idx = i; }
    }
    printf("   最大值: arr[%d] = %d\n", max_idx, max);
    printf("   最小值: arr[%d] = %d\n", min_idx, min);

    printf("\n3. 查找元素:\n");
    int target = 56;
    int found = -1;
    for (int i = 0; i < n; i++) {
        if (arr[i] == target) { found = i; break; }
    }
    if (found >= 0) {
        printf("   找到 %d 在位置 %d\n", target, found);
    } else {
        printf("   未找到 %d\n", target);
    }

    printf("\n4. 统计满足条件的元素:\n");
    int even_count = 0, positive_count = 0;
    for (int i = 0; i < n; i++) {
        if (arr[i] % 2 == 0) even_count++;
        if (arr[i] > 0) positive_count++;
    }
    printf("   偶数个数: %d, 正数个数: %d\n", even_count, positive_count);
}

void demo_array_bounds(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  数组越界(Array Out of Bounds)\n");
    printf("═══════════════════════════════════════\n\n");

    int arr[5] = {10, 20, 30, 40, 50};
    (void)arr;

    printf("数组 arr[5] 的合法索引: 0 ~ 4\n\n");

    printf("⚠️ C语言不做数组边界检查!\n");
    printf("  arr[5]  → 越界! 但编译器不会报错\n");
    printf("  arr[-1] → 越界! 同样不会报错\n");
    printf("  越界访问是未定义行为，可能:\n");
    printf("    - 读到垃圾值\n");
    printf("    - 修改其他变量的值\n");
    printf("    - 程序崩溃(段错误)\n");
    printf("    - 看似正常(最危险的情况!)\n\n");

    printf("常见越界场景:\n");
    printf("  1. 循环条件写错: for(i=0; i<=5; i++) arr[i]...\n");
    printf("     应该: for(i=0; i<5; i++)\n\n");
    printf("  2. 忘记数组从0开始: arr[n-1]是最后一个元素\n\n");
    printf("  3. sizeof误用: 传给函数后不能用sizeof计算大小\n\n");

    printf("防御措施:\n");
    printf("  ✓ 始终检查索引范围\n");
    printf("  ✓ 使用 #define 定义数组大小\n");
    printf("  ✓ 编译器选项: -fsanitize=address (检测越界)\n");
    printf("  ✓ 使用 size_t 类型作为索引\n");
}

int main(void) {
    demo_array_declaration();
    demo_array_traversal();
    demo_array_operations();
    demo_array_bounds();

    return 0;
}
