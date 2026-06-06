/**
 * @file 03_example_strategy.c
 * @brief 策略模式: 函数指针实现算法族
 * @description 对应文档: 30-C语言设计模式实现
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int (*SortStrategy)(const void *, const void *);

void int_sort(int *arr, int n, SortStrategy strategy) {
    qsort(arr, n, sizeof(int), strategy);
}

int sort_ascending(const void *a, const void *b) {
    int ia = *(const int *)a, ib = *(const int *)b;
    return (ia > ib) - (ia < ib);
}

int sort_descending(const void *a, const void *b) {
    int ia = *(const int *)a, ib = *(const int *)b;
    return (ib > ia) - (ib < ia);
}

int sort_by_abs(const void *a, const void *b) {
    long long va = llabs((long long)(*(const int *)a));
    long long vb = llabs((long long)(*(const int *)b));
    return (va > vb) - (va < vb);
}

void print_ints(const int *arr, int n) {
    printf("[");
    for (int i = 0; i < n; i++) {
        printf("%d", arr[i]);
        if (i < n - 1) printf(", ");
    }
    printf("]\n");
}

void demo_sort_strategy(void) {
    printf("\n=== demo_sort_strategy ===\n");
    printf("策略模式: 将算法封装为可互换的策略\n\n");

    int arr[] = {-3, 5, -1, 8, 2, -7, 4};
    int n = sizeof(arr) / sizeof(arr[0]);

    printf("原始数组: ");
    print_ints(arr, n);

    int arr1[7], arr2[7], arr3[7];
    memcpy(arr1, arr, sizeof(arr));
    memcpy(arr2, arr, sizeof(arr));
    memcpy(arr3, arr, sizeof(arr));

    int_sort(arr1, n, sort_ascending);
    printf("升序策略: ");
    print_ints(arr1, n);

    int_sort(arr2, n, sort_descending);
    printf("降序策略: ");
    print_ints(arr2, n);

    int_sort(arr3, n, sort_by_abs);
    printf("绝对值策略: ");
    print_ints(arr3, n);
}

typedef double (*DiscountStrategy)(double price);

double no_discount(double price) {
    return price;
}

double percent10_discount(double price) {
    return price * 0.9;
}

double percent20_discount(double price) {
    return price * 0.8;
}

double black_friday_discount(double price) {
    if (price > 500) return price * 0.5;
    return price * 0.7;
}

typedef struct {
    char name[64];
    double price;
    DiscountStrategy discount;
} Product;

double product_final_price(const Product *p) {
    return p->discount ? p->discount(p->price) : p->price;
}

void product_print(const Product *p) {
    printf("  %s: 原价%.2f -> 折后%.2f\n",
           p->name, p->price, product_final_price(p));
}

void demo_discount_strategy(void) {
    printf("\n=== demo_discount_strategy ===\n");
    printf("折扣策略: 不同商品使用不同折扣算法\n\n");

    Product products[] = {
        {"笔记本电脑", 5999.0, black_friday_discount},
        {"键盘", 299.0, percent10_discount},
        {"鼠标", 99.0, no_discount},
        {"显示器", 1999.0, percent20_discount}
    };

    for (int i = 0; i < 4; i++) {
        product_print(&products[i]);
    }

    printf("\n策略模式优势:\n");
    printf("  1. 算法可独立变化, 不影响使用方\n");
    printf("  2. 运行时切换策略\n");
    printf("  3. 新增策略不需要修改已有代码\n");
}

typedef struct {
    double (*compress)(const char *input, char *output, int out_size);
    const char *(*decompress)(const char *input, char *output, int out_size);
    const char *name;
} CompressionStrategy;

static double rle_compress(const char *input, char *output, int out_size) {
    int len = (int)strlen(input);
    int oi = 0;
    for (int i = 0; i < len && oi < out_size - 2; ) {
        char ch = input[i];
        int count = 1;
        while (i + count < len && input[i + count] == ch && count < 9) count++;
        oi += snprintf(output + oi, out_size - oi, "%d%c", count, ch);
        i += count;
    }
    return (double)oi / len;
}

static const char *rle_decompress(const char *input, char *output, int out_size) {
    int oi = 0;
    for (int i = 0; input[i] && oi < out_size - 1; ) {
        int count = input[i] - '0';
        char ch = input[i + 1];
        for (int j = 0; j < count && oi < out_size - 1; j++) {
            output[oi++] = ch;
        }
        i += 2;
    }
    output[oi] = '\0';
    return output;
}

static double no_compress(const char *input, char *output, int out_size) {
    strncpy(output, input, out_size - 1);
    output[out_size - 1] = '\0';
    return 1.0;
}

static const char *no_decompress(const char *input, char *output, int out_size) {
    strncpy(output, input, out_size - 1);
    output[out_size - 1] = '\0';
    return output;
}

static CompressionStrategy rle_strategy = { rle_compress, rle_decompress, "RLE" };
static CompressionStrategy none_strategy = { no_compress, no_decompress, "None" };

typedef struct {
    CompressionStrategy *strategy;
} Compressor;

void compressor_set_strategy(Compressor *c, CompressionStrategy *s) {
    c->strategy = s;
}

double compressor_compress(Compressor *c, const char *input, char *output, int out_size) {
    printf("  使用 %s 压缩策略\n", c->strategy->name);
    return c->strategy->compress(input, output, out_size);
}

void demo_compression_strategy(void) {
    printf("\n=== demo_compression_strategy ===\n");
    printf("压缩策略: 运行时选择压缩算法\n\n");

    Compressor comp;
    char input[] = "AAAAAABBBCCDDDD";
    char output[256] = {0};

    compressor_set_strategy(&comp, &rle_strategy);
    double ratio = compressor_compress(&comp, input, output, sizeof(output));
    printf("  输入: \"%s\"\n", input);
    printf("  输出: \"%s\" (压缩率: %.0f%%)\n", output, ratio * 100);

    char decompressed[256] = {0};
    rle_decompress(output, decompressed, sizeof(decompressed));
    printf("  解压: \"%s\"\n", decompressed);

    printf("\n切换到无压缩策略:\n");
    compressor_set_strategy(&comp, &none_strategy);
    ratio = compressor_compress(&comp, input, output, sizeof(output));
    printf("  输出: \"%s\" (压缩率: %.0f%%)\n", output, ratio * 100);

    printf("\n策略模式核心:\n");
    printf("  将算法族封装为函数指针\n");
    printf("  调用方通过统一接口使用不同算法\n");
    printf("  新增算法只需添加函数, 不修改调用方\n");
}

int main(void) {
    printf("策略模式: 函数指针实现算法族\n");

    demo_sort_strategy();
    demo_discount_strategy();
    demo_compression_strategy();

    printf("\n所有演示完成!\n");
    return 0;
}
