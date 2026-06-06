/** @file 01_example_if_switch.c
 *  @brief 条件语句：if/else、switch/case、嵌套条件
 *  @description 对应文档: 03-control-structure | 演示if/else和switch/case的用法与注意事项
 *  编译命令: gcc -std=c17 01_example_if_switch.c -o 01_example_if_switch
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

void demo_if_else(void) {
    printf("═══════════════════════════════════════\n");
    printf("  if/else 语句\n");
    printf("═══════════════════════════════════════\n\n");

    int score = 85;

    if (score >= 90) {
        printf("成绩 %d: 优秀\n", score);
    } else if (score >= 80) {
        printf("成绩 %d: 良好\n", score);
    } else if (score >= 60) {
        printf("成绩 %d: 及格\n", score);
    } else {
        printf("成绩 %d: 不及格\n", score);
    }

    printf("\nif语句的三种形式:\n");
    printf("  1. if (条件) 语句\n");
    printf("  2. if (条件) 语句 else 语句\n");
    printf("  3. if (条件) 语句 else if (条件) 语句 ... else 语句\n");

    printf("\n⚠️ 悬空else问题 (Dangling Else):\n");
    int a = 1, b = 0;
    if (a > 0) {
        if (b > 0)
            printf("  a和b都大于0\n");
        else
            printf("  a不大于0\n");
    }
    printf("  实际上else与最近的if匹配! 缩进不能决定匹配关系\n");
    printf("  修正: 用大括号明确代码块\n");
}

void demo_if_common_patterns(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  if语句常见模式\n");
    printf("═══════════════════════════════════════\n\n");

    printf("1. 卫语句 (Guard Clause) —— 提前返回:\n");
    printf("   if (ptr == NULL) return;\n");
    printf("   if (size <= 0) return -1;\n");
    printf("   // 正常逻辑...\n\n");

    printf("2. 输入验证:\n");
    int age = 150;
    if (age < 0 || age > 120) {
        printf("   年龄 %d 无效! 应在0-120之间\n", age);
    }

    printf("\n3. 范围检查:\n");
    int month = 6;
    if (month >= 1 && month <= 12) {
        printf("   月份 %d 有效\n", month);
    }

    printf("\n4. 单行if(不加括号的风险):\n");
    printf("   ✗ if (x > 0)\n");
    printf("       y = x;\n");
    printf("       z = x * 2;  // 这行不在if内! 缩进误导\n");
    printf("   ✓ 始终使用大括号(即使只有一行)\n");
}

void demo_switch_case(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  switch/case 语句\n");
    printf("═══════════════════════════════════════\n\n");

    int day = 3;
    printf("星期%d → ", day);

    switch (day) {
        case 1:  printf("星期一\n"); break;
        case 2:  printf("星期二\n"); break;
        case 3:  printf("星期三\n"); break;
        case 4:  printf("星期四\n"); break;
        case 5:  printf("星期五\n"); break;
        case 6:  printf("星期六\n"); break;
        case 7:  printf("星期日\n"); break;
        default: printf("无效的星期数\n"); break;
    }

    printf("\nswitch的规则:\n");
    printf("  1. case标签必须是整型常量表达式\n");
    printf("  2. 不加break会贯穿(fall-through)到下一个case\n");
    printf("  3. default处理所有未匹配的情况\n");
    printf("  4. switch表达式必须是整型\n");

    printf("\n利用fall-through合并多个case:\n");
    char grade = 'B';
    printf("等级 '%c' → ", grade);
    switch (grade) {
        case 'A':
        case 'B':
        case 'C':
            printf("通过\n");
            break;
        case 'D':
        case 'F':
            printf("未通过\n");
            break;
        default:
            printf("无效等级\n");
            break;
    }
}

void demo_switch_patterns(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  switch实用模式\n");
    printf("═══════════════════════════════════════\n\n");

    printf("1. 状态机:\n");
    enum State { STATE_START, STATE_RUNNING, STATE_STOPPED };
    enum State state = STATE_RUNNING;
    switch (state) {
        case STATE_START:   printf("   状态: 启动\n"); break;
        case STATE_RUNNING: printf("   状态: 运行中\n"); break;
        case STATE_STOPPED: printf("   状态: 已停止\n"); break;
    }

    printf("\n2. 命令解析:\n");
    char cmd = 'h';
    switch (cmd) {
        case 'h': printf("   帮助: h-帮助 q-退出 r-运行\n"); break;
        case 'q': printf("   退出程序\n"); break;
        case 'r': printf("   开始运行\n"); break;
        default:  printf("   未知命令: %c\n", cmd); break;
    }

    printf("\n3. 字符分类:\n");
    char ch = '5';
    switch (ch) {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            printf("   '%c' 是数字\n", ch); break;
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
            printf("   '%c' 是十六进制字符\n", ch); break;
        default:
            printf("   '%c' 是其他字符\n", ch); break;
    }

    printf("\n4. switch中声明变量需要大括号:\n");
    printf("   case 1: { int x = 10; ... }  // 需要大括号\n");
    printf("   case 2: int y = 20;          // 编译错误! 可能跳过初始化\n");
}

void demo_if_vs_switch(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  if-else vs switch 对比\n");
    printf("═══════════════════════════════════════\n\n");

    printf("适用场景:\n");
    printf("  switch: 对单个整型值的多路分支\n");
    printf("  if-else: 范围判断、浮点比较、复杂条件\n\n");

    printf("  特性          if-else         switch\n");
    printf("  ──────────────────────────────────────\n");
    printf("  条件类型      任意            整型/枚举\n");
    printf("  范围判断      ✓               ✗\n");
    printf("  编译器优化    通常分支预测     可能跳转表\n");
    printf("  可读性        条件多时差       多值匹配时好\n");
    printf("  fall-through  无               有(需注意)\n\n");

    printf("选择建议:\n");
    printf("  3个以上离散值匹配 → switch\n");
    printf("  范围判断、复杂条件 → if-else\n");
}

int main(void) {
    demo_if_else();
    demo_if_common_patterns();
    demo_switch_case();
    demo_switch_patterns();
    demo_if_vs_switch();

    return 0;
}
