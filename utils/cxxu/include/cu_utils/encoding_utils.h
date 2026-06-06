/**
 * @file encoding_utils.h
 * @brief 编码转换工具
 *
 * 提供 Base64、URL、十六进制等编码转换功能。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#ifndef CU_UTILS_ENCODING_UTILS_H
#define CU_UTILS_ENCODING_UTILS_H

#include "cu_utils/export.h"

#include <string>

namespace cu {

/**
 * @brief 编码转换工具类
 *
 * 提供静态方法进行各种编码格式的转换操作。
 */
class CXXU_API EncodingUtils {
public:
    /** @brief 禁用默认构造函数 */
    EncodingUtils() = delete;
    /** @brief 禁用拷贝构造函数 */
    EncodingUtils(const EncodingUtils&) = delete;
    /** @brief 禁用赋值运算符 */
    EncodingUtils& operator=(const EncodingUtils&) = delete;

    /**
     * @brief Base64 编码
     *
     * 将字符串编码为 Base64 格式。
     *
     * @param data 要编码的原始数据
     * @return Base64 编码后的字符串
     */
    static std::string base64Encode(const std::string& data);

    /**
     * @brief Base64 解码
     *
     * 将 Base64 编码的字符串解码为原始数据。
     *
     * @param encodedData Base64 编码的字符串
     * @return 解码后的原始数据，解码失败返回空字符串
     */
    static std::string base64Decode(const std::string& encodedData);

    /**
     * @brief URL 编码
     *
     * 对 URL 中的特殊字符进行编码，转换为 %XX 格式。
     *
     * @param url 原始 URL 字符串
     * @return URL 编码后的字符串
     */
    static std::string urlEncode(const std::string& url);

    /**
     * @brief URL 解码
     *
     * 将 %XX 格式的编码还原为原始字符。
     *
     * @param encodedUrl URL 编码后的字符串
     * @return 解码后的原始 URL
     */
    static std::string urlDecode(const std::string& encodedUrl);

    /**
     * @brief 十六进制编码
     *
     * 将字符串编码为十六进制格式（大写表示）。
     *
     * @param data 要编码的原始数据
     * @return 十六进制编码字符串
     */
    static std::string hexEncode(const std::string& data);

    /**
     * @brief 十六进制解码
     *
     * 将十六进制字符串解码为原始数据。
     *
     * @param hexData 十六进制字符串
     * @return 解码后的原始数据，输入不合法返回空字符串
     */
    static std::string hexDecode(const std::string& hexData);
};

}

#endif