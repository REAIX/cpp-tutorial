/** @file 01_example_variadic_functions.c
 *  @brief 可变参数函数：va_list、va_start、va_arg、va_end、自定义printf类函数
 *  @description 对应文档: 11-可变参数与命令行
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static int sum_ints(int count, ...) {
    va_list args;
    va_start(args, count);

    int total = 0;
    for (int i = 0; i < count; i++) {
        total += va_arg(args, int);
    }

    va_end(args);
    return total;
}

static double average(int count, ...) {
    if (count <= 0) return 0.0;

    va_list args;
    va_start(args, count);

    double total = 0.0;
    for (int i = 0; i < count; i++) {
        total += va_arg(args, double);
    }

    va_end(args);
    return total / count;
}

static int max_int(int count, ...) {
    if (count <= 0) return 0;

    va_list args;
    va_start(args, count);

    int max_val = va_arg(args, int);
    for (int i = 1; i < count; i++) {
        int val = va_arg(args, int);
        if (val > max_val) max_val = val;
    }

    va_end(args);
    return max_val;
}

static void log_message(const char *level, const char *format, ...) {
    va_list args;
    va_start(args, format);

    printf("[%s] ", level);
    vprintf(format, args);
    printf("\n");

    va_end(args);
}

static int format_string(char *buf, size_t size, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, size, fmt, args);
    va_end(args);
    return n;
}

static int sum_and_product(int count, int *product, ...) {
    va_list args;
    va_start(args, product);

    va_list args_copy;
    va_copy(args_copy, args);

    int total = 0;
    *product = 1;
    for (int i = 0; i < count; i++) {
        int val = va_arg(args, int);
        total += val;
    }

    for (int i = 0; i < count; i++) {
        int val = va_arg(args_copy, int);
        *product *= val;
    }

    va_end(args);
    va_end(args_copy);
    return total;
}

void demo_va_basics(void) {
    printf("=== 可变参数基础 ===\n");

    printf("sum_ints(3, 10, 20, 30) = %d\n", sum_ints(3, 10, 20, 30));
    printf("sum_ints(5, 1, 2, 3, 4, 5) = %d\n", sum_ints(5, 1, 2, 3, 4, 5));
    printf("sum_ints(0) = %d\n", sum_ints(0));

    printf("\n可变参数函数的要素:\n");
    printf("  va_list  声明参数列表变量\n");
    printf("  va_start 初始化参数列表, 需要最后一个固定参数\n");
    printf("  va_arg   获取下一个参数, 需要指定类型\n");
    printf("  va_end   清理参数列表\n");

    printf("\n");
}

void demo_va_average(void) {
    printf("=== 可变参数: 计算平均值 ===\n");

    printf("average(3, 80.0, 90.0, 85.0) = %.2f\n", average(3, 80.0, 90.0, 85.0));
    printf("average(5, 1.0, 2.0, 3.0, 4.0, 5.0) = %.2f\n", average(5, 1.0, 2.0, 3.0, 4.0, 5.0));

    printf("\n注意: float 参数会自动提升为 double\n");
    printf("char/short 会提升为 int\n");

    printf("\n");
}

void demo_va_max(void) {
    printf("=== 可变参数: 求最大值 ===\n");

    printf("max_int(5, 3, 7, 1, 9, 4) = %d\n", max_int(5, 3, 7, 1, 9, 4));
    printf("max_int(1, 42) = %d\n", max_int(1, 42));

    printf("\n");
}

void demo_custom_printf(void) {
    printf("=== 自定义 printf 类函数 ===\n");

    log_message("INFO", "程序启动, 版本 %d.%d", 1, 0);
    log_message("WARN", "内存使用率 %d%%", 85);
    log_message("ERROR", "文件 %s 不存在, 错误码: %d", "data.txt", 2);

    printf("\nvprintf: 用 va_list 参数调用 printf 的核心逻辑\n");
    printf("类似函数: vfprintf, vsnprintf, vsprintf\n");

    printf("\n");
}

void demo_vsnprintf_wrapper(void) {
    printf("=== vsnprintf 封装 ===\n");

    char buffer[64];
    int written = format_string(buffer, sizeof(buffer), "姓名: %s, 年龄: %d", "张三", 25);
    printf("格式化结果: \"%s\" (写入 %d 字符)\n", buffer, written);

    written = format_string(buffer, sizeof(buffer), "分数: %.2f", 95.5);
    printf("格式化结果: \"%s\" (写入 %d 字符)\n", buffer, written);

    printf("\nvsnprintf 是构建自定义格式化函数的基础\n");

    printf("\n");
}

void demo_va_copy(void) {
    printf("=== va_copy (C99) ===\n");

    int product;
    int sum = sum_and_product(4, &product, 2, 3, 4, 5);
    printf("sum_and_product(2, 3, 4, 5): sum = %d, product = %d\n", sum, product);

    printf("\nva_copy: 复制 va_list 的当前状态\n");
    printf("用于需要多次遍历参数列表的场景\n");
    printf("每个 va_copy 都必须有对应的 va_end\n");

    printf("\n");
}

int main(void) {
    demo_va_basics();
    demo_va_average();
    demo_va_max();
    demo_custom_printf();
    demo_vsnprintf_wrapper();
    demo_va_copy();

    return 0;
}
