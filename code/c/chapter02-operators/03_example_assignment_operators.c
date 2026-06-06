/** @file 03_example_assignment_operators.c
 *  @brief 赋值运算符、三元运算符、sizeof、逗号运算符
 *  @description 对应文档: 02-operators | 演示赋值运算符、复合赋值、三元条件、sizeof、逗号运算符
 *  编译命令: gcc -std=c17 03_example_assignment_operators.c -o 03_example_assignment_operators
 */

#include <stdio.h>
#include <stdlib.h>

void demo_basic_assignment(void) {
    printf("═══════════════════════════════════════\n");
    printf("  基本赋值运算符 =\n");
    printf("═══════════════════════════════════════\n\n");

    int a = 10;
    printf("a = 10 → a = %d\n", a);

    int b, c, d;
    b = c = d = 5;
    printf("b = c = d = 5 → b=%d, c=%d, d=%d (右结合性)\n", b, c, d);

    printf("\n赋值是表达式，有返回值:\n");
    int x;
    printf("x = 42 的值 = %d\n", x = 42);

    printf("\n⚠️ 常见错误: = 与 == 混淆\n");
    int val = 5;
    if (val = 10) {
        printf("  val = 10 在if中: 赋值成功，val=%d，条件为真!\n", val);
    }
    printf("  应该用: if (val == 10)\n");
    printf("  防御写法: if (10 == val) → 写错为 if (10 = val) 编译报错\n");
}

void demo_compound_assignment(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  复合赋值运算符\n");
    printf("═══════════════════════════════════════\n\n");

    int a = 20;

    printf("初始值: a = %d\n\n", a);

    a += 5;  printf("a += 5   → a = %d\n", a);
    a -= 3;  printf("a -= 3   → a = %d\n", a);
    a *= 2;  printf("a *= 2   → a = %d\n", a);
    a /= 4;  printf("a /= 4   → a = %d\n", a);
    a %= 3;  printf("a %%= 3   → a = %d\n", a);

    a = 0xFF;
    a &= 0x0F; printf("a &= 0x0F → a = 0x%02X\n", a);
    a |= 0xA0; printf("a |= 0xA0 → a = 0x%02X\n", a);
    a ^= 0xFF; printf("a ^= 0xFF → a = 0x%02X\n", a);
    a <<= 2;   printf("a <<= 2   → a = 0x%02X\n", a);
    a >>= 4;   printf("a >>= 4   → a = 0x%02X\n", a);

    printf("\n复合赋值等价关系:\n");
    printf("  a += b  ≡  a = a + b\n");
    printf("  但复合赋值只计算左操作数一次(对有副作用的表达式很重要)\n");
    printf("  例: arr[func()] += 1  → func()只调用一次\n");
    printf("      arr[func()] = arr[func()] + 1  → func()调用两次!\n");
}

void demo_ternary_operator(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  三元条件运算符 ? :\n");
    printf("═══════════════════════════════════════\n\n");

    int a = 10, b = 20;
    int max = (a > b) ? a : b;
    printf("max = (a > b) ? a : b → max = %d\n", max);

    int score = 85;
    const char *level = (score >= 90) ? "优秀" :
                        (score >= 80) ? "良好" :
                        (score >= 60) ? "及格" : "不及格";
    printf("分数 %d → %s\n", score, level);

    printf("\n三元运算符用于赋值:\n");
    int x = 5;
    int abs_x = (x >= 0) ? x : -x;
    printf("|%d| = %d\n", x, abs_x);
    x = -3;
    abs_x = (x >= 0) ? x : -x;
    printf("|%d| = %d\n", x, abs_x);

    printf("\n⚠️ 三元运算符的陷阱:\n");
    printf("  不要嵌套过深，可读性差\n");
    printf("  两个分支的类型会进行隐式转换\n");
    printf("  (1?1:0.5) 的类型是double, 不是int → %f\n", (1 ? 1 : 0.5));
}

void demo_sizeof_operator(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  sizeof 运算符\n");
    printf("═══════════════════════════════════════\n\n");

    printf("sizeof 是编译期运算符(不是函数!)\n\n");

    printf("基本类型的大小:\n");
    printf("  sizeof(char)      = %zu\n", sizeof(char));
    printf("  sizeof(short)     = %zu\n", sizeof(short));
    printf("  sizeof(int)       = %zu\n", sizeof(int));
    printf("  sizeof(long)      = %zu\n", sizeof(long));
    printf("  sizeof(long long) = %zu\n", sizeof(long long));
    printf("  sizeof(float)     = %zu\n", sizeof(float));
    printf("  sizeof(double)    = %zu\n", sizeof(double));
    printf("  sizeof(void*)     = %zu\n", sizeof(void*));

    printf("\n数组的大小:\n");
    int arr[] = {1, 2, 3, 4, 5};
    printf("  sizeof(arr)       = %zu (整个数组)\n", sizeof(arr));
    printf("  sizeof(arr[0])    = %zu (单个元素)\n", sizeof(arr[0]));
    printf("  数组元素个数 = %zu\n", sizeof(arr) / sizeof(arr[0]));

    printf("\n⚠️ sizeof 对表达式不求值:\n");
    int x = 10;
    printf("  sizeof(x++) → %zu (x不会自增!)\n", sizeof(x++));
    printf("  x = %d (仍然是10)\n", x);

    printf("\n⚠️ sizeof 对指针和数组不同:\n");
    int *p = arr;
    printf("  sizeof(arr) = %zu (数组: 整个大小)\n", sizeof(arr));
    printf("  sizeof(p)   = %zu (指针: 指针本身大小)\n", sizeof(p));
}

void demo_comma_operator(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  逗号运算符 ,\n");
    printf("═══════════════════════════════════════\n\n");

    printf("逗号运算符: 从左到右依次求值，返回最右边的值\n\n");

    int a = (1, 2, 3);
    printf("a = (1, 2, 3) → a = %d\n", a);

    int b, c;
    b = (c = 3, c + 2);
    printf("b = (c = 3, c + 2) → b = %d, c = %d\n", b, c);

    printf("\n在for循环中的常见用法:\n");
    printf("  for (i = 0, j = n-1; i < j; i++, j--)\n");
    int arr[] = {1, 2, 3, 4, 5};
    int n = 5;
    printf("  反转数组: ");
    for (int i = 0, j = n - 1; i < j; i++, j--) {
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    printf("\n⚠️ 注意区分逗号运算符和函数参数分隔符:\n");
    printf("  func(a, b)     → 逗号是参数分隔符\n");
    printf("  func((a, b))   → 逗号是运算符，传入b的值\n");
}

int main(void) {
    demo_basic_assignment();
    demo_compound_assignment();
    demo_ternary_operator();
    demo_sizeof_operator();
    demo_comma_operator();

    return 0;
}
