/** @file 01_example_hello_world.c
 *  @brief C语言第一个程序：Hello World
 *  @description 对应文档: 00-c-overview | 演示最基本的C程序结构、编译步骤
 *  编译命令: gcc -std=c17 01_example_hello_world.c -o 01_example_hello_world
 */

#include <stdio.h>

void demo_hello_world(void) {
    printf("Hello, World!\n");
}

void demo_basic_structure(void) {
    printf("=== C程序基本结构解析 ===\n\n");

    printf("一个C程序由以下部分组成:\n");
    printf("1. 预处理指令 (如 #include <stdio.h>)\n");
    printf("2. 全局声明 (全局变量、函数声明)\n");
    printf("3. main() 函数 —— 程序入口\n");
    printf("4. 其他自定义函数\n");

    printf("\nmain() 函数是程序的唯一入口点，操作系统从这里开始执行。\n");
    printf("标准写法: int main(void) 或 int main(int argc, char *argv[])\n");
}

void demo_compilation_steps(void) {
    printf("\n=== C程序编译步骤 ===\n\n");

    printf("源代码(.c)到可执行文件需要四个阶段:\n\n");

    printf("1. 预处理 (Preprocessing)\n");
    printf("   - 处理所有以 # 开头的指令\n");
    printf("   - 展开 #include (头文件内容插入)\n");
    printf("   - 展开 #define 宏替换\n");
    printf("   - 条件编译 #ifdef/#endif\n");
    printf("   - 命令: gcc -E hello.c -o hello.i\n\n");

    printf("2. 编译 (Compilation)\n");
    printf("   - 将预处理后的代码翻译为汇编语言\n");
    printf("   - 进行语法检查、类型检查\n");
    printf("   - 命令: gcc -S hello.i -o hello.s\n\n");

    printf("3. 汇编 (Assembly)\n");
    printf("   - 将汇编代码翻译为机器码(目标文件)\n");
    printf("   - 生成 .o 或 .obj 文件\n");
    printf("   - 命令: gcc -c hello.s -o hello.o\n\n");

    printf("4. 链接 (Linking)\n");
    printf("   - 将目标文件与库文件合并\n");
    printf("   - 解析外部符号引用\n");
    printf("   - 生成最终可执行文件\n");
    printf("   - 命令: gcc hello.o -o hello\n\n");

    printf("一键编译: gcc hello.c -o hello (自动完成以上四步)\n");
}

void demo_return_value(void) {
    printf("\n=== main函数的返回值 ===\n\n");

    printf("main 函数返回 int 类型:\n");
    printf("  return 0;  —— 表示程序正常结束\n");
    printf("  return 1;  —— 表示程序异常结束\n\n");

    printf("在 C99 标准中，如果 main 函数末尾没有 return 语句，\n");
    printf("编译器会自动添加 return 0;\n");
}

void demo_escape_sequences(void) {
    printf("\n=== 常用转义字符 ===\n\n");

    printf("转义字符    含义        示例输出\n");
    printf("--------    ----        --------\n");
    printf("  \\n        换行        光标移到下一行\n");
    printf("  \\t        水平制表    增加缩进\n");
    printf("  \\\\        反斜杠      输出 \\\n");
    printf("  \\\"        双引号      输出 \"\n");
    printf("  \\\'        单引号      输出 \'\n");
    printf("  \\0        空字符      字符串结束标志\n");
    printf("  \\r        回车        光标移到行首\n");
    printf("  \\b        退格        光标后退一格\n");
    printf("  \\a        响铃        系统发出提示音\n");
}

int main(void) {
    demo_hello_world();
    demo_basic_structure();
    demo_compilation_steps();
    demo_return_value();
    demo_escape_sequences();

    return 0;
}
