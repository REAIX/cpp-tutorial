/**
 * @file collection_utils.cpp
 * @brief 集合与列表运算工具实现
 *
 * 实现 Set/List 的交集、并集、差集运算及 Map/List 辅助方法。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#include "cu_utils/collection_utils.h"
#include <algorithm>

namespace cu {

/**
 * @brief 获取 key 为 int 的 Map 中当前最大的 key 值
 * @param map 输入的整数键映射
 * @return 最大的 key 值，映射为空返回 -1
 */
int CollectionUtils::getIMapMaxIndex(const std::map<int, std::string>& map) {
    if (map.empty()) return -1;
    return map.rbegin()->first;
}

/**
 * @brief 将行列表拼接成字符串，每行末尾加换行符 \n（含最后一行）
 * @param lines 输入的字符串向量指针
 * @return 拼接后的字符串
 */
std::string CollectionUtils::listToString(const std::vector<std::string>* lines) {
    if (!lines) return "";
    if (lines->empty()) return "";

    std::string result;
    for (const std::string& line : *lines) {
        result += line + "\n";
    }

    return result;
}

/**
 * @brief 用指定分隔符拼接列表，仅元素之间加分隔符，末尾不加
 * @param lines 输入的字符串向量指针
 * @param separator 分隔符指针，为 nullptr 时使用换行符
 * @return 拼接后的字符串
 */
std::string CollectionUtils::listToString(const std::vector<std::string>* lines, const std::string* separator) {
    if (!lines) return "";
    if (lines->empty()) return "";

    const std::string& sep = separator ? *separator : "\n";
    std::string result;

    for (size_t i = 0; i < lines->size(); ++i) {
        if (i > 0) {
            result += sep;
        }
        result += (*lines)[i];
    }

    return result;
}

}