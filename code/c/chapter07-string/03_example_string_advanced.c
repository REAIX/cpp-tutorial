/** @file 03_example_string_advanced.c
 *  @brief 字符串进阶：字符串数组、动态字符串、自定义字符串函数
 *  @description 对应文档: 07-字符串处理
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void demo_string_array(void) {
    printf("=== 字符串数组 ===\n");

    const char *names[] = {"Alice", "Bob", "Charlie", "Diana", "Eve"};
    int count = sizeof(names) / sizeof(names[0]);

    printf("方式1: 指针数组 (每个元素指向一个字符串字面量)\n");
    for (int i = 0; i < count; i++) {
        printf("  names[%d] = \"%s\" (长度 %zu)\n", i, names[i], strlen(names[i]));
    }

    char cities[][20] = {"Beijing", "Shanghai", "Guangzhou", "Shenzhen"};
    int city_count = sizeof(cities) / sizeof(cities[0]);

    printf("\n方式2: 二维字符数组 (每行固定大小)\n");
    for (int i = 0; i < city_count; i++) {
        printf("  cities[%d] = \"%s\"\n", i, cities[i]);
    }

    printf("\n对比:\n");
    printf("指针数组: 节省空间, 但指向的字面量不可修改\n");
    printf("二维数组: 可修改, 但每行大小固定, 可能浪费空间\n");

    printf("\n");
}

void demo_dynamic_string(void) {
    printf("=== 动态字符串 ===\n");

    char *str = (char *)malloc(10);
    if (str == NULL) {
        printf("内存分配失败\n");
        return;
    }
    strcpy(str, "Hello");
    printf("初始: str = \"%s\", 容量 = 10\n", str);

    size_t capacity = 10;
    const char *append = ", World!";
    size_t needed = strlen(str) + strlen(append) + 1;

    if (needed > capacity) {
        char *new_str = (char *)realloc(str, needed);
        if (new_str == NULL) {
            free(str);
            printf("重新分配失败\n");
            return;
        }
        str = new_str;
        capacity = needed;
        printf("扩容到 %zu 字节\n", capacity);
    }

    strcat(str, append);
    printf("追加后: str = \"%s\"\n", str);

    free(str);
    str = NULL;

    printf("\n");
}

size_t my_strlen(const char *s) {
    size_t len = 0;
    while (s[len] != '\0') {
        len++;
    }
    return len;
}

char *my_strcpy(char *dest, const char *src) {
    char *d = dest;
    while ((*d++ = *src++) != '\0') {
    }
    return dest;
}

int my_strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

void *my_memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

void demo_custom_string_functions(void) {
    printf("=== 自定义字符串函数 ===\n");

    const char *test = "Hello";
    printf("my_strlen(\"%s\") = %zu\n", test, my_strlen(test));

    char dest[20];
    my_strcpy(dest, test);
    printf("my_strcpy: dest = \"%s\"\n", dest);

    printf("my_strcmp(\"abc\", \"abd\") = %d\n", my_strcmp("abc", "abd"));
    printf("my_strcmp(\"abc\", \"abc\") = %d\n", my_strcmp("abc", "abc"));
    printf("my_strcmp(\"abd\", \"abc\") = %d\n", my_strcmp("abd", "abc"));

    char buf[10];
    my_memcpy(buf, "ABCDEFGHI", 5);
    buf[5] = '\0';
    printf("my_memcpy: buf = \"%s\"\n", buf);

    printf("\n理解标准库函数的实现原理, 有助于写出更高效的代码\n");

    printf("\n");
}

void demo_string_to_number(void) {
    printf("=== 字符串与数值转换 ===\n");

    printf("atoi: \"%s\" => %d\n", "42", atoi("42"));
    printf("atof: \"%s\" => %.2f\n", "3.14", atof("3.14"));
    printf("atoi 无效输入: \"%s\" => %d (行为未定义, 推荐用 strtol)\n",
           "abc", atoi("abc"));

    char *endptr;
    long val = strtol("42abc", &endptr, 10);
    printf("\nstrtol: \"42abc\" => %ld, 停止于: \"%s\"\n", val, endptr);

    long hex_val = strtol("0xFF", &endptr, 16);
    printf("strtol(16进制): \"0xFF\" => %ld\n", hex_val);

    printf("\n推荐: strtol/strtod 优于 atoi/atof, 因为有错误检测\n");

    printf("\n");
}

void demo_toupper_tolower(void) {
    printf("=== 字符串大小写转换 ===\n");

    char str[] = "Hello World 123";

    printf("原字符串: \"%s\"\n", str);

    for (int i = 0; str[i]; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] = str[i] - 'a' + 'A';
        }
    }
    printf("转大写: \"%s\"\n", str);

    for (int i = 0; str[i]; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] = str[i] - 'A' + 'a';
        }
    }
    printf("转小写: \"%s\"\n", str);

    printf("\n也可以使用 ctype.h 中的 toupper()/tolower()\n");

    printf("\n");
}

int main(void) {
    demo_string_array();
    demo_dynamic_string();
    demo_custom_string_functions();
    demo_string_to_number();
    demo_toupper_tolower();

    return 0;
}
