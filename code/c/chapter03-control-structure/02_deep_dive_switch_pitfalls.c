/** @file 02_deep_dive_switch_pitfalls.c
 *  @brief 深入理解switch陷阱：fall-through、Duff's device、switch vs if-else性能
 *  @description 对应文档: 03-control-structure | switch的fall-through机制、Duff设备、性能分析
 *  编译命令: gcc -std=c17 02_deep_dive_switch_pitfalls.c -o 02_deep_dive_switch_pitfalls
 */

#include <stdio.h>
#include <stdlib.h>

void demo_fall_through(void) {
    printf("═══════════════════════════════════════\n");
    printf("  switch fall-through 详解\n");
    printf("═══════════════════════════════════════\n\n");

    printf("fall-through: 没有break时，执行流会「贯穿」到下一个case\n\n");

    printf("示例: 不加break的后果\n");
    int choice = 2;
    printf("  choice = %d, 不加break:\n", choice);
    switch (choice) {
        case 1: printf("    case 1 执行\n");
                /* fall through */
        case 2: printf("    case 2 执行\n");
                /* fall through */
        case 3: printf("    case 3 执行\n");
                /* fall through */
        default: printf("    default 执行\n");
    }

    printf("\n  choice = %d, 加上break:\n", choice);
    switch (choice) {
        case 1: printf("    case 1 执行\n"); break;
        case 2: printf("    case 2 执行\n"); break;
        case 3: printf("    case 3 执行\n"); break;
        default: printf("    default 执行\n"); break;
    }

    printf("\n⚠️ fall-through是C语言最常见bug来源之一!\n");
    printf("  GCC/Clang: -Wimplicit-fallthrough 警告选项\n");
    printf("  C23: 引入 [[fallthrough]] 属性标注有意fall-through\n");
}

void demo_intentional_fall_through(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  有意的 fall-through 模式\n");
    printf("═══════════════════════════════════════\n\n");

    printf("1. 合并多个case执行相同逻辑:\n");
    char grade = 'B';
    switch (grade) {
        case 'A':
        case 'B':
        case 'C':
            printf("  等级 %c: 通过\n", grade);
            break;
        case 'D':
        case 'F':
            printf("  等级 %c: 未通过\n", grade);
            break;
    }

    printf("\n2. 累积执行(每个case增加功能):\n");
    enum Level { BASIC, STANDARD, PREMIUM };
    enum Level subscription = PREMIUM;
    int features = 0;
    switch (subscription) {
        case PREMIUM:
            features |= 4;
            printf("  + 高级分析功能\n");
            /* fall through */
        case STANDARD:
            features |= 2;
            printf("  + 多用户支持\n");
            /* fall through */
        case BASIC:
            features |= 1;
            printf("  + 基本功能\n");
            break;
    }
    printf("  功能标志: 0x%X\n", features);

    printf("\n3. 标注有意fall-through(C23或编译器扩展):\n");
    printf("  case 1:\n");
    printf("      do_something();\n");
    printf("      [[fallthrough]];  // C23属性，告诉编译器这是有意的\n");
    printf("  case 2:\n");
    printf("      do_other();\n");
    printf("      break;\n");
}

void demo_duffs_device(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  Duff's Device (达夫设备)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("Duff's Device: switch和do-while的惊人组合\n");
    printf("由Tom Duff于1983年发现，用于优化循环展开\n\n");

    int src[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    int dst[11] = {0};
    int count = 11;
    int *from = src;
    int *to = dst;

    int n = (count + 7) / 8;
    switch (count % 8) {
        case 0: do { *to++ = *from++; /* fall through */
        case 7:      *to++ = *from++; /* fall through */
        case 6:      *to++ = *from++; /* fall through */
        case 5:      *to++ = *from++; /* fall through */
        case 4:      *to++ = *from++; /* fall through */
        case 3:      *to++ = *from++; /* fall through */
        case 2:      *to++ = *from++; /* fall through */
        case 1:      *to++ = *from++;
                } while (--n > 0);
    }

    printf("复制结果: ");
    for (int i = 0; i < 11; i++) {
        printf("%d ", dst[i]);
    }
    printf("\n");

    printf("\nDuff's Device的工作原理:\n");
    printf("  1. 计算余数 count%%8，跳转到switch对应位置\n");
    printf("  2. 先执行余数次赋值\n");
    printf("  3. 然后进入do-while循环，每次执行8次赋值\n");
    printf("  4. 减少循环控制开销(条件判断次数减少8倍)\n\n");

    printf("评价:\n");
    printf("  ✓ 历史上确实能提升性能\n");
    printf("  ✗ 现代编译器自动做循环展开，效果不如以前\n");
    printf("  ✗ 代码可读性极差\n");
    printf("  ✗ 可能干扰编译器的自动优化\n");
    printf("  → 了解即可，实际编程中不要使用!\n");
}

void demo_switch_performance(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  switch vs if-else 性能分析\n");
    printf("═══════════════════════════════════════\n\n");

    printf("编译器对switch的优化:\n\n");

    printf("1. 稀疏case值 → 等价于if-else链\n");
    printf("   case 1: case 100: case 1000:\n");
    printf("   编译器生成逐个比较的代码\n\n");

    printf("2. 密集case值 → 跳转表(Jump Table)\n");
    printf("   case 0: case 1: case 2: ... case 9:\n");
    printf("   编译器生成数组，O(1)时间跳转\n\n");

    printf("3. 部分密集 → 混合策略\n");
    printf("   对密集部分用跳转表，稀疏部分用比较\n\n");

    printf("性能对比(理论分析):\n");
    printf("  结构          时间复杂度    适用场景\n");
    printf("  ─────────────────────────────────────\n");
    printf("  if-else链     O(n)         少量分支\n");
    printf("  switch稀疏    O(log n)     二分搜索\n");
    printf("  switch密集    O(1)         跳转表\n\n");

    printf("实际建议:\n");
    printf("  1. 3个以下分支: if-else更直观\n");
    printf("  2. 5个以上离散值: switch可能更快\n");
    printf("  3. case值密集连续时switch优势最大\n");
    printf("  4. 过早优化是万恶之源，先写清晰的代码\n");
}

void demo_switch_advanced_patterns(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  switch高级模式\n");
    printf("═══════════════════════════════════════\n\n");

    printf("1. 用switch实现对象多态(模拟虚函数表):\n");
    enum ShapeType { SHAPE_CIRCLE, SHAPE_RECT, SHAPE_TRIANGLE };
    struct Shape { enum ShapeType type; double params[3]; };

    struct Shape s = {SHAPE_CIRCLE, {5.0, 0, 0}};
    double area = 0;
    switch (s.type) {
        case SHAPE_CIRCLE:
            area = 3.14159265 * s.params[0] * s.params[0];
            printf("  圆面积: %.2f\n", area);
            break;
        case SHAPE_RECT:
            area = s.params[0] * s.params[1];
            printf("  矩形面积: %.2f\n", area);
            break;
        case SHAPE_TRIANGLE:
            area = 0.5 * s.params[0] * s.params[1];
            printf("  三角形面积: %.2f\n", area);
            break;
    }

    printf("\n2. switch中的变量作用域:\n");
    printf("   switch (x) {\n");
    printf("       case 1: {\n");
    printf("           int temp = 10;  // 需要大括号!\n");
    printf("           printf(\"%%d\", temp);\n");
    printf("           break;\n");
    printf("       }\n");
    printf("       case 2: ...\n");
    printf("   }\n\n");

    printf("3. C11的 _Generic(泛型选择):\n");
    printf("   _Generic 是编译期选择，不是运行时switch\n");
    printf("   _Generic(x,\n");
    printf("       int:   print_int,\n");
    printf("       double: print_double,\n");
    printf("       default: print_unknown\n");
    printf("   )(x);\n\n");

    #define PRINT_TYPE(x) _Generic((x), \
        int: "int", \
        double: "double", \
        float: "float", \
        char: "char", \
        default: "unknown" \
    )

    int vi = 42;
    double vd = 3.14;
    printf("   %d 的类型: %s\n", vi, PRINT_TYPE(vi));
    printf("   %.2f 的类型: %s\n", vd, PRINT_TYPE(vd));

    printf("\n4. 常见switch陷阱汇总:\n");
    printf("   ✗ 忘记break → fall-through bug\n");
    printf("   ✗ case值重复 → 编译错误\n");
    printf("   ✗ case不是常量表达式 → 编译错误\n");
    printf("   ✗ switch条件是浮点 → 编译错误\n");
    printf("   ✗ 在case中声明变量不加括号 → 编译错误\n");
    printf("   ✗ 忘记default → 未处理的情况\n");
}

int main(void) {
    demo_fall_through();
    demo_intentional_fall_through();
    demo_duffs_device();
    demo_switch_performance();
    demo_switch_advanced_patterns();

    return 0;
}
