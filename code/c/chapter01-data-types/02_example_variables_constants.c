/** @file 02_example_variables_constants.c
 *  @brief 变量与常量：声明、初始化、const、#define、enum
 *  @description 对应文档: 01-data-types | 演示变量声明、初始化方式、各种常量定义方法
 *  编译命令: gcc -std=c17 02_example_variables_constants.c -o 02_example_variables_constants
 */

#include <stdio.h>
#include <stdlib.h>

#define MAX_BUFFER_SIZE 1024
#define VERSION_STRING "1.0.0"
#define PI 3.14159265

void demo_variable_declaration(void) {
    printf("═══════════════════════════════════════\n");
    printf("  变量声明与定义\n");
    printf("═══════════════════════════════════════\n\n");

    int a;
    int b, c, d;
    int x = 10;
    int y = 20, z = 30;

    a = 5;
    b = 10;
    c = 15;
    d = 20;

    printf("先声明后赋值: a=%d, b=%d, c=%d, d=%d\n", a, b, c, d);
    printf("声明并初始化: x=%d, y=%d, z=%d\n", x, y, z);

    printf("\nC99/C11支持混合声明和代码:\n");
    for (int i = 0; i < 3; i++) {
        printf("  循环变量 i=%d (在for内声明)\n", i);
    }

    printf("\n变量命名规则:\n");
    printf("  ✓ 由字母、数字、下划线组成\n");
    printf("  ✓ 必须以字母或下划线开头\n");
    printf("  ✓ 区分大小写 (count ≠ Count)\n");
    printf("  ✗ 不能用关键字 (int, return等)\n");
    printf("  ✗ 不能以数字开头\n");

    printf("\n命名风格建议:\n");
    printf("  snake_case:  max_buffer_size (C语言常用)\n");
    printf("  camelCase:   maxBufferSize  (部分项目使用)\n");
    printf("  全大写常量:  MAX_BUFFER_SIZE\n");
}

void demo_initialization(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  变量初始化方式\n");
    printf("═══════════════════════════════════════\n\n");

    int arr1[5] = {1, 2, 3, 4, 5};
    int arr2[5] = {1, 2};
    int arr3[] = {10, 20, 30};
    int arr4[5] = {0};

    printf("完全初始化: {1,2,3,4,5} → ");
    for (int i = 0; i < 5; i++) printf("%d ", arr1[i]);
    printf("\n");

    printf("部分初始化: {1,2}       → ");
    for (int i = 0; i < 5; i++) printf("%d ", arr2[i]);
    printf(" (未初始化的自动为0)\n");

    printf("省略大小:   {10,20,30}  → ");
    for (int i = 0; i < 3; i++) printf("%d ", arr3[i]);
    printf(" (大小由初始化列表决定)\n");

    printf("全零初始化: {0}         → ");
    for (int i = 0; i < 5; i++) printf("%d ", arr4[i]);
    printf("\n");

    printf("\nC99指定初始化器:\n");
    int arr5[10] = {[2] = 30, [5] = 60, [8] = 90};
    printf("  {[2]=30, [5]=60, [8]=90} → ");
    for (int i = 0; i < 10; i++) printf("%d ", arr5[i]);
    printf("\n");

    printf("\n⚠️ 未初始化的局部变量值是不确定的(垃圾值):\n");
    printf("  int x; // x的值未知，可能是任意值\n");
    printf("  全局变量和静态变量默认初始化为0\n");
}

void demo_const_keyword(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  const 关键字\n");
    printf("═══════════════════════════════════════\n\n");

    const int MAX_AGE = 150;
    const double GRAVITY = 9.8;
    const char NEWLINE = '\n';

    printf("const int MAX_AGE = %d\n", MAX_AGE);
    printf("const double GRAVITY = %.1f\n", GRAVITY);
    printf("const char NEWLINE = '\\n'%c", NEWLINE);

    printf("const的特点:\n");
    printf("  ✓ 编译器会检查，不允许直接修改const变量\n");
    printf("  ✓ 必须在声明时初始化\n");
    printf("  ✓ 有类型检查，比#define更安全\n");
    printf("  ✓ 可以通过指针绕过(未定义行为，不推荐)\n\n");

    printf("const与指针的组合:\n");
    printf("  const int *p1;        → 指向const int的指针，*p1不可修改\n");
    printf("  int *const p2 = &x;   → const指针，p2不可修改，*p2可修改\n");
    printf("  const int *const p3;  → 都不可修改\n");
}

void demo_define_macro(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  #define 宏定义常量\n");
    printf("═══════════════════════════════════════\n\n");

    printf("MAX_BUFFER_SIZE = %d\n", MAX_BUFFER_SIZE);
    printf("VERSION_STRING = %s\n", VERSION_STRING);
    printf("PI = %.8f\n", PI);

    printf("\n#define vs const 对比:\n");
    printf("  特性        #define          const\n");
    printf("  ────────────────────────────────────────\n");
    printf("  处理阶段    预处理           编译\n");
    printf("  类型检查    无               有\n");
    printf("  作用域      文件级(到#undef) 遵循作用域规则\n");
    printf("  调试可见    宏名不存在       变量名可见\n");
    printf("  内存占用    文本替换         可能占用内存\n");
    printf("  取地址      不可以           可以\n\n");

    printf("选择建议:\n");
    printf("  简单数值常量 → 优先用 const 或 enum\n");
    printf("  条件编译需要 → 用 #define\n");
    printf("  跨平台配置   → 用 #define\n");
}

void demo_enum_constants(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  枚举常量 (enum)\n");
    printf("═══════════════════════════════════════\n\n");

    enum Color { RED, GREEN, BLUE };
    enum Color favorite = GREEN;

    printf("enum Color { RED, GREEN, BLUE };\n");
    printf("  RED   = %d\n", RED);
    printf("  GREEN = %d\n", GREEN);
    printf("  BLUE  = %d\n", BLUE);
    printf("  favorite = %d\n", favorite);

    printf("\n指定枚举值:\n");
    enum HttpStatus {
        HTTP_OK = 200,
        HTTP_NOT_FOUND = 404,
        HTTP_SERVER_ERROR = 500
    };
    printf("  HTTP_OK          = %d\n", HTTP_OK);
    printf("  HTTP_NOT_FOUND   = %d\n", HTTP_NOT_FOUND);
    printf("  HTTP_SERVER_ERROR = %d\n", HTTP_SERVER_ERROR);

    printf("\n枚举的特性:\n");
    printf("  ✓ 枚举常量是int类型的编译期常量\n");
    printf("  ✓ 默认从0开始递增\n");
    printf("  ✓ 可以指定任意整数值\n");
    printf("  ✓ 比宏定义更有可读性和类型安全\n");
    printf("  ✗ C中枚举不严格限制取值范围\n");

    printf("\n实际应用——用枚举定义状态机:\n");
    enum State { STATE_IDLE, STATE_RUNNING, STATE_PAUSED, STATE_STOPPED };
    enum State current = STATE_RUNNING;
    const char *state_names[] = {"IDLE", "RUNNING", "PAUSED", "STOPPED"};
    printf("  当前状态: %s (%d)\n", state_names[current], current);
}

void demo_literal_types(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  字面量(Literal)类型\n");
    printf("═══════════════════════════════════════\n\n");

    printf("整数字面量:\n");
    printf("  42        → int\n");
    printf("  42U       → unsigned int\n");
    printf("  42L       → long\n");
    printf("  42LL      → long long\n");
    printf("  0x2A      → int (十六进制) = %d\n", 0x2A);
    printf("  052       → int (八进制)   = %d\n", 052);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    printf("  0b101010  → int (二进制,C99GCC扩展) = %d\n",
        0b101010
    );
#pragma GCC diagnostic pop

    printf("\n浮点字面量:\n");
    printf("  3.14      → double\n");
    printf("  3.14f     → float\n");
    printf("  3.14L     → long double\n");
    printf("  1e5       → double (科学计数法) = %f\n", 1e5);

    printf("\n字符字面量:\n");
    printf("  'A'       → int (C中字符字面量是int类型!) = %d\n", 'A');
    printf("  \"hello\"   → char[6] (含结尾'\\0')\n");

    printf("\n⚠️ 注意: C中 'A' 的类型是 int，不是 char\n");
    printf("  sizeof('A') = %zu (不是1!)\n", sizeof('A'));
}

int main(void) {
    demo_variable_declaration();
    demo_initialization();
    demo_const_keyword();
    demo_define_macro();
    demo_enum_constants();
    demo_literal_types();

    return 0;
}
