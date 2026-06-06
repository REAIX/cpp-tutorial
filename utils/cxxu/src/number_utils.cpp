/**
 * @file number_utils.cpp
 * @brief 数值转换工具实现
 *
 * 实现字符串与数值之间的类型转换功能。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#include "cu_utils/number_utils.h"
#include <cstdlib>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace cu {

/**
 * @brief 字符串转整数
 * @param s 要转换的字符串
 * @param defaultValue 转换失败时的默认值
 * @return 转换后的整数值
 */
int NumberUtils::toInt(const std::string& s, int defaultValue) {
    if (s.empty()) return defaultValue;

    try {
        return std::stoi(s);
    } catch (...) {
        return defaultValue;
    }
}

/**
 * @brief 字符串转双精度浮点数
 * @param s 要转换的字符串
 * @param defaultValue 转换失败时的默认值
 * @return 转换后的双精度浮点数
 */
double NumberUtils::toDouble(const std::string& s, double defaultValue) {
    if (s.empty()) return defaultValue;

    try {
        return std::stod(s);
    } catch (...) {
        return defaultValue;
    }
}

/**
 * @brief 字符串转布尔值
 * @param s 要转换的字符串
 * @param defaultValue 转换失败时的默认值
 * @return 转换后的布尔值
 */
bool NumberUtils::toBool(const std::string& s, bool defaultValue) {
    if (s.empty()) return defaultValue;

    std::string lower = s;
    std::transform(lower.begin(), lower.end(), lower.begin(),
        [](unsigned char c) { return std::tolower(c); });

    if (lower == "true" || lower == "yes" || lower == "1" || lower == "on") {
        return true;
    }
    if (lower == "false" || lower == "no" || lower == "0" || lower == "off") {
        return false;
    }

    return defaultValue;
}

/**
 * @brief 整数转字符串
 * @param value 要转换的整数值
 * @return 转换后的字符串
 */
std::string NumberUtils::toString(int value) {
    return std::to_string(value);
}

/**
 * @brief 双精度浮点数转字符串
 * @param value 要转换的浮点数值
 * @param decimals 小数位数（-1 表示默认格式）
 * @return 转换后的字符串
 */
std::string NumberUtils::toString(double value, int decimals) {
    std::ostringstream oss;
    if (decimals >= 0) {
        oss << std::fixed << std::setprecision(decimals);
    }
    oss << value;
    return oss.str();
}

/**
 * @brief 判断字符串是否为有效数字
 * @param s 要检查的字符串
 * @return 是有效数字返回 true
 */
bool NumberUtils::isNumber(const std::string& s) {
    if (s.empty()) return false;

    size_t start = 0;
    if (s[0] == '-' || s[0] == '+') {
        start = 1;
    }

    bool hasDigit = false;
    bool hasDot = false;

    for (size_t i = start; i < s.size(); i++) {
        if (std::isdigit(static_cast<unsigned char>(s[i]))) {
            hasDigit = true;
        } else if (s[i] == '.' && !hasDot) {
            hasDot = true;
        } else {
            return false;
        }
    }

    return hasDigit;
}

}