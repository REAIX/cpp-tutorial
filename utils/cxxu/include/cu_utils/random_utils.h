/**
 * @file random_utils.h
 * @brief 随机工具
 *
 * 提供 UUID 生成、随机字符串、随机数生成等功能。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#ifndef CU_UTILS_RANDOM_UTILS_H
#define CU_UTILS_RANDOM_UTILS_H

#include "cu_utils/export.h"

#include <string>

namespace cu {

/**
 * @brief 随机工具类
 *
 * 提供静态方法生成各种随机数据，使用 C++11 随机库。
 */
class CXXU_API RandomUtils {
public:
    /** @brief 禁用默认构造函数 */
    RandomUtils() = delete;
    /** @brief 禁用拷贝构造函数 */
    RandomUtils(const RandomUtils&) = delete;
    /** @brief 禁用赋值运算符 */
    RandomUtils& operator=(const RandomUtils&) = delete;

    /**
     * @brief 生成 UUID 字符串
     *
     * 生成符合 UUID v4 规范的唯一标识符。
     * 格式: "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx"
     *
     * @return UUID 字符串
     */
    static std::string generateUuid();

    /**
     * @brief 生成随机字符串
     *
     * 生成包含大小写字母和数字的随机字符串。
     *
     * @param length 要生成的字符串长度
     * @return 随机字符串
     */
    static std::string randomString(size_t length);

    /**
     * @brief 生成指定范围内的随机整数
     *
     * @param minValue 最小值（包含）
     * @param maxValue 最大值（包含）
     * @return 范围内的随机整数
     */
    static int randomInt(int minValue, int maxValue);

    /**
     * @brief 生成指定范围内的随机浮点数
     *
     * @param minValue 最小值
     * @param maxValue 最大值
     * @return 范围内的随机浮点数
     */
    static double randomDouble(double minValue, double maxValue);
};

}

#endif