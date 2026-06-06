/**
 * @file encoding_utils.c
 * @brief 编码转换工具实现 (C 版本)
 *
 * 实现 Base64、URL、十六进制等编码转换功能。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#include "cu/encoding_utils.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/** Base64 编码表 */
static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
 * @brief Base64 编码
 *
 * 将二进制数据编码为 Base64 字符串。
 *
 * @param data 要编码的原始数据
 * @param data_len 数据长度
 * @param out_len 输出字符串长度（传出参数）
 * @return Base64 编码后的字符串（需手动释放）
 */
char* base64_encode(const char* data, size_t data_len, size_t* out_len) {
    if (!data || data_len == 0) {
        if (out_len) *out_len = 0;
        return NULL;
    }

    /* 计算编码后长度（每3字节转为4字符，末尾补 =） */
    size_t encoded_len = 4 * ((data_len + 2) / 3);
    char* encoded = (char*)malloc(encoded_len + 1);
    if (!encoded) {
        if (out_len) *out_len = 0;
        return NULL;
    }

    /* 每3字节为一组，编码为4个字符 */
    size_t i, j;
    for (i = 0, j = 0; i < data_len; ) {
        /* 获取3字节 */
        unsigned int a = i < data_len ? (unsigned char)data[i++] : 0;
        unsigned int b = i < data_len ? (unsigned char)data[i++] : 0;
        unsigned int c = i < data_len ? (unsigned char)data[i++] : 0;

        /* 合并为24位整数 */
        unsigned int triple = (a << 16) | (b << 8) | c;

        /* 拆分为4个6位索引 */
        encoded[j++] = base64_table[(triple >> 18) & 0x3F];
        encoded[j++] = base64_table[(triple >> 12) & 0x3F];
        encoded[j++] = base64_table[(triple >> 6) & 0x3F];
        encoded[j++] = base64_table[triple & 0x3F];
    }

    /* 末尾补 = */
    if (data_len % 3 >= 1) encoded[j - 1] = '=';
    if (data_len % 3 == 1) encoded[j - 2] = '=';
    encoded[j] = '\0';

    if (out_len) *out_len = encoded_len;
    return encoded;
}

/**
 * @brief Base64 解码
 *
 * 将 Base64 字符串解码为原始数据。
 *
 * @param encoded_data Base64 编码的字符串
 * @param out_len 输出数据长度（传出参数）
 * @return 解码后的原始数据（需手动释放）
 */
char* base64_decode(const char* encoded_data, size_t* out_len) {
    if (!encoded_data) {
        if (out_len) *out_len = 0;
        return NULL;
    }

    size_t input_len = strlen(encoded_data);
    if (input_len == 0 || input_len % 4 != 0) {
        if (out_len) *out_len = 0;
        return NULL;
    }

    size_t decoded_len = input_len / 4 * 3;
    if (encoded_data[input_len - 1] == '=') decoded_len--;
    if (encoded_data[input_len - 2] == '=') decoded_len--;

    char* decoded = (char*)malloc(decoded_len + 1);
    if (!decoded) {
        if (out_len) *out_len = 0;
        return NULL;
    }

    int decode_table[256];
    memset(decode_table, -1, sizeof(decode_table));
    for (int i = 0; i < 64; i++) {
        decode_table[(unsigned char)base64_table[i]] = i;
    }

    size_t i, j;
    for (i = 0, j = 0; i < input_len; ) {
        int a = decode_table[(unsigned char)encoded_data[i++]];
        int b = decode_table[(unsigned char)encoded_data[i++]];
        int c = decode_table[(unsigned char)encoded_data[i++]];
        int d = decode_table[(unsigned char)encoded_data[i++]];

        if (a < 0 || b < 0) {
            free(decoded);
            if (out_len) *out_len = 0;
            return NULL;
        }
        if ((encoded_data[i - 2] != '=' && c < 0) ||
            (encoded_data[i - 1] != '=' && d < 0)) {
            free(decoded);
            if (out_len) *out_len = 0;
            return NULL;
        }

        unsigned int triple = ((a < 0 ? 0 : a) << 18) | ((b < 0 ? 0 : b) << 12) |
                              ((c < 0 ? 0 : c) << 6) | (d < 0 ? 0 : d);

        if (j < decoded_len) decoded[j++] = (triple >> 16) & 0xFF;
        if (j < decoded_len) decoded[j++] = (triple >> 8) & 0xFF;
        if (j < decoded_len) decoded[j++] = triple & 0xFF;
    }
    decoded[j] = '\0';

    if (out_len) *out_len = decoded_len;
    return decoded;
}

/**
 * @brief 判断字符是否为 URL 安全字符
 *
 * @param c 要检查的字符
 * @return URL 安全返回1，否则返回0
 */
static int is_url_safe(unsigned char c) {
    return isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~';
}

/**
 * @brief URL 编码
 *
 * 对 URL 中的特殊字符进行编码，转换为 %XX 格式。
 *
 * @param url 原始 URL 字符串
 * @return URL 编码后的字符串（需手动释放）
 */
char* url_encode(const char* url) {
    if (!url) return NULL;

    size_t len = strlen(url);
    /* 最坏情况：每个字符转为 %XX，占用3字节 */
    char* encoded = (char*)malloc(len * 3 + 1);
    if (!encoded) return NULL;

    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = url[i];
        if (is_url_safe(c)) {
            encoded[j++] = c;
        } else {
            sprintf(encoded + j, "%%%02X", c);
            j += 3;
        }
    }
    encoded[j] = '\0';

    return encoded;
}

/**
 * @brief URL 解码
 *
 * 将 %XX 格式的编码还原为原始字符。
 *
 * @param encoded_url URL 编码后的字符串
 * @return 解码后的原始 URL（需手动释放）
 */
char* url_decode(const char* encoded_url) {
    if (!encoded_url) return NULL;

    size_t len = strlen(encoded_url);
    char* decoded = (char*)malloc(len + 1);
    if (!decoded) return NULL;

    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        if (encoded_url[i] == '%' && i + 2 < len &&
            isxdigit((unsigned char)encoded_url[i + 1]) && isxdigit((unsigned char)encoded_url[i + 2])) {
            /* 处理 %XX 格式 */
            char hex[3] = {encoded_url[i + 1], encoded_url[i + 2], '\0'};
            decoded[j++] = (char)(unsigned char)strtol(hex, NULL, 16);
            i += 2;
        } else if (encoded_url[i] == '+') {
            /* URL 编码中 + 代表空格 */
            decoded[j++] = ' ';
        } else {
            decoded[j++] = encoded_url[i];
        }
    }
    decoded[j] = '\0';

    return decoded;
}

/**
 * @brief 十六进制编码
 *
 * 将二进制数据编码为十六进制字符串（大写）。
 *
 * @param data 要编码的原始数据
 * @param data_len 数据长度
 * @return 十六进制编码字符串（需手动释放）
 */
char* hex_encode(const char* data, size_t data_len) {
    if (!data || data_len == 0) return NULL;

    char* hex = (char*)malloc(data_len * 2 + 1);
    if (!hex) return NULL;

    for (size_t i = 0; i < data_len; i++) {
        sprintf(hex + i * 2, "%02X", (unsigned char)data[i]);
    }
    hex[data_len * 2] = '\0';

    return hex;
}

/**
 * @brief 十六进制解码
 *
 * 将十六进制字符串解码为原始数据。
 *
 * @param hex_data 十六进制字符串
 * @param out_len 输出数据长度（传出参数）
 * @return 解码后的原始数据（需手动释放）
 */
char* hex_decode(const char* hex_data, size_t* out_len) {
    if (!hex_data) {
        if (out_len) *out_len = 0;
        return NULL;
    }

    size_t hex_len = strlen(hex_data);
    if (hex_len % 2 != 0) {
        if (out_len) *out_len = 0;
        return NULL;
    }

    size_t decoded_len = hex_len / 2;
    char* decoded = (char*)malloc(decoded_len + 1);
    if (!decoded) {
        if (out_len) *out_len = 0;
        return NULL;
    }

    for (size_t i = 0; i < decoded_len; i++) {
        char c1 = hex_data[i * 2];
        char c2 = hex_data[i * 2 + 1];
        if (!isxdigit((unsigned char)c1) || !isxdigit((unsigned char)c2)) {
            free(decoded);
            if (out_len) *out_len = 0;
            return NULL;
        }
        char hex[3] = {c1, c2, '\0'};
        decoded[i] = (char)(unsigned char)strtol(hex, NULL, 16);
    }
    decoded[decoded_len] = '\0';

    if (out_len) *out_len = decoded_len;
    return decoded;
}