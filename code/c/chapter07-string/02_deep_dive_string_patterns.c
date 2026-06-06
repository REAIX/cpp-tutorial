/** @file 02_deep_dive_string_patterns.c
 *  @brief 字符串模式：解析、分词、构建器模式、常见算法(反转、回文、变位词)
 *  @description 对应文档: 07-字符串处理 | 举一反三：字符串处理的高级模式
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void demo_string_parsing(void) {
    printf("=== 字符串解析模式 ===\n");

    const char *config = "host=127.0.0.1;port=8080;timeout=30";

    char buf[128];
    strncpy(buf, config, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *pair = strtok(buf, ";");
    while (pair != NULL) {
        char key[32] = {0};
        char value[64] = {0};

        char *eq = strchr(pair, '=');
        if (eq != NULL) {
            size_t key_len = (size_t)(eq - pair);
            if (key_len >= sizeof(key)) key_len = sizeof(key) - 1;
            memcpy(key, pair, key_len);
            key[key_len] = '\0';

            strncpy(value, eq + 1, sizeof(value) - 1);
            value[sizeof(value) - 1] = '\0';

            printf("  key = \"%s\", value = \"%s\"\n", key, value);
        }
        pair = strtok(NULL, ";");
    }

    printf("\n");
}

void demo_tokenization_pattern(void) {
    printf("=== 分词模式 ===\n");

    const char *expr = "3 + 4 * 2 - 1";
    char buf[64];
    strncpy(buf, expr, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    printf("表达式: \"%s\"\n", expr);
    printf("分词结果:\n");

    char *token = strtok(buf, " ");
    while (token != NULL) {
        if (isdigit((unsigned char)token[0])) {
            printf("  数字: %d\n", atoi(token));
        } else {
            printf("  运算符: %s\n", token);
        }
        token = strtok(NULL, " ");
    }

    printf("\n");
}

typedef struct {
    char *data;
    size_t len;
    size_t capacity;
} StringBuilder;

void sb_init(StringBuilder *sb, size_t initial_cap) {
    sb->data = (char *)malloc(initial_cap);
    if (sb->data) {
        sb->data[0] = '\0';
        sb->len = 0;
        sb->capacity = initial_cap;
    }
}

void sb_append(StringBuilder *sb, const char *str) {
    size_t str_len = strlen(str);
    while (sb->len + str_len + 1 > sb->capacity) {
        sb->capacity *= 2;
        char *new_data = (char *)realloc(sb->data, sb->capacity);
        if (!new_data) return;
        sb->data = new_data;
    }
    memcpy(sb->data + sb->len, str, str_len + 1);
    sb->len += str_len;
}

void sb_free(StringBuilder *sb) {
    free(sb->data);
    sb->data = NULL;
    sb->len = 0;
    sb->capacity = 0;
}

void demo_string_builder(void) {
    printf("=== 字符串构建器模式 ===\n");

    StringBuilder sb;
    sb_init(&sb, 16);

    sb_append(&sb, "Hello");
    sb_append(&sb, ", ");
    sb_append(&sb, "World");
    sb_append(&sb, "!");

    printf("构建结果: \"%s\"\n", sb.data);
    printf("长度: %zu, 容量: %zu\n", sb.len, sb.capacity);

    sb_free(&sb);

    printf("\n优势: 避免反复 realloc, 自动扩容, O(n) 总时间\n");
    printf("对比: 多次 strcat 每次都要扫描到末尾, O(n^2) 总时间\n");

    printf("\n");
}

void string_reverse(char *str) {
    size_t len = strlen(str);
    for (size_t i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - 1 - i];
        str[len - 1 - i] = temp;
    }
}

void demo_string_reverse(void) {
    printf("=== 字符串反转 ===\n");

    char str1[] = "Hello, World!";
    printf("原字符串: \"%s\"\n", str1);
    string_reverse(str1);
    printf("反转后: \"%s\"\n", str1);

    char str2[] = "AB";
    printf("\n\"%s\" => ", str2);
    string_reverse(str2);
    printf("\"%s\"\n", str2);

    char str3[] = "A";
    printf("\"%s\" => ", str3);
    string_reverse(str3);
    printf("\"%s\"\n", str3);

    printf("\n");
}

int is_palindrome(const char *str) {
    size_t len = strlen(str);
    for (size_t i = 0; i < len / 2; i++) {
        if (str[i] != str[len - 1 - i]) {
            return 0;
        }
    }
    return 1;
}

int is_palindrome_ignore_case(const char *str) {
    size_t len = strlen(str);
    for (size_t i = 0; i < len / 2; i++) {
        char left = (char)tolower((unsigned char)str[i]);
        char right = (char)tolower((unsigned char)str[len - 1 - i]);
        if (left != right) return 0;
    }
    return 1;
}

void demo_palindrome(void) {
    printf("=== 回文检测 ===\n");

    const char *tests[] = {"racecar", "hello", "level", "Aba", "12321", "abc"};
    for (int i = 0; i < 6; i++) {
        printf("\"%s\": 回文=%s, 忽略大小写回文=%s\n",
               tests[i],
               is_palindrome(tests[i]) ? "是" : "否",
               is_palindrome_ignore_case(tests[i]) ? "是" : "否");
    }

    printf("\n");
}

int is_anagram(const char *s1, const char *s2) {
    int count[256] = {0};

    for (size_t i = 0; s1[i]; i++) {
        count[(unsigned char)s1[i]]++;
    }
    for (size_t i = 0; s2[i]; i++) {
        count[(unsigned char)s2[i]]--;
    }
    for (int i = 0; i < 256; i++) {
        if (count[i] != 0) return 0;
    }
    return 1;
}

void demo_anagram(void) {
    printf("=== 变位词检测 (Anagram) ===\n");

    const char *pairs[][2] = {
        {"listen", "silent"},
        {"hello", "world"},
        {"triangle", "integral"},
        {"abc", "abcd"}
    };

    for (int i = 0; i < 4; i++) {
        printf("\"%s\" 和 \"%s\": %s\n",
               pairs[i][0], pairs[i][1],
               is_anagram(pairs[i][0], pairs[i][1]) ? "是变位词" : "不是变位词");
    }

    printf("\n算法: 统计每个字符出现次数, 比较频率表\n");
    printf("时间复杂度: O(n), 空间复杂度: O(1) (固定256大小的计数数组)\n");

    printf("\n");
}

static char left_trim(char *s) {
    size_t len = strlen(s);
    size_t start = 0;
    while (start < len && isspace((unsigned char)s[start])) start++;
    if (start > 0) memmove(s, s + start, len - start + 1);
    return s[0];
}

static char right_trim(char *s) {
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        s[--len] = '\0';
    }
    return s[0];
}

void demo_trim_pattern(void) {
    printf("=== 字符串修剪 (Trim) ===\n");

    char str1[] = "   Hello, World!   ";
    printf("原字符串: \"%s\"\n", str1);
    left_trim(str1);
    printf("左修剪: \"%s\"\n", str1);
    right_trim(str1);
    printf("右修剪: \"%s\"\n", str1);

    printf("\n");
}

int main(void) {
    demo_string_parsing();
    demo_tokenization_pattern();
    demo_string_builder();
    demo_string_reverse();
    demo_palindrome();
    demo_anagram();
    demo_trim_pattern();

    return 0;
}
