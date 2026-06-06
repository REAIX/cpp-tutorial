#include "cu_utils/log_utils.h"
#include <iostream>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace cu {

static std::atomic<LogLevel> g_log_level{LogLevel::Info};

void LogUtils::setLevel(LogLevel level) {
    g_log_level.store(level, std::memory_order_relaxed);
}

static std::string current_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    std::tm tm_buf{};
#ifdef _WIN32
    localtime_s(&tm_buf, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_buf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

void LogUtils::debug(const std::string& message) {
    if (LogLevel::Debug >= g_log_level.load(std::memory_order_relaxed)) {
        std::cout << "[" << current_timestamp() << "] [DEBUG] " << message << std::endl;
    }
}

void LogUtils::info(const std::string& message) {
    if (LogLevel::Info >= g_log_level.load(std::memory_order_relaxed)) {
        std::cout << "[" << current_timestamp() << "] [INFO] " << message << std::endl;
    }
}

void LogUtils::warn(const std::string& message) {
    if (LogLevel::Warn >= g_log_level.load(std::memory_order_relaxed)) {
        std::cerr << "[" << current_timestamp() << "] [WARN] " << message << std::endl;
    }
}

void LogUtils::error(const std::string& message) {
    if (LogLevel::Error >= g_log_level.load(std::memory_order_relaxed)) {
        std::cerr << "[" << current_timestamp() << "] [ERROR] " << message << std::endl;
    }
}

}
