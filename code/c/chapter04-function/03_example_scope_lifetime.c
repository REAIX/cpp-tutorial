/** @file 03_example_scope_lifetime.c
 *  @brief 作用域与生命周期：局部/全局/静态变量、存储类(auto, register, extern, static)
 *  @description 对应文档: 04-function | 演示变量的作用域规则、生命周期和存储类说明符
 *  编译命令: gcc -std=c17 03_example_scope_lifetime.c -o 03_example_scope_lifetime
 */

#include <stdio.h>
#include <stdlib.h>

int global_var = 100;
static int file_static_var = 200;
const int global_const = 300;

void demo_scope_rules(void) {
    printf("═══════════════════════════════════════\n");
    printf("  变量作用域(Scope)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("1. 块作用域(Block Scope):\n");
    {
        int block_var = 10;
        printf("  块内: block_var = %d\n", block_var);
    }
    printf("  块外: block_var 不可访问(已超出作用域)\n");

    printf("\n2. 内层作用域遮蔽外层:\n");
    int x = 10;
    printf("  外层 x = %d\n", x);
    {
        int x = 20;
        printf("  内层 x = %d (遮蔽了外层x)\n", x);
    }
    printf("  外层 x = %d (恢复原值)\n", x);

    printf("\n3. for循环中的作用域(C99+):\n");
    for (int i = 0; i < 3; i++) {
        printf("  循环内 i = %d\n", i);
    }
    printf("  循环外 i 不可访问(C99+作用域规则)\n");

    printf("\n4. 函数作用域:\n");
    printf("  只有goto标签具有函数作用域(整个函数内可见)\n");

    printf("\n5. 文件作用域(File Scope):\n");
    printf("  全局变量和函数声明具有文件作用域\n");
    printf("  global_var = %d (文件作用域)\n", global_var);
    printf("  file_static_var = %d (文件作用域，static限制链接)\n", file_static_var);
}

void demo_lifetime(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  变量生命周期(Lifetime)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("1. 自动存储期(Automatic Storage Duration):\n");
    printf("   局部变量: 从进入块到离开块\n");
    printf("   每次进入块时创建，离开时销毁\n\n");

    void auto_demo(void) {
        int count = 0;
        count++;
        printf("  auto: count = %d (每次调用都重新创建)\n", count);
    }
    auto_demo();
    auto_demo();
    auto_demo();

    printf("\n2. 静态存储期(Static Storage Duration):\n");
    printf("   全局变量和static局部变量: 整个程序运行期间\n\n");

    void static_demo(void) {
        static int count = 0;
        count++;
        printf("  static: count = %d (保持上次的值)\n", count);
    }
    static_demo();
    static_demo();
    static_demo();

    printf("\n3. 动态存储期(Dynamic Storage Duration):\n");
    printf("   malloc分配的内存: 从malloc到free\n");
    int *dynamic = malloc(sizeof(int));
    *dynamic = 42;
    printf("  动态分配: *dynamic = %d\n", *dynamic);
    free(dynamic);
    printf("  free后: 内存已释放，指针变为悬空指针\n");

    printf("\n4. 线程存储期(Thread Storage Duration):\n");
    printf("   _Thread_local 变量: 每个线程一份(C11)\n");
}

void demo_storage_classes(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  存储类说明符(Storage Class Specifiers)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("1. auto (自动存储期，默认)\n");
    printf("   auto int x = 10;  // 等价于 int x = 10;\n");
    printf("   局部变量默认就是auto，几乎从不显式使用\n");
    printf("   注意: C++11中auto含义完全不同(类型推导)\n\n");

    printf("2. register (建议寄存器存储)\n");
    printf("   register int i;\n");
    printf("   建议编译器将变量放在寄存器中(加速访问)\n");
    printf("   ✗ 不能对register变量取地址(&)\n");
    printf("   ✗ 现代编译器自动优化寄存器分配，register已无实际意义\n");
    printf("   ✗ C17中register已被弃用(deprecated)\n\n");

    printf("3. extern (外部链接声明)\n");
    printf("   extern int global_var;  // 声明在其他文件中定义的变量\n");
    printf("   用于多文件编程中访问其他文件的变量/函数\n");
    printf("   extern声明不分配内存，只是引用\n\n");

    printf("4. static (静态存储期 + 限制链接)\n");
    printf("   局部static: 延长生命周期到程序结束\n");
    printf("   全局static: 限制作用域为当前文件(内部链接)\n");
    printf("   函数static: 限制函数为当前文件可见\n\n");

    printf("存储类对比:\n");
    printf("  存储类    存储期    作用域      链接性\n");
    printf("  ──────────────────────────────────────\n");
    printf("  auto      自动      块          无\n");
    printf("  register  自动      块          无\n");
    printf("  static    静态      块/文件     内部\n");
    printf("  extern    静态      文件        外部\n");
}

void demo_linkage(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  链接性(Linkage)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("链接性决定标识符在不同翻译单元(源文件)中的可见性:\n\n");

    printf("1. 外部链接(External Linkage):\n");
    printf("   普通全局变量和函数声明\n");
    printf("   其他文件可以通过extern访问\n");
    printf("   例: int global_var; // 其他文件可访问\n\n");

    printf("2. 内部链接(Internal Linkage):\n");
    printf("   static全局变量和static函数\n");
    printf("   只在当前文件可见\n");
    printf("   例: static int file_static_var; // 仅本文件可见\n\n");

    printf("3. 无链接(No Linkage):\n");
    printf("   局部变量、函数参数、typedef\n");
    printf("   只在声明它的块内可见\n\n");

    printf("多文件示例:\n");
    printf("  file1.c:                    file2.c:\n");
    printf("    int shared = 10;            extern int shared;  // 引用file1的shared\n");
    printf("    static int local = 20;      // 无法访问file1的local\n");
    printf("    void func() { ... }         void func();  // 声明file1的func\n");
    printf("    static void helper() {}     // 无法访问file1的helper\n");
}

void demo_initialization_rules(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  变量初始化规则\n");
    printf("═══════════════════════════════════════\n\n");

    printf("静态存储期变量(全局/static局部):\n");
    printf("  ✓ 自动初始化为0(整型0、浮点0.0、指针NULL)\n");
    printf("  ✓ 初始化在main()之前完成\n\n");

    printf("自动存储期变量(普通局部变量):\n");
    printf("  ✗ 不自动初始化，值是不确定的(垃圾值)\n");
    printf("  ✗ 读取未初始化的局部变量是未定义行为\n\n");

    printf("最佳实践:\n");
    printf("  ✓ 声明变量时立即初始化\n");
    printf("  ✓ int x = 0; 而非 int x;\n");
    printf("  ✓ const int size = 100; 常量必须初始化\n");
    printf("  ✓ 使用编译器警告: -Wuninitialized\n");
}

int main(void) {
    demo_scope_rules();
    demo_lifetime();
    demo_storage_classes();
    demo_linkage();
    demo_initialization_rules();

    return 0;
}
