/** @file 02_example_loops.c
 *  @brief 循环语句：for、while、do-while、break、continue、嵌套循环
 *  @description 对应文档: 03-control-structure | 演示三种循环结构、循环控制语句和嵌套循环
 *  编译命令: gcc -std=c17 02_example_loops.c -o 02_example_loops
 */

#include <stdio.h>
#include <stdlib.h>

void demo_for_loop(void) {
    printf("═══════════════════════════════════════\n");
    printf("  for 循环\n");
    printf("═══════════════════════════════════════\n\n");

    printf("基本for循环:\n");
    for (int i = 0; i < 5; i++) {
        printf("  i = %d\n", i);
    }

    printf("\nfor循环结构: for(初始化; 条件; 更新) { 循环体 }\n");
    printf("  初始化: 只执行一次\n");
    printf("  条件:   每次循环前检查，为假则退出\n");
    printf("  更新:   每次循环体执行后执行\n");

    printf("\n递减for循环:\n");
    for (int i = 5; i > 0; i--) {
        printf("  %d ", i);
    }
    printf("\n");

    printf("\n步长为2:\n");
    for (int i = 0; i <= 10; i += 2) {
        printf("  %d ", i);
    }
    printf("\n");

    printf("\n省略部分表达式:\n");
    int k = 0;
    for (; k < 3; ) {
        printf("  k = %d\n", k);
        k++;
    }
    printf("  (三个表达式都可以省略，但分号不能省)\n");

    printf("\nC99: for中声明变量:\n");
    printf("  for(int i = 0; i < n; i++) ← i的作用域限于for循环内\n");
}

void demo_while_loop(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  while 循环\n");
    printf("═══════════════════════════════════════\n\n");

    printf("while: 先检查条件，再执行循环体\n\n");

    printf("示例: 计算数字位数\n");
    int num = 123456;
    int temp = num;
    int digits = 0;
    while (temp > 0) {
        temp /= 10;
        digits++;
    }
    printf("  %d 有 %d 位\n", num, digits);

    printf("\n示例: 猜数字(模拟)\n");
    int target = 7;
    int guess = 1;
    int attempts = 0;
    while (guess != target) {
        attempts++;
        guess++;
    }
    printf("  猜了 %d 次找到目标 %d\n", attempts, target);

    printf("\nwhile适合的场景:\n");
    printf("  循环次数未知，需要根据条件决定是否继续\n");
    printf("  读取输入直到满足条件\n");
    printf("  处理数据直到遇到结束标记\n");
}

void demo_do_while_loop(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  do-while 循环\n");
    printf("═══════════════════════════════════════\n\n");

    printf("do-while: 先执行循环体，再检查条件(至少执行一次)\n\n");

    printf("示例: 获取有效输入(模拟)\n");
    int input = -1;
    int attempts = 0;
    do {
        attempts++;
        input = attempts * 5;
    } while (input < 10);
    printf("  第 %d 次获得有效输入: %d\n", attempts, input);

    printf("示例: 反转数字\n");
    int n = 12345;
    int reversed = 0;
    int temp = n;
    do {
        reversed = reversed * 10 + temp % 10;
        temp /= 10;
    } while (temp > 0);
    printf("  %d 反转后: %d\n", n, reversed);

    printf("\ndo-while适合的场景:\n");
    printf("  至少需要执行一次的情况\n");
    printf("  菜单选择(先显示菜单，再判断)\n");
    printf("  输入验证(先获取输入，再检查有效性)\n");

    printf("\n⚠️ do-while末尾必须有分号:\n");
    printf("  do { ... } while(条件);  ← 别忘了分号!\n");
}

void demo_break_continue(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  break 和 continue\n");
    printf("═══════════════════════════════════════\n\n");

    printf("break: 立即退出当前循环\n");
    printf("  示例: 找到第一个能被7整除的数\n");
    for (int i = 1; i <= 100; i++) {
        if (i % 7 == 0) {
            printf("  找到: %d\n", i);
            break;
        }
    }

    printf("\ncontinue: 跳过本次循环的剩余部分，进入下一次迭代\n");
    printf("  示例: 只打印奇数\n");
    printf("  1-10中的奇数: ");
    for (int i = 1; i <= 10; i++) {
        if (i % 2 == 0) {
            continue;
        }
        printf("%d ", i);
    }
    printf("\n");

    printf("\n⚠️ break和continue只影响最内层循环:\n");
    printf("  嵌套循环中，break只退出内层循环\n");
    printf("  如果需要退出多层循环，用标志变量或goto\n");

    printf("\nbreak也可用于switch语句:\n");
    printf("  switch中break退出switch块，不是循环\n");
}

void demo_nested_loops(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  嵌套循环\n");
    printf("═══════════════════════════════════════\n\n");

    printf("示例1: 九九乘法表(部分)\n");
    for (int i = 1; i <= 5; i++) {
        for (int j = 1; j <= i; j++) {
            printf("  %d×%d=%-2d", j, i, i * j);
        }
        printf("\n");
    }

    printf("\n示例2: 打印直角三角形\n");
    int rows = 5;
    for (int i = 1; i <= rows; i++) {
        for (int j = 1; j <= i; j++) {
            printf("* ");
        }
        printf("\n");
    }

    printf("\n示例3: 冒泡排序\n");
    int arr[] = {64, 34, 25, 12, 22, 11, 90};
    int n = 7;
    printf("  排序前: ");
    for (int i = 0; i < n; i++) printf("%d ", arr[i]);
    printf("\n");

    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            if (arr[j] > arr[j + 1]) {
                int tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
            }
        }
    }

    printf("  排序后: ");
    for (int i = 0; i < n; i++) printf("%d ", arr[i]);
    printf("\n");

    printf("\n嵌套循环的时间复杂度:\n");
    printf("  两层嵌套: O(n²)\n");
    printf("  三层嵌套: O(n³)\n");
    printf("  注意性能影响!\n");
}

int main(void) {
    demo_for_loop();
    demo_while_loop();
    demo_do_while_loop();
    demo_break_continue();
    demo_nested_loops();

    return 0;
}
