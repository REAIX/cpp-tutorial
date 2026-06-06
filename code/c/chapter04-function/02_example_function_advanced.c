/** @file 02_example_function_advanced.c
 *  @brief 函数进阶：递归、静态局部变量、函数指针参数
 *  @description 对应文档: 04-function | 演示递归、static变量在函数中的用法、函数指针作为参数
 *  编译命令: gcc -std=c17 02_example_function_advanced.c -o 02_example_function_advanced
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void demo_recursion_basics(void) {
    printf("═══════════════════════════════════════\n");
    printf("  递归基础\n");
    printf("═══════════════════════════════════════\n\n");

    printf("递归: 函数直接或间接调用自身\n");
    printf("递归三要素:\n");
    printf("  1. 递归终止条件(基准情况)\n");
    printf("  2. 递归步骤(问题规模缩小)\n");
    printf("  3. 递归调用\n\n");

    long long fib(int n) {
        if (n <= 0) return 0;
        if (n == 1) return 1;
        return fib(n - 1) + fib(n - 2);
    }

    printf("斐波那契数列(递归版):\n  ");
    for (int i = 0; i < 15; i++) {
        printf("%lld ", fib(i));
    }
    printf("\n");

    printf("\n递归求阶乘:\n");
    long long fact(int n) {
        if (n <= 1) return 1;
        return n * fact(n - 1);
    }
    printf("  10! = %lld\n", fact(10));

    printf("\n⚠️ 上述fib递归效率极低: O(2^n)\n");
    printf("  fib(40)需要约10亿次调用!\n");
}

void demo_recursion_practical(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  递归实用示例\n");
    printf("═══════════════════════════════════════\n\n");

    printf("1. 递归打印整数的每一位:\n");
    void print_digits(int n) {
        if (n < 0) {
            printf("-");
            n = -n;
        }
        if (n >= 10) {
            print_digits(n / 10);
        }
        printf("%d ", n % 10);
    }
    printf("  12345 → ");
    print_digits(12345);
    printf("\n");

    printf("\n2. 递归求幂:\n");
    double power(double base, int exp) {
        if (exp == 0) return 1.0;
        if (exp < 0) return 1.0 / power(base, -exp);
        if (exp % 2 == 0) {
            double half = power(base, exp / 2);
            return half * half;
        }
        return base * power(base, exp - 1);
    }
    printf("  2^10 = %.0f\n", power(2, 10));
    printf("  3^5 = %.0f\n", power(3, 5));
    printf("  (快速幂: O(log n) 复杂度)\n");

    printf("\n3. 递归反转字符串:\n");
    void reverse_print(const char *s) {
        if (*s == '\0') return;
        reverse_print(s + 1);
        printf("%c", *s);
    }
    printf("  \"Hello\" 反转: ");
    reverse_print("Hello");
    printf("\n");

    printf("\n4. 汉诺塔:\n");
    int hanoi_moves = 0;
    void hanoi(int n, char from, char via, char to) {
        if (n == 1) {
            hanoi_moves++;
            return;
        }
        hanoi(n - 1, from, to, via);
        hanoi_moves++;
        hanoi(n - 1, via, from, to);
    }
    hanoi(5, 'A', 'B', 'C');
    printf("  5层汉诺塔需要 %d 步\n", hanoi_moves);
}

void demo_static_in_function(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  函数中的 static 局部变量\n");
    printf("═══════════════════════════════════════\n\n");

    printf("static局部变量的特点:\n");
    printf("  ✓ 生命周期: 整个程序运行期间\n");
    printf("  ✓ 作用域: 仍限于函数内部\n");
    printf("  ✓ 初始化: 只在第一次调用时初始化\n");
    printf("  ✓ 默认值: 自动初始化为0\n\n");

    int counter(void) {
        static int count = 0;
        count++;
        return count;
    }

    printf("调用counter()5次:\n");
    for (int i = 0; i < 5; i++) {
        printf("  第%d次调用: 返回%d\n", i + 1, counter());
    }

    printf("\n实用场景: 生成唯一ID\n");
    int generate_id(void) {
        static int next_id = 1000;
        return next_id++;
    }
    printf("  ID1 = %d\n", generate_id());
    printf("  ID2 = %d\n", generate_id());
    printf("  ID3 = %d\n", generate_id());

    printf("\n举一反三 —— static的两种用法:\n");
    printf("  1. static局部变量: 延长生命周期，保持状态\n");
    printf("  2. static全局变量/函数: 限制作用域为当前文件(内部链接)\n");
}

void demo_function_pointer_param(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  函数指针作为参数\n");
    printf("═══════════════════════════════════════\n\n");

    int add(int a, int b) { return a + b; }
    int subtract(int a, int b) { return a - b; }
    int multiply(int a, int b) { return a * b; }

    int compute(int x, int y, int (*operation)(int, int)) {
        return operation(x, y);
    }

    printf("函数指针参数: int (*operation)(int, int)\n\n");

    int a = 10, b = 3;
    printf("  compute(%d, %d, add)      = %d\n", a, b, compute(a, b, add));
    printf("  compute(%d, %d, subtract) = %d\n", a, b, compute(a, b, subtract));
    printf("  compute(%d, %d, multiply) = %d\n", a, b, compute(a, b, multiply));

    printf("\n实际应用: qsort的比较函数\n");
    int arr[] = {5, 2, 8, 1, 9, 3};
    int n = 6;

    int compare_asc(const void *a, const void *b) {
        return (*(const int *)a - *(const int *)b);
    }

    qsort(arr, n, sizeof(int), compare_asc);
    printf("  升序排序: ");
    for (int i = 0; i < n; i++) printf("%d ", arr[i]);
    printf("\n");

    printf("\n回调函数模式:\n");
    void foreach(int *arr, int n, void (*callback)(int)) {
        for (int i = 0; i < n; i++) {
            callback(arr[i]);
        }
    }

    void print_element(int x) {
        printf("  [%d] ", x);
    }

    printf("  遍历数组: ");
    foreach(arr, n, print_element);
    printf("\n");
}

int main(void) {
    demo_recursion_basics();
    demo_recursion_practical();
    demo_static_in_function();
    demo_function_pointer_param();

    return 0;
}
