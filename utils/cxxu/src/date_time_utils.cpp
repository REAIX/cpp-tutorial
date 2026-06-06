/**
 * @file date_time_utils.cpp
 * @brief 日期时间工具实现
 *
 * 实现日期时间的获取、格式化等功能。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#include "cu_utils/date_time_utils.h"
#include <iomanip>
#include <sstream>
#include <chrono>

namespace cu {

/**
 * @brief 获取当前日期时间
 * @return 日期时间字符串（格式：YYYY-MM-DD HH:MM:SS）
 */
std::string DateTimeUtils::getDateTime() {
    auto now = std::chrono::system_clock::now();
    time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm local_time;
#ifdef _WIN32
    localtime_s(&local_time, &now_time);
#else
    localtime_r(&now_time, &local_time);
#endif

    std::ostringstream oss;
    oss << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

/**
 * @brief 获取当前日期
 * @return 日期字符串（格式：YYYY-MM-DD）
 */
std::string DateTimeUtils::getDate() {
    auto now = std::chrono::system_clock::now();
    time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm local_time;
#ifdef _WIN32
    localtime_s(&local_time, &now_time);
#else
    localtime_r(&now_time, &local_time);
#endif

    std::ostringstream oss;
    oss << std::put_time(&local_time, "%Y-%m-%d");
    return oss.str();
}

/**
 * @brief 获取当前时间
 * @return 时间字符串（格式：HH:MM:SS）
 */
std::string DateTimeUtils::getTime() {
    auto now = std::chrono::system_clock::now();
    time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm local_time;
#ifdef _WIN32
    localtime_s(&local_time, &now_time);
#else
    localtime_r(&now_time, &local_time);
#endif

    std::ostringstream oss;
    oss << std::put_time(&local_time, "%H:%M:%S");
    return oss.str();
}

/**
 * @brief 获取当前时间戳（秒）
 * @return 当前时间戳
 */
time_t DateTimeUtils::getTimestamp() {
    return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

/**
 * @brief 获取当前时间戳（毫秒）
 * @return 当前时间戳（毫秒）
 */
long long DateTimeUtils::getTimestampMillis() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

/**
 * @brief 格式化时间戳
 * @param timestamp 时间戳
 * @param format 日期时间格式，默认为 "%Y-%m-%d %H:%M:%S"
 * @return 格式化后的日期时间字符串
 */
std::string DateTimeUtils::formatTimestamp(time_t timestamp, const std::string& format) {
    std::tm local_time;
#ifdef _WIN32
    localtime_s(&local_time, &timestamp);
#else
    localtime_r(&timestamp, &local_time);
#endif
    std::ostringstream oss;
    oss << std::put_time(&local_time, format.c_str());
    return oss.str();
}

/**
 * @brief 计算两个时间戳之间的时间差
 * @param start 开始时间戳
 * @param end 结束时间戳
 * @return 时间差（秒）
 */
double DateTimeUtils::calculateTimeDifference(time_t start, time_t end) {
    return difftime(end, start);
}

time_t DateTimeUtils::parseDateTime(const std::string& datetimeStr, const std::string& format) {
    std::tm tm = {};
    std::istringstream iss(datetimeStr);
    iss >> std::get_time(&tm, format.c_str());
    if (iss.fail()) return -1;
#ifdef _WIN32
    return _mkgmtime(&tm);
#else
    return timegm(&tm);
#endif
}

time_t DateTimeUtils::addDays(time_t timestamp, int days) {
    return timestamp + static_cast<time_t>(days) * 86400;
}

time_t DateTimeUtils::addHours(time_t timestamp, int hours) {
    return timestamp + static_cast<time_t>(hours) * 3600;
}

int DateTimeUtils::diffDays(time_t start, time_t end) {
    return static_cast<int>((end - start) / 86400);
}

bool DateTimeUtils::isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

}