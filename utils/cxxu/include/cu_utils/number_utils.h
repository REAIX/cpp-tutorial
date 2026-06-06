/**
 * @file number_utils.h
 * @brief 数值转换工具
 *
 * 提供字符串与数值之间的类型转换功能。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#ifndef CU_UTILS_NUMBER_UTILS_H
#define CU_UTILS_NUMBER_UTILS_H

#include "cu_utils/export.h"

#include <string>

namespace cu {

/**
 * @brief 数值转换工具类
 *
 * 提供静态方法进行字符串与各种数值类型之间的转换。
 */
class CXXU_API NumberUtils {
public:
    /** @brief 禁用默认构造函数 */
    NumberUtils() = delete;
    /** @brief 禁用拷贝构造函数 */
    NumberUtils(const NumberUtils&) = delete;
    /** @brief 禁用赋值运算符 */
    NumberUtils& operator=(const NumberUtils&) = delete;

    /**
     * @brief 字符串转整数
     *
     * @param s 要转换的字符串
     * @param defaultValue 转换失败时的默认值
     * @return 转换后的整数值
     */
    static int toInt(const std::string& s, int defaultValue = 0);

    /**
     * @brief 字符串转双精度浮点数
     *
     * @param s 要转换的字符串
     * @param defaultValue 转换失败时的默认值
     * @return 转换后的双精度浮点数
     */
    static double toDouble(const std::string& s, double defaultValue = 0.0);

    /**
     * @brief 字符串转布尔值
     *
     * 支持的 true 值: "true", "yes", "1", "on"（不区分大小写）
     * 支持的 false 值: "false", "no", "0", "off"（不区分大小写）
     *
     * @param s 要转换的字符串
     * @param defaultValue 转换失败时的默认值
     * @return 转换后的布尔值
     */
    static bool toBool(const std::string& s, bool defaultValue = false);

    /**
     * @brief 整数转字符串
     *
     * @param value 要转换的整数值
     * @return 转换后的字符串
     */
    static std::string toString(int value);

    /**
     * @brief 双精度浮点数转字符串
     *
     * @param value 要转换的浮点数值
     * @param decimals 小数位数（-1 表示默认格式）
     * @return 转换后的字符串
     */
    static std::string toString(double value, int decimals = -1);

    /**
     * @brief 判断字符串是否为有效数字
     *
     * 支持整数和小数格式，支持正负号。
     *
     * @param s 要检查的字符串
     * @return 如果是有效数字返回 true
     */
    static bool isNumber(const std::string& s);
};

}

#endif