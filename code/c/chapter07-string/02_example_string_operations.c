/** @file 02_example_string_operations.c
 *  @brief 字符串操作：strcat、strcmp、strchr、strstr、strtok、sprintf、sscanf
 *  @description 对应文档: 07-字符串处理
 */

#include <stdio.h>
#include <string.h>

void demo_strcat(void) {
    printf("=== strcat / strncat ===\n");

    char dest[30] = "Hello";
    strcat(dest, ", ");
    strcat(dest, "World!");
    printf("strcat 结果: \"%s\"\n", dest);

    char dest2[10] = "Hi";
    strncat(dest2, ", World!", sizeof(dest2) - strlen(dest2) - 1);
    printf("strncat 结果: \"%s\" (安全版本, 限制追加长度)\n", dest2);

    printf("\nstrcat 危险: 不检查目标缓冲区大小\n");
    printf("strncat 安全: 最多追加 n 个字符, 并自动添加 '\\0'\n");
    printf("\n");
}

void demo_strcmp(void) {
    printf("=== strcmp / strncmp ===\n");

    printf("strcmp 比较规则: 逐字符比较 ASCII 值\n\n");

    const char *s1 = "apple";
    const char *s2 = "banana";
    const char *s3 = "apple";

    printf("strcmp(\"%s\", \"%s\") = %d (s1 < s2)\n", s1, s2, strcmp(s1, s2));
    printf("strcmp(\"%s\", \"%s\") = %d (s1 == s3)\n", s1, s3, strcmp(s1, s3));
    printf("strcmp(\"%s\", \"%s\") = %d (s2 > s1)\n", s2, s1, strcmp(s2, s1));

    printf("\n返回值: <0 表示 s1<s2, 0 表示相等, >0 表示 s1>s2\n");

    printf("\nstrncmp: 只比较前 n 个字符\n");
    printf("strncmp(\"apple\", \"apples\", 5) = %d\n", strncmp("apple", "apples", 5));
    printf("strncmp(\"apple\", \"apples\", 6) = %d\n", strncmp("apple", "apples", 6));

    printf("\n");
}

void demo_strchr_strrchr(void) {
    printf("=== strchr / strrchr ===\n");

    const char *str = "Hello, World!";

    char *pos = strchr(str, 'o');
    if (pos) {
        printf("strchr 查找第一个 'o': 位置 %td, 子串 \"%s\"\n", pos - str, pos);
    }

    char *last_pos = strrchr(str, 'o');
    if (last_pos) {
        printf("strrchr 查找最后一个 'o': 位置 %td, 子串 \"%s\"\n", last_pos - str, last_pos);
    }

    char *not_found = strchr(str, 'z');
    printf("strchr 查找 'z': %s\n", not_found ? "找到了" : "未找到 (返回 NULL)");

    printf("\n");
}

void demo_strstr(void) {
    printf("=== strstr ===\n");

    const char *text = "The quick brown fox jumps over the lazy dog";

    char *found = strstr(text, "fox");
    if (found) {
        printf("strstr 查找 \"fox\": 位置 %td\n", found - text);
        printf("从匹配位置开始: \"%s\"\n", found);
    }

    char *not_found = strstr(text, "cat");
    printf("strstr 查找 \"cat\": %s\n", not_found ? "找到了" : "未找到 (返回 NULL)");

    printf("\n");
}

void demo_strtok(void) {
    printf("=== strtok (字符串分割) ===\n");

    char str[] = "one,two;three:four";
    const char *delim = ",;:";

    printf("原字符串: \"%s\"\n", str);
    printf("分隔符: \"%s\"\n", delim);

    char *token = strtok(str, delim);
    int count = 0;
    while (token != NULL) {
        printf("  token[%d] = \"%s\"\n", count++, token);
        token = strtok(NULL, delim);
    }

    printf("\nstrtok 注意事项:\n");
    printf("1. 会修改原字符串 (将分隔符替换为 '\\0')\n");
    printf("2. 第一次调用传字符串, 后续传 NULL\n");
    printf("3. 不是线程安全的 (使用静态内部状态)\n");
    printf("4. C11 提供线程安全版本 strtok_r\n");

    printf("\n");
}

void demo_sprintf(void) {
    printf("=== sprintf / snprintf ===\n");

    char buf[50];
    int age = 25;
    float score = 92.5f;

    sprintf(buf, "年龄: %d, 分数: %.1f", age, score);
    printf("sprintf 结果: \"%s\"\n", buf);

    char safe_buf[50];
    snprintf(safe_buf, sizeof(safe_buf), "年龄: %d, 分数: %.1f", age, score);
    printf("snprintf 结果: \"%s\" (安全, 不会溢出)\n", safe_buf);

    printf("\nsprintf 危险: 不检查缓冲区大小\n");
    printf("snprintf 安全: 限制写入长度, 推荐始终使用\n");

    printf("\n");
}

void demo_sscanf(void) {
    printf("=== sscanf ===\n");

    const char *input = "2024-06-15";
    int year, month, day;
    sscanf(input, "%d-%d-%d", &year, &month, &day);
    printf("sscanf 解析日期: 年=%d, 月=%d, 日=%d\n", year, month, day);

    const char *csv = "Alice,25,88.5";
    char name[20];
    int age;
    float score;
    sscanf(csv, "%[^,],%d,%f", name, &age, &score);
    printf("sscanf 解析CSV: 名字=%s, 年龄=%d, 分数=%.1f\n", name, age, score);

    printf("\n%%[^,] 表示读取到逗号为止的所有字符 (扫描集)\n");

    printf("\n");
}

int main(void) {
    demo_strcat();
    demo_strcmp();
    demo_strchr_strrchr();
    demo_strstr();
    demo_strtok();
    demo_sprintf();
    demo_sscanf();

    return 0;
}
