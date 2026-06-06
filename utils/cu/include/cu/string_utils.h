/**
 * @file string_utils.h
 * @brief 字符串处理工具 (C 版本)
 *
 * 提供字符串截取、空白判断、填充、十六进制转换等功能。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#ifndef CU_STRING_UTILS_H
#define CU_STRING_UTILS_H

#include "cu/export.h"
#include <stddef.h>
#include "cu/collection_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 截取子串（左开右开区间）
 *
 * 从 src 中截取「第一个 fstr 之后」到「第一个 lstr 之前」的子串，
 * 不包含 fstr 和 lstr 本身。
 *
 * @param src 源字符串
 * @param fstr 起始标记
 * @param lstr 结束标记
 * @return 新建的子串（需手动释放）
 */
CU_API char* sub_str(const char* src, const char* fstr, const char* lstr);

/**
 * @brief 截取子串（左闭右开区间）
 *
 * 截取「从第一个 fstr 开始」到「第一个 lstr 结束」的子串，
 * 包含 fstr 但不包含 lstr。
 *
 * @param src 源字符串
 * @param fstr 起始标记
 * @param lstr 结束标记
 * @return 新建的子串（需手动释放）
 */
CU_API char* lsub_str(const char* src, const char* fstr, const char* lstr);

/**
 * @brief 截取子串（左开右闭区间）
 *
 * 截取「第一个 fstr 之后」到「第一个 lstr 结束」的子串，
 * 不包含 fstr 但包含 lstr。
 *
 * @param src 源字符串
 * @param fstr 起始标记
 * @param lstr 结束标记
 * @return 新建的子串（需手动释放）
 */
CU_API char* rsub_str(const char* src, const char* fstr, const char* lstr);

/**
 * @brief 截取子串（左闭右闭区间）
 *
 * 截取「从第一个 fstr 开始」到「第一个 lstr 结束」的子串，
 * 同时包含 fstr 和 lstr。
 *
 * @param src 源字符串
 * @param fstr 起始标记
 * @param lstr 结束标记
 * @return 新建的子串（需手动释放）
 */
CU_API char* asub_str(const char* src, const char* fstr, const char* lstr);

/**
 * @brief 判断字符串是否为空
 *
 * @param str 字符串指针
 * @return 为空返回1，否则返回0
 */
CU_API int is_empty(const char* str);

/**
 * @brief 判断字符串是否为空白
 *
 * 检查是否为 NULL 或仅包含空白字符。
 *
 * @param str 字符串指针
 * @return 为空白返回1，否则返回0
 */
CU_API int is_blank(const char* str);

/**
 * @brief 左侧填充字符
 *
 * 使用 pad 字符从左侧填充，使字符串达到 length 长度。
 * 如果 input 已够长则原样返回。
 *
 * @param input 输入字符串
 * @param length 目标长度
 * @param pad 填充字符
 * @return 新建的字符串（需手动释放）
 */
CU_API char* left_pad(const char* input, int length, char pad);

/**
 * @brief 右侧填充字符
 *
 * 使用 pad 字符从右侧填充，使字符串达到 length 长度。
 * 如果 input 已够长则原样返回。
 *
 * @param input 输入字符串
 * @param length 目标长度
 * @param pad 填充字符
 * @return 新建的字符串（需手动释放）
 */
CU_API char* right_pad(const char* input, int length, char pad);

/**
 * @brief 合并连续制表符
 *
 * 将连续多个制表符 \\t 合并为一个 \\t。
 *
 * @param str 输入字符串
 * @return 新建的字符串（需手动释放）
 */
CU_API char* one_tab(const char* str);

/**
 * @brief 合并连续空格
 *
 * 将制表符替换为空格，再将连续空格合并为一个空格。
 *
 * @param str 输入字符串
 * @return 新建的字符串（需手动释放）
 */
CU_API char* one_blank(const char* str);

/**
 * @brief 统计子字符串出现次数
 *
 * @param str 源字符串
 * @param substr 子字符串
 * @return 子字符串出现的次数
 */
CU_API int str_count(const char* str, const char* substr);

/**
 * @brief 十六进制字符串转 ASCII 字符串
 *
 * 将十六进制字符串解码为 ASCII 字符串，会去掉空格；
 * 长度为奇数时左侧补 0。
 *
 * @param hex_str 十六进制字符串
 * @return 新建的 ASCII 字符串（需手动释放）
 */
CU_API char* hex2_ascii(const char* hex_str);

/**
 * @brief 字符串转十六进制表示
 *
 * @param input 输入字符串
 * @return 新建的十六进制字符串（需手动释放）
 */
CU_API char* string_to_hex(const char* input);

/**
 * @brief 字符串转十六进制字节列表
 *
 * @param input 输入字符串
 * @return 新建的十六进制字节列表（需手动销毁）
 */
CU_API StringArray* string_to_hex_list(const char* input);

/**
 * @brief 字符串居中填充
 *
 * 将字符串居中填充到 total_length，左右用 padding_char 填充。
 *
 * @param input 输入字符串
 * @param total_length 总长度
 * @param padding_char 填充字符
 * @return 新建的字符串（需手动释放）
 */
CU_API char* format_center(const char* input, int total_length, const char* padding_char);

/**
 * @brief 去掉行首的第一个制表符
 *
 * @param line 输入字符串
 * @return 新建的字符串（需手动释放）
 */
CU_API char* remove_first_tab(const char* line);

CU_API StringArray* str_split(const char* str, const char* delimiter);
CU_API char* str_join(const StringArray* arr, const char* delimiter);
CU_API char* str_trim(const char* str);
CU_API char* str_ltrim(const char* str);
CU_API char* str_rtrim(const char* str);
CU_API char* str_replace(const char* str, const char* old_str, const char* new_str);
CU_API char* str_replace_first(const char* str, const char* old_str, const char* new_str);
CU_API char* str_to_upper(const char* str);
CU_API char* str_to_lower(const char* str);
CU_API int str_starts_with(const char* str, const char* prefix);
CU_API int str_ends_with(const char* str, const char* suffix);
CU_API int str_contains(const char* str, const char* substr);
CU_API char* str_format(const char* fmt, ...);
CU_API int is_alpha(const char* str);
CU_API int is_numeric(const char* str);
CU_API int is_alphanumeric(const char* str);
CU_API int is_email(const char* str);

#ifdef __cplusplus
}
#endif

#endif
