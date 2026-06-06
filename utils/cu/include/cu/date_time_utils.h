#ifndef CU_DATE_TIME_UTILS_H
#define CU_DATE_TIME_UTILS_H

#include "cu/export.h"

CU_API char* get_datetime(void);
CU_API char* get_date(void);
CU_API char* get_time_str(void);
CU_API long get_timestamp(void);
CU_API long long get_timestamp_millis(void);
CU_API char* format_timestamp(long timestamp, const char* fmt);
CU_API long parse_datetime(const char* datetime_str, const char* fmt);
CU_API long datetime_add_days(long timestamp, int days);
CU_API long datetime_add_hours(long timestamp, int hours);
CU_API int datetime_diff_days(long start, long end);
CU_API int is_leap_year(int year);

#endif
