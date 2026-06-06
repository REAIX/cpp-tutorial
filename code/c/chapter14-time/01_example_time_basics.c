/**
 * @file 01_example_time_basics.c
 * @brief 时间处理基础
 * @description 对应文档: 14-时间处理
 */
#include <stdio.h>
#include <time.h>

void demo_time_function(void) {
    printf("=== time()函数 ===\n");
    time_t now = time(NULL);
    printf("  当前时间戳: %lld 秒 (自1970-01-01 00:00:00 UTC)\n", (long long)now);

    time_t also_now;
    time(&also_now);
    printf("  time(&var)方式: %lld 秒\n", (long long)also_now);
    printf("  两种写法等价\n\n");
}

void demo_struct_tm(void) {
    printf("=== struct tm结构体 ===\n");
    time_t now = time(NULL);
    struct tm local_buf = *localtime(&now);
    struct tm utc_buf = *gmtime(&now);

    printf("  本地时间:\n");
    printf("    年=%d, 月=%d, 日=%d\n", local_buf.tm_year + 1900, local_buf.tm_mon + 1, local_buf.tm_mday);
    printf("    时=%d, 分=%d, 秒=%d\n", local_buf.tm_hour, local_buf.tm_min, local_buf.tm_sec);
    printf("    星期=%d(0=周日), 年中第%d天, 夏令时=%d\n",
           local_buf.tm_wday, local_buf.tm_yday, local_buf.tm_isdst);

    printf("  UTC时间:\n");
    printf("    年=%d, 月=%d, 日=%d\n", utc_buf.tm_year + 1900, utc_buf.tm_mon + 1, utc_buf.tm_mday);
    printf("    时=%d, 分=%d, 秒=%d\n\n", utc_buf.tm_hour, utc_buf.tm_min, utc_buf.tm_sec);
}

void demo_time_zone(void) {
    printf("=== 时区处理 ===\n");
    time_t now = time(NULL);
    struct tm local_buf = *localtime(&now);
    struct tm utc_buf = *gmtime(&now);

    int offset_hour = local_buf.tm_hour - utc_buf.tm_hour;
    if (local_buf.tm_mday != utc_buf.tm_mday) {
        offset_hour += (local_buf.tm_mday > utc_buf.tm_mday) ? 24 : -24;
    }
    printf("  本地时区偏移: UTC+%d\n", offset_hour);
    printf("  注意: localtime/gmtime使用C库内部时区设置\n");
    printf("  可通过环境变量TZ修改: setenv(\"TZ\", \"UTC0\", 1)\n\n");
}

void demo_clock_function(void) {
    printf("=== clock()函数 ===\n");
    clock_t start = clock();

    volatile double sum = 0;
    for (int i = 0; i < 1000000; i++) {
        sum += i * 0.001;
    }

    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("  循环耗时: %.6f 秒 (CPU时间)\n", elapsed);
    printf("  CLOCKS_PER_SEC = %ld\n", (long)CLOCKS_PER_SEC);
    printf("  注意: clock()测量的是CPU时间, 不是墙上时钟时间\n\n");
}

void demo_time_types(void) {
    printf("=== 时间相关类型 ===\n");
    printf("  time_t:     %zu 字节 (通常为long或long long)\n", sizeof(time_t));
    printf("  clock_t:    %zu 字节\n", sizeof(clock_t));
    printf("  struct tm:  %zu 字节\n", sizeof(struct tm));
    printf("  time_t范围: ");
    if (sizeof(time_t) == 4) {
        printf("32位, 2038年溢出!\n");
    } else {
        printf("64位, 足够使用数十亿年\n");
    }
    printf("\n");
}

int main(void) {
    printf("========== 时间处理基础示例 ==========\n\n");

    demo_time_function();
    demo_struct_tm();
    demo_time_zone();
    demo_clock_function();
    demo_time_types();

    printf("========== 所有演示完成 ==========\n");
    return 0;
}
