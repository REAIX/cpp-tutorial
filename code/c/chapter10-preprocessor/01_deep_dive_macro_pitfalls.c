/** @file 01_deep_dive_macro_pitfalls.c
 *  @brief 宏陷阱：副作用、多次求值、运算符优先级、do-while(0)惯用法、宏vs inline
 *  @description 对应文档: 10-预处理器 | 举一反三：避免宏的常见陷阱
 */

#include <stdio.h>
#include <string.h>

void demo_macro_side_effects(void) {
    printf("=== 宏的副作用: 多次求值 ===\n");

#define BAD_SQUARE(x) ((x) * (x))
#define BAD_INC(x) ((x) + 1)

    int a = 5;
    printf("a = %d\n", a);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsequence-point"
    printf("BAD_SQUARE(a++) = %d\n", BAD_SQUARE(a++));
#pragma GCC diagnostic pop
    printf("执行后 a = %d (a 被自增了两次!)\n", a);

    int b = 5;
    printf("\nb = %d\n", b);
    printf("BAD_INC(b++) = %d\n", BAD_INC(b++));
    printf("执行后 b = %d (b 被自增了一次, 但结果可能不符合预期)\n", b);

    printf("\n原因: 宏是文本替换, BAD_SQUARE(a++) 展开为 ((a++) * (a++))\n");
    printf("a++ 执行了两次, 行为是未定义的\n");

    printf("\n");
}

void demo_operator_precedence(void) {
    printf("=== 运算符优先级陷阱 ===\n");

#define BAD_MULTIPLE(x) (x) * (x)
#define GOOD_MULTIPLE(x) ((x) * (x))

    printf("不加外层括号:\n");
    printf("  BAD_MULTIPLE(2+3) 展开为 (2+3) * (2+3) = %d\n", BAD_MULTIPLE(2+3));

#define BAD_ADD_DOUBLE(x) (x) + (x)
    printf("\n  BAD_ADD_DOUBLE(5) * 2 展开为 (5) + (5) * 2 = %d\n", BAD_ADD_DOUBLE(5) * 2);
    printf("  期望 20, 实际 15! 因为 * 优先级高于 +\n");

#define GOOD_ADD_DOUBLE(x) ((x) + (x))
    printf("  GOOD_ADD_DOUBLE(5) * 2 = %d\n", GOOD_ADD_DOUBLE(5) * 2);

    printf("\n规则: 宏定义中, 每个参数用括号包围, 整个表达式也用括号包围\n");

    printf("\n");
}

static void hello(void) { printf("  hello called\n"); }

void demo_if_else_trap(void) {
    printf("=== if-else 陷阱 ===\n");

#define BAD_SAFE_CALL(func) \
    if (func != NULL) func()

    typedef void (*VoidFunc)(void);

    VoidFunc func = hello;

    printf("正常使用:\n");
    if (1) { BAD_SAFE_CALL(func); }
    else printf("  else branch\n");

    printf("\n问题: 多语句宏在 if-else 中可能出错\n");

#define BAD_SWAP(a, b) \
    int _temp = a;     \
    a = b;             \
    b = _temp

    printf("\n  if (cond)\n");
    printf("      BAD_SWAP(x, y);  // 只有第一句属于 if!\n");
    printf("  else\n");
    printf("      ...  // 编译错误: else 没有匹配的 if\n");

    printf("\n");
}

void demo_do_while_idiom(void) {
    printf("=== do-while(0) 惯用法 ===\n");

#define SAFE_SWAP(type, a, b) do { \
    type _temp = a;                \
    a = b;                         \
    b = _temp;                     \
} while(0)

    int x = 10, y = 20;
    printf("交换前: x = %d, y = %d\n", x, y);

    if (1)
        SAFE_SWAP(int, x, y);
    else
        printf("else branch\n");

    printf("交换后: x = %d, y = %d\n", x, y);

    printf("\ndo-while(0) 的作用:\n");
    printf("1. 创建独立作用域 (局部变量 _temp 不泄漏)\n");
    printf("2. 宏后面加分号是自然的语句结束\n");
    printf("3. 在 if-else 中使用安全\n");
    printf("4. 编译器优化掉循环 (条件为常量 false)\n");

    printf("\n");
}

static inline int square_inline(int x) {
    return x * x;
}

void demo_macro_vs_inline(void) {
    printf("=== 宏 vs inline 函数 ===\n");

#define SQUARE_MACRO(x) ((x) * (x))

    int a = 5;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsequence-point"
    printf("SQUARE_MACRO(a++) = %d (a 被求值两次)\n", SQUARE_MACRO(a++));
#pragma GCC diagnostic pop
    a = 5;
    printf("square_inline(a++) = %d (a 只被求值一次)\n", square_inline(a++));

    printf("\n对比:\n");
    printf("%-15s %-15s %-15s\n", "特性", "宏", "inline函数");
    printf("%-15s %-15s %-15s\n", "----", "--", "----------");
    printf("%-15s %-15s %-15s\n", "类型检查", "无", "有");
    printf("%-15s %-15s %-15s\n", "参数求值", "可能多次", "恰好一次");
    printf("%-15s %-15s %-15s\n", "调试", "困难", "容易");
    printf("%-15s %-15s %-15s\n", "作用域", "文件级", "可限制");
    printf("%-15s %-15s %-15s\n", "递归", "不可", "可以");

    printf("\n推荐: 优先使用 inline 函数, 仅在必须时使用宏\n");
    printf("宏的适用场景: 条件编译、代码生成、编译时计算\n");

    printf("\n");
}

void demo_macro_naming(void) {
    printf("=== 宏命名规范 ===\n");

    printf("1. 宏名全部大写, 用下划线分隔\n");
    printf("   #define MAX_BUFFER_SIZE 1024\n\n");

    printf("2. 区分宏和函数, 让使用者知道副作用风险\n");
    printf("   MAX(a, b) 是宏, max(a, b) 是函数\n\n");

    printf("3. 多语句宏使用 do-while(0)\n\n");

    printf("4. 宏参数加括号, 整个表达式加括号\n");
    printf("   #define CUBE(x) ((x) * (x) * (x))\n\n");

    printf("5. 避免在宏中改变控制流\n");
    printf("   不要在宏中使用 return, break, continue\n\n");

    printf("6. 宏定义不要太长, 考虑用 inline 函数替代\n");

    printf("\n");
}

void demo_macro_debug_techniques(void) {
    printf("=== 宏调试技巧 ===\n");

    printf("1. 查看宏展开结果:\n");
    printf("   gcc -E file.c  只预处理, 不编译\n\n");

    printf("2. 逐步展开复杂宏:\n");
    printf("   #define X(a, b) ...\n");
    printf("   #define Y(a) X(a, 10)\n");
    printf("   手动展开 Y(5) => X(5, 10)\n\n");

    printf("3. 使用 # 运算符打印参数:\n");
    printf("   #define DEBUG_MACRO(x) printf(#x \" = %%d\\n\", x)\n\n");

    printf("4. 编译器警告:\n");
    printf("   gcc -Wall -Wextra 会警告一些宏问题\n");

    printf("\n");
}

int main(void) {
    demo_macro_side_effects();
    demo_operator_precedence();
    demo_if_else_trap();
    demo_do_while_idiom();
    demo_macro_vs_inline();
    demo_macro_naming();
    demo_macro_debug_techniques();

    return 0;
}
