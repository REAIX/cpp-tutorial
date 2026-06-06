/**
 * @file random_utils.cpp
 * @brief 随机工具实现
 *
 * 实现 UUID 生成、随机字符串、随机数生成等功能。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#include "cu_utils/random_utils.h"
#include <random>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace cu {

/**
 * @brief 获取线程安全的随机数引擎
 * @return 随机数引擎引用
 */
static std::mt19937& getRandomEngine() {
    thread_local std::mt19937 engine(std::random_device{}());
    return engine;
}

/**
 * @brief 生成 UUID 字符串
 * @return UUID 字符串
 */
std::string RandomUtils::generateUuid() {
    std::uniform_int_distribution<int> dist(0, 15);
    std::uniform_int_distribution<int> dist4(8, 11);

    std::stringstream ss;
    ss << std::hex;

    /* 生成标准 UUID 格式 */
    for (int i = 0; i < 8; i++) {
        ss << dist(getRandomEngine());
    }
    ss << "-";
    for (int i = 0; i < 4; i++) {
        ss << dist(getRandomEngine());
    }
    ss << "-4";
    for (int i = 0; i < 3; i++) {
        ss << dist(getRandomEngine());
    }
    ss << "-";
    ss << dist4(getRandomEngine());
    for (int i = 0; i < 3; i++) {
        ss << dist(getRandomEngine());
    }
    ss << "-";
    for (int i = 0; i < 12; i++) {
        ss << dist(getRandomEngine());
    }

    return ss.str();
}

/**
 * @brief 生成随机字符串
 * @param length 字符串长度
 * @return 随机字符串
 */
std::string RandomUtils::randomString(size_t length) {
    static const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    std::uniform_int_distribution<size_t> dist(0, sizeof(chars) - 2);

    std::string result;
    result.reserve(length);

    for (size_t i = 0; i < length; i++) {
        result += chars[dist(getRandomEngine())];
    }

    return result;
}

/**
 * @brief 生成指定范围内的随机整数
 * @param minValue 最小值（包含）
 * @param maxValue 最大值（包含）
 * @return 范围内的随机整数
 */
int RandomUtils::randomInt(int minValue, int maxValue) {
    if (minValue >= maxValue) return minValue;
    std::uniform_int_distribution<int> dist(minValue, maxValue);
    return dist(getRandomEngine());
}

/**
 * @brief 生成指定范围内的随机浮点数
 * @param minValue 最小值
 * @param maxValue 最大值
 * @return 范围内的随机浮点数
 */
double RandomUtils::randomDouble(double minValue, double maxValue) {
    if (minValue >= maxValue) return minValue;
    std::uniform_real_distribution<double> dist(minValue, maxValue);
    return dist(getRandomEngine());
}

}