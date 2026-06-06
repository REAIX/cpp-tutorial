/** @file 01_deep_dive_array_memory.c
 *  @brief 深入理解数组内存布局：指针算术、VLA变长数组
 *  @description 对应文档: 05-array | 数组内存模型、指针与数组的关系、变长数组(VLA)
 *  编译命令: gcc -std=c17 01_deep_dive_array_memory.c -o 01_deep_dive_array_memory
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void demo_array_memory_model(void) {
    printf("═══════════════════════════════════════\n");
    printf("  数组的内存模型\n");
    printf("═══════════════════════════════════════\n\n");

    int arr[5] = {10, 20, 30, 40, 50};

    printf("int arr[5] = {10, 20, 30, 40, 50};\n\n");

    printf("数组在内存中是连续存储的:\n");
    printf("  索引    地址              值    偏移\n");
    for (int i = 0; i < 5; i++) {
        printf("  arr[%d]  %p  %3d   +%d字节\n",
               i, (void *)&arr[i], arr[i],
               (int)((char *)&arr[i] - (char *)&arr[0]));
    }

    printf("\n关键特性:\n");
    printf("  1. 元素在内存中连续排列\n");
    printf("  2. arr[i] 的地址 = 基地址 + i × sizeof(int)\n");
    printf("  3. 没有越界检查，访问arr[5]不会报错(但未定义行为)\n");
    printf("  4. 数组名是首元素地址(大多数情况下)\n");

    printf("\n数组名的特殊含义:\n");
    printf("  arr 等价于 &arr[0] (首元素地址)\n");
    printf("  &arr 是整个数组的地址(类型: int(*)[5])\n");
    printf("  arr 的值 = %p\n", (void *)arr);
    printf("  &arr 的值 = %p (数值相同，但类型不同!)\n", (void *)&arr);
    printf("  arr + 1 = %p (前进1个int = %zu字节)\n",
           (void *)(arr + 1), sizeof(int));
    printf("  &arr + 1 = %p (前进整个数组 = %zu字节)\n",
           (void *)(&arr + 1), sizeof(arr));
}

void demo_pointer_arithmetic(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  指针算术与数组\n");
    printf("═══════════════════════════════════════\n\n");

    int arr[] = {100, 200, 300, 400, 500};
    int *p = arr;

    printf("指针算术的基本规则:\n");
    printf("  p + i → 地址增加 i × sizeof(*p) 字节\n\n");

    printf("int *p = arr; (p指向arr[0])\n\n");

    printf("1. 指针加减:\n");
    printf("  p     → arr[0] = %d (地址: %p)\n", *p, (void *)p);
    printf("  p+1   → arr[1] = %d (地址: %p)\n", *(p + 1), (void *)(p + 1));
    printf("  p+2   → arr[2] = %d (地址: %p)\n", *(p + 2), (void *)(p + 2));
    printf("  p+4   → arr[4] = %d\n", *(p + 4));

    printf("\n2. 下标运算符等价:\n");
    printf("  arr[i] 完全等价于 *(arr + i)\n");
    printf("  &arr[i] 完全等价于 arr + i\n");
    printf("  甚至: i[arr] 等价于 arr[i] (虽然不要这么写!)\n");
    printf("  2[arr] = %d (等价于 arr[2] = %d)\n", 2[arr], arr[2]);

    printf("\n3. 指针差值:\n");
    int *p1 = &arr[1];
    int *p2 = &arr[4];
    printf("  p2 - p1 = %td (元素个数，不是字节数)\n", p2 - p1);

    printf("\n4. 指针遍历数组:\n");
    printf("  ");
    for (int *q = arr; q < arr + 5; q++) {
        printf("%d ", *q);
    }
    printf("\n");

    printf("\n5. 指针比较:\n");
    printf("  同一数组内的指针可以用 < > <= >= 比较\n");
    printf("  不同数组的指针比较是未定义行为\n");
}

void demo_array_vs_pointer(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  数组与指针的区别\n");
    printf("═══════════════════════════════════════\n\n");

    int arr[5] = {1, 2, 3, 4, 5};
    int *p = arr;

    printf("int arr[5] = {1,2,3,4,5};\n");
    printf("int *p = arr;\n\n");

    printf("相同点:\n");
    printf("  arr[i] 和 p[i] 都能访问元素\n");
    printf("  arr 和 p 都指向首元素\n\n");

    printf("不同点:\n");
    printf("  特性              数组 arr           指针 p\n");
    printf("  ────────────────────────────────────────────\n");
    printf("  sizeof            %zu (整个数组)     %zu (指针本身)\n",
           sizeof(arr), sizeof(p));
    printf("  可赋值            ✗ arr = p2;        ✓ p = arr2;\n");
    printf("  可自增            ✗ arr++;           ✓ p++;\n");
    printf("  取地址            &arr是数组指针      &p是指针的指针\n");
    printf("  存储位置          栈/数据段           指针变量本身在栈上\n");
    printf("  本质              分配了内存块        存储地址的变量\n\n");

    printf("数组名不可修改的根本原因:\n");
    printf("  arr 是一个地址常量，不是指针变量\n");
    printf("  arr 没有自己的存储空间来存放地址\n");
    printf("  arr 在编译时就被替换为地址值\n\n");

    printf("例外: sizeof 和 & 操作符下，数组名不退化为指针:\n");
    printf("  sizeof(arr) → 整个数组大小 (%zu)\n", sizeof(arr));
    printf("  &arr → 指向整个数组的指针 (int(*)[5])\n");
}

void demo_vla(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  变长数组 VLA (Variable Length Array)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("C99引入，C11变为可选特性\n");
    printf("数组大小可以在运行时确定:\n\n");

    int n = 5;
    int vla[n];

    for (int i = 0; i < n; i++) {
        vla[i] = i * 10;
    }

    printf("int n = 5; int vla[n];\n");
    printf("  vla = ");
    for (int i = 0; i < n; i++) {
        printf("%d ", vla[i]);
    }
    printf("\n\n");

    printf("VLA二维数组:\n");
    int rows = 3, cols = 4;
    int matrix[rows][cols];
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = i * cols + j + 1;
        }
    }
    for (int i = 0; i < rows; i++) {
        printf("  ");
        for (int j = 0; j < cols; j++) {
            printf("%3d ", matrix[i][j]);
        }
        printf("\n");
    }

    printf("\nVLA的特点:\n");
    printf("  ✓ 大小在运行时确定\n");
    printf("  ✓ 分配在栈上(速度快，但空间有限)\n");
    printf("  ✓ 作用域结束时自动释放\n");
    printf("  ✗ 不能在定义时初始化\n");
    printf("  ✗ 不能是静态存储期或文件作用域\n");
    printf("  ✗ C11中为可选特性，不是所有编译器支持\n");
    printf("  ✗ 大尺寸VLA可能导致栈溢出\n\n");

    printf("VLA vs malloc:\n");
    printf("  特性      VLA              malloc\n");
    printf("  ──────────────────────────────────────\n");
    printf("  分配位置   栈               堆\n");
    printf("  释放       自动(作用域结束)  手动free\n");
    printf("  大小限制   栈大小(通常1-8MB) 堆大小(通常更大)\n");
    printf("  初始化     不能             可以用calloc\n");
    printf("  可移植性   C11可选          完全标准\n\n");

    printf("建议: 生产代码中优先使用malloc，VLA只用于小数组\n");
}

void demo_array_address_calculation(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  数组地址计算公式\n");
    printf("═══════════════════════════════════════\n\n");

    printf("一维数组: int arr[n]\n");
    printf("  &arr[i] = base_addr + i × sizeof(int)\n\n");

    printf("二维数组: int arr[m][n]\n");
    printf("  &arr[i][j] = base_addr + (i × n + j) × sizeof(int)\n\n");

    printf("三维数组: int arr[l][m][n]\n");
    printf("  &arr[k][i][j] = base_addr + (k×m×n + i×n + j) × sizeof(int)\n\n");

    printf("验证二维数组地址计算:\n");
    int mat[3][4] = {{1,2,3,4},{5,6,7,8},{9,10,11,12}};
    int row = 1, col = 2;
    int *calc_addr = (int *)mat + (row * 4 + col);
    printf("  mat[%d][%d] = %d\n", row, col, mat[row][col]);
    printf("  通过公式计算: *calc_addr = %d\n", *calc_addr);
    printf("  地址一致: %s\n",
           calc_addr == &mat[row][col] ? "是" : "否");

    printf("\n举一反三 —— 理解地址计算的意义:\n");
    printf("  1. 知道基地址和维度，可以任意访问元素\n");
    printf("  2. 动态分配多维数组的基础\n");
    printf("  3. 理解缓存行和空间局部性\n");
    printf("  4. 优化遍历顺序(行优先 vs 列优先)\n");
}

int main(void) {
    demo_array_memory_model();
    demo_pointer_arithmetic();
    demo_array_vs_pointer();
    demo_vla();
    demo_array_address_calculation();

    return 0;
}
