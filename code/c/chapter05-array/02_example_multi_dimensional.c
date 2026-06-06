/** @file 02_example_multi_dimensional.c
 *  @brief 多维数组：二维数组、三维数组、内存布局
 *  @description 对应文档: 05-array | 演示二维和三维数组的声明、初始化、遍历及内存布局
 *  编译命令: gcc -std=c17 02_example_multi_dimensional.c -o 02_example_multi_dimensional
 */

#include <stdio.h>
#include <stdlib.h>

void demo_2d_array_basics(void) {
    printf("═══════════════════════════════════════\n");
    printf("  二维数组基础\n");
    printf("═══════════════════════════════════════\n\n");

    int matrix[3][4] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12}
    };

    printf("声明: int matrix[3][4]  (3行4列)\n\n");

    printf("完全初始化:\n");
    for (int i = 0; i < 3; i++) {
        printf("  行%d: ", i);
        for (int j = 0; j < 4; j++) {
            printf("%3d ", matrix[i][j]);
        }
        printf("\n");
    }

    printf("\n部分初始化:\n");
    int partial[3][4] = {
        {1, 2},
        {5},
        {9, 10, 11}
    };
    for (int i = 0; i < 3; i++) {
        printf("  行%d: ", i);
        for (int j = 0; j < 4; j++) {
            printf("%3d ", partial[i][j]);
        }
        printf("\n");
    }

    printf("\nC99指定初始化器:\n");
    int designated[3][4] = {
        [0] = {1, 2, 3, 4},
        [2] = {[1] = 20, [3] = 40}
    };
    for (int i = 0; i < 3; i++) {
        printf("  行%d: ", i);
        for (int j = 0; j < 4; j++) {
            printf("%3d ", designated[i][j]);
        }
        printf("\n");
    }

    printf("\n理解: int matrix[3][4] 是「3个元素的数组，每个元素是int[4]」\n");
    printf("  matrix[i] 是第i行(一个int[4]数组)\n");
    printf("  matrix[i][j] 是第i行第j列的int值\n");
}

void demo_2d_traversal(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  二维数组遍历\n");
    printf("═══════════════════════════════════════\n\n");

    int matrix[4][4] = {
        {1,  2,  3,  4},
        {5,  6,  7,  8},
        {9,  10, 11, 12},
        {13, 14, 15, 16}
    };

    printf("1. 按行遍历(缓存友好):\n  ");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            printf("%2d ", matrix[i][j]);
        }
        printf("\n  ");
    }

    printf("\n2. 按列遍历(缓存不友好):\n  ");
    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 4; i++) {
            printf("%2d ", matrix[i][j]);
        }
        printf("\n  ");
    }

    printf("\n3. 对角线遍历:\n");
    printf("  主对角线: ");
    for (int i = 0; i < 4; i++) {
        printf("%d ", matrix[i][i]);
    }
    printf("\n  副对角线: ");
    for (int i = 0; i < 4; i++) {
        printf("%d ", matrix[i][3 - i]);
    }
    printf("\n");

    printf("\n4. 行列式计算(2×2):\n");
    int a = matrix[0][0], b = matrix[0][1];
    int c = matrix[1][0], d = matrix[1][1];
    printf("  |%d %d| = %d×%d - %d×%d = %d\n",
           a, b, a, d, b, c, a * d - b * c);
}

void demo_2d_operations(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  二维数组常用操作\n");
    printf("═══════════════════════════════════════\n\n");

    printf("1. 矩阵转置:\n");
    int orig[3][3] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
    int trans[3][3];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            trans[i][j] = orig[j][i];
        }
    }
    printf("  原始:          转置:\n");
    for (int i = 0; i < 3; i++) {
        printf("  ");
        for (int j = 0; j < 3; j++) printf("%d ", orig[i][j]);
        printf("    ");
        for (int j = 0; j < 3; j++) printf("%d ", trans[i][j]);
        printf("\n");
    }

    printf("\n2. 矩阵加法:\n");
    int A[2][2] = {{1, 2}, {3, 4}};
    int B[2][2] = {{5, 6}, {7, 8}};
    int C[2][2];
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            C[i][j] = A[i][j] + B[i][j];
        }
    }
    printf("  A+B = {");
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            printf("%d%s", C[i][j], (i==1&&j==1) ? "" : ", ");
        }
    }
    printf("}\n");

    printf("\n3. 每行求和:\n");
    int scores[3][4] = {
        {85, 90, 78, 92},
        {76, 88, 95, 80},
        {90, 85, 82, 88}
    };
    for (int i = 0; i < 3; i++) {
        int row_sum = 0;
        for (int j = 0; j < 4; j++) row_sum += scores[i][j];
        printf("  学生%d 总分: %d, 平均: %.1f\n", i + 1, row_sum, row_sum / 4.0);
    }
}

void demo_3d_array(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  三维数组\n");
    printf("═══════════════════════════════════════\n\n");

    int cube[2][3][4] = {
        {
            {1, 2, 3, 4},
            {5, 6, 7, 8},
            {9, 10, 11, 12}
        },
        {
            {13, 14, 15, 16},
            {17, 18, 19, 20},
            {21, 22, 23, 24}
        }
    };

    printf("int cube[2][3][4]: 2层×3行×4列\n\n");

    for (int k = 0; k < 2; k++) {
        printf("第%d层:\n", k);
        for (int i = 0; i < 3; i++) {
            printf("  ");
            for (int j = 0; j < 4; j++) {
                printf("%3d ", cube[k][i][j]);
            }
            printf("\n");
        }
    }

    printf("\n理解: cube[2][3][4] 是2个int[3][4]的数组\n");
    printf("  cube[k]    → 第k层(一个int[3][4])\n");
    printf("  cube[k][i] → 第k层第i行(一个int[4])\n");
    printf("  cube[k][i][j] → 第k层第i行第j列的int\n\n");

    printf("sizeof分析:\n");
    printf("  sizeof(cube)       = %zu (整个三维数组)\n", sizeof(cube));
    printf("  sizeof(cube[0])    = %zu (一层)\n", sizeof(cube[0]));
    printf("  sizeof(cube[0][0]) = %zu (一行)\n", sizeof(cube[0][0]));
    printf("  sizeof(cube[0][0][0]) = %zu (一个元素)\n", sizeof(cube[0][0][0]));
}

void demo_memory_layout(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  多维数组的内存布局\n");
    printf("═══════════════════════════════════════\n\n");

    int arr[2][3] = {{10, 20, 30}, {40, 50, 60}};

    printf("C语言多维数组按行优先(Row-major)存储:\n\n");
    printf("  逻辑视图:\n");
    printf("    arr[0][0]=10  arr[0][1]=20  arr[0][2]=30\n");
    printf("    arr[1][0]=40  arr[1][1]=50  arr[1][2]=60\n\n");

    printf("  内存布局(连续):\n");
    printf("    地址         值\n");
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            printf("    %p  arr[%d][%d]=%d\n",
                   (void *)&arr[i][j], i, j, arr[i][j]);
        }
    }

    printf("\n  用一维方式访问(利用连续性):\n  ");
    int *flat = (int *)arr;
    for (int i = 0; i < 6; i++) {
        printf("%d ", flat[i]);
    }
    printf("\n\n");

    printf("  行优先的意义:\n");
    printf("    按行遍历 → 内存顺序访问 → 缓存友好\n");
    printf("    按列遍历 → 内存跳跃访问 → 缓存不友好\n");
    printf("    对大数组性能差异可达数倍!\n");
}

int main(void) {
    demo_2d_array_basics();
    demo_2d_traversal();
    demo_2d_operations();
    demo_3d_array();
    demo_memory_layout();

    return 0;
}
