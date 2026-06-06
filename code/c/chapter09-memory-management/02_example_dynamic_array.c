/** @file 02_example_dynamic_array.c
 *  @brief 动态数组：动态数组增长、二维动态数组
 *  @description 对应文档: 09-内存管理
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int *data;
    size_t size;
    size_t capacity;
} IntVector;

IntVector *ivec_create(size_t initial_cap) {
    IntVector *v = (IntVector *)malloc(sizeof(IntVector));
    if (!v) return NULL;

    v->data = (int *)malloc(initial_cap * sizeof(int));
    if (!v->data) {
        free(v);
        return NULL;
    }

    v->size = 0;
    v->capacity = initial_cap;
    return v;
}

void ivec_destroy(IntVector *v) {
    if (v) {
        free(v->data);
        free(v);
    }
}

int ivec_push_back(IntVector *v, int value) {
    if (v->size >= v->capacity) {
        size_t new_cap = v->capacity * 2;
        int *new_data = (int *)realloc(v->data, new_cap * sizeof(int));
        if (!new_data) return 0;
        v->data = new_data;
        v->capacity = new_cap;
    }
    v->data[v->size++] = value;
    return 1;
}

int ivec_at(const IntVector *v, size_t index) {
    return v->data[index];
}

void demo_dynamic_array_growth(void) {
    printf("=== 动态数组增长 ===\n");

    IntVector *v = ivec_create(2);
    if (!v) return;

    printf("初始: size=%zu, capacity=%zu\n", v->size, v->capacity);

    for (int i = 0; i < 10; i++) {
        ivec_push_back(v, i * 10);
        printf("push %2d: size=%zu, capacity=%zu\n",
               i * 10, v->size, v->capacity);
    }

    printf("\n数组内容: ");
    for (size_t i = 0; i < v->size; i++) {
        printf("%d ", ivec_at(v, i));
    }
    printf("\n");

    ivec_destroy(v);

    printf("\n容量翻倍策略: 摊还时间复杂度 O(1)\n");
    printf("类似 C++ std::vector 的增长策略\n");

    printf("\n");
}

void demo_2d_dynamic_array(void) {
    printf("=== 二维动态数组 ===\n");

    int rows = 3, cols = 4;

    int **matrix = (int **)malloc((size_t)rows * sizeof(int *));
    if (!matrix) return;

    for (int i = 0; i < rows; i++) {
        matrix[i] = (int *)malloc((size_t)cols * sizeof(int));
        if (!matrix[i]) {
            for (int j = 0; j < i; j++) free(matrix[j]);
            free(matrix);
            return;
        }
    }

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = i * cols + j + 1;
        }
    }

    printf("二维动态数组 (%d x %d):\n", rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%3d ", matrix[i][j]);
        }
        printf("\n");
    }

    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);

    printf("\n方式1: 指针数组, 每行独立分配\n");
    printf("  优点: 行可以不等长 (锯齿数组)\n");
    printf("  缺点: 多次 malloc, 内存不连续, 缓存不友好\n");

    printf("\n");
}

void demo_2d_contiguous_array(void) {
    printf("=== 连续内存的二维数组 ===\n");

    int rows = 3, cols = 4;

    int **matrix = (int **)malloc((size_t)rows * sizeof(int *));
    if (!matrix) return;
    matrix[0] = (int *)malloc((size_t)(rows * cols) * sizeof(int));
    if (!matrix[0]) {
        free(matrix);
        return;
    }

    for (int i = 1; i < rows; i++) {
        matrix[i] = matrix[0] + i * cols;
    }

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = i * cols + j + 1;
        }
    }

    printf("连续内存二维数组 (%d x %d):\n", rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%3d ", matrix[i][j]);
        }
        printf("\n");
    }

    printf("\n验证连续性:\n");
    printf("  matrix[0][3] 的下一个 = matrix[1][0] = %d\n", matrix[1][0]);
    printf("  &matrix[0][3]+1 == &matrix[1][0]: %s\n",
           (&matrix[0][3] + 1 == &matrix[1][0]) ? "true" : "false");

    free(matrix[0]);
    free(matrix);

    printf("\n方式2: 一次 malloc 分配所有数据, 行指针指向偏移位置\n");
    printf("  优点: 内存连续, 缓存友好, 只需2次 free\n");
    printf("  缺点: 行必须等长\n");

    printf("\n");
}

void demo_1d_as_2d(void) {
    printf("=== 一维数组模拟二维 ===\n");

    int rows = 3, cols = 4;
    int *flat = (int *)malloc((size_t)(rows * cols) * sizeof(int));
    if (!flat) return;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            flat[i * cols + j] = i * cols + j + 1;
        }
    }

    printf("一维数组模拟二维 (%d x %d):\n", rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%3d ", flat[i * cols + j]);
        }
        printf("\n");
    }

    free(flat);

    printf("\n方式3: 纯一维数组, 用 index = row * cols + col 访问\n");
    printf("  优点: 最简单, 只需1次 malloc/free, 缓存最友好\n");
    printf("  缺点: 语法不如 matrix[i][j] 直观\n");

    printf("\n");
}

int main(void) {
    demo_dynamic_array_growth();
    demo_2d_dynamic_array();
    demo_2d_contiguous_array();
    demo_1d_as_2d();

    return 0;
}
