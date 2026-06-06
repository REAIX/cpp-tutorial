/**
 * @file 01_example_gcc_options.c
 * @brief GCC常用编译选项示例
 * @description 对应文档: 21-gcc-g++
 *              演示 GCC 常用编译选项的效果，包括警告控制、调试信息、标准选择等
 *
 * 编译示例:
 *   基础编译:     gcc 01_example_gcc_options.c -o demo
 *   开启警告:     gcc -Wall -Wextra 01_example_gcc_options.c -o demo
 *   指定标准:     gcc -std=c17 01_example_gcc_options.c -o demo
 *   调试信息:     gcc -g 01_example_gcc_options.c -o demo
 *   全部组合:     gcc -Wall -Wextra -Wpedantic -std=c11 -g 01_example_gcc_options.c -o demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void demo_warning_options(void) {
    printf("===== 警告控制选项 =====\n\n");

    printf("-Wall: 开启常见警告\n");
    printf("  包含的警告类型:\n");
    printf("    -Wimplicit-function-declaration  隐式函数声明\n");
    printf("    -Wreturn-type                    返回值类型不匹配\n");
    printf("    -Wformat                         printf/scanf 格式字符串错误\n");
    printf("    -Wunused-variable                未使用的变量\n");
    printf("    -Wuninitialized                  使用未初始化的变量\n");
    printf("    -Wparentheses                    建议加括号\n\n");

    printf("-Wextra: 开启额外警告 (-Wall 不包含的)\n");
    printf("  包含的警告类型:\n");
    printf("    -Wmissing-field-initializers     结构体初始化不完整\n");
    printf("    -Wtype-limits                    类型范围限制\n");
    printf("    -Wempty-body                     空循环体/空if体\n");
    printf("    -Wunused-parameter               未使用的函数参数\n\n");

    printf("-Wpedantic: 严格遵循ISO C标准\n");
    printf("  禁止所有GNU扩展，确保代码可移植\n\n");

    printf("-Werror: 将所有警告视为错误\n");
    printf("  确保代码零警告，适合生产环境\n\n");

    printf("单独控制特定警告:\n");
    printf("  -Wno-unused-parameter     关闭某类警告\n");
    printf("  -Wno-missing-field-initializers\n");
    printf("  -Werror=return-type        将特定警告升为错误\n");
    printf("  -Wshadow                   变量遮蔽警告\n");
    printf("  -Wconversion               隐式类型转换警告\n\n");

    printf("推荐组合:\n");
    printf("  gcc -Wall -Wextra -Wpedantic -Wshadow -Wconversion \\\n");
    printf("      -Werror=return-type -std=c11 -g\n\n");
}

void demo_standard_options(void) {
    printf("===== C语言标准选项 =====\n\n");

    printf("-std= 选项指定C语言标准:\n\n");
    printf("  -std=c89 / -std=c90    ANSI C / ISO C90\n");
    printf("  -std=c99               ISO C99 (变长数组、//注释、混合声明)\n");
    printf("  -std=c11               ISO C11 (_Generic、_Static_assert、线程)\n");
    printf("  -std=c17               ISO C17 (C11的bug修复版)\n");
    printf("  -std=c23               ISO C23 (最新标准)\n\n");

    printf("  -std=gnu89 / gnu99 / gnu11 / gnu17\n");
    printf("    = 对应标准 + GNU扩展 (GCC默认)\n\n");

    printf("C99 新特性示例:\n");
    printf("  // 单行注释\n");
    printf("  for (int i = 0; i < n; i++) { }  // 循环内声明变量\n");
    printf("  int vla[n];                        // 变长数组 (VLA)\n\n");

    printf("C11 新特性示例:\n");
    printf("  _Static_assert(sizeof(int) == 4, \"int must be 4 bytes\");\n");
    printf("  _Generic(x, int: 1, float: 2, default: 0)\n\n");

    printf("查看编译器默认标准:\n");
    printf("  gcc -dM -E - < /dev/null | grep __STDC_VERSION__\n\n");
}

void demo_debug_options(void) {
    printf("===== 调试信息选项 =====\n\n");

    printf("-g: 生成调试信息\n");
    printf("  可以在GDB中设置断点、查看变量值、单步执行\n");
    printf("  不影响优化，可以和 -O 选项同时使用\n\n");

    printf("-g3: 生成最详细的调试信息\n");
    printf("  包含宏定义信息，可以在GDB中查看宏展开\n\n");

    printf("调试信息的格式:\n");
    printf("  -gdwarf-4    DWARF 4格式 (默认)\n");
    printf("  -gdwarf-5    DWARF 5格式 (更新)\n\n");

    printf("GDB 使用示例:\n");
    printf("  gcc -g -O0 program.c -o program\n");
    printf("  gdb ./program\n");
    printf("  (gdb) break main\n");
    printf("  (gdb) run\n");
    printf("  (gdb) next\n");
    printf("  (gdb) print variable\n");
    printf("  (gdb) continue\n\n");

    printf("注意: -O0 关闭优化，确保调试时代码执行顺序与源码一致\n\n");
}

void demo_preprocessor_options(void) {
    printf("===== 预处理器选项 =====\n\n");

    printf("-D<name>=<value>: 定义宏\n");
    printf("  gcc -DDEBUG=1 program.c       # 等同于 #define DEBUG 1\n");
    printf("  gcc -DVERSION=\\\"2.0\\\" program.c  # 定义字符串宏\n\n");

    printf("-I<path>: 添加头文件搜索路径\n");
    printf("  gcc -I./include -I../common program.c\n\n");

    printf("-E: 只运行预处理器\n");
    printf("  gcc -E program.c > program.i  # 查看预处理结果\n\n");

    printf("-dM: 输出所有预定义宏\n");
    printf("  gcc -dM -E - < /dev/null      # 查看编译器预定义的宏\n\n");

    printf("常用预定义宏:\n");
    printf("  __FILE__        当前文件名\n");
    printf("  __LINE__        当前行号\n");
    printf("  __DATE__        编译日期\n");
    printf("  __TIME__        编译时间\n");
    printf("  __func__        当前函数名 (C99)\n");
    printf("  __STDC_VERSION__ C标准版本号\n\n");

    printf("本文件信息:\n");
    printf("  文件: %s\n", __FILE__);
    printf("  行号: %d\n", __LINE__);
    printf("  函数: %s\n", __func__);
    printf("  日期: %s\n", __DATE__);
    printf("  时间: %s\n", __TIME__);
    printf("\n");
}

void demo_output_options(void) {
    printf("===== 输出控制选项 =====\n\n");

    printf("编译阶段控制:\n");
    printf("  -E              只预处理，输出到 stdout\n");
    printf("  -S              只编译到汇编，输出 .s 文件\n");
    printf("  -c              只编译不链接，输出 .o 文件\n");
    printf("  -o <file>       指定输出文件名\n\n");

    printf("示例:\n");
    printf("  gcc -E program.c -o program.i    # 预处理\n");
    printf("  gcc -S program.c -o program.s    # 生成汇编\n");
    printf("  gcc -c program.c -o program.o    # 生成目标文件\n");
    printf("  gcc program.o -o program          # 链接\n\n");

    printf("保存中间文件:\n");
    printf("  -save-temps     保存所有中间文件 (.i, .s, .o)\n");
    printf("  -save-temps=obj 保存中间文件，使用输出文件名前缀\n\n");

    printf("详细输出:\n");
    printf("  -v              显示编译器内部命令\n");
    printf("  -###            显示命令但不执行\n");
    printf("  -Wa,option      传递选项给汇编器\n");
    printf("  -Wl,option      传递选项给链接器\n\n");
}

int main(void) {
    printf("========== GCC 常用编译选项示例 ==========\n\n");

    demo_warning_options();
    demo_standard_options();
    demo_debug_options();
    demo_preprocessor_options();
    demo_output_options();

    printf("========== 程序结束 ==========\n");
    return 0;
}
