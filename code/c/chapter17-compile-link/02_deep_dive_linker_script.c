/**
 * @file 02_deep_dive_linker_script.c
 * @brief 链接器脚本与内存布局
 * @description 对应文档: 17-编译与链接 - 链接器脚本基础、段映射、内存布局、启动代码概念
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int g_initialized = 42;
int g_uninitialized;
const int g_readonly = 100;
static int g_static_init = 200;
static int g_static_uninit;

void demo_memory_layout(void) {
    printf("=== 程序内存布局 ===\n");
    printf("  高地址\n");
    printf("  ┌──────────────┐\n");
    printf("  │   栈(stack)   │ ← 局部变量, 函数调用帧\n");
    printf("  │      ↓↓      │   向低地址增长\n");
    printf("  │              │\n");
    printf("  │      ↑↑      │\n");
    printf("  │   堆(heap)    │ ← malloc分配, 向高地址增长\n");
    printf("  ├──────────────┤\n");
    printf("  │   .bss段     │ ← 未初始化全局变量(g_uninitialized等)\n");
    printf("  ├──────────────┤\n");
    printf("  │   .data段    │ ← 已初始化全局变量(g_initialized等)\n");
    printf("  ├──────────────┤\n");
    printf("  │   .rodata段  │ ← 只读数据(g_readonly, 字符串常量)\n");
    printf("  ├──────────────┤\n");
    printf("  │   .text段    │ ← 代码(函数指令)\n");
    printf("  └──────────────┘\n");
    printf("  低地址\n\n");

    int local_var = 10;
    static int local_static = 30;
    int *heap_var = malloc(sizeof(int));
    if (heap_var) *heap_var = 40;

    printf("  变量地址验证:\n");
    printf("    局部变量(栈):   %p\n", (void *)&local_var);
    printf("    堆变量:         %p\n", (void *)heap_var);
    printf("    全局已初始化:   %p (.data)\n", (void *)&g_initialized);
    printf("    全局未初始化:   %p (.bss)\n", (void *)&g_uninitialized);
    printf("    全局只读:       %p (.rodata)\n", (void *)&g_readonly);
    printf("    静态已初始化:   %p (.data)\n", (void *)&g_static_init);
    printf("    静态未初始化:   %p (.bss)\n", (void *)&g_static_uninit);
    printf("    局部静态:       %p (.data)\n", (void *)&local_static);
    printf("    函数地址(.text): %p\n", (void *)demo_memory_layout);

    free(heap_var);
    printf("\n");
}

void demo_section_mapping(void) {
    printf("=== ELF段(Section)映射 ===\n");
    printf("  查看段信息:\n");
    printf("    readelf -S program     (列出所有段)\n");
    printf("    objdump -h program     (段头信息)\n");
    printf("    size program           (段大小摘要)\n\n");

    printf("  主要段:\n");
    printf("    .text     : 机器指令(代码)\n");
    printf("    .rodata   : 只读数据(常量, 字符串)\n");
    printf("    .data     : 已初始化的可读写全局变量\n");
    printf("    .bss      : 未初始化的全局变量(不占文件空间)\n");
    printf("    .symtab   : 符号表\n");
    printf("    .strtab   : 字符串表\n");
    printf("    .rel.text : 代码重定位信息\n");
    printf("    .rel.data : 数据重定位信息\n\n");

    printf("  .bss段的特殊性:\n");
    printf("    - 不占用可执行文件空间(只有大小信息)\n");
    printf("    - 程序加载时由操作系统清零\n");
    printf("    - 适合大块未初始化数据(节省磁盘空间)\n\n");
}

void demo_linker_script_basics(void) {
    printf("=== 链接器脚本(Linker Script)基础 ===\n");
    printf("  链接器脚本控制:\n");
    printf("    - 各段在内存中的排列顺序和位置\n");
    printf("    - 输出段由哪些输入段合并\n");
    printf("    - 符号的特殊地址\n\n");

    printf("  简单链接器脚本示例:\n");
    printf("    ENTRY(_start)\n\n");
    printf("    SECTIONS {\n");
    printf("        . = 0x10000;          /* 起始地址 */\n");
    printf("        .text : { *(.text) }  /* 所有.text段合并 */\n");
    printf("        .rodata : { *(.rodata) }\n");
    printf("        . = 0x20000;          /* 数据段起始 */\n");
    printf("        .data : { *(.data) }\n");
    printf("        .bss : { *(.bss) }\n");
    printf("    }\n\n");

    printf("  使用自定义链接脚本:\n");
    printf("    gcc main.o -T my_script.ld -o program\n\n");
}

void demo_startup_code(void) {
    printf("=== 启动代码(Startup Code)概念 ===\n");
    printf("  程序并非从main()开始执行!\n\n");
    printf("  执行流程:\n");
    printf("    1. OS加载程序到内存\n");
    printf("    2. 跳转到入口点(_start或_start)\n");
    printf("    3. _start (crt0.o / crt1.o):\n");
    printf("       - 设置栈指针\n");
    printf("       - 清零.bss段\n");
    printf("       - 初始化标准I/O(stdin/stdout/stderr)\n");
    printf("       - 收集命令行参数(argc/argv/envp)\n");
    printf("       - 调用__libc_start_main\n");
    printf("    4. __libc_start_main:\n");
    printf("       - 调用全局构造函数(C++)\n");
    printf("       - 调用main(argc, argv, envp)\n");
    printf("       - 调用exit(main返回值)\n\n");

    printf("  裸机编程(嵌入式):\n");
    printf("    - 没有OS, 没有crt运行时\n");
    printf("    - 需要自己写Reset_Handler:\n");
    printf("      1. 初始化栈指针(从链接脚本获取)\n");
    printf("      2. 复制.data段从Flash到RAM\n");
    printf("      3. 清零.bss段\n");
    printf("      4. 调用SystemInit() (时钟配置等)\n");
    printf("      5. 调用main()\n\n");
}

void demo_relocation(void) {
    printf("=== 重定位(Relocation) ===\n");
    printf("  编译器生成的目标文件中, 地址是相对的\n");
    printf("  链接器需要将这些相对地址修正为最终地址\n\n");

    printf("  重定位类型:\n");
    printf("    R_X86_64_PC32 : 32位PC相对地址(函数调用)\n");
    printf("    R_X86_64_32   : 32位绝对地址\n");
    printf("    R_X86_64_64   : 64位绝对地址\n");
    printf("    R_X86_64_GOTPC: GOT表的PC相对偏移\n\n");

    printf("  查看重定位信息:\n");
    printf("    objdump -r program.o\n");
    printf("    readelf -r program.o\n\n");
}

int main(void) {
    printf("========== 链接器脚本与内存布局 ==========\n\n");

    demo_memory_layout();
    demo_section_mapping();
    demo_linker_script_basics();
    demo_startup_code();
    demo_relocation();

    printf("========== 所有演示完成 ==========\n");
    return 0;
}
