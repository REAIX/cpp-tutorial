/**
 * @file collection_utils.h
 * @brief 集合运算工具
 *
 * 提供 Set/List 的交集、并集、差集运算及 Map/List 辅助方法。
 * 使用模板实现，兼容多种数据类型。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#ifndef CU_UTILS_COLLECTION_UTILS_H
#define CU_UTILS_COLLECTION_UTILS_H

#include "cu_utils/export.h"

#include <string>
#include <vector>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <iostream>

namespace cu {

/**
 * @brief 集合运算工具类
 *
 * 提供静态方法进行集合与列表的交、并、差运算，
 * 以及列表转字符串、Map键值查找等辅助功能。
 */
class CXXU_API CollectionUtils {
public:
    /** @brief 禁用默认构造函数 */
    CollectionUtils() = delete;
    /** @brief 禁用拷贝构造函数 */
    CollectionUtils(const CollectionUtils&) = delete;
    /** @brief 禁用赋值运算符 */
    CollectionUtils& operator=(const CollectionUtils&) = delete;

    /**
     * @brief 计算两个集合的交集
     * @tparam T 集合元素类型
     * @param set1 第一个集合
     * @param set2 第二个集合
     * @return 同时存在于 set1 和 set2 中的元素组成的新集合
     */
    template <typename T>
    static std::unordered_set<T> setIntersection(const std::unordered_set<T>& set1, const std::unordered_set<T>& set2);

    /**
     * @brief 计算两个集合的并集
     * @tparam T 集合元素类型
     * @param set1 第一个集合
     * @param set2 第二个集合
     * @return 所有不重复元素组成的新集合
     */
    template <typename T>
    static std::unordered_set<T> setUnion(const std::unordered_set<T>& set1, const std::unordered_set<T>& set2);

    /**
     * @brief 计算两个集合的差集
     * @tparam T 集合元素类型
     * @param set1 第一个集合
     * @param set2 第二个集合
     * @return 存在于 set1 但不在 set2 中的元素组成的新集合
     */
    template <typename T>
    static std::unordered_set<T> setDifference(const std::unordered_set<T>& set1, const std::unordered_set<T>& set2);

    /**
     * @brief 计算两个列表的交集
     * @tparam T 列表元素类型
     * @param list1 第一个列表
     * @param list2 第二个列表
     * @return 保留 list1 中同时出现在 list2 中的元素，顺序同 list1
     */
    template <typename T>
    static std::vector<T> listIntersection(const std::vector<T>& list1, const std::vector<T>& list2);

    /**
     * @brief 计算两个列表的并集
     * @tparam T 列表元素类型
     * @param list1 第一个列表
     * @param list2 第二个列表
     * @return 先取 list1 全部，再追加 list2 中未出现的元素
     */
    template <typename T>
    static std::vector<T> listUnion(const std::vector<T>& list1, const std::vector<T>& list2);

    /**
     * @brief 计算两个列表的差集
     * @tparam T 列表元素类型
     * @param list1 第一个列表
     * @param list2 第二个列表
     * @return 存在于 list1 但不在 list2 中的元素，顺序同 list1
     */
    template <typename T>
    static std::vector<T> listDifference(const std::vector<T>& list1, const std::vector<T>& list2);

    /**
     * @brief 逐行打印列表到标准输出
     * @tparam T 列表元素类型
     * @param elements 要打印的列表指针，为 nullptr 时不打印
     */
    template <typename T>
    static void prtList(const std::vector<T>* elements);

    /**
     * @brief 获取 key 为 int 的 Map 中当前最大的 key 值
     * @param map 输入的整数键映射
     * @return 最大的 key 值，映射为空返回 -1
     */
    static int getIMapMaxIndex(const std::map<int, std::string>& map);

    /**
     * @brief 根据 value 查找第一个匹配的 key
     * @tparam K 键类型
     * @tparam V 值类型
     * @param map 输入的映射
     * @param value 要查找的值
     * @return 第一个匹配的 key，不存在时返回 K 的默认值
     */
    template <typename K, typename V>
    static K getKeyByValue(const std::map<K, V>& map, const V& value);

    /**
     * @brief 将行列表拼接成字符串，每行末尾加换行符 \n（含最后一行）
     * @param lines 输入的字符串向量指针
     * @return 拼接后的字符串
     */
    static std::string listToString(const std::vector<std::string>* lines);

    /**
     * @brief 用指定分隔符拼接列表，仅元素之间加分隔符，末尾不加
     * @param lines 输入的字符串向量指针
     * @param separator 分隔符指针，为 nullptr 时使用换行符
     * @return 拼接后的字符串
     */
    static std::string listToString(const std::vector<std::string>* lines, const std::string* separator);
};

/**
 * @brief Set 交集实现
 */
template <typename T>
std::unordered_set<T> CollectionUtils::setIntersection(const std::unordered_set<T>& set1, const std::unordered_set<T>& set2) {
    std::unordered_set<T> result;
    for (const T& item : set1) {
        if (set2.count(item) > 0) {
            result.insert(item);
        }
    }
    return result;
}

/**
 * @brief Set 并集实现
 */
template <typename T>
std::unordered_set<T> CollectionUtils::setUnion(const std::unordered_set<T>& set1, const std::unordered_set<T>& set2) {
    std::unordered_set<T> result(set1);
    result.insert(set2.begin(), set2.end());
    return result;
}

/**
 * @brief Set 差集实现
 */
template <typename T>
std::unordered_set<T> CollectionUtils::setDifference(const std::unordered_set<T>& set1, const std::unordered_set<T>& set2) {
    std::unordered_set<T> result;
    for (const T& item : set1) {
        if (set2.count(item) == 0) {
            result.insert(item);
        }
    }
    return result;
}

/**
 * @brief List 交集实现
 */
template <typename T>
std::vector<T> CollectionUtils::listIntersection(const std::vector<T>& list1, const std::vector<T>& list2) {
    std::unordered_set<T> set2(list2.begin(), list2.end());
    std::vector<T> result;
    for (const T& item : list1) {
        if (set2.count(item) > 0) {
            result.push_back(item);
        }
    }
    return result;
}

/**
 * @brief List 并集实现
 */
template <typename T>
std::vector<T> CollectionUtils::listUnion(const std::vector<T>& list1, const std::vector<T>& list2) {
    std::unordered_set<T> seen(list1.begin(), list1.end());
    std::vector<T> result(list1);
    for (const T& item : list2) {
        if (seen.insert(item).second) {
            result.push_back(item);
        }
    }
    return result;
}

/**
 * @brief List 差集实现
 */
template <typename T>
std::vector<T> CollectionUtils::listDifference(const std::vector<T>& list1, const std::vector<T>& list2) {
    std::unordered_set<T> set2(list2.begin(), list2.end());
    std::vector<T> result;
    for (const T& item : list1) {
        if (set2.count(item) == 0) {
            result.push_back(item);
        }
    }
    return result;
}

/**
 * @brief 逐行打印列表实现
 */
template <typename T>
void CollectionUtils::prtList(const std::vector<T>* elements) {
    if (!elements) return;
    for (const T& element : *elements) {
        std::cout << element << std::endl;
    }
}

/**
 * @brief 根据 value 查找 key 实现
 */
template <typename K, typename V>
K CollectionUtils::getKeyByValue(const std::map<K, V>& map, const V& value) {
    for (const auto& pair : map) {
        if (pair.second == value) {
            return pair.first;
        }
    }
    return K{};
}

}

#endif