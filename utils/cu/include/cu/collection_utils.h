/**
 * @file collection_utils.h
 * @brief 集合与列表运算工具 (C 版本)
 *
 * 提供字符串数组、整数映射的创建、销毁及基本操作，
 * 以及 Set/List 的交集、并集、差集运算。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#ifndef CU_COLLECTION_UTILS_H
#define CU_COLLECTION_UTILS_H

#include "cu/export.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 字符串数组结构
 *
 * 动态数组，用于存储字符串列表。
 */
typedef struct {
    /** @brief 元素数组 */
    char** elements;
    /** @brief 当前元素数量 */
    size_t size;
    /** @brief 已分配容量 */
    size_t capacity;
} StringArray;

/**
 * @brief 键值对结构
 *
 * 用于整数映射的键值对。
 */
typedef struct {
    /** @brief 整数键 */
    int key;
    /** @brief 字符串值 */
    char* value;
} KeyValuePair;

/**
 * @brief 整数到字符串的映射结构
 */
typedef struct {
    /** @brief 键值对数组 */
    KeyValuePair* pairs;
    /** @brief 当前元素数量 */
    size_t size;
    /** @brief 已分配容量 */
    size_t capacity;
} IntMap;

/**
 * @brief 创建空字符串数组
 * @return 新建的字符串数组指针，失败返回 NULL
 */
CU_API StringArray* string_array_create(void);

/**
 * @brief 销毁字符串数组
 *
 * 释放数组及所有元素占用的内存。
 *
 * @param arr 要销毁的字符串数组指针
 */
CU_API void string_array_destroy(StringArray* arr);

/**
 * @brief 向字符串数组添加元素
 *
 * @param arr 目标数组
 * @param str 要添加的字符串
 */
CU_API void string_array_add(StringArray* arr, const char* str);

/**
 * @brief 从字符串数组移除指定位置元素
 *
 * @param arr 目标数组
 * @param index 要移除的元素索引（从0开始）
 */
CU_API void string_array_remove(StringArray* arr, size_t index);

/**
 * @brief 检查字符串数组是否包含指定字符串
 *
 * @param arr 要检查的数组
 * @param str 要查找的字符串
 * @return 包含返回1，不包含返回0
 */
CU_API int string_array_contains(const StringArray* arr, const char* str);

/**
 * @brief 计算两个集合的交集
 *
 * @param set1 第一个集合
 * @param set2 第二个集合
 * @return 新建的结果集合（需手动销毁）
 */
CU_API StringArray* set_intersection(const StringArray* set1, const StringArray* set2);

/**
 * @brief 计算两个集合的并集
 *
 * @param set1 第一个集合
 * @param set2 第二个集合
 * @return 新建的结果集合（需手动销毁）
 */
CU_API StringArray* set_union(const StringArray* set1, const StringArray* set2);

/**
 * @brief 计算两个集合的差集
 *
 * @param set1 第一个集合
 * @param set2 第二个集合
 * @return 新建的结果集合（需手动销毁）
 */
CU_API StringArray* set_difference(const StringArray* set1, const StringArray* set2);

/**
 * @brief 计算两个列表的交集
 *
 * @param list1 第一个列表
 * @param list2 第二个列表
 * @return 新建的结果列表（需手动销毁）
 */
CU_API StringArray* list_intersection(const StringArray* list1, const StringArray* list2);

/**
 * @brief 计算两个列表的并集
 *
 * @param list1 第一个列表
 * @param list2 第二个列表
 * @return 新建的结果列表（需手动销毁）
 */
CU_API StringArray* list_union(const StringArray* list1, const StringArray* list2);

/**
 * @brief 计算两个列表的差集
 *
 * @param list1 第一个列表
 * @param list2 第二个列表
 * @return 新建的结果列表（需手动销毁）
 */
CU_API StringArray* list_difference(const StringArray* list1, const StringArray* list2);

/**
 * @brief 创建空整数映射
 * @return 新建的映射指针，失败返回 NULL
 */
CU_API IntMap* int_map_create(void);

/**
 * @brief 销毁整数映射
 *
 * 释放映射及所有值占用的内存。
 *
 * @param map 要销毁的映射指针
 */
CU_API void int_map_destroy(IntMap* map);

/**
 * @brief 向映射插入键值对
 *
 * 如果 key 已存在则更新其值。
 *
 * @param map 目标映射
 * @param key 整数键
 * @param value 字符串值
 */
CU_API void int_map_put(IntMap* map, int key, const char* value);

/**
 * @brief 根据键获取值
 *
 * @param map 要查询的映射
 * @param key 整数键
 * @return 对应的值，不存在返回 NULL
 */
CU_API const char* int_map_get(const IntMap* map, int key);

/**
 * @brief 获取映射中当前最大的键
 *
 * @param map 要查询的映射
 * @return 最大键值，映射为空返回0
 */
CU_API int int_map_get_max_index(const IntMap* map);

/**
 * @brief 根据值查找键
 *
 * @param map 要查询的映射
 * @param value 要查找的值
 * @return 第一个匹配的键，不存在返回-1
 */
CU_API int int_map_get_key_by_value(const IntMap* map, const char* value);

/**
 * @brief 逐行打印列表
 *
 * @param elements 要打印的字符串数组
 */
CU_API void prt_list(const StringArray* elements);

/**
 * @brief 将列表转为字符串（换行符分隔）
 *
 * @param lines 字符串数组
 * @return 新建的字符串（需手动释放）
 */
CU_API char* list_to_string(const StringArray* lines);

/**
 * @brief 将列表转为字符串（指定分隔符）
 *
 * @param lines 字符串数组
 * @param separator 分隔符
 * @return 新建的字符串（需手动释放）
 */
CU_API char* list_to_string_with_separator(const StringArray* lines, const char* separator);

#ifdef __cplusplus
}
#endif

#endif
