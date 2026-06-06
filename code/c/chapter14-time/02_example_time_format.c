/**
 * @file 02_example_time_format.c
 * @brief 时间格式化与解析
 * @description 对应文档: 14-时间处理
 */
#include <stdio.h>
#include <time.h>
#include <string.h>

void demo_strftime(void) {
    printf("=== strftime()格式化时间 ===\n");
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    char buf[128];

    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", local);
    printf("  标准格式: %s\n", buf);

    strftime(buf, sizeof(buf), "%Y年%m月%d日 %A", local);
    printf("  中文格式: %s\n", buf);

    strftime(buf, sizeof(buf), "%I:%M:%S %p", local);
    printf("  12小时制: %s\n", buf);

    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %z", local);
    printf("  RFC2822:  %s\n", buf);

    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", local);
    printf("  ISO8601:  %s\n", buf);

    printf("\n  常用格式符:\n");
    printf("  %%Y=4位年 %%m=月 %%d=日 %%H=24时 %%M=分 %%S=秒\n");
    printf("  %%a=短星期 %%A=长星期 %%b=短月名 %%B=长月名\n");
    printf("  %%I=12时 %%p=AM/PM %%j=年中第几天 %%U=年中第几周\n\n");
}

void demo_asctime_ctime(void) {
    printf("=== asctime()和ctime() ===\n");
    time_t now = time(NULL);

    char *ct = ctime(&now);
    printf("  ctime():  %s", ct);

    struct tm *local = localtime(&now);
    char *at = asctime(local);
    printf("  asctime(): %s", at);

    printf("  注意: 这两个函数返回静态缓冲区, 非线程安全!\n");
    printf("  线程安全替代: asctime_r(), ctime_r()\n\n");
}

void demo_gmtime_localtime(void) {
    printf("=== gmtime()和localtime() ===\n");
    time_t now = time(NULL);

    struct tm utc = *gmtime(&now);
    struct tm local = *localtime(&now);

    char utc_buf[64], local_buf[64];
    strftime(utc_buf, sizeof(utc_buf), "%Y-%m-%d %H:%M:%S", &utc);
    strftime(local_buf, sizeof(local_buf), "%Y-%m-%d %H:%M:%S", &local);

    printf("  UTC:   %s\n", utc_buf);
    printf("  本地:  %s\n", local_buf);
    printf("  注意: 返回指向静态缓冲区的指针, 非线程安全!\n");
    printf("  线程安全替代: gmtime_r(), localtime_r()\n\n");
}

void demo_mktime(void) {
    printf("=== mktime()时间转时间戳 ===\n");
    struct tm t = {0};
    t.tm_year = 2026 - 1900;
    t.tm_mon = 5 - 1;
    t.tm_mday = 29;
    t.tm_hour = 12;
    t.tm_min = 0;
    t.tm_sec = 0;
    t.tm_isdst = -1;

    time_t timestamp = mktime(&t);
    printf("  2026-05-29 12:00:00 -> 时间戳: %lld\n", (long long)timestamp);

    printf("  mktime会自动修正溢出值:\n");
    struct tm overflow = {0};
    overflow.tm_year = 2026 - 1900;
    overflow.tm_mon = 0;
    overflow.tm_mday = 32;
    overflow.tm_isdst = -1;
    mktime(&overflow);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d", &overflow);
    printf("  1月32日 -> 自动修正为: %s\n\n", buf);
}

void demo_strptime(void) {
    printf("=== strptime()字符串解析(POSIX) ===\n");
    printf("  注意: strptime是POSIX扩展, Windows可能不可用\n");
    printf("  用法示例:\n");
    printf("    struct tm tm = {0};\n");
    printf("    strptime(\"2026-05-29 12:00:00\", \"%%Y-%%m-%%d %%H:%%M:%%S\", &tm);\n");
    printf("    time_t t = mktime(&tm);\n\n");
    printf("  Windows替代方案: sscanf手动解析\n");

    struct tm tm_parsed = {0};
    int year, month, day, hour, min, sec;
    if (sscanf("2026-05-29 12:30:45", "%d-%d-%d %d:%d:%d",
               &year, &month, &day, &hour, &min, &sec) == 6) {
        tm_parsed.tm_year = year - 1900;
        tm_parsed.tm_mon = month - 1;
        tm_parsed.tm_mday = day;
        tm_parsed.tm_hour = hour;
        tm_parsed.tm_min = min;
        tm_parsed.tm_sec = sec;
        tm_parsed.tm_isdst = -1;
        time_t t = mktime(&tm_parsed);
        char buf[64];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_parsed);
        printf("  解析结果: %s (时间戳=%lld)\n\n", buf, (long long)t);
    }
}

int main(void) {
    printf("========== 时间格式化与解析示例 ==========\n\n");

    demo_strftime();
    demo_asctime_ctime();
    demo_gmtime_localtime();
    demo_mktime();
    demo_strptime();

    printf("========== 所有演示完成 ==========\n");
    return 0;
}
