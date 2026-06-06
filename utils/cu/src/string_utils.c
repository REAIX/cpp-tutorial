/**
 * @file string_utils.c
 * @brief 字符串处理工具实现 (C 版本)
 *
 * 实现字符串截取、空白判断、填充、十六进制转换等功能。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#include "cu/string_utils.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>

/**
 * @brief 将 NULL 指针转换为空字符串
 *
 * @param s 字符串指针
 * @return 非空字符串
 */
static const char* null_to_empty(const char* s) {
    return s == NULL ? "" : s;
}

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
char* sub_str(const char* src, const char* fstr, const char* lstr) {
    if (!src) return strdup("");

    const char* f = null_to_empty(fstr);
    const char* l = null_to_empty(lstr);

    /* 查找起始标记 */
    char* start = strstr(src, f);
    if (!start) return strdup("");
    start += strlen(f);

    /* 查找结束标记：如果l为空，则截取到字符串末尾 */
    char* end;
    if (l[0] != '\0') {
        end = strstr(start, l);
        if (!end) return strdup("");
    } else {
        end = start + strlen(start);
    }

    /* 截取子串 */
    size_t len = end - start;
    char* result = (char*)malloc(len + 1);
    if (result) {
        strncpy(result, start, len);
        result[len] = '\0';
    }
    return result;
}

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
char* lsub_str(const char* src, const char* fstr, const char* lstr) {
    if (!src) return strdup("");

    const char* f = null_to_empty(fstr);
    const char* l = null_to_empty(lstr);

    /* 查找起始标记 */
    char* start = strstr(src, f);
    if (!start) return strdup("");

    /* 查找结束标记：如果l为空，则截取到字符串末尾 */
    char* end;
    if (l[0] != '\0') {
        end = strstr(start, l);
        if (!end) return strdup("");
    } else {
        end = start + strlen(start);
    }

    /* 截取子串 */
    size_t len = end - start;
    char* result = (char*)malloc(len + 1);
    if (result) {
        strncpy(result, start, len);
        result[len] = '\0';
    }
    return result;
}

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
char* rsub_str(const char* src, const char* fstr, const char* lstr) {
    if (!src) return strdup("");

    const char* f = null_to_empty(fstr);
    const char* l = null_to_empty(lstr);

    /* 查找起始标记 */
    char* start = strstr(src, f);
    if (!start) return strdup("");
    start += strlen(f);

    /* 查找结束标记：如果l为空，则截取到字符串末尾 */
    char* end;
    if (l[0] != '\0') {
        end = strstr(start, l);
        if (!end) return strdup("");
        end += strlen(l);
    } else {
        end = start + strlen(start);
    }

    /* 截取子串 */
    size_t len = end - start;
    char* result = (char*)malloc(len + 1);
    if (result) {
        strncpy(result, start, len);
        result[len] = '\0';
    }
    return result;
}

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
char* asub_str(const char* src, const char* fstr, const char* lstr) {
    if (!src) return strdup("");

    const char* f = null_to_empty(fstr);
    const char* l = null_to_empty(lstr);

    /* 查找起始标记 */
    char* start = strstr(src, f);
    if (!start) return strdup("");

    /* 查找结束标记：如果l为空，则截取到字符串末尾 */
    char* end;
    if (l[0] != '\0') {
        end = strstr(start, l);
        if (!end) return strdup("");
        end += strlen(l);
    } else {
        end = start + strlen(start);
    }

    /* 截取子串 */
    size_t len = end - start;
    char* result = (char*)malloc(len + 1);
    if (result) {
        strncpy(result, start, len);
        result[len] = '\0';
    }
    return result;
}

/**
 * @brief 判断字符串是否为空
 *
 * @param str 字符串指针
 * @return 为空返回1，否则返回0
 */
int is_empty(const char* str) {
    return str == NULL || str[0] == '\0';
}

/**
 * @brief 判断字符串是否为空白
 *
 * 检查是否为 NULL 或仅包含空白字符。
 *
 * @param str 字符串指针
 * @return 为空白返回1，否则返回0
 */
int is_blank(const char* str) {
    if (str == NULL) return 1;

    for (size_t i = 0; str[i]; i++) {
        if (!isspace((unsigned char)str[i])) {
            return 0;
        }
    }
    return 1;
}

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
char* left_pad(const char* input, int length, char pad) {
    if (length < 0) return strdup(null_to_empty(input));
    const char* in = null_to_empty(input);
    size_t in_len = strlen(in);

    if (in_len >= (size_t)length) {
        return strdup(in);
    }

    /* 计算需要填充的字符数 */
    size_t pad_len = length - in_len;
    char* result = (char*)malloc(length + 1);
    if (result) {
        memset(result, pad, pad_len);
        strcpy(result + pad_len, in);
        result[length] = '\0';
    }
    return result;
}

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
char* right_pad(const char* input, int length, char pad) {
    if (length < 0) return strdup(null_to_empty(input));
    const char* in = null_to_empty(input);
    size_t in_len = strlen(in);

    if (in_len >= (size_t)length) {
        return strdup(in);
    }

    /* 计算需要填充的字符数 */
    size_t pad_len = length - in_len;
    char* result = (char*)malloc(length + 1);
    if (result) {
        strcpy(result, in);
        memset(result + in_len, pad, pad_len);
        result[length] = '\0';
    }
    return result;
}

/**
 * @brief 合并连续制表符
 *
 * 将连续多个制表符 \\t 合并为一个 \\t。
 *
 * @param str 输入字符串
 * @return 新建的字符串（需手动释放）
 */
char* one_tab(const char* str) {
    if (!str) return strdup("");

    size_t len = strlen(str);
    char* result = (char*)malloc(len + 1);
    if (!result) return NULL;

    size_t i = 0, j = 0;
    while (str[i]) {
        if (str[i] == '\t') {
            result[j++] = '\t';
            while (str[i + 1] == '\t') {
                i++;
            }
        } else {
            result[j++] = str[i];
        }
        i++;
    }
    result[j] = '\0';

    return result;
}

/**
 * @brief 合并连续空格
 *
 * 将制表符替换为空格，再将连续空格合并为一个空格。
 *
 * @param str 输入字符串
 * @return 新建的字符串（需手动释放）
 */
char* one_blank(const char* str) {
    if (!str) return strdup("");

    size_t len = strlen(str);
    char* result = (char*)malloc(len + 1);
    if (!result) return NULL;

    size_t i = 0, j = 0;
    while (str[i]) {
        if (str[i] == '\t') {
            result[j++] = ' ';
            while (str[i + 1] == '\t' || str[i + 1] == ' ') {
                i++;
            }
        } else if (str[i] == ' ') {
            result[j++] = ' ';
            while (str[i + 1] == ' ' || str[i + 1] == '\t') {
                i++;
            }
        } else {
            result[j++] = str[i];
        }
        i++;
    }
    result[j] = '\0';

    return result;
}

/**
 * @brief 统计子字符串出现次数
 *
 * 统计 substr 在 str 中不重叠出现的次数。
 *
 * @param str 源字符串
 * @param substr 子字符串
 * @return 子字符串出现的次数
 */
int str_count(const char* str, const char* substr) {
    if (!str || !substr) return 0;

    int count = 0;
    size_t substr_len = strlen(substr);
    if (substr_len == 0) return 0;

    const char* pos = str;
    while ((pos = strstr(pos, substr)) != NULL) {
        count++;
        pos += substr_len;
    }

    return count;
}

/**
 * @brief 十六进制字符串转 ASCII 字符串
 *
 * 将十六进制字符串解码为 ASCII 字符串，会去掉空格；
 * 长度为奇数时左侧补 0。
 *
 * @param hex_str 十六进制字符串
 * @return 新建的 ASCII 字符串（需手动释放）
 */
char* hex2_ascii(const char* hex_str) {
    if (!hex_str) return strdup("");

    /* 去掉空格 */
    size_t len = strlen(hex_str);
    char* cleaned = (char*)malloc(len + 1);
    if (!cleaned) return NULL;

    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        if (hex_str[i] != ' ') {
            cleaned[j++] = hex_str[i];
        }
    }
    cleaned[j] = '\0';

    /* 长度为奇数时左侧补 0 */
    if (j % 2 != 0) {
        char* temp = (char*)malloc(j + 2);
        if (!temp) {
            free(cleaned);
            return NULL;
        }
        temp[0] = '0';
        strcpy(temp + 1, cleaned);
        free(cleaned);
        cleaned = temp;
        j++;
    }

    /* 转换为 ASCII */
    size_t ascii_len = j / 2;
    char* result = (char*)malloc(ascii_len + 1);
    if (!result) {
        free(cleaned);
        return NULL;
    }

    for (size_t i = 0; i < ascii_len; i++) {
        char c1 = cleaned[i * 2];
        char c2 = cleaned[i * 2 + 1];
        if (!isxdigit((unsigned char)c1) || !isxdigit((unsigned char)c2)) {
            free(result);
            free(cleaned);
            return strdup("");
        }
        char hex[3] = {c1, c2, '\0'};
        result[i] = (char)(unsigned char)strtol(hex, NULL, 16);
    }
    result[ascii_len] = '\0';

    free(cleaned);
    return result;
}

/**
 * @brief 字符串转十六进制表示
 *
 * @param input 输入字符串
 * @return 新建的十六进制字符串（需手动释放）
 */
char* string_to_hex(const char* input) {
    if (!input) return strdup("");

    size_t len = strlen(input);
    char* result = (char*)malloc(len * 2 + 1);
    if (!result) return NULL;

    for (size_t i = 0; i < len; i++) {
        sprintf(result + i * 2, "%02X", (unsigned char)input[i]);
    }
    result[len * 2] = '\0';

    return result;
}

/**
 * @brief 字符串转十六进制字节列表
 *
 * @param input 输入字符串
 * @return 新建的十六进制字节列表（需手动调用 string_array_destroy 销毁）
 */
StringArray* string_to_hex_list(const char* input) {
    StringArray* result = string_array_create();
    if (!result) return NULL;

    if (!input) return result;

    size_t len = strlen(input);
    for (size_t i = 0; i < len; i++) {
        char hex[3];
        sprintf(hex, "%02X", (unsigned char)input[i]);
        string_array_add(result, hex);
    }

    return result;
}

/**
 * @brief 字符串居中填充
 *
 * 将字符串居中填充到 total_length，左右用 padding_char 填充；
 * 不足时左少右多。padding_char 为空时用空格。
 *
 * @param input 输入字符串
 * @param total_length 总长度
 * @param padding_char 填充字符
 * @return 新建的字符串（需手动释放）
 */
char* format_center(const char* input, int total_length, const char* padding_char) {
    const char* in = null_to_empty(input);
    size_t in_len = strlen(in);

    if (total_length < 0 || in_len >= (size_t)total_length) {
        return strdup(in);
    }

    const char* pad = padding_char && padding_char[0] ? padding_char : " ";

    int total_pad = total_length - (int)in_len;
    int left_pad = total_pad / 2;
    int right_pad = total_pad - left_pad;

    size_t result_len = in_len + left_pad + right_pad;
    char* result = (char*)malloc(result_len + 1);
    if (!result) return NULL;

    /* 添加左侧填充 */
    size_t pos = 0;
    for (int i = 0; i < left_pad; i++) {
        result[pos++] = pad[0];
    }

    /* 复制原字符串 */
    strcpy(result + pos, in);
    pos += in_len;

    /* 添加右侧填充 */
    for (int i = 0; i < right_pad; i++) {
        result[pos++] = pad[0];
    }

    result[pos] = '\0';

    return result;
}

/**
 * @brief 去掉行首的第一个制表符
 *
 * 如果行首是制表符则去掉，否则返回原字符串副本。
 *
 * @param line 输入字符串
 * @return 新建的字符串（需手动释放）
 */
char* remove_first_tab(const char* line) {
    if (!line || line[0] != '\t') {
        return strdup(null_to_empty(line));
    }

    return strdup(line + 1);
}

StringArray* str_split(const char* str, const char* delimiter) {
    StringArray* arr = string_array_create();
    if (!arr) return NULL;
    if (!str) return arr;

    const char* delim = delimiter ? delimiter : "";
    size_t delim_len = strlen(delim);

    if (delim_len == 0) {
        for (size_t i = 0; str[i]; i++) {
            char s[2] = {str[i], '\0'};
            string_array_add(arr, s);
        }
        return arr;
    }

    const char* start = str;
    const char* pos = strstr(start, delim);

    while (pos) {
        size_t len = pos - start;
        char* token = (char*)malloc(len + 1);
        if (token) {
            strncpy(token, start, len);
            token[len] = '\0';
            string_array_add(arr, token);
            free(token);
        }
        start = pos + delim_len;
        pos = strstr(start, delim);
    }

    string_array_add(arr, start);

    return arr;
}

char* str_join(const StringArray* arr, const char* delimiter) {
    if (!arr || arr->size == 0) return strdup("");

    const char* delim = delimiter ? delimiter : "";
    size_t delim_len = strlen(delim);

    size_t total_len = 0;
    for (size_t i = 0; i < arr->size; i++) {
        total_len += strlen(arr->elements[i]);
        if (i < arr->size - 1) {
            total_len += delim_len;
        }
    }

    char* result = (char*)malloc(total_len + 1);
    if (!result) return NULL;

    size_t pos = 0;
    for (size_t i = 0; i < arr->size; i++) {
        size_t elem_len = strlen(arr->elements[i]);
        strcpy(result + pos, arr->elements[i]);
        pos += elem_len;
        if (i < arr->size - 1) {
            strcpy(result + pos, delim);
            pos += delim_len;
        }
    }
    result[pos] = '\0';

    return result;
}

char* str_trim(const char* str) {
    if (!str) return strdup("");

    const char* start = str;
    while (*start && (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r')) {
        start++;
    }

    if (*start == '\0') return strdup("");

    const char* end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        end--;
    }

    size_t len = end - start + 1;
    char* result = (char*)malloc(len + 1);
    if (result) {
        strncpy(result, start, len);
        result[len] = '\0';
    }
    return result;
}

char* str_ltrim(const char* str) {
    if (!str) return strdup("");

    const char* start = str;
    while (*start && (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r')) {
        start++;
    }

    return strdup(start);
}

char* str_rtrim(const char* str) {
    if (!str) return strdup("");

    size_t len = strlen(str);
    const char* end = str + len - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        end--;
    }

    size_t new_len = end - str + 1;
    char* result = (char*)malloc(new_len + 1);
    if (result) {
        strncpy(result, str, new_len);
        result[new_len] = '\0';
    }
    return result;
}

char* str_replace(const char* str, const char* old_str, const char* new_str) {
    if (!str) return strdup("");
    if (!old_str || old_str[0] == '\0') return strdup(str);
    if (!new_str) new_str = "";

    size_t old_len = strlen(old_str);
    size_t new_len = strlen(new_str);

    int count = 0;
    const char* pos = str;
    while ((pos = strstr(pos, old_str)) != NULL) {
        count++;
        pos += old_len;
    }

    size_t src_len = strlen(str);
    size_t result_len;
    if (new_len >= old_len) {
        result_len = src_len + (size_t)count * (new_len - old_len);
    } else {
        result_len = src_len - (size_t)count * (old_len - new_len);
    }
    char* result = (char*)malloc(result_len + 1);
    if (!result) return NULL;

    char* dst = result;
    const char* src = str;
    while ((pos = strstr(src, old_str)) != NULL) {
        size_t chunk = pos - src;
        strncpy(dst, src, chunk);
        dst += chunk;
        strcpy(dst, new_str);
        dst += new_len;
        src = pos + old_len;
    }
    strcpy(dst, src);

    return result;
}

char* str_replace_first(const char* str, const char* old_str, const char* new_str) {
    if (!str) return strdup("");
    if (!old_str || old_str[0] == '\0') return strdup(str);
    if (!new_str) new_str = "";

    const char* pos = strstr(str, old_str);
    if (!pos) return strdup(str);

    size_t old_len = strlen(old_str);
    size_t new_len = strlen(new_str);
    size_t prefix_len = pos - str;
    size_t suffix_len = strlen(pos + old_len);

    char* result = (char*)malloc(prefix_len + new_len + suffix_len + 1);
    if (!result) return NULL;

    strncpy(result, str, prefix_len);
    strcpy(result + prefix_len, new_str);
    strcpy(result + prefix_len + new_len, pos + old_len);

    return result;
}

char* str_to_upper(const char* str) {
    if (!str) return strdup("");

    size_t len = strlen(str);
    char* result = (char*)malloc(len + 1);
    if (!result) return NULL;

    for (size_t i = 0; i < len; i++) {
        result[i] = (char)toupper((unsigned char)str[i]);
    }
    result[len] = '\0';

    return result;
}

char* str_to_lower(const char* str) {
    if (!str) return strdup("");

    size_t len = strlen(str);
    char* result = (char*)malloc(len + 1);
    if (!result) return NULL;

    for (size_t i = 0; i < len; i++) {
        result[i] = (char)tolower((unsigned char)str[i]);
    }
    result[len] = '\0';

    return result;
}

int str_starts_with(const char* str, const char* prefix) {
    if (!str || !prefix) return 0;
    size_t str_len = strlen(str);
    size_t prefix_len = strlen(prefix);
    if (prefix_len > str_len) return 0;
    return strncmp(str, prefix, prefix_len) == 0;
}

int str_ends_with(const char* str, const char* suffix) {
    if (!str || !suffix) return 0;
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    if (suffix_len > str_len) return 0;
    return strcmp(str + str_len - suffix_len, suffix) == 0;
}

int str_contains(const char* str, const char* substr) {
    if (!str || !substr) return 0;
    return strstr(str, substr) != NULL;
}

char* str_format(const char* fmt, ...) {
    if (!fmt) return strdup("");

    va_list args;
    va_start(args, fmt);

    va_list args_copy;
    va_copy(args_copy, args);
    int needed = vsnprintf(NULL, 0, fmt, args_copy);
    va_end(args_copy);

    if (needed < 0) {
        va_end(args);
        return strdup("");
    }

    char* result = (char*)malloc(needed + 1);
    if (!result) {
        va_end(args);
        return NULL;
    }

    vsnprintf(result, needed + 1, fmt, args);
    va_end(args);

    return result;
}

int is_alpha(const char* str) {
    if (!str || str[0] == '\0') return 0;

    for (size_t i = 0; str[i]; i++) {
        if (!isalpha((unsigned char)str[i])) return 0;
    }
    return 1;
}

int is_numeric(const char* str) {
    if (!str || str[0] == '\0') return 0;

    for (size_t i = 0; str[i]; i++) {
        if (!isdigit((unsigned char)str[i])) return 0;
    }
    return 1;
}

int is_alphanumeric(const char* str) {
    if (!str || str[0] == '\0') return 0;

    for (size_t i = 0; str[i]; i++) {
        if (!isalnum((unsigned char)str[i])) return 0;
    }
    return 1;
}

int is_email(const char* str) {
    if (!str || str[0] == '\0') return 0;

    const char* at = strchr(str, '@');
    if (!at) return 0;
    if (at == str) return 0;
    if (at[1] == '\0') return 0;

    const char* dot = strchr(at + 1, '.');
    if (!dot) return 0;
    if (dot == at + 1) return 0;
    if (dot[1] == '\0') return 0;

    for (const char* p = str; p < at; p++) {
        if (!isalnum((unsigned char)*p) && *p != '.' && *p != '_' && *p != '-') return 0;
    }

    return 1;
}