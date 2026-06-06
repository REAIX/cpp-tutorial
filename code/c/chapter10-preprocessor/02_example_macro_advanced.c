/** @file 02_example_macro_advanced.c
 *  @brief 宏进阶：带参数宏、token pasting(##)、stringification(#)、X-macro模式
 *  @description 对应文档: 10-预处理器
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void demo_stringification(void) {
    printf("=== # 字符串化运算符 ===\n");

#define TO_STRING(x) #x
#define PRINT_VALUE(x) printf(#x " = %d\n", x)

    printf("TO_STRING(Hello) = \"%s\"\n", TO_STRING(Hello));
    printf("TO_STRING(123 + 456) = \"%s\"\n", TO_STRING(123 + 456));
    printf("TO_STRING(int *) = \"%s\"\n", TO_STRING(int *));

    int score = 95;
    PRINT_VALUE(score);

    printf("\n# 将宏参数原样转为字符串字面量\n");
    printf("注意: 参数不会被展开, 直接文本替换\n");

    printf("\n");
}

void demo_token_pasting(void) {
    printf("=== ## 标记粘贴运算符 ===\n");

#define CONCAT(a, b) a ## b
#define MAKE_VAR(name, num) name ## num

    int CONCAT(var, 1) = 100;
    int CONCAT(var, 2) = 200;
    int MAKE_VAR(value, 3) = 300;

    printf("CONCAT(var, 1) = %d\n", var1);
    printf("CONCAT(var, 2) = %d\n", var2);
    printf("MAKE_VAR(value, 3) = %d\n", value3);

#define MAKE_FUNC(prefix, type) \
    type prefix ## _ ## type() { return (type)0; }

    printf("\n## 将两个标记粘贴成一个新的标记\n");
    printf("常用于生成变量名、函数名、枚举值等\n");

    printf("\n");
}

void demo_multiline_macro(void) {
    printf("=== 多行宏 ===\n");

#define SWAP(type, a, b) do { \
    type temp = a;            \
    a = b;                    \
    b = temp;                 \
} while(0)

    int x = 10, y = 20;
    printf("交换前: x = %d, y = %d\n", x, y);
    SWAP(int, x, y);
    printf("交换后: x = %d, y = %d\n", x, y);

    double d1 = 1.5, d2 = 2.5;
    printf("\n交换前: d1 = %.1f, d2 = %.1f\n", d1, d2);
    SWAP(double, d1, d2);
    printf("交换后: d1 = %.1f, d2 = %.1f\n", d1, d2);

    printf("\n多行宏用反斜杠续行\n");
    printf("do-while(0) 惯用法确保宏在 if/else 中使用时安全\n");

    printf("\n");
}

#define ERROR_LIST \
    X(ERR_NONE,    0, "无错误")       \
    X(ERR_IO,      1, "IO错误")       \
    X(ERR_MEMORY,  2, "内存不足")     \
    X(ERR_TIMEOUT, 3, "超时")         \
    X(ERR_INVALID, 4, "无效参数")

typedef enum {
#define X(name, code, desc) name = code,
    ERROR_LIST
#undef X
    ERR_COUNT
} ErrorCode;

static const char *error_desc(ErrorCode code) {
    switch (code) {
#define X(name, code, desc) case name: return desc;
        ERROR_LIST
#undef X
        default: return "未知错误";
    }
}

void demo_x_macro_pattern(void) {
    printf("=== X-Macro 模式 ===\n");

    printf("错误码枚举:\n");
#define X(name, code, desc) printf("  %s = %d: %s\n", #name, code, desc);
    ERROR_LIST
#undef X

    printf("\n通过错误码获取描述:\n");
    printf("  ERR_IO 的描述: %s\n", error_desc(ERR_IO));
    printf("  ERR_MEMORY 的描述: %s\n", error_desc(ERR_MEMORY));

    printf("\nX-Macro 优势: 数据只定义一次, 自动生成枚举、字符串、switch等\n");
    printf("新增错误只需在 ERROR_LIST 中添加一行\n");

    printf("\n");
}

void demo_generic_macro(void) {
    printf("=== 通用类型宏 (C11 _Generic) ===\n");

#define TYPE_NAME(x) _Generic((x), \
    char: "char",                   \
    int: "int",                     \
    long: "long",                   \
    float: "float",                 \
    double: "double",               \
    char *: "char*",                \
    int *: "int*",                  \
    void *: "void*",                \
    default: "unknown")

    char c = 'A';
    int i = 42;
    double d = 3.14;
    char *s = "hello";

    printf("c 的类型: %s\n", TYPE_NAME(c));
    printf("i 的类型: %s\n", TYPE_NAME(i));
    printf("d 的类型: %s\n", TYPE_NAME(d));
    printf("s 的类型: %s\n", TYPE_NAME(s));

    printf("\n_Generic 是 C11 引入的编译时类型选择\n");
    printf("可以实现类似函数重载的效果\n");

    printf("\n");
}

void demo_assert_macro(void) {
    printf("=== 自定义断言宏 ===\n");

#ifdef NDEBUG
#define MY_ASSERT(cond) ((void)0)
#else
#define MY_ASSERT(cond) do {                                     \
    if (!(cond)) {                                               \
        fprintf(stderr, "断言失败: %s, 文件: %s, 行: %d\n",      \
                #cond, __FILE__, __LINE__);                       \
        abort();                                                 \
    }                                                            \
} while(0)
#endif

    int value = 42;
    MY_ASSERT(value > 0);
    printf("MY_ASSERT(value > 0) 通过\n");

    printf("\nNDEBUG 定义时断言被替换为 ((void)0), 无任何开销\n");

    printf("\n");
}

int main(void) {
    demo_stringification();
    demo_token_pasting();
    demo_multiline_macro();
    demo_x_macro_pattern();
    demo_generic_macro();
    demo_assert_macro();

    return 0;
}
