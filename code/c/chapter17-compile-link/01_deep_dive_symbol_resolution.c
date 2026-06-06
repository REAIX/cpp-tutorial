/**
 * @file 01_deep_dive_symbol_resolution.c
 * @brief 符号解析深入
 * @description 对应文档: 17-编译与链接 - 符号表、名称修饰、弱符号、static与全局链接
 */
#include <stdio.h>
#include <stdlib.h>

int g_global_init = 42;
int g_global_uninit;
static int g_file_local __attribute__((used)) = 100;
const int g_const = 999;

void demo_symbol_table(void) {
    printf("=== 符号表(Symbol Table) ===\n");
    printf("  查看符号表命令:\n");
    printf("    nm program        (列出所有符号)\n");
    printf("    nm -C program     (C++解修饰)\n");
    printf("    objdump -t program (详细符号表)\n");
    printf("    readelf -s program (ELF符号表)\n\n");

    printf("  符号类型:\n");
    printf("    T (text)   : 代码段全局符号(函数)\n");
    printf("    D (data)   : 数据段已初始化全局符号\n");
    printf("    B (bss)    : BSS段未初始化全局符号\n");
    printf("    t (text)   : 代码段局部符号(static函数)\n");
    printf("    d (data)   : 数据段局部符号(static变量)\n");
    printf("    U (undefined): 未定义符号(需要链接)\n");
    printf("    W (weak)   : 弱符号\n\n");

    printf("  本文件中的符号:\n");
    printf("    g_global_init  -> D (已初始化全局)\n");
    printf("    g_global_uninit -> B (未初始化全局)\n");
    printf("    g_file_local   -> d (static全局)\n");
    printf("    g_const        -> R (只读数据段)\n\n");
}

void demo_name_mangling(void) {
    printf("=== C语言名称修饰(Name Mangling) ===\n");
    printf("  C语言: 几乎不做名称修饰\n");
    printf("    void foo(int x)  -> 符号名: foo\n\n");
    printf("  C++语言: 做名称修饰(支持函数重载)\n");
    printf("    void foo(int x)    -> 符号名: _Z3fooi\n");
    printf("    void foo(double x) -> 符号名: _Z3food\n");
    printf("    void foo(int, int) -> 符号名: _Z3fooii\n\n");
    printf("  C/C++混合编程:\n");
    printf("    #ifdef __cplusplus\n");
    printf("    extern \"C\" {\n");
    printf("    #endif\n");
    printf("    void c_api_function(void);  // C风格链接\n");
    printf("    #ifdef __cplusplus\n");
    printf("    }\n");
    printf("    #endif\n\n");
}

__attribute__((weak)) void optional_handler(void) {
    printf("  默认弱符号实现: 可被用户覆盖\n");
}

void demo_weak_symbol(void) {
    printf("=== 弱符号(Weak Symbol) ===\n");
    printf("  弱符号: 可被强符号覆盖的定义\n\n");
    printf("  GCC语法:\n");
    printf("    __attribute__((weak)) void handler(void) { ... }\n\n");
    printf("  用途:\n");
    printf("    1. 提供默认实现, 允许用户覆盖\n");
    printf("    2. 可选功能: 如果没有提供实现, 使用弱符号\n");
    printf("    3. 插件架构: 弱符号作为钩子点\n\n");

    printf("  测试弱符号:\n");
    optional_handler();

    printf("\n  强符号 vs 弱符号规则:\n");
    printf("    - 强符号 + 强符号 = 链接错误(多重定义)\n");
    printf("    - 强符号 + 弱符号 = 使用强符号\n");
    printf("    - 弱符号 + 弱符号 = 选择任意一个\n\n");
}

void demo_static_linkage(void) {
    printf("=== static与全局链接 ===\n");
    printf("  static函数:\n");
    printf("    - 内部链接, 只在当前编译单元可见\n");
    printf("    - 不会出现在目标文件的全局符号表中\n");
    printf("    - 编译器可以更积极优化(内联等)\n\n");
    printf("  static全局变量:\n");
    printf("    - 内部链接, 不会与其他文件冲突\n");
    printf("    - 存放在.data或.bss段\n");
    printf("    - 生命周期: 程序启动到结束\n\n");
    printf("  非static全局:\n");
    printf("    - 外部链接, 所有编译单元可见\n");
    printf("    - 可通过extern在其他文件中引用\n\n");
}

void demo_symbol_resolution_process(void) {
    printf("=== 符号解析过程 ===\n");
    printf("  步骤1: 收集所有目标文件的符号表\n");
    printf("  步骤2: 对每个未定义符号U, 查找已定义符号D/T\n");
    printf("  步骤3: 如果找到多个强符号定义 -> 报错\n");
    printf("  步骤4: 如果找到强符号+弱符号 -> 使用强符号\n");
    printf("  步骤5: 如果只找到弱符号 -> 使用弱符号\n");
    printf("  步骤6: 如果找不到任何定义 -> 报错(undefined reference)\n\n");

    printf("  示例:\n");
    printf("    main.o:   U printf, U malloc, T main, D g_val\n");
    printf("    utils.o:  T helper, D g_val\n");
    printf("    libc.a:   T printf, T malloc\n\n");
    printf("  解析结果:\n");
    printf("    printf -> libc.a中的定义\n");
    printf("    malloc -> libc.a中的定义\n");
    printf("    g_val  -> 如果两个都是强定义 -> 错误!\n");
    printf("    main   -> main.o中的定义\n");
    printf("    helper -> utils.o中的定义\n\n");
}

int main(void) {
    printf("========== 符号解析深入 ==========\n\n");

    demo_symbol_table();
    demo_name_mangling();
    demo_weak_symbol();
    demo_static_linkage();
    demo_symbol_resolution_process();

    printf("========== 所有演示完成 ==========\n");
    return 0;
}
