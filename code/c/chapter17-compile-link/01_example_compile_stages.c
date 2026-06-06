/**
 * @file 01_example_compile_stages.c
 * @brief 编译四阶段演示
 * @description 对应文档: 17-编译与链接 - 预处理、编译、汇编、链接
 */
#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 100
#define SQUARE(x) ((x) * (x))
#define VERSION "1.0"

#ifdef DEBUG
#define LOG(msg) printf("[DEBUG] %s\n", msg)
#else
#define LOG(msg) ((void)0)
#endif

#pragma message("编译阶段: 此pragma在预处理时输出")

static int add(int a, int b) {
    return a + b;
}

void demo_preprocessing(void) {
    printf("=== 阶段1: 预处理(Preprocessing) ===\n");
    printf("  命令: gcc -E 01_example_compile_stages.c -o stage1.i\n\n");
    printf("  预处理完成的工作:\n");
    printf("    1. 展开#include (将头文件内容插入)\n");
    printf("    2. 展开宏定义: MAX_SIZE -> 100, SQUARE(x) -> ((x)*(x))\n");
    printf("    3. 处理条件编译: #ifdef, #ifndef, #if, #else, #endif\n");
    printf("    4. 删除所有注释\n");
    printf("    5. 处理#pragma指令\n");
    printf("    6. 处理#line指令\n\n");
    printf("  当前宏展开结果:\n");
    printf("    MAX_SIZE = %d\n", MAX_SIZE);
    printf("    SQUARE(5) = %d\n", SQUARE(5));
    printf("    VERSION = \"%s\"\n", VERSION);
    LOG("调试信息(仅在DEBUG模式下输出)");
    printf("\n");
}

void demo_compilation(void) {
    printf("=== 阶段2: 编译(Compilation) ===\n");
    printf("  命令: gcc -S stage1.i -o stage2.s\n\n");
    printf("  编译完成的工作:\n");
    printf("    1. 词法分析: 源代码 -> 记号流(tokens)\n");
    printf("    2. 语法分析: 记号流 -> 语法树(AST)\n");
    printf("    3. 语义分析: 类型检查、作用域解析\n");
    printf("    4. 中间代码生成: AST -> 中间表示(IR)\n");
    printf("    5. 优化: 常量折叠、死代码消除、循环优化\n");
    printf("    6. 目标代码生成: IR -> 汇编代码\n\n");
    printf("  生成的汇编示例(x86-64):\n");
    printf("    add:\n");
    printf("        push rbp\n");
    printf("        mov rbp, rsp\n");
    printf("        mov DWORD PTR [rbp-4], edi\n");
    printf("        mov DWORD PTR [rbp-8], esi\n");
    printf("        mov edx, DWORD PTR [rbp-4]\n");
    printf("        mov eax, DWORD PTR [rbp-8]\n");
    printf("        add eax, edx\n");
    printf("        pop rbp\n");
    printf("        ret\n\n");
}

void demo_assembly(void) {
    printf("=== 阶段3: 汇编(Assembly) ===\n");
    printf("  命令: gcc -c stage2.s -o stage3.o\n");
    printf("  或:   as stage2.s -o stage3.o\n\n");
    printf("  汇编完成的工作:\n");
    printf("    1. 将汇编指令翻译为机器码\n");
    printf("    2. 生成目标文件(.o/.obj)\n");
    printf("    3. 生成重定位表(未解析的符号地址)\n");
    printf("    4. 生成符号表(函数名、全局变量等)\n\n");
    printf("  目标文件结构(ELF):\n");
    printf("    .text    : 代码段(机器指令)\n");
    printf("    .data    : 已初始化数据段\n");
    printf("    .bss     : 未初始化数据段\n");
    printf("    .symtab  : 符号表\n");
    printf("    .rel.text: 代码重定位表\n");
    printf("    .rel.data: 数据重定位表\n\n");
}

void demo_linking(void) {
    printf("=== 阶段4: 链接(Linking) ===\n");
    printf("  命令: gcc stage3.o -o program\n\n");
    printf("  链接完成的工作:\n");
    printf("    1. 符号解析: 将引用绑定到定义\n");
    printf("       - 扫描所有目标文件的符号表\n");
    printf("       - 匹配未定义符号和已定义符号\n");
    printf("    2. 重定位: 将相对地址转为绝对地址\n");
    printf("       - 合并所有目标文件的段\n");
    printf("       - 修正代码中的地址引用\n");
    printf("    3. 生成可执行文件\n\n");
    printf("  链接类型:\n");
    printf("    静态链接: 将库代码直接嵌入可执行文件\n");
    printf("      gcc main.o -static -o program\n");
    printf("    动态链接: 运行时加载共享库\n");
    printf("      gcc main.o -o program  (默认动态链接)\n\n");
}

void demo_one_step_compile(void) {
    printf("=== 一步编译(四阶段合一) ===\n");
    printf("  gcc 01_example_compile_stages.c -o program\n");
    printf("  等价于依次执行:\n");
    printf("    gcc -E xxx.c -o xxx.i    (预处理)\n");
    printf("    gcc -S xxx.i -o xxx.s    (编译)\n");
    printf("    gcc -c xxx.s -o xxx.o    (汇编)\n");
    printf("    gcc xxx.o -o program     (链接)\n\n");
    printf("  常用编译选项:\n");
    printf("    -E          只预处理\n");
    printf("    -S          只编译(到汇编)\n");
    printf("    -c          只编译+汇编(到目标文件)\n");
    printf("    -save-temps 保留所有中间文件\n");
    printf("    -v          显示详细编译过程\n\n");
}

int main(void) {
    printf("========== 编译四阶段演示 ==========\n\n");

    demo_preprocessing();
    demo_compilation();
    demo_assembly();
    demo_linking();
    demo_one_step_compile();

    printf("  add(3, 4) = %d (确保函数被使用)\n", add(3, 4));

    printf("\n========== 所有演示完成 ==========\n");
    return 0;
}
