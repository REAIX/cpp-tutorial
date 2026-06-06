/** @file 02_deep_dive_recursion_patterns.c
 *  @brief 深入理解递归模式：尾递归、互递归、递归vs迭代、栈溢出预防
 *  @description 对应文档: 04-function | 各种递归模式详解、递归优化、递归转迭代技巧
 *  编译命令: gcc -std=c17 02_deep_dive_recursion_patterns.c -o 02_deep_dive_recursion_patterns
 */

#include <stdio.h>
#include <stdlib.h>

void demo_tail_recursion(void) {
    printf("═══════════════════════════════════════\n");
    printf("  尾递归(Tail Recursion)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("尾递归: 递归调用是函数的最后一个操作\n");
    printf("尾递归可以被编译器优化为循环(不消耗额外栈空间)\n\n");

    long long factorial_naive(int n) {
        if (n <= 1) return 1;
        return n * factorial_naive(n - 1);
    }

    long long factorial_tail(int n, long long acc) {
        if (n <= 1) return acc;
        return factorial_tail(n - 1, acc * n);
    }

    printf("普通递归阶乘:\n");
    printf("  int fact(n) { return n * fact(n-1); }\n");
    printf("  递归返回后还要做乘法 → 不是尾递归\n\n");

    printf("尾递归阶乘(用累加器参数):\n");
    printf("  int fact(n, acc) { return fact(n-1, acc*n); }\n");
    printf("  递归返回后无需额外操作 → 尾递归\n\n");

    printf("  fact_naive(10) = %lld\n", factorial_naive(10));
    printf("  fact_tail(10, 1) = %lld\n", factorial_tail(10, 1));

    printf("\n尾递归优化(TCO)的条件:\n");
    printf("  1. 递归调用必须是最后一个操作\n");
    printf("  2. 返回值直接来自递归调用(不再做运算)\n");
    printf("  3. 编译器必须支持TCO(GCC -O2默认开启)\n\n");

    printf("将普通递归转为尾递归的技巧:\n");
    printf("  引入累加器参数，将计算结果向下传递\n");
    printf("  普通递归: 结果在返回时计算(向上传递)\n");
    printf("  尾递归:   结果在参数中计算(向下传递)\n");
}

int is_even(int n);
int is_odd(int n);

int is_even(int n) {
    if (n == 0) return 1;
    return is_odd(n - 1);
}

int is_odd(int n) {
    if (n == 0) return 0;
    return is_even(n - 1);
}

void demo_mutual_recursion(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  互递归(Mutual Recursion)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("互递归: 两个或多个函数互相调用\n\n");

    printf("互递归判断奇偶:\n");
    for (int i = 0; i <= 5; i++) {
        printf("  %d 是%s数\n", i, is_even(i) ? "偶" : "奇");
    }

    printf("\n互递归的问题:\n");
    printf("  ✗ 栈开销大(每个数需要n/2次函数调用)\n");
    printf("  ✗ 两个函数必须互相知道对方的声明\n");
    printf("  ✗ 不如直接 n%%2 高效\n\n");

    printf("互递归的实际应用:\n");
    printf("  1. 语法分析器: 表达式/项/因子的递归下降\n");
    printf("  2. 状态机: 状态间的转换\n");
    printf("  3. 树遍历: 不同节点类型的处理\n");
}

void demo_recursion_vs_iteration(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  递归 vs 迭代\n");
    printf("═══════════════════════════════════════\n\n");

    printf("斐波那契数列 —— 三种实现对比:\n\n");

    long long fib_recursive(int n) {
        if (n <= 0) return 0;
        if (n == 1) return 1;
        return fib_recursive(n - 1) + fib_recursive(n - 2);
    }

    long long fib_iterative(int n) {
        if (n <= 0) return 0;
        if (n == 1) return 1;
        long long prev2 = 0, prev1 = 1;
        long long current = 0;
        for (int i = 2; i <= n; i++) {
            current = prev1 + prev2;
            prev2 = prev1;
            prev1 = current;
        }
        return current;
    }

    long long fib_memoized(int n, long long *cache) {
        if (n <= 0) return 0;
        if (n == 1) return 1;
        if (cache[n] != -1) return cache[n];
        cache[n] = fib_memoized(n - 1, cache) + fib_memoized(n - 2, cache);
        return cache[n];
    }

    printf("1. 朴素递归: O(2^n) 时间, O(n) 空间\n");
    printf("   fib_recursive(30) = %lld\n", fib_recursive(30));

    printf("\n2. 迭代: O(n) 时间, O(1) 空间\n");
    printf("   fib_iterative(30) = %lld\n", fib_iterative(30));

    printf("\n3. 记忆化递归: O(n) 时间, O(n) 空间\n");
    long long cache[51];
    for (int i = 0; i < 51; i++) cache[i] = -1;
    printf("   fib_memoized(30) = %lld\n", fib_memoized(30, cache));

    printf("\n对比总结:\n");
    printf("  方法          时间复杂度   空间复杂度   特点\n");
    printf("  ──────────────────────────────────────────────\n");
    printf("  朴素递归      O(2^n)      O(n)        简洁但极慢\n");
    printf("  记忆化递归    O(n)        O(n)        空间换时间\n");
    printf("  尾递归        O(n)        O(1)*       需编译器TCO\n");
    printf("  迭代          O(n)        O(1)        最高效\n");
    printf("  矩阵快速幂    O(log n)    O(1)        数学方法\n\n");
    printf("  * 尾递归空间O(1)需要编译器优化支持\n");
}

void demo_recursion_to_iteration(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  递归转迭代的通用技巧\n");
    printf("═══════════════════════════════════════\n\n");

    printf("技巧1: 用循环替代单递归\n");
    printf("  递归: fact(n) = n * fact(n-1)\n");
    printf("  迭代:\n");
    long long fact_iter(int n) {
        long long result = 1;
        for (int i = 2; i <= n; i++) {
            result *= i;
        }
        return result;
    }
    printf("  fact_iter(10) = %lld\n", fact_iter(10));

    printf("\n技巧2: 用显式栈模拟递归\n");
    printf("  递归: 二叉树遍历\n");
    printf("  迭代: 用数组模拟栈\n\n");

    printf("技巧3: 用累加器参数转尾递归\n");
    printf("  递归: sum(n) = n + sum(n-1)\n");
    printf("  尾递归: sum(n, acc) = sum(n-1, acc+n)\n");
    int sum_tail(int n, int acc) {
        if (n <= 0) return acc;
        return sum_tail(n - 1, acc + n);
    }
    printf("  sum_tail(100, 0) = %d\n", sum_tail(100, 0));

    printf("\n技巧4: 记忆化(自顶向下) → 制表法(自底向上)\n");
    printf("  记忆化: 递归+缓存\n");
    printf("  制表法: 从小到大填表\n");
    long long fib_tabulation(int n) {
        if (n <= 0) return 0;
        long long *dp = calloc(n + 1, sizeof(long long));
        dp[1] = 1;
        for (int i = 2; i <= n; i++) {
            dp[i] = dp[i - 1] + dp[i - 2];
        }
        long long result = dp[n];
        free(dp);
        return result;
    }
    printf("  fib_tabulation(30) = %lld\n", fib_tabulation(30));
}

void demo_stack_overflow_prevention(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  栈溢出预防\n");
    printf("═══════════════════════════════════════\n\n");

    printf("1. 限制递归深度:\n");
    int safe_recursive(int n, int depth, int max_depth) {
        if (depth > max_depth) {
            printf("  递归深度超过限制 %d!\n", max_depth);
            return -1;
        }
        if (n <= 0) return 0;
        return 1 + safe_recursive(n - 1, depth + 1, max_depth);
    }
    printf("  safe_recursive(5, 0, 1000) = %d\n", safe_recursive(5, 0, 1000));

    printf("\n2. 尾递归优化:\n");
    printf("   GCC: -O2 -foptimize-sibling-calls\n");
    printf("   Clang: -O2 默认开启\n");
    printf("   MSVC: 不保证TCO\n\n");

    printf("3. 改用迭代:\n");
    printf("   任何递归都可以转为迭代(用显式栈)\n");
    printf("   简单递归(如阶乘)直接用循环\n\n");

    printf("4. 增大栈大小:\n");
    printf("   Linux: ulimit -s 65536 (设为64MB)\n");
    printf("   Windows: 链接器选项 /STACK:大小\n");
    printf("   pthread: pthread_attr_setstacksize()\n\n");

    printf("5. 使用堆分配:\n");
    printf("   大数组: 用malloc而非局部数组\n");
    printf("   深度递归: 用显式栈(数组)模拟\n\n");

    printf("何时选择递归 vs 迭代:\n");
    printf("  递归适合:\n");
    printf("    ✓ 问题天然递归(树遍历、分治、回溯)\n");
    printf("    ✓ 递归深度可控(如log n深度)\n");
    printf("    ✓ 代码清晰度更重要时\n");
    printf("  迭代适合:\n");
    printf("    ✓ 性能关键路径\n");
    printf("    ✓ 递归深度可能很大\n");
    printf("    ✓ 嵌入式/资源受限环境\n");
}

int main(void) {
    demo_tail_recursion();
    demo_mutual_recursion();
    demo_recursion_vs_iteration();
    demo_recursion_to_iteration();
    demo_stack_overflow_prevention();

    return 0;
}
