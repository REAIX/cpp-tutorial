/**
 * @file collection_utils.c
 * @brief 集合与列表运算工具实现 (C 版本)
 *
 * 实现字符串数组、整数映射的创建、销毁及基本操作，
 * 以及 Set/List 的交集、并集、差集运算。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#include "cu/collection_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * @brief 创建空字符串数组
 *
 * 分配一个新的 StringArray 结构并初始化为空。
 *
 * @return 新建的字符串数组指针，失败返回 NULL
 */
StringArray* string_array_create(void) {
    StringArray* arr = (StringArray*)malloc(sizeof(StringArray));
    if (arr) {
        arr->elements = NULL;
        arr->size = 0;
        arr->capacity = 0;
    }
    return arr;
}

/**
 * @brief 销毁字符串数组
 *
 * 释放数组及所有元素占用的内存，包括每个元素字符串和数组本身。
 *
 * @param arr 要销毁的字符串数组指针
 */
void string_array_destroy(StringArray* arr) {
    if (arr) {
        for (size_t i = 0; i < arr->size; i++) {
            free(arr->elements[i]);
        }
        free(arr->elements);
        free(arr);
    }
}

/**
 * @brief 向字符串数组添加元素
 *
 * 如果容量不足则自动扩容（容量翻倍）。
 *
 * @param arr 目标数组
 * @param str 要添加的字符串（会被复制）
 */
void string_array_add(StringArray* arr, const char* str) {
    if (!arr || !str) return;

    /* 容量不足时扩容 */
    if (arr->size >= arr->capacity) {
        size_t new_capacity = arr->capacity == 0 ? 8 : arr->capacity * 2;
        char** new_elements = (char**)realloc(arr->elements, new_capacity * sizeof(char*));
        if (!new_elements) return;
        arr->elements = new_elements;
        arr->capacity = new_capacity;
    }

    /* 复制字符串并添加到数组 */
    char* dup = strdup(str);
    if (!dup) return;
    arr->elements[arr->size] = dup;
    arr->size++;
}

/**
 * @brief 从字符串数组移除指定位置元素
 *
 * 移除指定索引的元素，后续元素前移。
 *
 * @param arr 目标数组
 * @param index 要移除的元素索引（从0开始）
 */
void string_array_remove(StringArray* arr, size_t index) {
    if (!arr || index >= arr->size) return;

    /* 释放被移除元素的内存 */
    free(arr->elements[index]);

    /* 后续元素前移 */
    for (size_t i = index; i < arr->size - 1; i++) {
        arr->elements[i] = arr->elements[i + 1];
    }
    arr->size--;
}

/**
 * @brief 检查字符串数组是否包含指定字符串
 *
 * @param arr 要检查的数组
 * @param str 要查找的字符串
 * @return 包含返回1，不包含返回0
 */
int string_array_contains(const StringArray* arr, const char* str) {
    if (!arr || !str) return 0;

    for (size_t i = 0; i < arr->size; i++) {
        if (strcmp(arr->elements[i], str) == 0) {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief 计算两个集合的交集
 *
 * 返回同时存在于 set1 和 set2 中的元素。
 *
 * @param set1 第一个集合
 * @param set2 第二个集合
 * @return 新建的结果集合（需手动调用 string_array_destroy 销毁）
 */
StringArray* set_intersection(const StringArray* set1, const StringArray* set2) {
    StringArray* result = string_array_create();
    if (!result) return NULL;

    if (!set1 || !set2) return result;

    for (size_t i = 0; i < set1->size; i++) {
        if (string_array_contains(set2, set1->elements[i])) {
            string_array_add(result, set1->elements[i]);
        }
    }

    return result;
}

/**
 * @brief 计算两个集合的并集
 *
 * 返回所有不重复的元素，set1 元素在前，set2 特有元素在后。
 *
 * @param set1 第一个集合
 * @param set2 第二个集合
 * @return 新建的结果集合（需手动调用 string_array_destroy 销毁）
 */
StringArray* set_union(const StringArray* set1, const StringArray* set2) {
    StringArray* result = string_array_create();
    if (!result) return NULL;

    /* 添加 set1 的所有元素 */
    if (set1) {
        for (size_t i = 0; i < set1->size; i++) {
            string_array_add(result, set1->elements[i]);
        }
    }

    /* 添加 set2 中不重复的元素 */
    if (set2) {
        for (size_t i = 0; i < set2->size; i++) {
            if (!string_array_contains(result, set2->elements[i])) {
                string_array_add(result, set2->elements[i]);
            }
        }
    }

    return result;
}

/**
 * @brief 计算两个集合的差集
 *
 * 返回存在于 set1 但不在 set2 中的元素。
 *
 * @param set1 第一个集合
 * @param set2 第二个集合
 * @return 新建的结果集合（需手动调用 string_array_destroy 销毁）
 */
StringArray* set_difference(const StringArray* set1, const StringArray* set2) {
    StringArray* result = string_array_create();
    if (!result) return NULL;

    if (!set1) return result;

    for (size_t i = 0; i < set1->size; i++) {
        if (!set2 || !string_array_contains(set2, set1->elements[i])) {
            string_array_add(result, set1->elements[i]);
        }
    }

    return result;
}

/**
 * @brief 计算两个列表的交集
 *
 * 保留 list1 中同时出现在 list2 中的元素，顺序同 list1。
 *
 * @param list1 第一个列表
 * @param list2 第二个列表
 * @return 新建的结果列表（需手动调用 string_array_destroy 销毁）
 */
StringArray* list_intersection(const StringArray* list1, const StringArray* list2) {
    StringArray* result = string_array_create();
    if (!result) return NULL;

    if (!list1 || !list2) return result;

    for (size_t i = 0; i < list1->size; i++) {
        if (string_array_contains(list2, list1->elements[i])) {
            string_array_add(result, list1->elements[i]);
        }
    }

    return result;
}

/**
 * @brief 计算两个列表的并集
 *
 * 先取 list1 全部，再追加 list2 中未出现的元素。
 *
 * @param list1 第一个列表
 * @param list2 第二个列表
 * @return 新建的结果列表（需手动调用 string_array_destroy 销毁）
 */
StringArray* list_union(const StringArray* list1, const StringArray* list2) {
    StringArray* result = string_array_create();
    if (!result) return NULL;

    if (list1) {
        for (size_t i = 0; i < list1->size; i++) {
            string_array_add(result, list1->elements[i]);
        }
    }

    if (list2) {
        for (size_t i = 0; i < list2->size; i++) {
            if (!string_array_contains(result, list2->elements[i])) {
                string_array_add(result, list2->elements[i]);
            }
        }
    }

    return result;
}

/**
 * @brief 计算两个列表的差集
 *
 * 返回存在于 list1 但不在 list2 中的元素，顺序同 list1。
 *
 * @param list1 第一个列表
 * @param list2 第二个列表
 * @return 新建的结果列表（需手动调用 string_array_destroy 销毁）
 */
StringArray* list_difference(const StringArray* list1, const StringArray* list2) {
    StringArray* result = string_array_create();
    if (!result) return NULL;

    if (!list1) return result;

    for (size_t i = 0; i < list1->size; i++) {
        if (!list2 || !string_array_contains(list2, list1->elements[i])) {
            string_array_add(result, list1->elements[i]);
        }
    }

    return result;
}

/**
 * @brief 创建空整数映射
 *
 * @return 新建的映射指针，失败返回 NULL
 */
IntMap* int_map_create(void) {
    IntMap* map = (IntMap*)malloc(sizeof(IntMap));
    if (map) {
        map->pairs = NULL;
        map->size = 0;
        map->capacity = 0;
    }
    return map;
}

/**
 * @brief 销毁整数映射
 *
 * 释放映射及所有值占用的内存。
 *
 * @param map 要销毁的映射指针
 */
void int_map_destroy(IntMap* map) {
    if (map) {
        for (size_t i = 0; i < map->size; i++) {
            free(map->pairs[i].value);
        }
        free(map->pairs);
        free(map);
    }
}

/**
 * @brief 向映射插入键值对
 *
 * 如果 key 已存在则更新其值。
 *
 * @param map 目标映射
 * @param key 整数键
 * @param value 字符串值（会被复制）
 */
void int_map_put(IntMap* map, int key, const char* value) {
    if (!map || !value) return;

    /* 检查 key 是否已存在，存在则更新值 */
    for (size_t i = 0; i < map->size; i++) {
        if (map->pairs[i].key == key) {
            char* new_value = strdup(value);
            if (!new_value) return;
            free(map->pairs[i].value);
            map->pairs[i].value = new_value;
            return;
        }
    }

    /* 容量不足时扩容 */
    if (map->size >= map->capacity) {
        size_t new_capacity = map->capacity == 0 ? 8 : map->capacity * 2;
        KeyValuePair* new_pairs = (KeyValuePair*)realloc(map->pairs, new_capacity * sizeof(KeyValuePair));
        if (!new_pairs) return;
        map->pairs = new_pairs;
        map->capacity = new_capacity;
    }

    /* 添加新的键值对 */
    map->pairs[map->size].key = key;
    map->pairs[map->size].value = strdup(value);
    if (!map->pairs[map->size].value) return;
    map->size++;
}

/**
 * @brief 根据键获取值
 *
 * @param map 要查询的映射
 * @param key 整数键
 * @return 对应的值，不存在返回 NULL
 */
const char* int_map_get(const IntMap* map, int key) {
    if (!map) return NULL;

    for (size_t i = 0; i < map->size; i++) {
        if (map->pairs[i].key == key) {
            return map->pairs[i].value;
        }
    }
    return NULL;
}

/**
 * @brief 获取映射中当前最大的键
 *
 * @param map 要查询的映射
 * @return 最大键值，映射为空返回0
 */
int int_map_get_max_index(const IntMap* map) {
    if (!map || map->size == 0) return 0;

    int max = map->pairs[0].key;
    for (size_t i = 1; i < map->size; i++) {
        if (map->pairs[i].key > max) {
            max = map->pairs[i].key;
        }
    }
    return max;
}

/**
 * @brief 根据值查找键
 *
 * 返回第一个匹配该值的键。
 *
 * @param map 要查询的映射
 * @param value 要查找的值
 * @return 第一个匹配的键，不存在返回-1
 */
int int_map_get_key_by_value(const IntMap* map, const char* value) {
    if (!map || !value) return -1;

    for (size_t i = 0; i < map->size; i++) {
        if (strcmp(map->pairs[i].value, value) == 0) {
            return map->pairs[i].key;
        }
    }
    return -1;
}

/**
 * @brief 逐行打印列表
 *
 * @param elements 要打印的字符串数组
 */
void prt_list(const StringArray* elements) {
    if (!elements) return;

    for (size_t i = 0; i < elements->size; i++) {
        printf("%s\n", elements->elements[i]);
    }
}

/**
 * @brief 将列表转为字符串（换行符分隔）
 *
 * 每行末尾加换行符 \\n，包含最后一行。
 *
 * @param lines 字符串数组
 * @return 新建的字符串（需手动释放）
 */
char* list_to_string(const StringArray* lines) {
    if (!lines || lines->size == 0) {
        return strdup("");
    }

    /* 计算总长度 */
    size_t total_length = 0;
    for (size_t i = 0; i < lines->size; i++) {
        total_length += strlen(lines->elements[i]) + 1;
    }

    /* 拼接字符串 */
    char* result = (char*)malloc(total_length + 1);
    if (!result) return NULL;

    result[0] = '\0';
    for (size_t i = 0; i < lines->size; i++) {
        strcat(result, lines->elements[i]);
        strcat(result, "\n");
    }

    return result;
}

/**
 * @brief 将列表转为字符串（指定分隔符）
 *
 * 仅元素之间加分隔符，末尾不加。
 *
 * @param lines 字符串数组
 * @param separator 分隔符
 * @return 新建的字符串（需手动释放）
 */
char* list_to_string_with_separator(const StringArray* lines, const char* separator) {
    if (!lines || lines->size == 0) {
        return strdup("");
    }

    if (!separator) {
        separator = "\n";
    }

    size_t sep_len = strlen(separator);

    /* 计算总长度 */
    size_t total_length = 0;
    for (size_t i = 0; i < lines->size; i++) {
        total_length += strlen(lines->elements[i]);
        if (i < lines->size - 1) {
            total_length += sep_len;
        }
    }

    /* 拼接字符串 */
    char* result = (char*)malloc(total_length + 1);
    if (!result) return NULL;

    result[0] = '\0';
    for (size_t i = 0; i < lines->size; i++) {
        strcat(result, lines->elements[i]);
        if (i < lines->size - 1) {
            strcat(result, separator);
        }
    }

    return result;
}