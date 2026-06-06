/**
 * @file encoding_utils.cpp
 * @brief 编码转换工具实现
 *
 * 实现 Base64、URL、十六进制等编码转换功能。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#include "cu_utils/encoding_utils.h"
#include <sstream>
#include <iomanip>
#include <cctype>
#include <algorithm>

namespace cu {

/** Base64 编码表 */
static const char base64Table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
 * @brief Base64 编码
 * @param data 要编码的原始数据
 * @return Base64 编码后的字符串
 */
std::string EncodingUtils::base64Encode(const std::string& data) {
    if (data.empty()) return "";

    std::string encoded;
    encoded.reserve(((data.size() + 2) / 3) * 4);

    size_t i = 0;
    while (i < data.size()) {
        unsigned int a = i < data.size() ? (unsigned char)data[i++] : 0;
        unsigned int b = i < data.size() ? (unsigned char)data[i++] : 0;
        unsigned int c = i < data.size() ? (unsigned char)data[i++] : 0;

        unsigned int triple = (a << 16) | (b << 8) | c;

        encoded += base64Table[(triple >> 18) & 0x3F];
        encoded += base64Table[(triple >> 12) & 0x3F];
        encoded += base64Table[(triple >> 6) & 0x3F];
        encoded += base64Table[triple & 0x3F];
    }

    if (data.size() % 3 >= 1) encoded[encoded.size() - 1] = '=';
    if (data.size() % 3 == 1) encoded[encoded.size() - 2] = '=';

    return encoded;
}

/**
 * @brief Base64 解码
 * @param encodedData Base64 编码的字符串
 * @return 解码后的原始数据
 */
std::string EncodingUtils::base64Decode(const std::string& encodedData) {
    if (encodedData.empty() || encodedData.size() % 4 != 0) return "";

    int decodeTable[256];
    std::fill_n(decodeTable, 256, -1);
    for (int i = 0; i < 64; i++) {
        decodeTable[(unsigned char)base64Table[i]] = i;
    }

    size_t decodedLen = encodedData.size() / 4 * 3;
    if (encodedData[encodedData.size() - 1] == '=') decodedLen--;
    if (encodedData[encodedData.size() - 2] == '=') decodedLen--;

    std::string decoded;
    decoded.reserve(decodedLen);

    for (size_t i = 0; i < encodedData.size(); i += 4) {
        int a = decodeTable[(unsigned char)encodedData[i]];
        int b = decodeTable[(unsigned char)encodedData[i + 1]];
        int c = decodeTable[(unsigned char)encodedData[i + 2]];
        int d = decodeTable[(unsigned char)encodedData[i + 3]];

        if (a < 0 || b < 0) return "";
        if ((encodedData[i + 2] != '=' && c < 0) ||
            (encodedData[i + 3] != '=' && d < 0)) return "";

        unsigned int triple = ((a < 0 ? 0 : a) << 18) | ((b < 0 ? 0 : b) << 12) |
                              ((c < 0 ? 0 : c) << 6) | (d < 0 ? 0 : d);

        if (decoded.size() < decodedLen) decoded += static_cast<char>((triple >> 16) & 0xFF);
        if (decoded.size() < decodedLen) decoded += static_cast<char>((triple >> 8) & 0xFF);
        if (decoded.size() < decodedLen) decoded += static_cast<char>(triple & 0xFF);
    }

    return decoded;
}

/**
 * @brief URL 编码
 * @param url 原始 URL 字符串
 * @return URL 编码后的字符串
 */
std::string EncodingUtils::urlEncode(const std::string& url) {
    std::ostringstream encoded;
    encoded << std::hex << std::uppercase;

    for (unsigned char c : url) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded << c;
        } else {
            encoded << '%' << std::setw(2) << std::setfill('0') << (int)c;
        }
    }

    return encoded.str();
}

/**
 * @brief URL 解码
 * @param encodedUrl URL 编码后的字符串
 * @return 解码后的原始 URL
 */
std::string EncodingUtils::urlDecode(const std::string& encodedUrl) {
    std::string decoded;
    decoded.reserve(encodedUrl.size());

    for (size_t i = 0; i < encodedUrl.size(); i++) {
        if (encodedUrl[i] == '%' && i + 2 < encodedUrl.size() &&
            std::isxdigit(static_cast<unsigned char>(encodedUrl[i + 1])) &&
            std::isxdigit(static_cast<unsigned char>(encodedUrl[i + 2]))) {
            std::string hex = encodedUrl.substr(i + 1, 2);
            decoded += (char)std::stoi(hex, nullptr, 16);
            i += 2;
        } else if (encodedUrl[i] == '+') {
            decoded += ' ';
        } else {
            decoded += encodedUrl[i];
        }
    }

    return decoded;
}

/**
 * @brief 十六进制编码
 * @param data 要编码的原始数据
 * @return 十六进制编码字符串
 */
std::string EncodingUtils::hexEncode(const std::string& data) {
    std::ostringstream hex;
    hex << std::hex << std::uppercase;

    for (unsigned char c : data) {
        hex << std::setw(2) << std::setfill('0') << (int)c;
    }

    return hex.str();
}

/**
 * @brief 十六进制解码
 * @param hexData 十六进制字符串
 * @return 解码后的原始数据
 */
std::string EncodingUtils::hexDecode(const std::string& hexData) {
    if (hexData.size() % 2 != 0) return "";

    std::string decoded;
    decoded.reserve(hexData.size() / 2);

    for (size_t i = 0; i < hexData.size(); i += 2) {
        if (!std::isxdigit(static_cast<unsigned char>(hexData[i])) ||
            !std::isxdigit(static_cast<unsigned char>(hexData[i + 1]))) {
            return "";
        }
        std::string hex = hexData.substr(i, 2);
        decoded += static_cast<char>(std::stoi(hex, nullptr, 16));
    }

    return decoded;
}

}