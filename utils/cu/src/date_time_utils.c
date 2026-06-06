#include "cu/date_time_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

char* get_datetime(void) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char* buf = (char*)malloc(20);
    if (!buf) return NULL;
    strftime(buf, 20, "%Y-%m-%d %H:%M:%S", t);
    return buf;
}

char* get_date(void) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char* buf = (char*)malloc(11);
    if (!buf) return NULL;
    strftime(buf, 11, "%Y-%m-%d", t);
    return buf;
}

char* get_time_str(void) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char* buf = (char*)malloc(9);
    if (!buf) return NULL;
    strftime(buf, 9, "%H:%M:%S", t);
    return buf;
}

long get_timestamp(void) {
    return (long)time(NULL);
}

long long get_timestamp_millis(void) {
#ifdef _WIN32
    SYSTEMTIME st;
    GetSystemTime(&st);
    FILETIME ft;
    SystemTimeToFileTime(&st, &ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return (long long)(uli.QuadPart / 10000 - 11644473600000LL);
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000 + (long long)tv.tv_usec / 1000;
#endif
}

char* format_timestamp(long timestamp, const char* fmt) {
    if (!fmt) return NULL;
    time_t t = (time_t)timestamp;
    struct tm* tm_info = localtime(&t);
    if (!tm_info) return NULL;

    size_t buf_size = 128;
    char* buf = (char*)malloc(buf_size);
    if (!buf) return NULL;
    strftime(buf, buf_size, fmt, tm_info);
    return buf;
}

long parse_datetime(const char* datetime_str, const char* fmt) {
    if (!datetime_str || !fmt) return -1;

    struct tm tm_info;
    memset(&tm_info, 0, sizeof(struct tm));

#ifdef _WIN32
    int year, month, day, hour, minute, second;
    hour = 0; minute = 0; second = 0;

    if (strcmp(fmt, "%Y-%m-%d %H:%M:%S") == 0) {
        if (sscanf(datetime_str, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second) >= 3) {
            tm_info.tm_year = year - 1900;
            tm_info.tm_mon = month - 1;
            tm_info.tm_mday = day;
            tm_info.tm_hour = hour;
            tm_info.tm_min = minute;
            tm_info.tm_sec = second;
        } else {
            return -1;
        }
    } else if (strcmp(fmt, "%Y-%m-%d") == 0) {
        if (sscanf(datetime_str, "%d-%d-%d", &year, &month, &day) == 3) {
            tm_info.tm_year = year - 1900;
            tm_info.tm_mon = month - 1;
            tm_info.tm_mday = day;
        } else {
            return -1;
        }
    } else if (strcmp(fmt, "%H:%M:%S") == 0) {
        if (sscanf(datetime_str, "%d:%d:%d", &hour, &minute, &second) == 3) {
            time_t now = time(NULL);
            struct tm* now_tm = localtime(&now);
            tm_info = *now_tm;
            tm_info.tm_hour = hour;
            tm_info.tm_min = minute;
            tm_info.tm_sec = second;
        } else {
            return -1;
        }
    } else {
        if (sscanf(datetime_str, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second) >= 3) {
            tm_info.tm_year = year - 1900;
            tm_info.tm_mon = month - 1;
            tm_info.tm_mday = day;
            tm_info.tm_hour = hour;
            tm_info.tm_min = minute;
            tm_info.tm_sec = second;
        } else {
            return -1;
        }
    }
#else
    char* result = strptime(datetime_str, fmt, &tm_info);
    if (!result) return -1;
#endif

    return (long)mktime(&tm_info);
}

long datetime_add_days(long timestamp, int days) {
    return timestamp + (long)days * 86400;
}

long datetime_add_hours(long timestamp, int hours) {
    return timestamp + (long)hours * 3600;
}

int datetime_diff_days(long start, long end) {
    long diff = end - start;
    if (diff >= 0) {
        return (int)(diff / 86400);
    } else {
        return -(int)((-diff) / 86400);
    }
}

int is_leap_year(int year) {
    return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}
