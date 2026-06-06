/**
 * @file 01_deep_dive_time_patterns.c
 * @brief 时间处理模式深入
 * @description 对应文档: 14-时间处理
 */
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

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

typedef struct {
    double start;
    double end;
    int running;
} my_timer_t;

static void timer_start(my_timer_t *t) {
    t->start = get_time_sec();
    t->running = 1;
}

static double timer_stop(my_timer_t *t) {
    t->end = get_time_sec();
    t->running = 0;
    return t->end - t->start;
}

static double timer_elapsed(const my_timer_t *t) {
    return get_time_sec() - t->start;
}

void demo_timer_implementation(void) {
    printf("=== 简易计时器实现 ===\n");

    my_timer_t my_timer = {0};
    timer_start(&my_timer);

    volatile double sum = 0;
    for (int i = 0; i < 1000000; i++) sum += i * 0.001;

    double running_time = timer_elapsed(&my_timer);
    double stopped_time = timer_stop(&my_timer);
    printf("  运行中耗时: %.6f 秒\n", running_time);
    printf("  停止后耗时: %.6f 秒\n\n", stopped_time);
}

void demo_elapsed_time(void) {
    printf("=== 经过时间计算 ===\n");

    double start = get_time_sec();

    volatile int dummy = 0;
    for (int i = 0; i < 5000000; i++) dummy += i;

    double end = get_time_sec();
    double elapsed = end - start;
    printf("  精确经过时间: %.9f 秒\n\n", elapsed);
}

static int is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

void demo_leap_year(void) {
    printf("=== 闰年判断 ===\n");
    int years[] = {1900, 2000, 2024, 2025, 2026, 2100};
    for (int i = 0; i < (int)(sizeof(years) / sizeof(years[0])); i++) {
        printf("  %d年: %s闰年\n", years[i], is_leap_year(years[i]) ? "是" : "不是");
    }
    printf("  规则: 能被4整除且不能被100整除, 或能被400整除\n\n");
}

static int days_in_month(int year, int month) {
    static const int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && is_leap_year(year)) return 29;
    if (month >= 1 && month <= 12) return days[month];
    return 0;
}

void demo_date_arithmetic(void) {
    printf("=== 日期运算 ===\n");

    int year = 2026, month = 5, day = 29;
    printf("  起始日期: %d-%02d-%02d\n", year, month, day);

    day += 100;
    while (day > days_in_month(year, month)) {
        day -= days_in_month(year, month);
        month++;
        if (month > 12) {
            month = 1;
            year++;
        }
    }
    printf("  100天后: %d-%02d-%02d\n", year, month, day);

    year = 2026; month = 5; day = 29;
    day -= 60;
    while (day <= 0) {
        month--;
        if (month < 1) {
            month = 12;
            year--;
        }
        day += days_in_month(year, month);
    }
    printf("  60天前: %d-%02d-%02d\n\n", year, month, day);
}

void demo_timestamp_conversion(void) {
    printf("=== 时间戳与日期互转 ===\n");

    time_t ts = time(NULL);
    printf("  当前时间戳: %lld\n", (long long)ts);

    struct tm *local = localtime(&ts);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", local);
    printf("  -> 本地时间: %s\n", buf);

    time_t back = mktime(local);
    printf("  -> 回转时间戳: %lld\n", (long long)back);
    printf("  转换一致性: %s\n\n", ts == back ? "是" : "否(可能因夏令时)");
}

void demo_days_between_dates(void) {
    printf("=== 计算两个日期之间的天数 ===\n");

    struct tm date1 = {0}, date2 = {0};
    date1.tm_year = 2024 - 1900; date1.tm_mon = 1 - 1; date1.tm_mday = 1;
    date2.tm_year = 2026 - 1900; date2.tm_mon = 5 - 1; date2.tm_mday = 29;
    date1.tm_isdst = -1;
    date2.tm_isdst = -1;

    time_t t1 = mktime(&date1);
    time_t t2 = mktime(&date2);
    double days = difftime(t2, t1) / (60 * 60 * 24);
    printf("  2024-01-01 到 2026-05-29: %.0f 天\n\n", days);
}

int main(void) {
    printf("========== 时间处理模式深入 ==========\n\n");

    demo_timer_implementation();
    demo_elapsed_time();
    demo_leap_year();
    demo_date_arithmetic();
    demo_timestamp_conversion();
    demo_days_between_dates();

    printf("========== 所有演示完成 ==========\n");
    return 0;
}
