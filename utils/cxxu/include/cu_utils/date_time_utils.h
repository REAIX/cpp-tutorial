/**
 * @file date_time_utils.h
 * @brief 日期时间工具
 *
 * 提供日期时间的获取、格式化等功能。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#ifndef CU_UTILS_DATE_TIME_UTILS_H
#define CU_UTILS_DATE_TIME_UTILS_H

#include "cu_utils/export.h"

#include <string>
#include <ctime>

namespace cu {

/**
 * @brief 日期时间工具类
 *
 * 提供静态方法获取和格式化日期时间。
 */
class CXXU_API DateTimeUtils {
public:
    /** @brief 禁用默认构造函数 */
    DateTimeUtils() = delete;
    /** @brief 禁用拷贝构造函数 */
    DateTimeUtils(const DateTimeUtils&) = delete;
    /** @brief 禁用赋值运算符 */
    DateTimeUtils& operator=(const DateTimeUtils&) = delete;

    /**
     * @brief 获取当前日期时间
     *
     * @return 日期时间字符串（格式：YYYY-MM-DD HH:MM:SS）
     */
    static std::string getDateTime();

    /**
     * @brief 获取当前日期
     *
     * @return 日期字符串（格式：YYYY-MM-DD）
     */
    static std::string getDate();

    /**
     * @brief 获取当前时间
     *
     * @return 时间字符串（格式：HH:MM:SS）
     */
    static std::string getTime();

    /**
     * @brief 获取当前时间戳（秒）
     *
     * @return 当前时间戳
     */
    static time_t getTimestamp();

    /**
     * @brief 获取当前时间戳（毫秒）
     *
     * @return 当前时间戳（毫秒）
     */
    static long long getTimestampMillis();

    /**
     * @brief 格式化时间戳
     *
     * @param timestamp 时间戳
     * @param format 日期时间格式，默认为 "%Y-%m-%d %H:%M:%S"
     * @return 格式化后的日期时间字符串
     */
    static std::string formatTimestamp(time_t timestamp, const std::string& format = "%Y-%m-%d %H:%M:%S");

    /**
     * @brief 计算两个时间戳之间的时间差
     *
     * @param start 开始时间戳
     * @param end 结束时间戳
     * @return 时间差（秒）
     */
    static double calculateTimeDifference(time_t start, time_t end);

    static time_t parseDateTime(const std::string& datetimeStr, const std::string& format = "%Y-%m-%d %H:%M:%S");
    static time_t addDays(time_t timestamp, int days);
    static time_t addHours(time_t timestamp, int hours);
    static int diffDays(time_t start, time_t end);
    static bool isLeapYear(int year);
};

}

#endif