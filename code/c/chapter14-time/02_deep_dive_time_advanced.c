/**
 * @file 02_deep_dive_time_advanced.c
 * @brief 时间高级主题
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

void demo_high_resolution_timer(void) {
    printf("=== 高分辨率计时器 ===\n");
#ifdef _WIN32
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    printf("  Windows: QueryPerformanceCounter可用\n");
    printf("  频率: %lld Hz (精度: %.3f 纳秒)\n",
           (long long)freq.QuadPart, 1e9 / (double)freq.QuadPart);
    printf("  当前计数: %lld\n\n", (long long)count.QuadPart);
#else
    printf("  C11: timespec_get() + TIME_UTC (纳秒级)\n");
    printf("  POSIX: clock_gettime(CLOCK_MONOTONIC, &ts)\n\n");
#endif
}

void demo_monotonic_vs_wall_clock(void) {
    printf("=== 单调时钟 vs 墙上时钟 ===\n");
    printf("  墙上时钟(wall clock): time(), gettimeofday()\n");
    printf("    - 可被NTP/系统时间修改回拨\n");
    printf("    - 不适合测量经过时间!\n\n");
    printf("  单调时钟(monotonic clock):\n");
#ifdef _WIN32
    printf("    Windows: QueryPerformanceCounter (单调递增)\n");
#else
    printf("    POSIX: clock_gettime(CLOCK_MONOTONIC) (单调递增)\n");
#endif
    printf("    - 不会被修改, 只会单调递增\n");
    printf("    - 适合测量经过时间\n\n");

    printf("  陷阱示例:\n");
    printf("    start = time(NULL);\n");
    printf("    // ... 做一些事 ...\n");
    printf("    end = time(NULL);\n");
    printf("    elapsed = end - start;  // 如果系统时间被回拨, 可能为负!\n\n");

    printf("  正确做法:\n");
#ifdef _WIN32
    printf("    QueryPerformanceCounter(&start);\n");
#else
    printf("    clock_gettime(CLOCK_MONOTONIC, &start);\n");
#endif
    printf("    // ... 做一些事 ...\n");
#ifdef _WIN32
    printf("    QueryPerformanceCounter(&end);\n");
#else
    printf("    clock_gettime(CLOCK_MONOTONIC, &end);\n");
#endif
    printf("    elapsed = end - start;  // 保证非负\n\n");
}

void demo_time_drift(void) {
    printf("=== 时间漂移 ===\n");
    printf("  时间漂移原因:\n");
    printf("    1. 系统时钟精度有限(通常15.6ms分辨率 on Windows)\n");
    printf("    2. 系统调用的开销和不确定性\n");
    printf("    3. CPU频率动态调节(省电模式)\n");
    printf("    4. NTP校时导致的时间跳变\n\n");

    printf("  减少漂移影响的方法:\n");
    printf("    - 使用高精度计时器\n");
    printf("    - 多次测量取平均值\n");
    printf("    - 使用单调时钟\n");
    printf("    - 避免在计时中做I/O操作\n\n");

    double t1 = get_time_sec();
    double t2 = get_time_sec();
    double min_resolution = t2 - t1;
    printf("  两次连续计时器调用间隔: %.9f 秒\n", min_resolution);
    printf("  (这反映了本系统计时器的实际分辨率)\n\n");
}

void demo_profiling(void) {
    printf("=== 使用时间做性能剖析 ===\n");

    typedef struct {
        const char *name;
        double total;
        int calls;
    } profile_entry_t;

    profile_entry_t entries[3] = {
        {"整数运算", 0, 0},
        {"浮点运算", 0, 0},
        {"数学函数", 0, 0},
    };

    double ts_start, ts_end;

    ts_start = get_time_sec();
    volatile int x = 0;
    for (int i = 0; i < 10000000; i++) x += i;
    ts_end = get_time_sec();
    entries[0].total = ts_end - ts_start;
    entries[0].calls = 10000000;

    ts_start = get_time_sec();
    volatile double y = 0;
    for (int i = 0; i < 10000000; i++) y += i * 0.001;
    ts_end = get_time_sec();
    entries[1].total = ts_end - ts_start;
    entries[1].calls = 10000000;

    ts_start = get_time_sec();
    volatile double z = 0;
    for (int i = 0; i < 1000000; i++) z += sqrt((double)i);
    ts_end = get_time_sec();
    entries[2].total = ts_end - ts_start;
    entries[2].calls = 1000000;

    printf("  %-12s %10s %12s %12s\n", "操作", "调用次数", "总耗时(秒)", "单次(纳秒)");
    printf("  %-12s %10s %12s %12s\n", "--------", "----------", "----------", "----------");
    for (int i = 0; i < 3; i++) {
        double per_call = entries[i].total / entries[i].calls * 1e9;
        printf("  %-12s %10d %12.6f %12.2f\n",
               entries[i].name, entries[i].calls, entries[i].total, per_call);
    }
    printf("\n");
}

void demo_time_best_practices(void) {
    printf("=== 时间处理最佳实践 ===\n");
    printf("  1. 测量经过时间用单调时钟, 不要用time()\n");
    printf("  2. 存储时间用UTC时间戳, 显示时再转本地时间\n");
    printf("  3. 注意time_t 2038年问题(32位溢出)\n");
    printf("  4. 不要假设1分钟=60秒(闰秒)\n");
    printf("  5. 性能测试多次运行取中位数(非平均值)\n");
    printf("  6. 注意夏令时转换导致的时间跳跃\n");
    printf("  7. 线程安全: 用_r后缀版本(localtime_r等)\n\n");
}

int main(void) {
    printf("========== 时间高级主题 ==========\n\n");

    demo_high_resolution_timer();
    demo_monotonic_vs_wall_clock();
    demo_time_drift();
    demo_profiling();
    demo_time_best_practices();

    printf("========== 所有演示完成 ==========\n");
    return 0;
}
