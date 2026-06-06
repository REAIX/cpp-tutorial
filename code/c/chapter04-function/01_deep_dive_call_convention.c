/** @file 01_deep_dive_call_convention.c
 *  @brief 深入理解调用约定：栈帧、参数传递、返回值机制
 *  @description 对应文档: 04-function | 深入剖析函数调用的底层机制、栈帧结构、调用约定
 *  编译命令: gcc -std=c17 01_deep_dive_call_convention.c -o 01_deep_dive_call_convention
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void demo_stack_frame(void) {
    printf("═══════════════════════════════════════\n");
    printf("  栈帧(Stack Frame)结构\n");
    printf("═══════════════════════════════════════\n\n");

    printf("每次函数调用都会在栈上创建一个栈帧:\n\n");

    printf("  高地址\n");
    printf("  ┌──────────────────┐\n");
    printf("  │   调用者的栈帧    │\n");
    printf("  ├──────────────────┤\n");
    printf("  │   参数n          │ ← 最后一个参数先压栈\n");
    printf("  │   参数2          │\n");
    printf("  │   参数1          │\n");
    printf("  │   返回地址        │ ← call指令自动压入\n");
    printf("  ├──────────────────┤\n");
    printf("  │   旧的EBP/RBP    │ ← 保存调用者的帧指针\n");
    printf("  │   局部变量1      │\n");
    printf("  │   局部变量2      │\n");
    printf("  │   临时空间        │\n");
    printf("  └──────────────────┘\n");
    printf("  低地址 (栈增长方向)\n\n");

    printf("观察局部变量的地址(验证栈增长方向):\n");
    void inner(int param) {
        int local1 = 1;
        int local2 = 2;
        printf("  参数地址:     %p\n", (void *)&param);
        printf("  局部变量1地址: %p\n", (void *)&local1);
        printf("  局部变量2地址: %p\n", (void *)&local2);
        printf("  (地址递减 → 栈向低地址增长)\n");
    }
    inner(42);
}

void demo_parameter_passing(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  参数传递机制\n");
    printf("═══════════════════════════════════════\n\n");

    printf("C语言只有「值传递」:\n");
    printf("  实参的值被复制一份传给形参\n");
    printf("  形参和实参是独立的变量\n\n");

    printf("1. 基本类型传值:\n");
    void modify_int(int x) {
        x = 999;
        (void)x;
    }
    int val = 10;
    modify_int(val);
    printf("  修改后 val = %d (不变，因为传的是副本)\n", val);

    printf("\n2. 通过指针模拟「引用传递」:\n");
    void modify_via_ptr(int *x) {
        *x = 999;
    }
    modify_via_ptr(&val);
    printf("  通过指针修改后 val = %d (改变了!)\n", val);

    printf("\n3. 结构体传值(整个结构体被复制):\n");
    struct Point { int x; int y; };
    void modify_point(struct Point p) {
        p.x = 999;
        (void)p;
    }
    struct Point pt = {1, 2};
    modify_point(pt);
    printf("  修改后 pt.x = %d (不变，整个结构体被复制)\n", pt.x);

    printf("\n4. 结构体传指针(避免复制开销):\n");
    void modify_point_ptr(struct Point *p) {
        p->x = 999;
    }
    modify_point_ptr(&pt);
    printf("  通过指针修改后 pt.x = %d (改变了)\n", pt.x);

    printf("\n举一反三 —— 何时传指针:\n");
    printf("  ✓ 需要修改调用者的变量\n");
    printf("  ✓ 结构体较大时(避免复制开销)\n");
    printf("  ✓ 数组(数组自动退化为指针)\n");
    printf("  ✓ 可选参数(传NULL表示不使用)\n");
}

void demo_calling_convention(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  调用约定(Calling Convention)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("调用约定规定了:\n");
    printf("  1. 参数如何传递(栈/寄存器)\n");
    printf("  2. 返回值如何传递\n");
    printf("  3. 谁负责清理栈(调用者/被调用者)\n");
    printf("  4. 哪些寄存器需要保存\n\n");

    printf("x86 (32位) 常见调用约定:\n");
    printf("  cdecl:     参数从右到左压栈，调用者清理栈 (C默认)\n");
    printf("  stdcall:   参数从右到左压栈，被调用者清理栈 (WinAPI)\n");
    printf("  fastcall:  前两个参数通过寄存器传递\n\n");

    printf("x86-64 常见调用约定:\n");
    printf("  System V AMD64 (Linux/macOS):\n");
    printf("    前6个整数参数: RDI, RSI, RDX, RCX, R8, R9\n");
    printf("    前8个浮点参数: XMM0-XMM7\n");
    printf("    返回值: RAX (整数), XMM0 (浮点)\n\n");
    printf("  Microsoft x64 (Windows):\n");
    printf("    前4个参数: RCX, RDX, R8, R9 (整数)\n");
    printf("    前4个浮点: XMM0-XMM3\n");
    printf("    返回值: RAX (整数), XMM0 (浮点)\n\n");

    printf("参数求值顺序:\n");
    printf("  ⚠️ C标准未规定函数参数的求值顺序!\n");
    printf("  printf(\"%%d %%d\", f(), g()); // f和g的调用顺序未定义\n");
}

void demo_return_value_mechanism(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  返回值机制\n");
    printf("═══════════════════════════════════════\n\n");

    printf("1. 小返回值(≤寄存器大小):\n");
    printf("   通过寄存器返回(RAX/EAX)\n");
    printf("   int func() { return 42; }  // 42放入EAX寄存器\n\n");

    printf("2. 大返回值(结构体):\n");
    printf("   调用者分配空间，通过隐藏指针参数传递\n");
    printf("   实际等价于: void func(struct BigStruct *hidden_ret)\n\n");

    printf("3. 返回局部变量的值 vs 指针:\n");
    int safe_return(void) {
        int local = 42;
        return local;
    }
    printf("   返回值: %d (安全! 返回的是值的副本)\n", safe_return());

    printf("\n   ⚠️ 返回局部变量的指针是未定义行为:\n");
    printf("   int* dangerous(void) {\n");
    printf("       int local = 42;\n");
    printf("       return &local;  // 局部变量在函数返回后销毁!\n");
    printf("   }\n\n");

    printf("4. 返回结构体(C99+):\n");
    struct Pair { int first; int second; };
    struct Pair make_pair(int a, int b) {
        struct Pair p = {a, b};
        return p;
    }
    struct Pair result = make_pair(10, 20);
    printf("   make_pair(10, 20) = (%d, %d)\n", result.first, result.second);
}

void demo_stack_overflow(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  栈溢出(Stack Overflow)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("栈溢出原因:\n\n");

    printf("1. 无限递归:\n");
    printf("   void recurse() { recurse(); }  // 没有终止条件\n\n");

    printf("2. 过大的局部变量:\n");
    printf("   void func() {\n");
    printf("       int huge[1000000];  // 4MB栈空间!\n");
    printf("   }\n\n");

    printf("3. 栈默认大小:\n");
    printf("   Linux: 通常8MB (ulimit -s 查看)\n");
    printf("   Windows: 通常1MB (链接器可调整)\n");
    printf("   嵌入式: 可能只有几KB\n\n");

    printf("防止栈溢出:\n");
    printf("  ✓ 限制递归深度\n");
    printf("  ✓ 大数组用malloc分配在堆上\n");
    printf("  ✓ 尾递归优化(或改写为迭代)\n");
    printf("  ✓ 编译器选项: -fstack-protector (栈保护)\n");
}

int main(void) {
    demo_stack_frame();
    demo_parameter_passing();
    demo_calling_convention();
    demo_return_value_mechanism();
    demo_stack_overflow();

    return 0;
}
