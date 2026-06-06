/** @file 03_example_array_and_function.c
 *  @brief 数组与函数：传递数组给函数、数组参数退化
 *  @description 对应文档: 05-array | 演示数组作为函数参数的传递方式、参数退化机制、常见模式
 *  编译命令: gcc -std=c17 03_example_array_and_function.c -o 03_example_array_and_function
 */

#include <stdio.h>
#include <stdlib.h>

int array_sum(const int arr[], int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

void array_print(const int arr[], int n) {
    printf("[");
    for (int i = 0; i < n; i++) {
        printf("%d", arr[i]);
        if (i < n - 1) printf(", ");
    }
    printf("]");
}

void array_double(int arr[], int n) {
    for (int i = 0; i < n; i++) {
        arr[i] *= 2;
    }
}

int array_max(const int arr[], int n) {
    int max = arr[0];
    for (int i = 1; i < n; i++) {
        if (arr[i] > max) max = arr[i];
    }
    return max;
}

void demo_passing_array(void) {
    printf("═══════════════════════════════════════\n");
    printf("  数组传递给函数\n");
    printf("═══════════════════════════════════════\n\n");

    int arr[] = {1, 2, 3, 4, 5};
    int n = sizeof(arr) / sizeof(arr[0]);

    printf("数组: ");
    array_print(arr, n);
    printf("\n\n");

    printf("1. 求和:\n");
    printf("   array_sum(arr, %d) = %d\n", n, array_sum(arr, n));

    printf("\n2. 求最大值:\n");
    printf("   array_max(arr, %d) = %d\n", n, array_max(arr, n));

    printf("\n3. 修改数组(函数内修改影响原数组):\n");
    printf("   修改前: ");
    array_print(arr, n);
    printf("\n");
    array_double(arr, n);
    printf("   修改后: ");
    array_print(arr, n);
    printf("\n");

    printf("\n关键: 数组传给函数时，传递的是首元素地址\n");
    printf("  函数内修改数组元素 → 原数组也被修改\n");
    printf("  这不是「值传递」的例外，而是传递了指针的值\n");
}

void demo_array_decay(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  数组参数退化(Array Decay)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("当数组作为函数参数时，退化为指向首元素的指针:\n\n");

    void show_sizeof(int arr_param[], int n) {
        (void)n;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsizeof-array-argument"
        printf("  函数内 sizeof(arr_param) = %zu (指针大小!)\n", sizeof(arr_param));
#pragma GCC diagnostic pop
        printf("  函数内 sizeof(arr_param[0]) = %zu\n", sizeof(arr_param[0]));
    }

    int arr[5] = {1, 2, 3, 4, 5};
    printf("调用者: sizeof(arr) = %zu (整个数组)\n", sizeof(arr));
    show_sizeof(arr, 5);

    printf("\n退化的含义:\n");
    printf("  int arr[] 参数 ← 等价于 → int *arr 参数\n");
    printf("  以下函数签名完全等价:\n");
    printf("    void func(int arr[], int n);\n");
    printf("    void func(int *arr, int n);\n");
    printf("    void func(int arr[10], int n);  // 10被忽略!\n\n");

    printf("⚠️ 退化的后果:\n");
    printf("  1. 函数内无法用sizeof获取数组大小\n");
    printf("  2. 必须额外传递数组长度参数\n");
    printf("  3. 函数内修改数组会影响原数组\n");
    printf("  4. 丢失了数组边界信息\n");
}

void demo_const_array_param(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  const数组参数\n");
    printf("═══════════════════════════════════════\n\n");

    printf("用const修饰数组参数，防止函数内修改:\n\n");

    printf("  只读:   void print(const int arr[], int n);\n");
    printf("  可修改: void sort(int arr[], int n);\n\n");

    printf("const数组参数的规则:\n");
    printf("  ✓ 非const数组可以传给const参数(安全提升)\n");
    printf("  ✗ const数组不能传给非const参数(编译错误)\n\n");

    int arr[] = {10, 20, 30};
    printf("  非const数组 → const参数: 合法\n");
    printf("  array_sum(arr, 3) = %d\n", array_sum(arr, 3));

    const int carr[] = {100, 200, 300};
    printf("  const数组 → const参数: 合法\n");
    printf("  array_sum(carr, 3) = %d\n", array_sum(carr, 3));
}

void demo_2d_array_param(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  二维数组作为函数参数\n");
    printf("═══════════════════════════════════════\n\n");

    void print_matrix(int rows, int cols, int mat[][cols]) {
        for (int i = 0; i < rows; i++) {
            printf("  ");
            for (int j = 0; j < cols; j++) {
                printf("%3d ", mat[i][j]);
            }
            printf("\n");
        }
    }

    int matrix[3][4] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12}
    };

    printf("方法1: 指定列数(C99 VLA参数)\n");
    printf("  void print_matrix(int rows, int cols, int mat[][cols]);\n");
    print_matrix(3, 4, matrix);

    printf("\n方法2: 固定列数\n");
    printf("  void func(int mat[][4], int rows);  // 列数必须指定\n\n");

    printf("方法3: 传递一维指针+手动计算偏移\n");
    printf("  void func(int *mat, int rows, int cols);\n");
    printf("  访问: mat[i*cols + j]\n\n");

    printf("方法4: 传递数组指针\n");
    printf("  void func(int (*mat)[4], int rows);\n\n");

    printf("⚠️ 二维数组参数的规则:\n");
    printf("  第一维大小可以省略\n");
    printf("  第二维及之后的大小必须指定(或用VLA)\n");
    printf("  原因: 编译器需要列数来计算元素地址\n");
}

void demo_common_patterns(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  数组函数参数的常见模式\n");
    printf("═══════════════════════════════════════\n\n");

    printf("1. 输入+长度模式(最常见):\n");
    printf("   void func(const int arr[], int n);\n\n");

    printf("2. 输入+输出模式:\n");
    printf("   void copy(const int src[], int dst[], int n);\n\n");

    void copy_array(const int src[], int dst[], int n) {
        for (int i = 0; i < n; i++) {
            dst[i] = src[i];
        }
    }

    int src[] = {1, 2, 3, 4, 5};
    int dst[5];
    copy_array(src, dst, 5);
    printf("   copy结果: ");
    array_print(dst, 5);
    printf("\n\n");

    printf("3. 返回结果通过输出参数:\n");
    void find_min_max(const int arr[], int n, int *min_val, int *max_val) {
        *min_val = *max_val = arr[0];
        for (int i = 1; i < n; i++) {
            if (arr[i] < *min_val) *min_val = arr[i];
            if (arr[i] > *max_val) *max_val = arr[i];
        }
    }

    int lo, hi;
    find_min_max(src, 5, &lo, &hi);
    printf("   min=%d, max=%d\n\n", lo, hi);

    printf("4. 结构体包装数组+长度:\n");
    printf("   struct IntArray { int *data; int size; };\n");
    printf("   void func(struct IntArray *arr);\n\n");

    printf("5. 哨兵终止(字符串风格):\n");
    printf("   int sum_sentinel(const int arr[]); // 以特定值结束\n");
}

int main(void) {
    demo_passing_array();
    demo_array_decay();
    demo_const_array_param();
    demo_2d_array_param();
    demo_common_patterns();

    return 0;
}
