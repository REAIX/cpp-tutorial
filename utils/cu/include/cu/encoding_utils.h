/**
 * @file encoding_utils.h
 * @brief 编码转换工具 (C 版本)
 *
 * 提供 Base64、URL、十六进制等编码转换功能。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#ifndef CU_ENCODING_UTILS_H
#define CU_ENCODING_UTILS_H

#include "cu/export.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Base64 编码
 *
 * @param data 要编码的原始数据
 * @param data_len 数据长度
 * @param out_len 输出字符串长度（传出参数）
 * @return Base64 编码后的字符串（需手动释放）
 */
CU_API char* base64_encode(const char* data, size_t data_len, size_t* out_len);

/**
 * @brief Base64 解码
 *
 * @param encoded_data Base64 编码的字符串
 * @param out_len 输出数据长度（传出参数）
 * @return 解码后的原始数据（需手动释放）
 */
CU_API char* base64_decode(const char* encoded_data, size_t* out_len);

/**
 * @brief URL 编码
 *
 * @param url 原始 URL 字符串
 * @return URL 编码后的字符串（需手动释放）
 */
CU_API char* url_encode(const char* url);

/**
 * @brief URL 解码
 *
 * @param encoded_url URL 编码后的字符串
 * @return 解码后的原始 URL（需手动释放）
 */
CU_API char* url_decode(const char* encoded_url);

/**
 * @brief 十六进制编码
 *
 * @param data 要编码的原始数据
 * @param data_len 数据长度
 * @return 十六进制编码字符串（需手动释放）
 */
CU_API char* hex_encode(const char* data, size_t data_len);

/**
 * @brief 十六进制解码
 *
 * @param hex_data 十六进制字符串
 * @param out_len 输出数据长度（传出参数）
 * @return 解码后的原始数据（需手动释放）
 */
CU_API char* hex_decode(const char* hex_data, size_t* out_len);

#ifdef __cplusplus
}
#endif

#endif
