/** @file 01_example_string_basics.c
 *  @brief 字符串基础：字符串字面量、字符数组、空终止符、strlen、strcpy、strncpy
 *  @description 对应文档: 07-字符串处理
 */

#include <stdio.h>
#include <string.h>

void demo_string_literal(void) {
    printf("=== 字符串字面量 ===\n");

    char *s1 = "Hello";
    char *s2 = "Hello";

    printf("s1 = \"%s\", 地址: %p\n", s1, (void *)s1);
    printf("s2 = \"%s\", 地址: %p\n", s2, (void *)s2);
    printf("s1 == s2: %s (编译器可能合并相同字面量)\n", s1 == s2 ? "true" : "false");

    printf("\n字符串字面量存储在只读区, 修改它是未定义行为:\n");
    printf("  s1[0] = 'h';  // 错误! 可能崩溃\n");

    printf("\nsizeof 字面量包含末尾的 '\\0':\n");
    printf("  sizeof(\"Hello\") = %zu (5个字符 + 1个空字符)\n", sizeof("Hello"));

    printf("\n");
}

void demo_char_array(void) {
    printf("=== 字符数组 ===\n");

    char arr1[] = "Hello";
    printf("char arr1[] = \"Hello\";\n");
    printf("  arr1 = \"%s\"\n", arr1);
    printf("  sizeof(arr1) = %zu (自动包含 '\\0')\n", sizeof(arr1));
    printf("  字符数组可以修改:\n");
    arr1[0] = 'h';
    printf("  arr1[0] = 'h' => \"%s\"\n", arr1);

    char arr2[10] = "Hi";
    printf("\nchar arr2[10] = \"Hi\";\n");
    printf("  sizeof(arr2) = %zu\n", sizeof(arr2));
    printf("  strlen(arr2) = %zu (未使用的部分填0)\n", strlen(arr2));

    char arr3[4] = "ABC";
    printf("\nchar arr3[4] = \"ABC\";\n");
    printf("  大小4刚好容纳3个字符+'\\0'\n");
    printf("  若大小为3则无空间放'\\0', strlen可能越界\n");

    printf("\n");
}

void demo_null_terminator(void) {
    printf("=== 空终止符 '\\0' ===\n");

    char str[6] = {'H', 'e', 'l', 'l', 'o', '\0'};
    printf("手动初始化: str = \"%s\"\n", str);

    char no_term[5] = {'H', 'e', 'l', 'l', 'o'};
    (void)no_term;
    printf("\n没有 '\\0' 的字符数组:\n");
    printf("  no_term[5] = {'H','e','l','l','o'}\n");
    printf("  这不是有效的 C 字符串! strlen 等函数会越界\n");

    printf("\nC 字符串的本质: 以 '\\0' 结尾的字符数组\n");
    printf("所有字符串函数都依赖 '\\0' 来判断结尾\n");

    printf("\n");
}

void demo_strlen_function(void) {
    printf("=== strlen 函数 ===\n");

    const char *s = "Hello, World!";
    printf("字符串: \"%s\"\n", s);
    printf("strlen = %zu\n", strlen(s));
    printf("sizeof  = %zu (包含 '\\0')\n", sizeof("Hello, World!"));

    printf("\nstrlen 返回有效字符数, 不包含 '\\0'\n");
    printf("strlen 的时间复杂度是 O(n), 需要遍历到 '\\0'\n");

    printf("\n");
}

void demo_strcpy_function(void) {
    printf("=== strcpy 函数 ===\n");

    char src[] = "Hello, C!";
    char dest[20];

    strcpy(dest, src);
    printf("strcpy(dest, src) => dest = \"%s\"\n", dest);

    printf("\nstrcpy 的危险: 不检查目标缓冲区大小\n");
    printf("  char small[5];\n");
    printf("  strcpy(small, \"Hello, World!\");  // 缓冲区溢出!\n");

    printf("\n");
}

void demo_strncpy_function(void) {
    printf("=== strncpy 函数 ===\n");

    char src[] = "Hello, World!";
    char dest[8];

    strncpy(dest, src, sizeof(dest) - 1);
    dest[sizeof(dest) - 1] = '\0';
    printf("strncpy(dest, src, 7) + 手动加 '\\0' => \"%s\"\n", dest);

    printf("\nstrncpy 的注意事项:\n");
    printf("1. 如果源字符串短于 n, 会在剩余位置填 '\\0'\n");
    printf("2. 如果源字符串长于或等于 n, 不会自动添加 '\\0'!\n");
    printf("3. 必须手动在 dest[n-1] 处添加 '\\0'\n");

    char dest2[5];
    strncpy(dest2, "Hi", sizeof(dest2) - 1);
    dest2[sizeof(dest2) - 1] = '\0';
    printf("\n短字符串示例: strncpy(dest2, \"Hi\", 4) => \"%s\"\n", dest2);

    printf("\n");
}

void demo_string_initialization(void) {
    printf("=== 字符串初始化方式对比 ===\n");

    char way1[] = "Hello";
    printf("char way1[] = \"Hello\";  栈上数组, 可修改\n");

    const char *way2 = "Hello";
    printf("const char *way2 = \"Hello\";  指向字面量, 不可修改\n");

    char way3[20] = "Hello";
    printf("char way3[20] = \"Hello\";  指定大小, 多余填0\n");

    char way4[20];
    memset(way4, 0, sizeof(way4));
    strncpy(way4, "Hello", sizeof(way4) - 1);
    printf("memset + strncpy: 最安全的初始化方式\n");

    (void)way1; (void)way2; (void)way3; (void)way4;
    printf("\n");
}

int main(void) {
    demo_string_literal();
    demo_char_array();
    demo_null_terminator();
    demo_strlen_function();
    demo_strcpy_function();
    demo_strncpy_function();
    demo_string_initialization();

    return 0;
}
