/**
 * @file test_utils.c
 * @brief C Utils 测试程序
 *
 * 测试字符串数组、集合运算、映射、字符串操作、文件操作等功能。
 * 使用自定义断言宏进行验证，并统计通过率。
 *
 * @author CU Utils Project
 * @version 2.0
 */

#include "cu/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ==================== 简易测试框架 ==================== */

static int g_tests_passed = 0;
static int g_tests_failed = 0;
static int g_tests_total = 0;

#define TEST_ASSERT(condition, message)                                       \
    do {                                                                      \
        g_tests_total++;                                                      \
        if (condition) {                                                      \
            g_tests_passed++;                                                 \
        } else {                                                              \
            g_tests_failed++;                                                 \
            printf("  FAIL: %s (at %s:%d)\n", message, __FILE__, __LINE__);   \
        }                                                                     \
    } while (0)

#define TEST_ASSERT_EQ(expected, actual, message)                             \
    do {                                                                      \
        g_tests_total++;                                                      \
        if ((expected) == (actual)) {                                         \
            g_tests_passed++;                                                 \
        } else {                                                              \
            g_tests_failed++;                                                 \
            printf("  FAIL: %s — expected %d, got %d (at %s:%d)\n",           \
                   message, (int)(expected), (int)(actual),                    \
                   __FILE__, __LINE__);                                        \
        }                                                                     \
    } while (0)

#define TEST_ASSERT_STR_EQ(expected, actual, message)                         \
    do {                                                                      \
        g_tests_total++;                                                      \
        const char* _e = (expected);                                          \
        const char* _a = (actual);                                            \
        if (_e && _a && strcmp(_e, _a) == 0) {                               \
            g_tests_passed++;                                                 \
        } else {                                                              \
            g_tests_failed++;                                                 \
            printf("  FAIL: %s — expected \"%s\", got \"%s\" (at %s:%d)\n",   \
                   message, _e ? _e : "(null)", _a ? _a : "(null)",           \
                   __FILE__, __LINE__);                                        \
        }                                                                     \
    } while (0)

#define TEST_SUMMARY()                                                        \
    do {                                                                      \
        printf("\n=== Test Summary ===\n");                                    \
        printf("Total: %d  Passed: %d  Failed: %d\n",                         \
               g_tests_total, g_tests_passed, g_tests_failed);                \
        if (g_tests_total > 0) {                                              \
            printf("Pass rate: %.1f%%\n",                                     \
                   100.0 * g_tests_passed / g_tests_total);                   \
        }                                                                     \
        printf("==============================\n");                            \
    } while (0)

/* ==================== 测试用例 ==================== */

/**
 * @brief 主测试函数
 * @return 测试全部通过返回0，存在失败返回1
 */
int main() {
    printf("=== Test C Utils Library ===\n\n");

    /* 测试1: 字符串数组操作 */
    printf("1. Test String Array\n");
    StringArray* arr = string_array_create();
    string_array_add(arr, "apple");
    string_array_add(arr, "banana");
    string_array_add(arr, "cherry");

    TEST_ASSERT_EQ(3, (int)arr->size, "arr->size should be 3 after adding 3 elements");
    TEST_ASSERT(string_array_contains(arr, "banana"), "arr should contain 'banana'");
    TEST_ASSERT(!string_array_contains(arr, "orange"), "arr should not contain 'orange'");

    /* 测试2: 集合运算 */
    printf("\n2. Test Set Operations\n");
    StringArray* set1 = string_array_create();
    string_array_add(set1, "a");
    string_array_add(set1, "b");
    string_array_add(set1, "c");

    StringArray* set2 = string_array_create();
    string_array_add(set2, "b");
    string_array_add(set2, "c");
    string_array_add(set2, "d");

    StringArray* intersection = set_intersection(set1, set2);
    printf("Intersection: ");
    prt_list(intersection);
    TEST_ASSERT(string_array_contains(intersection, "b"), "intersection should contain 'b'");
    TEST_ASSERT(string_array_contains(intersection, "c"), "intersection should contain 'c'");
    TEST_ASSERT(!string_array_contains(intersection, "a"), "intersection should not contain 'a'");
    TEST_ASSERT(!string_array_contains(intersection, "d"), "intersection should not contain 'd'");

    StringArray* union_set = set_union(set1, set2);
    printf("Union: ");
    prt_list(union_set);
    TEST_ASSERT(string_array_contains(union_set, "a"), "union should contain 'a'");
    TEST_ASSERT(string_array_contains(union_set, "b"), "union should contain 'b'");
    TEST_ASSERT(string_array_contains(union_set, "c"), "union should contain 'c'");
    TEST_ASSERT(string_array_contains(union_set, "d"), "union should contain 'd'");

    StringArray* difference = set_difference(set1, set2);
    printf("Difference: ");
    prt_list(difference);
    TEST_ASSERT(string_array_contains(difference, "a"), "difference should contain 'a'");
    TEST_ASSERT(!string_array_contains(difference, "b"), "difference should not contain 'b'");

    /* 测试3: 映射操作 */
    printf("\n3. Test Map Operations\n");
    IntMap* map = int_map_create();
    int_map_put(map, 1, "one");
    int_map_put(map, 2, "two");
    int_map_put(map, 3, "three");

    TEST_ASSERT_STR_EQ("two", int_map_get(map, 2), "int_map_get(map, 2) should return 'two'");
    TEST_ASSERT_STR_EQ("one", int_map_get(map, 1), "int_map_get(map, 1) should return 'one'");
    TEST_ASSERT_STR_EQ("three", int_map_get(map, 3), "int_map_get(map, 3) should return 'three'");
    printf("Max index: %d\n", int_map_get_max_index(map));
    printf("Value 'two's key: %d\n", int_map_get_key_by_value(map, "two"));

    /* 测试4: 字符串操作 */
    printf("\n4. Test String Operations\n");
    const char* str = "Hello, World!";
    printf("Original: %s\n", str);
    TEST_ASSERT(!is_empty(str), "'Hello, World!' should not be empty");
    TEST_ASSERT(!is_blank(str), "'Hello, World!' should not be blank");

    char* padded = left_pad(str, 20, '-');
    printf("left_pad: %s\n", padded);
    TEST_ASSERT_STR_EQ("-------Hello, World!", padded, "left_pad result");
    free(padded);

    /* 测试5: 十六进制转换 */
    printf("\n5. Test Hex Conversion\n");
    char* hex = string_to_hex("test");
    printf("string_to_hex: %s\n", hex);

    char* ascii = hex2_ascii(hex);
    printf("hex2_ascii: %s\n", ascii);
    TEST_ASSERT_STR_EQ("test", ascii, "hex2_ascii(string_to_hex('test')) should return 'test'");

    free(hex);
    free(ascii);

    /* 测试6: 文件操作 */
    printf("\n6. Test File Operations\n");
    StringArray* lines = string_array_create();
    string_array_add(lines, "Line 1");
    string_array_add(lines, "Line 2");
    string_array_add(lines, "Line 3");

    if (write_lines("test.txt", lines)) {
        printf("File write success\n");

        StringArray* lines_read = read_lines("test.txt");
        printf("Read file content:\n");
        prt_list(lines_read);

        TEST_ASSERT(lines_read != NULL, "read_lines should return non-NULL");
        TEST_ASSERT_EQ(3, (int)lines_read->size, "read_lines should return 3 lines");
        if (lines_read && lines_read->size >= 1) {
            TEST_ASSERT_STR_EQ("Line 1", lines_read->data[0], "first line should be 'Line 1'");
        }
        if (lines_read && lines_read->size >= 2) {
            TEST_ASSERT_STR_EQ("Line 2", lines_read->data[1], "second line should be 'Line 2'");
        }
        if (lines_read && lines_read->size >= 3) {
            TEST_ASSERT_STR_EQ("Line 3", lines_read->data[2], "third line should be 'Line 3'");
        }

        string_array_destroy(lines_read);
    } else {
        printf("File write failed\n");
        TEST_ASSERT(0, "write_lines should succeed");
    }

    /* 释放所有资源 */
    string_array_destroy(arr);
    string_array_destroy(set1);
    string_array_destroy(set2);
    string_array_destroy(intersection);
    string_array_destroy(union_set);
    string_array_destroy(difference);
    string_array_destroy(lines);
    int_map_destroy(map);

    TEST_SUMMARY();
    return g_tests_failed > 0 ? 1 : 0;
}
