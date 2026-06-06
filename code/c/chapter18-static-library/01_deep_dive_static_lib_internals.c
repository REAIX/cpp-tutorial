/**
 * @file 01_deep_dive_static_lib_internals.c
 * @brief 静态库深入剖析 - 符号解析、归档格式、链接顺序、重复符号
 * @description 对应文档: 18-static-library
 *              本文件以独立可运行代码演示静态库的内部工作原理，
 *              包括符号解析机制、归档文件格式、链接顺序问题、重复符号处理等
 *
 * 编译: gcc 01_deep_dive_static_lib_internals.c -o deep_dive_static_lib
 * 运行: ./deep_dive_static_lib
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========================================================================
 * 第一部分: 符号与符号解析
 * ======================================================================== */

/*
 * 符号(Symbol)是链接器工作的基本单位。每个函数和全局变量在目标文件中
 * 都有一个对应的符号。链接器的核心任务就是解析这些符号的引用。
 *
 * 符号分为三类:
 *   1. 全局符号: 非static的函数和全局变量，可被其他目标文件引用
 *   2. 局部符号: static函数和static全局变量，只在当前编译单元可见
 *   3. 外部符号: 在当前编译单元中声明但未定义，需要在其他地方找到定义
 *
 * 可以用 nm 命令查看目标文件的符号表:
 *   nm mathlib.o
 * 输出格式示例:
 *   0000000000000000 T mathlib_add      # T = 代码段的全局符号
 *   0000000000000050 T mathlib_factorial
 *                    U printf            # U = 未定义符号(外部引用)
 */

static int global_var = 42;
static int static_var = 100;

static int helper_add(int a, int b) {
    return a + b;
}

void demo_symbol_types(void) {
    printf("===== 符号类型演示 =====\n");
    printf("全局符号 (global_var):    值 = %d, 非static全局变量，外部可见\n", global_var);
    printf("局部符号 (static_var):    值 = %d, static全局变量，仅本文件可见\n", static_var);
    printf("局部函数 (helper_add):    结果 = %d, static函数，仅本文件可见\n", helper_add(3, 5));
    printf("\n");

    printf("查看符号的命令:\n");
    printf("  nm <object_file>              # 查看符号表\n");
    printf("  nm -C <object_file>           # 解码C++符号名\n");
    printf("  readelf -s <object_file>      # ELF格式详细符号表\n");
    printf("  objdump -t <object_file>      # 目标文件符号表\n");
    printf("\n");

    printf("符号类型标识:\n");
    printf("  T - 代码段全局符号 (Text section, global)\n");
    printf("  t - 代码段局部符号 (Text section, local/static)\n");
    printf("  D - 数据段全局符号 (Data section, global)\n");
    printf("  d - 数据段局部符号 (Data section, local/static)\n");
    printf("  U - 未定义符号   (Undefined, 需要链接器解析)\n");
    printf("  W - 弱符号       (Weak symbol)\n");
    printf("\n");
}

/* ========================================================================
 * 第二部分: 归档文件格式 (Archive Format)
 * ======================================================================== */

/*
 * 静态库 (.a 文件) 实际上是 ar 归档工具创建的归档文件，
 * 包含多个目标文件 (.o) 的集合。
 *
 * 归档文件结构:
 *   +------------------+
 *   | 归档头部 (8字节)  |  "!<arch>\n"
 *   +------------------+
 *   | 成员头部         |  包含文件名、大小、权限等元数据
 *   +------------------+
 *   | 目标文件 1 (.o)  |
 *   +------------------+
 *   | 成员头部         |
 *   +------------------+
 *   | 目标文件 2 (.o)  |
 *   +------------------+
 *   | ...              |
 *   +------------------+
 *   | 符号索引         |  由 ar s 或 ranlib 生成，加速符号查找
 *   +------------------+
 *
 * 符号索引的作用:
 *   链接器不需要遍历所有 .o 文件来查找符号，直接查索引即可。
 *   如果没有符号索引，链接速度会显著下降。
 *   ar 的 's' 选项会自动创建符号索引，等同于运行 ranlib。
 */

void demo_archive_format(void) {
    printf("===== 归档文件格式演示 =====\n");
    printf("静态库 (.a) 本质是 ar 归档格式，包含多个 .o 文件的集合\n\n");

    printf("归档文件内部结构:\n");
    printf("  [归档魔数]  \"!<arch>\\n\"  (8字节)\n");
    printf("  [成员头部]  文件名、修改时间、大小、权限\n");
    printf("  [目标文件]  mathlib.o 的完整内容\n");
    printf("  [成员头部]  ...\n");
    printf("  [目标文件]  另一个 .o\n");
    printf("  [符号索引]  加速链接器查找符号的索引表\n\n");

    printf("常用归档操作命令:\n");
    printf("  ar rcs libfoo.a a.o b.o c.o    # 创建静态库\n");
    printf("  ar -t libfoo.a                  # 列出所有成员\n");
    printf("  ar -x libfoo.a a.o              # 提取单个成员\n");
    printf("  ar -d libfoo.a a.o              # 删除成员\n");
    printf("  ar -s libfoo.a                  # 重建符号索引 (= ranlib)\n");
    printf("  nm -s libfoo.a                  # 查看符号索引\n\n");

    printf("查看归档详细信息的命令:\n");
    printf("  ar -tv libfoo.a                 # 详细列出成员(含大小、时间)\n");
    printf("  objdump -a libfoo.a             # 显示归档头部信息\n");
    printf("\n");
}

/* ========================================================================
 * 第三部分: 链接顺序问题
 * ======================================================================== */

/*
 * 链接器处理静态库的方式与处理目标文件的方式不同:
 *
 * 目标文件 (.o): 链接器无条件地将其所有符号加入符号表。
 * 静态库 (.a):   链接器只在需要解析未定义符号时才从库中提取 .o 文件。
 *
 * 这导致了链接顺序的重要性:
 *
 *   正确: gcc main.o -lmathlib        # main.o 先被处理，产生未定义符号，
 *                                      # 然后从 libmathlib.a 中查找解析
 *
 *   错误: gcc -lmathlib main.o        # 先处理库，此时没有未定义符号需要解析，
 *                                      # 库被跳过；然后处理 main.o 时发现未定义符号
 *                                      # 但已经来不及了！
 *
 * 规则: 引用符号的文件放在前面，定义符号的库放在后面！
 *
 * 依赖链: A 依赖 B，B 依赖 C
 *   正确: gcc A.o -lB -lC
 *   错误: gcc -lC -lB A.o
 *
 * 循环依赖: A 依赖 B，B 也依赖 A
 *   解决: gcc A.o -lB -lA   (重复列出库)
 */

void demo_linking_order(void) {
    printf("===== 链接顺序演示 =====\n");
    printf("链接器处理静态库的规则:\n");
    printf("  1. 从左到右依次处理命令行参数\n");
    printf("  2. 遇到 .o 文件: 无条件加入，提取所有全局符号\n");
    printf("  3. 遇到 .a 文件: 只提取能解析当前未定义符号的 .o 成员\n");
    printf("  4. 如果库中没有能解析的符号，整个库被跳过\n\n");

    printf("常见错误示例:\n");
    printf("  gcc -lmathlib main.c    # 错误! 库在 main 之前，被跳过\n");
    printf("  gcc main.c -lmathlib    # 正确! main 先产生未定义符号，库再解析\n\n");

    printf("依赖链的链接顺序:\n");
    printf("  app 依赖 libA, libA 依赖 libB:\n");
    printf("  gcc main.o -lA -lB     # 正确\n");
    printf("  gcc -lB -lA main.o     # 错误\n\n");

    printf("循环依赖的解决:\n");
    printf("  libA 和 libB 互相依赖:\n");
    printf("  gcc main.o -lA -lB -lA   # 重复列出库\n\n");

    printf("举一反三:\n");
    printf("  - 第三方库始终放在自己库的后面\n");
    printf("  - 系统库 (-lpthread, -lm) 放在最后\n");
    printf("  - CMake 自动处理链接顺序，无需手动关心\n");
    printf("\n");
}

/* ========================================================================
 * 第四部分: 重复符号处理
 * ======================================================================== */

/*
 * 当多个目标文件定义了同名全局符号时，链接器需要决定如何处理。
 *
 * 强符号 (Strong Symbol): 普通的全局函数和全局变量定义
 * 弱符号 (Weak Symbol):   使用 __attribute__((weak)) 声明的符号
 *
 * 规则:
 *   1. 不允许两个强符号同名 → 链接错误 (multiple definition)
 *   2. 一个强符号和一个弱符号同名 → 选择强符号
 *   3. 两个弱符号同名 → 选择任意一个（不可预测）
 *
 * 规则2的应用: 库函数可以用弱符号提供默认实现，用户可以定义强符号覆盖。
 */

int common_variable = 10;

__attribute__((weak)) int weak_function(void) {
    return 100;
}

__attribute__((weak)) int weak_default_value = 200;

void demo_duplicate_symbols(void) {
    printf("===== 重复符号处理演示 =====\n");
    printf("链接器处理重复符号的规则:\n\n");

    printf("规则1: 两个强符号同名 → 链接错误\n");
    printf("  file1.c: int foo = 1;\n");
    printf("  file2.c: int foo = 2;\n");
    printf("  结果: multiple definition of 'foo'\n\n");

    printf("规则2: 一个强符号 + 一个弱符号 → 强符号胜出\n");
    printf("  本文件中 weak_function() 是弱符号 (返回100)\n");
    printf("  如果另一个文件定义了 int weak_function() { return 999; }\n");
    printf("  则链接时强符号版本会被使用\n");
    printf("  当前 weak_function() 返回值: %d\n", weak_function());
    printf("\n");

    printf("规则3: 两个弱符号同名 → 随机选择一个（不可预测）\n");
    printf("  这是未定义行为，应避免\n\n");

    printf("弱符号的实际应用:\n");
    printf("  1. 库提供默认实现，用户可覆盖:\n");
    printf("     __attribute__((weak)) void on_error(void) { /* 默认空实现 */ }\n");
    printf("  2. 可选功能检测:\n");
    printf("     if (weak_function) { /* 功能可用 */ } else { /* 不可用 */ }\n\n");

    printf("举一反三 - 避免重复符号的最佳实践:\n");
    printf("  - 全局变量使用 static 限制作用域\n");
    printf("  - 头文件中只放声明，不放定义\n");
    printf("  - 使用命名前缀避免符号冲突\n");
    printf("  - 使用 -fvisibility=hidden 控制符号可见性\n");
    printf("\n");
}

/* ========================================================================
 * 第五部分: 静态库 vs 动态库对比
 * ======================================================================== */

void demo_static_vs_dynamic(void) {
    printf("===== 静态库 vs 动态库对比 =====\n");
    printf("特性          静态库 (.a)        动态库 (.so)\n");
    printf("─────────────────────────────────────────────────\n");
    printf("链接时机      编译时              运行时\n");
    printf("代码复制      完整复制到可执行文件 多个程序共享一份\n");
    printf("可执行文件    较大                较小\n");
    printf("内存占用      每个进程独立副本    共享内存映射\n");
    printf("更新方式      需重新编译          替换 .so 即可\n");
    printf("启动速度      较快                稍慢(需加载)\n");
    printf("部署复杂度    简单(单文件)        需确保 .so 可找到\n");
    printf("版本管理      无                  需要soname机制\n");
    printf("\n");
}

/* ========================================================================
 * 第六部分: 实用技巧与常见陷阱
 * ======================================================================== */

void demo_tips_and_pitfalls(void) {
    printf("===== 实用技巧与常见陷阱 =====\n\n");

    printf("陷阱1: 忘记运行 ranlib / ar s\n");
    printf("  没有符号索引的库链接速度极慢\n");
    printf("  解决: 始终使用 ar rcs (带s选项)\n\n");

    printf("陷阱2: 静态库中混用 C 和 C++\n");
    printf("  C++ 有名称修饰(name mangling)，链接时找不到符号\n");
    printf("  解决: 在头文件中使用 extern \"C\"\n");
    printf("    #ifdef __cplusplus\n");
    printf("    extern \"C\" {\n");
    printf("    #endif\n");
    printf("    int foo(void);\n");
    printf("    #ifdef __cplusplus\n");
    printf("    }\n");
    printf("    #endif\n\n");

    printf("陷阱3: 静态库中使用了其他库的符号\n");
    printf("  静态库不会记录自己的依赖关系\n");
    printf("  解决: 链接时需要显式列出所有依赖库\n");
    printf("  gcc main.c -lmylib -lm -lpthread\n\n");

    printf("技巧1: 使用 objdump 检查库内容\n");
    printf("  objdump -d libfoo.a     # 反汇编所有成员\n");
    printf("  objdump -t libfoo.a     # 查看符号表\n\n");

    printf("技巧2: 使用 -Wl,--whole-archive 强制链接整个库\n");
    printf("  gcc main.o -Wl,--whole-archive -lfoo -Wl,--no-whole-archive\n");
    printf("  即使没有直接引用，也会链接库中所有对象\n\n");

    printf("技巧3: 合并多个 .o 为单个静态库\n");
    printf("  ar rcs libcombined.a a.o b.o c.o\n\n");
}

int main(void) {
    printf("============================================\n");
    printf("   静态库深入剖析 - 符号解析与链接机制\n");
    printf("============================================\n\n");

    demo_symbol_types();
    demo_archive_format();
    demo_linking_order();
    demo_duplicate_symbols();
    demo_static_vs_dynamic();
    demo_tips_and_pitfalls();

    printf("============================================\n");
    printf("   演示结束\n");
    printf("============================================\n");
    return 0;
}
