#ifndef CU_UTILS_LOG_UTILS_H
#define CU_UTILS_LOG_UTILS_H

#include "cu_utils/export.h"
#include <string>

namespace cu {

enum class LogLevel {
    Debug = 0,
    Info = 1,
    Warn = 2,
    Error = 3
};

class CXXU_API LogUtils {
public:
    LogUtils() = delete;
    LogUtils(const LogUtils&) = delete;
    LogUtils& operator=(const LogUtils&) = delete;

    static void setLevel(LogLevel level);
    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warn(const std::string& message);
    static void error(const std::string& message);
};

}

#endif
