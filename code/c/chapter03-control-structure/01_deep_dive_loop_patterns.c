/** @file 01_deep_dive_loop_patterns.c
 *  @brief 深入理解循环模式：哨兵、标志、累加器、循环优化、无限循环、循环展开
 *  @description 对应文档: 03-control-structure | 常见循环设计模式、性能优化技巧、无限循环的正确写法
 *  编译命令: gcc -std=c17 01_deep_dive_loop_patterns.c -o 01_deep_dive_loop_patterns
 */

#include <stdio.h>
#include <stdlib.h>

void demo_sentinel_pattern(void) {
    printf("═══════════════════════════════════════\n");
    printf("  哨兵循环模式 (Sentinel Pattern)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("哨兵值: 特殊值标记数据结束\n\n");

    printf("示例: 求一组成绩的平均分(以-1结束)\n");
    int scores[] = {85, 92, 78, 95, 88, -1};
    int sum = 0, count = 0;
    int i = 0;
    while (scores[i] != -1) {
        sum += scores[i];
        count++;
        i++;
    }
    if (count > 0) {
        printf("  %d个成绩的平均分: %.1f\n", count, (double)sum / count);
    }

    printf("\n举一反三 —— 哨兵模式的常见应用:\n");
    printf("  1. 字符串以'\\0'结尾(C字符串的本质就是哨兵模式)\n");
    printf("  2. 链表以NULL指针结尾\n");
    printf("  3. 文件读取以EOF结尾\n");
    printf("  4. 用户输入以特定值(如-1, q)结束\n");

    printf("\n哨兵模式 vs 计数模式:\n");
    printf("  哨兵: 不知道数量，通过特殊值判断结束\n");
    printf("  计数: 已知数量，用计数器控制循环\n");
}

void demo_flag_pattern(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  标志循环模式 (Flag Pattern)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("用布尔标志控制循环退出:\n\n");

    printf("示例: 判断一个数是否为素数\n");
    int num = 97;
    int is_prime = 1;
    for (int i = 2; i * i <= num; i++) {
        if (num % i == 0) {
            is_prime = 0;
            break;
        }
    }
    printf("  %d %s素数\n", num, is_prime ? "是" : "不是");

    printf("\n示例: 在有序数组中查找(简化版)\n");
    int arr[] = {2, 5, 8, 12, 16, 23, 38, 56, 72, 91};
    int n = 10;
    int target = 23;
    int found = 0;
    int pos = -1;
    for (int i = 0; i < n && !found; i++) {
        if (arr[i] == target) {
            found = 1;
            pos = i;
        } else if (arr[i] > target) {
            found = 1;
        }
    }
    if (pos >= 0) {
        printf("  找到 %d 在位置 %d\n", target, pos);
    }

    printf("\n举一反三 —— 标志模式的应用:\n");
    printf("  1. 搜索: 找到目标设标志退出\n");
    printf("  2. 验证: 发现无效数据设标志\n");
    printf("  3. 收敛: 条件满足时设标志\n");
}

void demo_accumulator_pattern(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  累加器模式 (Accumulator Pattern)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("在循环中累积结果:\n\n");

    printf("1. 求和:\n");
    int sum = 0;
    for (int i = 1; i <= 100; i++) {
        sum += i;
    }
    printf("   1+2+...+100 = %d\n", sum);

    printf("\n2. 求阶乘:\n");
    long long factorial = 1;
    int n = 15;
    for (int i = 1; i <= n; i++) {
        factorial *= i;
    }
    printf("   %d! = %lld\n", n, factorial);

    printf("\n3. 求最大值:\n");
    int data[] = {34, 12, 56, 78, 23, 89, 45};
    int max = data[0];
    for (int i = 1; i < 7; i++) {
        if (data[i] > max) {
            max = data[i];
        }
    }
    printf("   最大值: %d\n", max);

    printf("\n4. 计数:\n");
    int even_count = 0;
    for (int i = 0; i < 7; i++) {
        if (data[i] % 2 == 0) {
            even_count++;
        }
    }
    printf("   偶数个数: %d\n", even_count);

    printf("\n5. 字符串长度:\n");
    char str[] = "Hello, C!";
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    printf("   \"%s\" 的长度: %d\n", str, len);
}

void demo_infinite_loops(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  无限循环\n");
    printf("═══════════════════════════════════════\n\n");

    printf("三种写法:\n");
    printf("  1. while(1) { ... }    —— 最常见，意图清晰\n");
    printf("  2. for(;;) { ... }     —— 经典C风格\n");
    printf("  3. do { ... } while(1) —— 至少执行一次\n\n");

    printf("推荐: while(1) 或 for(;;)\n");
    printf("  两者等价，for(;;)在C中是惯用写法\n");
    printf("  while(1)在现代代码中更常见，意图更明确\n\n");

    printf("无限循环的合法使用场景:\n");
    printf("  1. 事件循环 (GUI/网络服务器)\n");
    printf("  2. 嵌入式系统的超级循环\n");
    printf("  3. 游戏主循环\n");
    printf("  4. 命令行交互循环\n\n");

    printf("示例: 简单命令循环(模拟3次后退出)\n");
    int cmd_count = 0;
    while (1) {
        cmd_count++;
        printf("  执行命令 #%d\n", cmd_count);
        if (cmd_count >= 3) {
            printf("  收到退出指令\n");
            break;
        }
    }

    printf("\n⚠️ 避免意外创建无限循环:\n");
    printf("  int i = 0;\n");
    printf("  while (i < 10);\n");  // 注意这个分号!
    printf("      i++;  // 这行不在循环内!\n");
}

void demo_loop_optimization(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  循环优化技巧\n");
    printf("═══════════════════════════════════════\n\n");

    printf("1. 循环不变量外提 (Loop Invariant Code Motion):\n");
    printf("   ✗ for(i=0; i<n*factor; i++)  // 每次都计算n*factor\n");
    printf("   ✓ int limit = n * factor;\n");
    printf("     for(i=0; i<limit; i++)     // 只计算一次\n\n");

    printf("2. 强度削减 (Strength Reduction):\n");
    printf("   ✗ for(i=0; i<n; i++) sum += i * 4;  // 每次乘法\n");
    printf("   ✓ for(i=0; i<n*4; i+=4) sum += i;   // 用加法替代乘法\n\n");

    printf("3. 循环展开 (Loop Unrolling):\n");
    int arr[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int sum = 0;

    printf("   普通循环:\n");
    printf("   for(i=0; i<8; i++) sum += arr[i];\n");
    sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += arr[i];
    }
    printf("   结果: %d\n", sum);

    printf("\n   手动展开(4倍):\n");
    printf("   for(i=0; i<8; i+=4) sum += arr[i]+arr[i+1]+arr[i+2]+arr[i+3];\n");
    sum = 0;
    for (int i = 0; i < 8; i += 4) {
        sum += arr[i] + arr[i + 1] + arr[i + 2] + arr[i + 3];
    }
    printf("   结果: %d\n", sum);

    printf("\n   展开的利弊:\n");
    printf("   ✓ 减少循环控制开销(条件判断、跳转)\n");
    printf("   ✓ 可能更好利用CPU流水线\n");
    printf("   ✗ 代码体积增大(可能影响指令缓存)\n");
    printf("   ✗ 现代编译器通常自动展开，手动展开意义不大\n\n");

    printf("4. 减少循环内的函数调用:\n");
    printf("   ✗ for(i=0; i<strlen(s); i++)  // 每次调用strlen!\n");
    printf("   ✓ int len = strlen(s);\n");
    printf("     for(i=0; i<len; i++)        // 只调用一次\n\n");

    printf("5. 数据局部性 (Cache友好):\n");
    printf("   ✓ 按行遍历二维数组(内存连续访问)\n");
    printf("   ✗ 按列遍历(缓存不友好)\n\n");

    printf("⚠️ 优化原则: 先写正确，再优化。用性能分析工具找到瓶颈再优化!\n");
}

int main(void) {
    demo_sentinel_pattern();
    demo_flag_pattern();
    demo_accumulator_pattern();
    demo_infinite_loops();
    demo_loop_optimization();

    return 0;
}
