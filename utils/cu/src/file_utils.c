/**
 * @file file_utils.c
 * @brief 文件处理工具实现 (C 版本)
 *
 * 实现文件读写、目录操作、路径处理等功能。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include "cu/file_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#include <sys/stat.h>
#else
#include <sys/stat.h>
#endif

/**
 * @brief 检查文件是否存在
 *
 * @param path 文件路径
 * @return 存在返回1，不存在返回0
 */
int file_exists(const char* path) {
    if (!path) return 0;

    struct stat st;
    return stat(path, &st) == 0;
}

/**
 * @brief 检查路径是否为目录
 *
 * @param path 路径
 * @return 是目录返回1，否则返回0
 */
int is_directory(const char* path) {
    if (!path) return 0;

    struct stat st;
    if (stat(path, &st) != 0) {
        return 0;
    }
    return S_ISDIR(st.st_mode);
}

/**
 * @brief 检查路径是否为常规文件
 *
 * @param path 路径
 * @return 是常规文件返回1，否则返回0
 */
int is_regular_file(const char* path) {
    if (!path) return 0;

    struct stat st;
    if (stat(path, &st) != 0) {
        return 0;
    }
    return S_ISREG(st.st_mode);
}

/**
 * @brief 按行读取文件
 *
 * 读取文件所有行到字符串数组中，每行末尾的换行符会被去掉。
 *
 * @param file_path 文件路径
 * @return 包含文件所有行的字符串数组（需手动调用 string_array_destroy 销毁）
 */
StringArray* read_lines(const char* file_path) {
    StringArray* result = string_array_create();
    if (!result) return NULL;

    FILE* fp = fopen(file_path, "r");
    if (!fp) return result;

    size_t buf_size = 1024;
    char* buffer = (char*)malloc(buf_size);
    if (!buffer) {
        fclose(fp);
        return result;
    }

    while (fgets(buffer, (int)buf_size, fp) != NULL) {
        size_t len = strlen(buffer);
        while (len == buf_size - 1 && buffer[len - 1] != '\n') {
            size_t new_buf_size = buf_size * 2;
            char* new_buffer = (char*)realloc(buffer, new_buf_size);
            if (!new_buffer) break;
            buffer = new_buffer;
            buf_size = new_buf_size;
            if (!fgets(buffer + len, (int)(buf_size - len), fp)) break;
            len = strlen(buffer);
        }
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }
        if (len > 0 && buffer[len - 1] == '\r') {
            buffer[len - 1] = '\0';
            len--;
        }
        string_array_add(result, buffer);
    }

    free(buffer);

    fclose(fp);

    return result;
}

/**
 * @brief 将行列表写入文件
 *
 * 覆盖模式，会清空原文件内容。
 *
 * @param file_path 文件路径
 * @param lines 字符串数组
 * @return 成功返回1，失败返回0
 */
int write_lines(const char* file_path, const StringArray* lines) {
    if (!file_path || !lines) return 0;

    FILE* fp = fopen(file_path, "w");
    if (!fp) return 0;

    for (size_t i = 0; i < lines->size; i++) {
        fprintf(fp, "%s\n", lines->elements[i]);
    }

    fclose(fp);
    return 1;
}

/**
 * @brief 追加行列表到文件
 *
 * 追加模式，在文件末尾添加内容。
 *
 * @param file_path 文件路径
 * @param lines 字符串数组
 * @return 成功返回1，失败返回0
 */
int append_lines(const char* file_path, const StringArray* lines) {
    if (!file_path || !lines) return 0;

    FILE* fp = fopen(file_path, "a");
    if (!fp) return 0;

    for (size_t i = 0; i < lines->size; i++) {
        fprintf(fp, "%s\n", lines->elements[i]);
    }

    fclose(fp);
    return 1;
}

/**
 * @brief 从路径获取文件名
 *
 * 自动处理 Windows 反斜杠和 Unix 正斜杠。
 *
 * @param path 文件路径
 * @return 文件名字符串（需手动释放）
 */
char* get_filename(const char* path) {
    if (!path) return strdup("");

    /* 查找最后一个路径分隔符 */
    char* last_slash = strrchr(path, '\\');
    if (!last_slash) {
        last_slash = strrchr(path, '/');
    }

    if (last_slash) {
        return strdup(last_slash + 1);
    } else {
        return strdup(path);
    }
}

/**
 * @brief 从路径获取目录部分
 *
 * @param path 文件路径
 * @return 目录字符串（需手动释放）
 */
char* get_directory(const char* path) {
    if (!path) return strdup("");

    /* 查找最后一个路径分隔符 */
    char* last_slash = strrchr(path, '\\');
    if (!last_slash) {
        last_slash = strrchr(path, '/');
    }

    if (last_slash) {
        size_t len = last_slash - path;
        char* result = (char*)malloc(len + 1);
        if (result) {
            strncpy(result, path, len);
            result[len] = '\0';
        }
        return result;
    } else {
        return strdup(".");
    }
}

/**
 * @brief 合并两个路径
 *
 * @param path1 第一个路径
 * @param path2 第二个路径
 * @return 合并后的路径字符串（需手动释放）
 */
char* combine_paths(const char* path1, const char* path2) {
    if (!path1 || !path2) return NULL;

    size_t len1 = strlen(path1);
    size_t len2 = strlen(path2);

    /* 检查 path1 是否以分隔符结尾 */
    int has_slash = (len1 > 0 && (path1[len1 - 1] == '\\' || path1[len1 - 1] == '/'));

    /* 计算结果长度 */
    size_t result_len = len1 + len2 + (has_slash ? 0 : 1) + 1;
    char* result = (char*)malloc(result_len);
    if (!result) return NULL;

    strcpy(result, path1);
    if (!has_slash) {
#ifdef _WIN32
        strcat(result, "\\");
#else
        strcat(result, "/");
#endif
    }
    strcat(result, path2);

    return result;
}

/**
 * @brief 获取文件大小
 *
 * @param file_path 文件路径
 * @return 文件大小（字节），失败返回-1
 */
long get_file_size(const char* file_path) {
    if (!file_path) return -1;

    struct stat st;
    if (stat(file_path, &st) != 0) {
        return -1;
    }

    return (long)st.st_size;
}

/**
 * @brief 检查指定位置是否在字符串内
 *
 * 用于排除字符串内的注释符号。
 * 正确处理转义字符：只有奇数个连续反斜杠才表示转义。
 *
 * @param line 行内容
 * @param position 要检查的位置
 * @return 在字符串内返回1，否则返回0
 */
static int is_in_string(const char* line, size_t position) {
    int in_string = 0;
    char string_char = 0;

    for (size_t i = 0; i < position && line[i]; i++) {
        char c = line[i];

        /* 处理引号（单引号或双引号） */
        if (c == '"' || c == '\'') {
            /* 统计当前位置之前的连续反斜杠数量 */
            int backslash_count = 0;
            for (int j = (int)i - 1; j >= 0 && line[j] == '\\'; j--) {
                backslash_count++;
            }

            /* 只有偶数个反斜杠（包括0个）时，引号才有效 */
            if (backslash_count % 2 == 0) {
                if (!in_string) {
                    in_string = 1;
                    string_char = c;
                } else if (c == string_char) {
                    in_string = 0;
                }
            }
        }
    }

    return in_string;
}

/**
 * @brief 查找多行注释标记
 *
 * @param comment_types 注释类型数组
 * @param num_types 注释类型数量
 * @param block_start 传出参数：块注释开始标记
 * @param block_end 传出参数：块注释结束标记
 * @return 找到返回1，未找到返回0
 */
static int find_block_comment_markers(const CommentType* comment_types, size_t num_types,
                                      const char** block_start, const char** block_end) {
    for (size_t i = 0; i < num_types; i++) {
        if (comment_types[i] == COMMENT_MULTI_LINE_C) {
            *block_start = "/*";
            *block_end = "*/";
            return 1;
        } else if (comment_types[i] == COMMENT_MULTI_LINE_HTML) {
            *block_start = "<!--";
            *block_end = "-->";
            return 1;
        }
    }
    return 0;
}

/**
 * @brief 处理单行注释
 *
 * @param line 行内容（会被修改）
 * @param comment_type 注释类型
 * @return 处理后的行内容
 */
static char* process_single_line_comment(const char* line, CommentType comment_type) {
    if (!line) return strdup("");

    char* result = strdup(line);
    if (!result) return NULL;

    switch (comment_type) {
        case COMMENT_SINGLE_LINE_C:
            /* C/C++ 单行注释 // */
            {
                char* idx = strstr(result, "//");
                if (idx && !is_in_string(result, idx - result)) {
                    *idx = '\0';
                }
            }
            break;

        case COMMENT_SINGLE_LINE_PYTHON:
            /* Python 单行注释 # */
            {
                char* idx = strchr(result, '#');
                if (idx && !is_in_string(result, idx - result)) {
                    *idx = '\0';
                }
            }
            break;

        case COMMENT_SINGLE_LINE_SQL:
            /* SQL 单行注释 -- */
            {
                char* idx = strstr(result, "--");
                if (idx && !is_in_string(result, idx - result)) {
                    *idx = '\0';
                }
            }
            break;

        case COMMENT_SINGLE_LINE_BATCH:
            /* Batch REM 或 :: */
            {
                char* upper = strdup(result);
                if (upper) {
                    for (char* p = upper; *p; p++) {
                        *p = toupper((unsigned char)*p);
                    }
                    char* rem_idx = NULL;
                    if (strncmp(upper, "REM ", 4) == 0) {
                        rem_idx = upper;
                    }
                    if (rem_idx) {
                        result[rem_idx - upper] = '\0';
                    } else {
                        if (strncmp(result, "::", 2) == 0) {
                            result[0] = '\0';
                        }
                    }
                    free(upper);
                }
            }
            break;

        default:
            break;
    }

    return result;
}

/**
 * @brief 移除文件中的指定类型注释
 *
 * 支持单行和多行注释类型。
 *
 * @param file_path 源文件路径
 * @param comment_types 要移除的注释类型数组
 * @param num_types 注释类型数量
 * @param output_path 输出文件路径（可选，为 NULL 时不输出）
 * @return 处理后的行列表（需手动调用 string_array_destroy 销毁）
 */
StringArray* remove_comments(const char* file_path, const CommentType* comment_types,
                             size_t num_types, const char* output_path) {
    if (!file_path) return NULL;

    /* 如果没有指定注释类型，直接读取原文件 */
    if (!comment_types || num_types == 0) {
        return read_lines(file_path);
    }

    /* 读取原文件 */
    StringArray* lines = read_lines(file_path);
    if (!lines) return NULL;

    /* 创建结果数组 */
    StringArray* result = string_array_create();
    if (!result) {
        string_array_destroy(lines);
        return NULL;
    }

    /* 查找块注释标记 */
    int in_block_comment = 0;
    const char* block_start = NULL;
    const char* block_end = NULL;
    find_block_comment_markers(comment_types, num_types, &block_start, &block_end);

    /* 处理每一行 */
    for (size_t i = 0; i < lines->size; i++) {
        const char* line = lines->elements[i];
        char* processed_line = strdup(line);
        if (!processed_line) continue;

        /* 处理块注释（跨行） */
        if (in_block_comment) {
            char* end_idx = processed_line ? strstr(processed_line, block_end) : NULL;
            if (end_idx) {
                /* 块注释结束 */
                char* temp = strdup(end_idx + strlen(block_end));
                free(processed_line);
                processed_line = temp;
                in_block_comment = 0;
            } else {
                /* 整行都是注释 */
                free(processed_line);
                continue;
            }
        }

        /* 处理块注释（单行） */
        if (block_start && processed_line && strstr(processed_line, block_start)) {
            char* start_idx = strstr(processed_line, block_start);
            if (start_idx && !is_in_string(processed_line, start_idx - processed_line)) {
                int skip_block = 0;
                char* single_c = strstr(processed_line, "//");
                char* single_py = strchr(processed_line, '#');
                char* single_sql = strstr(processed_line, "--");
                if (single_c && single_c < start_idx) skip_block = 1;
                if (single_py && single_py < start_idx) skip_block = 1;
                if (single_sql && single_sql < start_idx) skip_block = 1;

                if (!skip_block) {
                    do {
                        start_idx = strstr(processed_line, block_start);
                        if (!start_idx || is_in_string(processed_line, start_idx - processed_line)) break;
                        char* end_idx = strstr(start_idx + strlen(block_start), block_end);

                        if (end_idx) {
                            size_t prefix_len = start_idx - processed_line;
                            size_t suffix_start = end_idx + strlen(block_end) - processed_line;
                            size_t new_len = prefix_len + strlen(processed_line + suffix_start);
                            char* temp = (char*)malloc(new_len + 1);
                            if (temp) {
                                strncpy(temp, processed_line, prefix_len);
                                strcpy(temp + prefix_len, processed_line + suffix_start);
                                free(processed_line);
                                processed_line = temp;
                            } else {
                                break;
                            }
                        } else {
                            *start_idx = '\0';
                            in_block_comment = 1;
                            break;
                        }
                    } while (processed_line && strstr(processed_line, block_start));
                }
            }
        }

        /* 处理单行注释 */
        for (size_t j = 0; j < num_types && processed_line; j++) {
            if (comment_types[j] == COMMENT_SINGLE_LINE_C ||
                comment_types[j] == COMMENT_SINGLE_LINE_PYTHON ||
                comment_types[j] == COMMENT_SINGLE_LINE_SQL ||
                comment_types[j] == COMMENT_SINGLE_LINE_BATCH) {
                char* temp = process_single_line_comment(processed_line, comment_types[j]);
                free(processed_line);
                processed_line = temp;
            }
        }

        /* 添加到结果 */
        if (processed_line) {
            string_array_add(result, processed_line);
            free(processed_line);
        }
    }

    string_array_destroy(lines);

    /* 写入输出文件 */
    if (output_path) {
        write_lines(output_path, result);
    }

    return result;
}