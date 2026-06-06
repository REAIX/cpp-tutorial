#ifndef CU_LOG_UTILS_H
#define CU_LOG_UTILS_H

#include "cu/export.h"

typedef enum {
    CU_LOG_DEBUG = 0,
    CU_LOG_INFO = 1,
    CU_LOG_WARN = 2,
    CU_LOG_ERROR = 3
} CuLogLevel;

CU_API void cu_log_set_level(CuLogLevel level);
CU_API void cu_log(CuLogLevel level, const char* file, int line, const char* fmt, ...);

#define CU_LOG_DEBUG(fmt, ...) cu_log(CU_LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define CU_LOG_INFO(fmt, ...)  cu_log(CU_LOG_INFO,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define CU_LOG_WARN(fmt, ...)  cu_log(CU_LOG_WARN,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define CU_LOG_ERROR(fmt, ...) cu_log(CU_LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif
