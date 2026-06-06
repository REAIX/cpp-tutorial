/** @file 01_deep_dive_compilation_process.c
 *  @brief 深入理解C语言编译过程
 *  @description 对应文档: 00-c-overview | 深入剖析预处理、编译、汇编、链接四个阶段的细节
 *  编译命令: gcc -std=c17 01_deep_dive_compilation_process.c -o 01_deep_dive_compilation_process
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SQUARE(x) ((x) * (x))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define DEBUG_PRINT(fmt, val) printf("  [DEBUG] " fmt "\n", val)
#define STRINGIFY(x) #x
#define CONCAT(a, b) a##b

void demo_preprocessing_detail(void) {
    printf("═══════════════════════════════════════\n");
    printf("  第一阶段：预处理 (Preprocessing)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("预处理器执行以下操作:\n\n");

    printf("1. 宏展开:\n");
    printf("   SQUARE(5) 展开为: ((5) * (5)) = %d\n", SQUARE(5));
    printf("   MAX(3, 7) 展开为: ((3) > (7) ? (3) : (7)) = %d\n", MAX(3, 7));
    printf("   STRINGIFY(hello) 展开为: %s\n", STRINGIFY(hello));

    int CONCAT(my, Var) = 100;
    printf("   CONCAT(my, Var) 生成标识符: myVar = %d\n", myVar);

    printf("\n2. 文件包含:\n");
    printf("   #include <stdio.h> 将整个 stdio.h 的内容插入到源文件中\n");
    printf("   典型的 stdio.h 包含数百行声明\n");

    printf("\n3. 条件编译:\n");
    printf("   #ifdef / #ifndef / #if / #elif / #else / #endif\n");
    printf("   用于平台适配、调试开关、版本控制\n");

    printf("\n4. 删除所有注释:\n");
    printf("   注释在预处理阶段就被移除，不影响运行时性能\n");

    printf("\n查看预处理输出: gcc -E source.c -o source.i\n");
    printf("预处理后的文件通常比源文件大很多(因为头文件展开)\n");
}

void demo_compilation_detail(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  第二阶段：编译 (Compilation)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("编译器将预处理后的C代码翻译为汇编语言，主要步骤:\n\n");

    printf("1. 词法分析 (Lexical Analysis)\n");
    printf("   将源代码字符流分解为记号(token)\n");
    printf("   int x = 42; → [int] [x] [=] [42] [;]\n\n");

    printf("2. 语法分析 (Syntax Analysis)\n");
    printf("   根据语法规则构建语法树(AST)\n");
    printf("   检查括号匹配、语句结构等\n\n");

    printf("3. 语义分析 (Semantic Analysis)\n");
    printf("   类型检查、作用域解析\n");
    printf("   检测未声明变量、类型不匹配等错误\n\n");

    printf("4. 中间代码生成\n");
    printf("   生成与机器无关的中间表示(IR)\n\n");

    printf("5. 代码优化\n");
    printf("   常量折叠: 3+5 → 8\n");
    printf("   死代码消除: 删除不可达代码\n");
    printf("   循环优化: 循环不变量外提等\n\n");

    printf("6. 目标代码生成\n");
    printf("   生成特定架构的汇编代码\n");

    printf("\n查看汇编输出: gcc -S source.c -o source.s\n");
    printf("查看优化后的汇编: gcc -O2 -S source.c -o source.s\n");
}

void demo_assembly_detail(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  第三阶段：汇编 (Assembly)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("汇编器将汇编代码翻译为机器码:\n\n");

    printf("  汇编指令              机器码(示意)\n");
    printf("  mov eax, 42     →    B8 2A 00 00 00\n");
    printf("  add eax, ebx    →    01 D8\n");
    printf("  ret             →    C3\n\n");

    printf("目标文件(.o)的结构:\n");
    printf("  .text    —— 代码段(机器指令)\n");
    printf("  .data    —— 已初始化数据段\n");
    printf("  .bss     —— 未初始化数据段\n");
    printf("  符号表   —— 函数和变量的地址信息\n");
    printf("  重定位表 —— 需要链接器填充的地址\n\n");

    printf("只编译不链接: gcc -c source.c -o source.o\n");
    printf("查看目标文件: objdump -d source.o (Linux)\n");
    printf("              dumpbin /disasm source.obj (MSVC)\n");
}

void demo_linking_detail(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  第四阶段：链接 (Linking)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("链接器完成以下工作:\n\n");

    printf("1. 符号解析 (Symbol Resolution)\n");
    printf("   将每个符号引用与对应的符号定义关联\n");
    printf("   例如: 代码中调用 printf，链接器找到 libc 中的 printf 实现\n\n");

    printf("2. 地址重定位 (Relocation)\n");
    printf("   将相对地址修改为绝对地址\n");
    printf("   合并所有目标文件的段(section)\n\n");

    printf("3. 静态链接 vs 动态链接:\n");
    printf("   静态链接: 库代码复制到可执行文件中\n");
    printf("     优点: 独立运行  缺点: 文件大，更新需重新编译\n");
    printf("   动态链接: 运行时加载共享库(.so/.dll)\n");
    printf("     优点: 文件小，库可独立更新  缺点: 依赖环境\n\n");

    printf("常见链接错误:\n");
    printf("   undefined reference to 'xxx'    —— 符号未定义\n");
    printf("   multiple definition of 'xxx'    —— 符号重复定义\n");
}

void demo_compiler_flags(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  常用GCC编译选项\n");
    printf("═══════════════════════════════════════\n\n");

    printf("标准与警告:\n");
    printf("  -std=c11          指定C11标准\n");
    printf("  -Wall             开启常见警告\n");
    printf("  -Wextra           开启额外警告\n");
    printf("  -Werror           将警告视为错误\n");
    printf("  -pedantic         严格遵循标准\n\n");

    printf("优化级别:\n");
    printf("  -O0               不优化(默认，便于调试)\n");
    printf("  -O1               基本优化\n");
    printf("  -O2               推荐的发布优化级别\n");
    printf("  -O3               激进优化(可能增大体积)\n");
    printf("  -Os               优化代码大小\n\n");

    printf("调试与分析:\n");
    printf("  -g                生成调试信息\n");
    printf("  -g3               最详细的调试信息\n");
    printf("  -pg               生成gprof性能分析代码\n");
    printf("  -fsanitize=address  地址消毒器(检测内存错误)\n\n");

    printf("输出控制:\n");
    printf("  -E                只预处理\n");
    printf("  -S                只编译到汇编\n");
    printf("  -c                只编译不链接\n");
    printf("  -save-temps       保留所有中间文件\n");
}

void demo_build_pipeline(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  完整编译流程演示\n");
    printf("═══════════════════════════════════════\n\n");

    printf("假设有一个项目包含 main.c 和 utils.c:\n\n");
    printf("  步骤1: gcc -E main.c -o main.i        (预处理)\n");
    printf("  步骤2: gcc -S main.i -o main.s        (编译)\n");
    printf("  步骤3: gcc -c main.s -o main.o        (汇编)\n");
    printf("  步骤4: gcc -c utils.c -o utils.o      (编译utils)\n");
    printf("  步骤5: gcc main.o utils.o -o program  (链接)\n\n");

    printf("或者一步到位:\n");
    printf("  gcc -Wall -std=c11 main.c utils.c -o program\n\n");

    printf("使用Makefile自动化构建:\n");
    printf("  CC = gcc\n");
    printf("  CFLAGS = -Wall -Wextra -std=c11 -O2\n");
    printf("  program: main.o utils.o\n");
    printf("      $(CC) $(CFLAGS) $^ -o $@\n");
    printf("  %%o: %%c\n");
    printf("      $(CC) $(CFLAGS) -c $< -o $@\n");
}

int main(void) {
    demo_preprocessing_detail();
    demo_compilation_detail();
    demo_assembly_detail();
    demo_linking_detail();
    demo_compiler_flags();
    demo_build_pipeline();

    return 0;
}
