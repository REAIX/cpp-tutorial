/** @file 01_deep_dive_string_security.c
 *  @brief 字符串安全：缓冲区溢出、安全替代函数、输入验证
 *  @description 对应文档: 07-字符串处理 | 举一反三：字符串安全编程
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

void demo_buffer_overflow_strcpy(void) {
    printf("=== strcpy 缓冲区溢出演示 ===\n");

    printf("危险代码:\n");
    printf("  char buf[5];\n");
    printf("  strcpy(buf, \"Hello, World!\");  // 溢出! 写入14字节到5字节缓冲区\n\n");

    printf("缓冲区溢出的后果:\n");
    printf("1. 覆盖栈上的其他变量\n");
    printf("2. 覆盖返回地址 (经典的栈溢出攻击)\n");
    printf("3. 程序崩溃或行为异常\n");
    printf("4. 安全漏洞: 攻击者可执行任意代码\n");

    printf("\n");
}

void demo_safe_strncpy(void) {
    printf("=== 安全替代: strncpy ===\n");

    char buf[8];
    const char *src = "Hello, World!";

    strncpy(buf, src, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    printf("strncpy + 手动终止: \"%s\"\n", buf);

    printf("\nstrncpy 的陷阱:\n");
    printf("1. 如果源字符串 >= n, 不会自动添加 '\\0'\n");
    printf("2. 如果源字符串 < n, 剩余部分填 '\\0' (可能浪费性能)\n");
    printf("3. 必须手动在 buf[n-1] 添加 '\\0'\n");

    printf("\n");
}

void demo_safe_snprintf(void) {
    printf("=== 安全替代: snprintf ===\n");

    char buf[32];
    int n = snprintf(buf, sizeof(buf), "Value: %d", 12345);
    printf("snprintf 结果: \"%s\"\n", buf);
    printf("snprintf 返回值: %d (本应写入的总字符数)\n", n);
    printf("实际写入: %zu 字节 (截断到缓冲区大小)\n", strlen(buf));

    printf("\nsnprintf 的优势:\n");
    printf("1. 保证结果以 '\\0' 结尾 (只要 size > 0)\n");
    printf("2. 返回值可判断是否发生截断\n");
    printf("3. 支持格式化, 比 strncpy 更灵活\n");

    printf("\n推荐: 用 snprintf 替代 strcpy 和 sprintf\n");

    printf("\n");
}

void demo_safe_strncat(void) {
    printf("=== 安全替代: strncat ===\n");

    char buf[10] = "Hi";
    strncat(buf, ", World!", sizeof(buf) - strlen(buf) - 1);
    printf("strncat 结果: \"%s\"\n", buf);

    printf("\nstrncat 的第三个参数是追加的最大字符数 (不含 '\\0')\n");
    printf("安全公式: strncat(dest, src, sizeof(dest) - strlen(dest) - 1)\n");

    printf("\n");
}

int is_valid_name(const char *name) {
    if (name == NULL) return 0;
    size_t len = strlen(name);
    if (len == 0 || len > 50) return 0;
    for (size_t i = 0; i < len; i++) {
        if (!isalpha((unsigned char)name[i]) && name[i] != ' ') {
            return 0;
        }
    }
    return 1;
}

int is_valid_age(const char *age_str) {
    if (age_str == NULL) return 0;
    for (size_t i = 0; age_str[i] != '\0'; i++) {
        if (!isdigit((unsigned char)age_str[i])) return 0;
    }
    long age = strtol(age_str, NULL, 10);
    return age >= 0 && age <= 150;
}

void demo_input_validation(void) {
    printf("=== 输入验证 ===\n");

    const char *test_names[] = {"Alice", "Bob123", "", "A very very very very very very very very very very very very long name"};
    for (int i = 0; i < 4; i++) {
        printf("名字 \"%s\": %s\n", test_names[i],
               is_valid_name(test_names[i]) ? "有效" : "无效");
    }

    const char *test_ages[] = {"25", "-5", "abc", "200"};
    for (int i = 0; i < 4; i++) {
        printf("年龄 \"%s\": %s\n", test_ages[i],
               is_valid_age(test_ages[i]) ? "有效" : "无效");
    }

    printf("\n输入验证原则:\n");
    printf("1. 永远不要信任用户输入\n");
    printf("2. 验证长度、类型、范围、格式\n");
    printf("3. 拒绝非法输入, 而非尝试修复\n");
    printf("4. 使用白名单而非黑名单\n");

    printf("\n");
}

void demo_format_string_security(void) {
    printf("=== 格式化字符串安全 ===\n");

    char user_input[] = "%s%s%s%s%s";

    printf("危险代码:\n");
    printf("  printf(user_input);  // 格式化字符串攻击!\n");
    printf("  用户输入的 %%s 会被当作格式说明符, 读取栈上的数据\n\n");

    printf("安全做法:\n");
    printf("  printf(\"%%s\", user_input);  // 将用户输入作为普通字符串\n");
    printf("  输出: %s\n", user_input);

    printf("\n规则: 永远不要将用户输入直接作为 printf 的格式字符串\n");

    printf("\n");
}

static int safe_str_copy(char *dst, size_t dst_size, const char *src) {
    if (dst == NULL || src == NULL || dst_size == 0) return -1;
    size_t src_len = strlen(src);
    size_t copy_len = src_len < dst_size - 1 ? src_len : dst_size - 1;
    memcpy(dst, src, copy_len);
    dst[copy_len] = '\0';
    return (int)copy_len;
}

void demo_safe_string_copy_utility(void) {
    printf("=== 安全字符串拷贝工具函数 ===\n");

    char buf[8];
    int copied = safe_str_copy(buf, sizeof(buf), "Hello, World!");
    printf("safe_str_copy: \"%s\" (复制了 %d 字符)\n", buf, copied);

    copied = safe_str_copy(buf, sizeof(buf), "Hi");
    printf("safe_str_copy: \"%s\" (复制了 %d 字符)\n", buf, copied);

    printf("\n封装安全函数, 减少重复的边界检查代码\n");

    printf("\n");
}

void demo_security_best_practices(void) {
    printf("=== 字符串安全最佳实践总结 ===\n");
    printf("1. 用 snprintf 替代 sprintf\n");
    printf("2. 用 strncpy + 手动终止 替代 strcpy\n");
    printf("3. 用 strncat 替代 strcat\n");
    printf("4. 用 strtol/strtod 替代 atoi/atof\n");
    printf("5. 始终验证输入的长度、类型和范围\n");
    printf("6. 永远不要将用户输入作为格式字符串\n");
    printf("7. 分配缓冲区时预留足够空间\n");
    printf("8. 使用静态分析工具检测潜在溢出\n");
    printf("\n");
}

int main(void) {
    demo_buffer_overflow_strcpy();
    demo_safe_strncpy();
    demo_safe_snprintf();
    demo_safe_strncat();
    demo_input_validation();
    demo_format_string_security();
    demo_safe_string_copy_utility();
    demo_security_best_practices();

    return 0;
}
