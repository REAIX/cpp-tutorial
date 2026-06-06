/**
 * @file 03_example_time_measure.c
 * @brief 时间测量与性能计时
 * @description 对应文档: 14-时间处理
 */
#include <stdio.h>
#include <time.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#endif

static double get_time_sec(void) {
#ifdef _WIN32
    static LARGE_INTEGER freq = {0};
    static int initialized = 0;
    if (!initialized) {
        QueryPerformanceFrequency(&freq);
        initialized = 1;
    }
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    return (double)count.QuadPart / (double)freq.QuadPart;
#else
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ts.tv_sec + ts.tv_nsec / 1e9;
#endif
}

void demo_clock_measurement(void) {
    printf("=== clock()测量CPU时间 ===\n");
    clock_t start = clock();

    volatile double result = 0;
    for (int i = 1; i <= 10000000; i++) {
        result += sqrt((double)i);
    }

    clock_t end = clock();
    double cpu_time = (double)(end - start) / CLOCKS_PER_SEC;
    printf("  计算sqrt 1000万次\n");
    printf("  CPU时间: %.4f 秒\n", cpu_time);
    printf("  (result=%.2f, 防止优化消除)\n\n", result);
}

void demo_high_resolution_timer(void) {
    printf("=== 高分辨率计时器 ===\n");
    double t1 = get_time_sec();
    double t2 = get_time_sec();
    printf("  两次连续调用间隔: %.9f 秒\n", t2 - t1);
    printf("  实现方式:\n");
#ifdef _WIN32
    printf("    Windows: QueryPerformanceCounter (高精度)\n");
#else
    printf("    POSIX: timespec_get (纳秒级)\n");
#endif
    printf("\n");
}

void demo_wall_clock_vs_cpu(void) {
    printf("=== 墙上时钟时间 vs CPU时间 ===\n");

    double wall_start = get_time_sec();
    clock_t cpu_start = clock();

    volatile double sum = 0;
    for (int i = 0; i < 5000000; i++) {
        sum += sin((double)i) * cos((double)i);
    }

    clock_t cpu_end = clock();
    double wall_end = get_time_sec();

    double cpu_elapsed = (double)(cpu_end - cpu_start) / CLOCKS_PER_SEC;
    double wall_elapsed = wall_end - wall_start;

    printf("  CPU时间:  %.6f 秒\n", cpu_elapsed);
    printf("  墙钟时间: %.6f 秒\n", wall_elapsed);
    printf("  差异原因: 墙钟时间包含I/O等待、调度等\n\n");
}

void benchmark_comparison(void) {
    printf("=== 基准测试对比 ===\n");

    double start, end;

    start = get_time_sec();
    volatile int x = 0;
    for (int i = 0; i < 100000000; i++) {
        x += i;
    }
    end = get_time_sec();
    printf("  整数加法1亿次: %.4f 秒\n", end - start);

    start = get_time_sec();
    volatile double y = 0;
    for (int i = 0; i < 10000000; i++) {
        y += sqrt((double)i);
    }
    end = get_time_sec();
    printf("  sqrt计算1千万次: %.4f 秒\n", end - start);

    start = get_time_sec();
    volatile double z = 0;
    for (int i = 0; i < 10000000; i++) {
        z += sin((double)i);
    }
    end = get_time_sec();
    printf("  sin计算1千万次: %.4f 秒\n\n", end - start);
}

void demo_timer_precision(void) {
    printf("=== 计时器精度比较 ===\n");
    printf("  time():        秒级精度 (1秒)\n");
    printf("  clock():       CLOCKS_PER_SEC精度 (通常1微秒)\n");
#ifdef _WIN32
    printf("  QPC:           纳秒级精度 (QueryPerformanceCounter)\n");
#else
    printf("  timespec_get():纳秒级精度 (实际取决于系统)\n");
#endif
    printf("  选择建议:\n");
    printf("    - 粗略计时: time()\n");
    printf("    - CPU时间: clock()\n");
    printf("    - 高精度: QPC(Windows) / timespec_get(POSIX)\n\n");
}

int main(void) {
    printf("========== 时间测量与性能计时示例 ==========\n\n");

    demo_clock_measurement();
    demo_high_resolution_timer();
    demo_wall_clock_vs_cpu();
    benchmark_comparison();
    demo_timer_precision();

    printf("========== 所有演示完成 ==========\n");
    return 0;
}
