#include "cu/log_utils.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static CuLogLevel g_log_level = CU_LOG_INFO;

static const char* level_name(CuLogLevel level) {
    switch (level) {
        case CU_LOG_DEBUG: return "DEBUG";
        case CU_LOG_INFO:  return "INFO";
        case CU_LOG_WARN:  return "WARN";
        case CU_LOG_ERROR: return "ERROR";
        default:           return "UNKNOWN";
    }
}

void cu_log_set_level(CuLogLevel level) {
    g_log_level = level;
}

void cu_log(CuLogLevel level, const char* file, int line, const char* fmt, ...) {
    if (level < g_log_level) {
        return;
    }

    char buf[4096];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    const char* short_file = strrchr(file, '/');
#ifdef _WIN32
    const char* short_file_win = strrchr(file, '\\');
    if (short_file_win && (!short_file || short_file_win > short_file)) {
        short_file = short_file_win;
    }
#endif
    if (short_file) {
        short_file++;
    } else {
        short_file = file;
    }

    FILE* out = (level >= CU_LOG_WARN) ? stderr : stdout;
    fprintf(out, "[%s] %s:%d: %s\n", level_name(level), short_file, line, buf);
}
