/** @file 01_deep_dive_operator_precedence.c
 *  @brief 深入理解运算符优先级、结合性、序列点与求值顺序
 *  @description 对应文档: 02-operators | 完整优先级表、常见优先级陷阱、序列点概念、求值顺序问题
 *  编译命令: gcc -std=c17 01_deep_dive_operator_precedence.c -o 01_deep_dive_operator_precedence
 */

#include <stdio.h>
#include <stdlib.h>

void demo_precedence_table(void) {
    printf("═══════════════════════════════════════\n");
    printf("  C语言运算符优先级表(从高到低)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("优先级  运算符              结合性    说明\n");
    printf("──────  ──────────────────  ────────  ──────────────\n");
    printf("  1     () [] -> .          左到右    后缀/成员访问\n");
    printf("  2     ++ -- ! ~ + - * &   右到左    一元运算符\n");
    printf("        (type) sizeof\n");
    printf("  3     * / %%              左到右    乘除\n");
    printf("  4     + -                 左到右    加减\n");
    printf("  5     << >>               左到右    移位\n");
    printf("  6     < <= > >=           左到右    关系\n");
    printf("  7     == !=               左到右    相等\n");
    printf("  8     &                   左到右    位与\n");
    printf("  9     ^                   左到右    位异或\n");
    printf(" 10     |                   左到右    位或\n");
    printf(" 11     &&                  左到右    逻辑与\n");
    printf(" 12     ||                  左到右    逻辑或\n");
    printf(" 13     ?:                  右到左    条件\n");
    printf(" 14     = += -= 等          右到左    赋值\n");
    printf(" 15     ,                   左到右    逗号\n\n");

    printf("记忆口诀:\n");
    printf("  括号成员第一     () [] -> .\n");
    printf("  全体单目第二     ! ~ ++ -- + - * & (type) sizeof\n");
    printf("  乘除余三加减四   * / %%  + -\n");
    printf("  移位五关六       << >>   < <= > >=\n");
    printf("  等于不等排第七   == !=\n");
    printf("  位与异或位或八九十 & ^ |\n");
    printf("  逻辑与或十一十二 && ||\n");
    printf("  条件十三赋十四   ?:   = += -=\n");
    printf("  逗号十五最低     ,\n");
}

void demo_precedence_pitfalls(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  常见优先级陷阱\n");
    printf("═══════════════════════════════════════\n\n");

    printf("陷阱1: 位运算符优先级低于比较运算符\n");
    int flags = 0x0F;
    if (flags & 0x01 == 1) {
        printf("  flags & 0x01 == 1 → 真\n");
    } else {
        printf("  flags & 0x01 == 1 → 假! (因为 == 先于 & 计算)\n");
        printf("  实际等价于: flags & (0x01 == 1) → flags & 1 → %d\n", flags & (0x01 == 1));
    }
    printf("  修正: (flags & 0x01) == 1 → %s\n",
           (flags & 0x01) == 1 ? "真" : "假");

    printf("\n陷阱2: 自增与解引用\n");
    int arr[] = {10, 20, 30};
    int *p = arr;
    printf("  *p++ = %d (先取*p，再p++)\n", *p++);
    printf("  现在p指向arr[1] = %d\n", *p);

    p = arr;
    printf("  (*p)++ = %d (先取*p，再(*p)++)\n", (*p)++);
    printf("  arr[0]现在 = %d\n", arr[0]);

    printf("\n陷阱3: 三元运算符嵌套\n");
    int a = 1, b = 2, c = 3;
    int val = a + b > c ? a : b;
    printf("  a + b > c ? a : b → %d\n", val);
    printf("  等价于: (a + b > c) ? a : b\n");
    printf("  不是: a + (b > c ? a : b)\n");

    printf("\n陷阱4: 移位与加减\n");
    printf("  1 << 2 + 3 = %d (不是 4+3=7!)\n", 1 << 2 + 3);
    printf("  等价于: 1 << (2+3) = 1 << 5 = %d\n", 1 << (2 + 3));
    printf("  修正: (1 << 2) + 3 = %d\n", (1 << 2) + 3);
}

void demo_associativity(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  结合性(Associativity)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("当优先级相同时，结合性决定计算顺序:\n\n");

    printf("左结合(从左到右): 大部分二元运算符\n");
    printf("  10 - 3 - 2 = %d (等价于 (10-3)-2 = 5)\n", 10 - 3 - 2);
    printf("  100 / 10 / 2 = %d (等价于 (100/10)/2 = 5)\n", 100 / 10 / 2);

    printf("\n右结合(从右到左): 赋值、一元、三元\n");
    int a, b, c;
    a = b = c = 42;
    printf("  a = b = c = 42 → a=%d, b=%d, c=%d (从右往左赋值)\n", a, b, c);

    int x = 1, y = 2;
    x += y *= 3;
    printf("  x=1, y=2; x += y *= 3 → x=%d, y=%d\n", x, y);
    printf("  等价于: y = y*3 = 6; x = x+y = 7\n");
}

void demo_sequence_points(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  序列点(Sequence Point)与求值顺序\n");
    printf("═══════════════════════════════════════\n\n");

    printf("序列点: 程序执行中，之前的所有副作用都已完成的点\n\n");

    printf("C语言中的序列点:\n");
    printf("  1. 分号 ; (完整表达式结束时)\n");
    printf("  2. && 和 || 的左操作数求值后(短路求值)\n");
    printf("  3. 逗号运算符 , 的左操作数求值后\n");
    printf("  4. 三元运算符 ? 的条件求值后\n");
    printf("  5. 函数调用时，所有参数求值后(但参数之间的求值顺序未定义)\n");
    printf("  6. return语句\n\n");

    printf("⚠️ 两个序列点之间，同一变量只能被修改一次:\n\n");

    printf("未定义行为的例子:\n");
    printf("  a[i] = i++;       // i被读取和修改，无序列点分隔\n");
    printf("  a = a++;          // a被修改两次\n");
    printf("  a = ++a + 1;      // a被修改和读取\n");
    printf("  printf(\"%%d %%d\", a++, a++); // a被修改两次\n\n");

    printf("正确写法:\n");
    printf("  a[i] = i; i++;    // 分号是序列点，分开写\n");
    printf("  a = a + 1; a++;   // 或者 a += 1; a++;\n");
}

void demo_evaluation_order(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  求值顺序(Evaluation Order)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("C标准不规定大多数表达式的求值顺序!\n\n");

    printf("函数参数的求值顺序未定义:\n");
    int v = 1;
    printf("  printf(\"%%d %%d\", v++, v++) 的结果是不确定的\n");
    printf("  不同编译器可能输出: 1 2 或 2 1\n\n");

    printf("二元运算符操作数的求值顺序未定义:\n");
    printf("  a() + b() → a()和b()谁先执行未定义\n");
    printf("  如果a和b有共享状态，结果不可预测\n\n");

    printf("唯一保证的求值顺序:\n");
    printf("  ✓ && 左操作数先求值，为假则右操作数不求值\n");
    printf("  ✓ || 左操作数先求值，为真则右操作数不求值\n");
    printf("  ✓ ,  左操作数先求值(逗号运算符，非函数参数)\n");
    printf("  ✓ ?: 条件先求值，只求值匹配的分支\n\n");

    printf("最佳实践:\n");
    printf("  1. 不要在表达式中对同一变量多次修改\n");
    printf("  2. 有副作用的表达式单独成语句\n");
    printf("  3. 需要特定顺序时，拆分为多条语句\n");
    printf("  4. 用括号明确优先级，不要依赖记忆\n");
}

int main(void) {
    demo_precedence_table();
    demo_precedence_pitfalls();
    demo_associativity();
    demo_sequence_points();
    demo_evaluation_order();

    return 0;
}
