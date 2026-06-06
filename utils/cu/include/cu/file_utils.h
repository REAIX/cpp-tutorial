/**
 * @file file_utils.h
 * @brief 文件处理工具 (C 版本)
 *
 * 提供文件读写、目录操作、路径处理等功能。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#ifndef CU_FILE_UTILS_H
#define CU_FILE_UTILS_H

#include "cu/export.h"
#include <stddef.h>
#include "cu/collection_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 检查文件是否存在
 *
 * @param path 文件路径
 * @return 存在返回1，不存在返回0
 */
CU_API int file_exists(const char* path);

/**
 * @brief 检查路径是否为目录
 *
 * @param path 路径
 * @return 是目录返回1，否则返回0
 */
CU_API int is_directory(const char* path);

/**
 * @brief 检查路径是否为常规文件
 *
 * @param path 路径
 * @return 是常规文件返回1，否则返回0
 */
CU_API int is_regular_file(const char* path);

/**
 * @brief 按行读取文件
 *
 * @param file_path 文件路径
 * @return 包含文件所有行的字符串数组（需手动销毁）
 */
CU_API StringArray* read_lines(const char* file_path);

/**
 * @brief 将行列表写入文件
 *
 * @param file_path 文件路径
 * @param lines 字符串数组
 * @return 成功返回1，失败返回0
 */
CU_API int write_lines(const char* file_path, const StringArray* lines);

/**
 * @brief 追加行列表到文件
 *
 * @param file_path 文件路径
 * @param lines 字符串数组
 * @return 成功返回1，失败返回0
 */
CU_API int append_lines(const char* file_path, const StringArray* lines);

/**
 * @brief 从路径获取文件名
 *
 * @param path 文件路径
 * @return 文件名字符串（需手动释放）
 */
CU_API char* get_filename(const char* path);

/**
 * @brief 从路径获取目录部分
 *
 * @param path 文件路径
 * @return 目录字符串（需手动释放）
 */
CU_API char* get_directory(const char* path);

/**
 * @brief 合并两个路径
 *
 * @param path1 第一个路径
 * @param path2 第二个路径
 * @return 合并后的路径字符串（需手动释放）
 */
CU_API char* combine_paths(const char* path1, const char* path2);

/**
 * @brief 获取文件大小
 *
 * @param file_path 文件路径
 * @return 文件大小（字节），失败返回-1
 */
CU_API long get_file_size(const char* file_path);

/**
 * @brief 注释类型枚举
 */
typedef enum {
    COMMENT_SINGLE_LINE_C,       /**< C/C++ 单行注释 // */
    COMMENT_SINGLE_LINE_PYTHON, /**< Python 单行注释 # */
    COMMENT_SINGLE_LINE_SQL,     /**< SQL 单行注释 -- */
    COMMENT_SINGLE_LINE_BATCH,   /**< Batch 单行注释 REM 或 :: */
    COMMENT_MULTI_LINE_C,        /**< C 多行注释 */
    COMMENT_MULTI_LINE_HTML      /**< HTML 注释 <!-- ... --> */
} CommentType;

/**
 * @brief 移除文件中的指定类型注释
 *
 * @param file_path 源文件路径
 * @param comment_types 要移除的注释类型数组
 * @param num_types 注释类型数量
 * @param output_path 输出文件路径（可选，为 NULL 时不输出）
 * @return 处理后的行列表（需手动销毁）
 */
CU_API StringArray* remove_comments(const char* file_path, const CommentType* comment_types,
                             size_t num_types, const char* output_path);

#ifdef __cplusplus
}
#endif

#endif
