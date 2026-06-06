/**
 * @file 02_deep_dive_linking_issues.c
 * @brief 链接问题深入
 * @description 对应文档: 16-多文件编程 - 多重定义、未定义引用、头文件中inline、暂定定义、公共符号
 */
#include <stdio.h>
#include <stdlib.h>

void demo_multiple_definition(void) {
    printf("=== 多重定义错误(multiple definition) ===\n");
    printf("  场景: 同一符号在多个目标文件中有定义\n\n");
    printf("  错误示例:\n");
    printf("    file1.c:  int g_counter = 0;  // 定义\n");
    printf("    file2.c:  int g_counter = 0;  // 也定义了! -> 链接错误!\n\n");
    printf("  正确做法:\n");
    printf("    file1.c:  int g_counter = 0;  // 定义(唯一)\n");
    printf("    file2.c:  extern int g_counter;  // 声明(引用)\n\n");
    printf("  头文件中只放声明:\n");
    printf("    header.h: extern int g_counter;  // 声明\n");
    printf("    source.c: int g_counter = 0;     // 定义(仅一个.c文件)\n\n");
}

void demo_undefined_reference(void) {
    printf("=== 未定义引用(undefined reference) ===\n");
    printf("  场景: 使用了声明但未定义的符号\n\n");
    printf("  常见原因:\n");
    printf("    1. 忘记链接库: gcc main.o -lm (数学库)\n");
    printf("    2. 忘记编译源文件: gcc main.c utils.c (缺少utils.c)\n");
    printf("    3. 函数名拼写错误\n");
    printf("    4. C/C++混合编译未用extern \"C\"\n");
    printf("    5. 库的链接顺序错误(依赖者在前, 被依赖者在后)\n\n");
    printf("  排查方法:\n");
    printf("    - nm工具: 查看目标文件的符号表\n");
    printf("    - nm -u main.o: 查看未定义符号\n");
    printf("    - ldd: 查看可执行文件的动态库依赖\n\n");
}

void demo_inline_in_header(void) {
    printf("=== 头文件中的inline函数 ===\n");
    printf("  C99 inline的复杂性:\n");
    printf("    - inline只是建议, 不是指令\n");
    printf("    - inline函数可能有多个定义(每个包含头文件的编译单元)\n\n");
    printf("  C99/C11正确用法:\n");
    printf("    header.h:\n");
    printf("      static inline int max(int a, int b) {\n");
    printf("          return a > b ? a : b;\n");
    printf("      }\n");
    printf("    (static inline是头文件中最安全的做法)\n\n");
    printf("  C99 extern inline:\n");
    printf("    header.h:\n");
    printf("      inline int max(int a, int b);  // 声明\n");
    printf("    source.c:\n");
    printf("      extern inline int max(int a, int b) {  // 外部定义\n");
    printf("          return a > b ? a : b;\n");
    printf("      }\n\n");
    printf("  建议: 头文件中用 static inline, 避免C99 inline的坑\n\n");
}

void demo_tentative_definition(void) {
    printf("=== 暂定定义(tentative definition) ===\n");
    printf("  C语言特有概念:\n");
    printf("    int g_val;  // 暂定定义(没有初始化器的全局变量)\n\n");
    printf("  规则:\n");
    printf("    - 如果编译单元内没有后续的完整定义, 暂定定义变为定义\n");
    printf("    - 多个暂定定义是合法的(合并为一个), 但这是C的陷阱!\n\n");
    printf("  示例:\n");
    printf("    file1.c:  int g_val;  // 暂定定义1\n");
    printf("    file2.c:  int g_val;  // 暂定定义2\n");
    printf("    链接器可能合并(公共符号), 也可能报错(取决于编译器)\n\n");
    printf("  最佳实践:\n");
    printf("    - 全局变量始终提供初始化器: int g_val = 0;\n");
    printf("    - 使用 -fno-common 编译选项(禁止公共符号合并)\n");
    printf("    - GCC 10+默认启用 -fno-common\n\n");
}

void demo_common_symbols(void) {
    printf("=== 公共符号(common symbols) ===\n");
    printf("  历史遗留: Fortran的COMMON块概念\n");
    printf("  多个未初始化的同名全局变量被合并为一个\n\n");
    printf("  问题:\n");
    printf("    file1.c:  int g_buf[100];  // 400字节\n");
    printf("    file2.c:  int g_buf[200];  // 800字节\n");
    printf("    链接器取较大者, 不报错! 潜在的bug!\n\n");
    printf("  解决:\n");
    printf("    - 使用 -fno-common 强制报错\n");
    printf("    - 始终初始化全局变量\n");
    printf("    - 使用static限制作用域\n\n");
}

void demo_static_vs_global_linkage(void) {
    printf("=== static vs 全局链接 ===\n");

    static int file_local_var = 42;

    printf("  static全局变量/函数:\n");
    printf("    - 内部链接: 只在当前编译单元可见\n");
    printf("    - 不会与其他文件的同名符号冲突\n");
    printf("    - file_local_var = %d (本文件私有)\n\n", file_local_var);

    printf("  非static全局变量/函数:\n");
    printf("    - 外部链接: 所有编译单元可见\n");
    printf("    - 可被其他文件通过extern引用\n\n");

    printf("  规则:\n");
    printf("    - 默认全局可见(外部链接)\n");
    printf("    - static限制为内部链接\n");
    printf("    - 头文件中声明用extern(显式标记外部链接)\n\n");
}

void demo_linking_order(void) {
    printf("=== 链接顺序问题 ===\n");
    printf("  GCC链接器从左到右处理, 符号引用必须在其定义之后\n\n");
    printf("  错误顺序:\n");
    printf("    gcc -lm main.o  (数学库在main.o之前, 找不到引用)\n\n");
    printf("  正确顺序:\n");
    printf("    gcc main.o -lm  (main.o先, 数学库后)\n\n");
    printf("  依赖关系:\n");
    printf("    如果A依赖B, B依赖C:\n");
    printf("    gcc main.o libA.a libB.a libC.a\n");
    printf("    (被依赖的库放后面)\n\n");
}

int main(void) {
    printf("========== 链接问题深入 ==========\n\n");

    demo_multiple_definition();
    demo_undefined_reference();
    demo_inline_in_header();
    demo_tentative_definition();
    demo_common_symbols();
    demo_static_vs_global_linkage();
    demo_linking_order();

    printf("========== 所有演示完成 ==========\n");
    return 0;
}
