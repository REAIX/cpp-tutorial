/**
 * @file 02_example_gcc_optimization.c
 * @brief GCC优化级别对比示例
 * @description 对应文档: 21-gcc-g++
 *              演示不同优化级别对代码生成的影响，
 *              包括 -O0, -O1, -O2, -O3, -Os, -Og 的对比
 *
 * 编译示例:
 *   gcc -O0 -S 02_example_gcc_optimization.c -o opt_O0.s
 *   gcc -O1 -S 02_example_gcc_optimization.c -o opt_O1.s
 *   gcc -O2 -S 02_example_gcc_optimization.c -o opt_O2.s
 *   gcc -O3 -S 02_example_gcc_optimization.c -o opt_O3.s
 *   gcc -Os -S 02_example_gcc_optimization.c -o opt_Os.s
 *   gcc -Og -S 02_example_gcc_optimization.c -o opt_Og.s
 *
 * 对比汇编输出:
 *   diff opt_O0.s opt_O2.s
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 10000
#define ITERATIONS 100000

static int data[ARRAY_SIZE];

void init_data(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = rand() % 1000;
    }
}

/* ========================================================================
 * 示例函数: 不同优化级别会有不同的代码生成策略
 * ======================================================================== */

int sum_array(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int sum_with_barrier(int *arr, int n) {
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int find_max(int *arr, int n) {
    int max_val = arr[0];
    for (int i = 1; i < n; i++) {
        if (arr[i] > max_val) {
            max_val = arr[i];
        }
    }
    return max_val;
}

int count_above(int *arr, int n, int threshold) {
    int count = 0;
    for (int i = 0; i < n; i++) {
        if (arr[i] > threshold) {
            count++;
        }
    }
    return count;
}

long long fibonacci_iter(int n) {
    if (n <= 0) return 0;
    if (n == 1) return 1;
    long long prev = 0;
    long long curr = 1;
    for (int i = 2; i <= n; i++) {
        long long next = prev + curr;
        prev = curr;
        curr = next;
    }
    return curr;
}

/* ========================================================================
 * 演示函数
 * ======================================================================== */

void demo_optimization_levels(void) {
    printf("===== 优化级别详解 =====\n\n");

    printf("-O0: 不优化 (默认)\n");
    printf("  - 完全按照源码生成代码\n");
    printf("  - 最适合调试，代码执行顺序与源码一致\n");
    printf("  - 生成的代码最慢、体积最大\n");
    printf("  - 变量存储在栈上，每次都从内存读取\n\n");

    printf("-O1: 基础优化\n");
    printf("  - 减少代码体积，提高速度\n");
    printf("  - 开启约40个优化pass\n");
    printf("  - 包括: 死代码消除、常量折叠、内联小函数\n");
    printf("  - 编译时间增加不多\n\n");

    printf("-O2: 标准优化 (最常用)\n");
    printf("  - 在-O1基础上增加更多优化\n");
    printf("  - 约50+个优化pass\n");
    printf("  - 包括: 循环优化、指令调度、分支预测\n");
    printf("  - 不会显著增加代码体积\n");
    printf("  - 推荐用于生产构建\n\n");

    printf("-O3: 激进优化\n");
    printf("  - 在-O2基础上开启更激进的优化\n");
    printf("  - 包括: 函数内联、循环向量化、循环展开\n");
    printf("  - 可能增大代码体积（指令缓存压力）\n");
    printf("  - 某些情况下反而比-O2慢\n\n");

    printf("-Os: 优化代码体积\n");
    printf("  - 大部分-O2优化，但关闭增大体积的优化\n");
    printf("  - 适合嵌入式系统、存储受限环境\n");
    printf("  - 更小的代码 = 更好的指令缓存利用率\n\n");

    printf("-Og: 优化调试体验\n");
    printf("  - GCC 6+ 引入\n");
    printf("  - 开启不影响调试的优化\n");
    printf("  - 比-O0快，但仍可正常调试\n");
    printf("  - 推荐用于开发阶段\n\n");
}

void demo_optimization_comparison(void) {
    printf("===== 优化效果实际对比 =====\n\n");

    init_data();

    clock_t start, end;

    start = clock();
    long long total_sum = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        total_sum += sum_array(data, ARRAY_SIZE);
    }
    end = clock();
    double time_sum = (double)(end - start) / CLOCKS_PER_SEC;

    start = clock();
    long long total_volatile = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        total_volatile += sum_with_barrier(data, ARRAY_SIZE);
    }
    end = clock();
    double time_volatile = (double)(end - start) / CLOCKS_PER_SEC;

    printf("sum_array (普通):           结果=%lld, 耗时=%.4f秒\n", total_sum, time_sum);
    printf("sum_with_barrier (volatile): 结果=%lld, 耗时=%.4f秒\n", total_volatile, time_volatile);
    printf("volatile 阻止了优化，每次循环都必须写回内存\n\n");

    printf("volatile 的含义:\n");
    printf("  告诉编译器该变量可能被外部修改，不要缓存到寄存器\n");
    printf("  适用场景: 硬件寄存器、信号处理、多线程共享变量\n");
    printf("  注意: volatile 不保证原子性，不替代锁\n\n");
}

void demo_specific_optimizations(void) {
    printf("===== 具体优化技术说明 =====\n\n");

    printf("1. 常量折叠 (Constant Folding)\n");
    printf("   int x = 3 + 5;  →  int x = 8;  // 编译时计算\n\n");

    printf("2. 死代码消除 (Dead Code Elimination)\n");
    printf("   if (0) { foo(); }  →  删除整个if块\n\n");

    printf("3. 函数内联 (Function Inlining)\n");
    printf("   小函数的调用被替换为函数体本身\n");
    printf("   消除函数调用开销，但可能增大代码体积\n\n");

    printf("4. 循环展开 (Loop Unrolling)\n");
    printf("   for (i=0; i<4; i++) a[i]=0;\n");
    printf("   → a[0]=0; a[1]=0; a[2]=0; a[3]=0;\n");
    printf("   减少循环控制开销，增加指令级并行\n\n");

    printf("5. 循环向量化 (Auto-Vectorization)\n");
    printf("   使用SIMD指令(SSE/AVX)一次处理多个数据\n");
    printf("   -O3 或 -O2 -ftree-vectorize 开启\n");
    printf("   查看向量化报告: -fopt-info-vec\n\n");

    printf("6. 尾调用优化 (Tail Call Optimization)\n");
    printf("   递归调用是函数最后一步时，复用当前栈帧\n");
    printf("   将递归转换为循环，避免栈溢出\n\n");

    printf("7. 链接时优化 (Link-Time Optimization, LTO)\n");
    printf("   -flto 选项，跨编译单元优化\n");
    printf("   可以内联其他 .c 文件中的函数\n\n");
}

void demo_optimization_tips(void) {
    printf("===== 优化实践建议 =====\n\n");

    printf("1. 先写正确的代码，再考虑优化\n");
    printf("   过早优化是万恶之源 —— Donald Knuth\n\n");

    printf("2. 用性能分析工具定位瓶颈\n");
    printf("   gcc -pg program.c -o program    # 插入性能分析代码\n");
    printf("   ./program                        # 运行生成 gmon.out\n");
    printf("   gprof program gmon.out           # 分析结果\n\n");

    printf("3. 对比不同优化级别的效果\n");
    printf("   gcc -O2 -S program.c -o O2.s\n");
    printf("   gcc -O3 -S program.c -o O3.s\n");
    printf("   diff O2.s O3.s                   # 查看差异\n\n");

    printf("4. 使用 -fomit-frame-pointer 节省栈帧\n");
    printf("   -O1及以上默认开启（不影响调试的平台上）\n\n");

    printf("5. 使用 __attribute__((optimize)) 控制单函数优化\n");
    printf("   __attribute__((optimize(\"O3\")))\n");
    printf("   int hot_function(void) { ... }\n\n");

    printf("6. 使用 likely/unlikely 提示分支预测\n");
    printf("   #define likely(x)   __builtin_expect(!!(x), 1)\n");
    printf("   #define unlikely(x) __builtin_expect(!!(x), 0)\n\n");

    printf("7. 查看优化决策\n");
    printf("   -fopt-info              所有优化信息\n");
    printf("   -fopt-info-vec          向量化信息\n");
    printf("   -fopt-info-inline       内联信息\n");
    printf("\n");
}

int main(void) {
    printf("========== GCC 优化级别对比示例 ==========\n\n");

    demo_optimization_levels();
    demo_optimization_comparison();
    demo_specific_optimizations();
    demo_optimization_tips();

    printf("========== 程序结束 ==========\n");
    return 0;
}
