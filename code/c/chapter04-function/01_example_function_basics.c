/** @file 01_example_function_basics.c
 *  @brief 函数基础：声明、定义、参数、返回值
 *  @description 对应文档: 04-function | 演示函数的声明、定义、参数传递和返回值机制
 *  编译命令: gcc -std=c17 01_example_function_basics.c -o 01_example_function_basics
 */

#include <stdio.h>
#include <stdlib.h>

int add(int a, int b);
double average(int a, int b);
void print_line(const char *msg);
int max_of_three(int a, int b, int c);
int factorial(int n);

int add(int a, int b) {
    return a + b;
}

double average(int a, int b) {
    return (a + b) / 2.0;
}

void print_line(const char *msg) {
    printf("── %s ──\n", msg);
}

int max_of_three(int a, int b, int c) {
    int max = a;
    if (b > max) max = b;
    if (c > max) max = c;
    return max;
}

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

void demo_function_declaration(void) {
    printf("═══════════════════════════════════════\n");
    printf("  函数声明与定义\n");
    printf("═══════════════════════════════════════\n\n");

    printf("函数声明(原型): 告诉编译器函数的签名\n");
    printf("  int add(int a, int b);  // 参数名可省略\n");
    printf("  int add(int, int);      // 等价写法\n\n");

    printf("函数定义: 函数的实际实现\n");
    printf("  int add(int a, int b) {\n");
    printf("      return a + b;\n");
    printf("  }\n\n");

    printf("为什么要先声明后使用?\n");
    printf("  编译器从上到下处理代码，遇到未声明的函数会报错或隐式假设返回int\n");
    printf("  头文件的作用就是提供函数声明\n");

    printf("\n示例调用:\n");
    printf("  add(3, 5) = %d\n", add(3, 5));
    printf("  average(3, 7) = %.1f\n", average(3, 7));
}

void demo_parameters(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  函数参数\n");
    printf("═══════════════════════════════════════\n\n");

    printf("C语言函数参数是「值传递」(Pass by Value):\n");
    printf("  调用时，实参的值被复制到形参\n");
    printf("  函数内修改形参不影响实参\n\n");

    void try_modify(int x) {
        x = 100;
        printf("  函数内 x = %d\n", x);
    }

    int val = 42;
    printf("调用前 val = %d\n", val);
    try_modify(val);
    printf("调用后 val = %d (未被修改!)\n", val);

    printf("\n要在函数内修改调用者的变量，需传指针:\n");
    void modify_via_pointer(int *p) {
        *p = 100;
        printf("  函数内 *p = %d\n", *p);
    }

    val = 42;
    printf("调用前 val = %d\n", val);
    modify_via_pointer(&val);
    printf("调用后 val = %d (已被修改!)\n", val);

    printf("\n无参数函数:\n");
    printf("  void func(void);  // 明确表示无参数(C标准推荐)\n");
    printf("  void func();      // C中表示参数未指定(不推荐!)\n");
    printf("  注意: C++中 func() 等价于 func(void)\n");
}

void demo_return_value(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  函数返回值\n");
    printf("═══════════════════════════════════════\n\n");

    printf("返回类型可以是: void、int、double、指针、结构体等\n\n");

    printf("1. 返回基本类型:\n");
    printf("  max_of_three(3, 7, 5) = %d\n", max_of_three(3, 7, 5));

    printf("\n2. 返回指针:\n");
    int global_val = 42;
    int *get_pointer(void) {
        return &global_val;
    }
    int *p = get_pointer();
    printf("  *get_pointer() = %d\n", *p);

    printf("\n3. void函数不返回值:\n");
    print_line("这是一个void函数");

    printf("\n⚠️ 返回值陷阱:\n");
    printf("  ✗ 返回局部变量的指针(悬空指针!)\n");
    printf("    int* bad_func() { int x=10; return &x; } // x在函数返回后销毁\n");
    printf("  ✗ 忘记return(非void函数)\n");
    printf("  ✓ 返回值类型与函数声明不一致 → 隐式转换\n");
}

void demo_function_call_mechanism(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  函数调用机制\n");
    printf("═══════════════════════════════════════\n\n");

    printf("函数调用过程:\n");
    printf("  1. 计算实参的值\n");
    printf("  2. 将实参值压入栈(或通过寄存器传递)\n");
    printf("  3. 保存返回地址\n");
    printf("  4. 跳转到函数代码执行\n");
    printf("  5. 函数执行完毕，将返回值放入指定位置\n");
    printf("  6. 恢复栈，跳回返回地址\n\n");

    printf("函数调用的开销:\n");
    printf("  - 参数传递(压栈/寄存器)\n");
    printf("  - 栈帧创建和销毁\n");
    printf("  - 控制流跳转(可能影响流水线)\n\n");

    printf("inline关键字(C99):\n");
    printf("  建议编译器内联展开函数(消除调用开销)\n");
    printf("  inline int square(int x) { return x*x; }\n");
    printf("  注意: inline只是建议，编译器可能忽略\n");
}

void demo_multi_file(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  多文件编程中的函数\n");
    printf("═══════════════════════════════════════\n\n");

    printf("典型项目结构:\n");
    printf("  mylib.h    —— 头文件(函数声明、类型定义)\n");
    printf("  mylib.c    —— 源文件(函数实现)\n");
    printf("  main.c     —— 主程序(调用函数)\n\n");

    printf("mylib.h 内容:\n");
    printf("  #ifndef MYLIB_H\n");
    printf("  #define MYLIB_H\n");
    printf("  int add(int a, int b);  // 函数声明\n");
    printf("  #endif\n\n");

    printf("mylib.c 内容:\n");
    printf("  #include \"mylib.h\"\n");
    printf("  int add(int a, int b) { return a + b; }  // 函数定义\n\n");

    printf("main.c 内容:\n");
    printf("  #include \"mylib.h\"\n");
    printf("  int main(void) { return add(1, 2); }\n\n");

    printf("编译: gcc -std=c17 main.c mylib.c -o program\n");
}

int main(void) {
    demo_function_declaration();
    demo_parameters();
    demo_return_value();
    demo_function_call_mechanism();
    demo_multi_file();

    return 0;
}
