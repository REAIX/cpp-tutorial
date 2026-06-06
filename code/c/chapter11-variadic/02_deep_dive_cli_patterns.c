/** @file 02_deep_dive_cli_patterns.c
 *  @brief CLI设计模式：子命令模式、参数验证、帮助文本生成
 *  @description 对应文档: 11-可变参数与命令行 | 举一反三：命令行工具的高级设计
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    const char *name;
    const char *description;
    int (*handler)(int argc, char *argv[]);
} Command;

void demo_subcommand_pattern(void) {
    printf("=== 子命令模式 ===\n");

    int cmd_add(int argc, char *argv[]) {
        if (argc < 2) {
            printf("  用法: program add <num1> <num2> ...\n");
            return 1;
        }
        double total = 0;
        for (int i = 1; i < argc; i++) {
            total += atof(argv[i]);
        }
        printf("  结果: %.2f\n", total);
        return 0;
    }

    int cmd_multiply(int argc, char *argv[]) {
        if (argc < 2) {
            printf("  用法: program multiply <num1> <num2> ...\n");
            return 1;
        }
        double product = 1.0;
        for (int i = 1; i < argc; i++) {
            product *= atof(argv[i]);
        }
        printf("  结果: %.2f\n", product);
        return 0;
    }

    int cmd_echo(int argc, char *argv[]) {
        for (int i = 1; i < argc; i++) {
            printf("%s ", argv[i]);
        }
        printf("\n");
        return 0;
    }

    Command commands[] = {
        {"add",      "加法运算",    cmd_add},
        {"multiply", "乘法运算",    cmd_multiply},
        {"echo",     "回显参数",    cmd_echo}
    };
    int cmd_count = sizeof(commands) / sizeof(commands[0]);

    void dispatch(const char *cmd_name, int argc, char *argv[]) {
        for (int i = 0; i < cmd_count; i++) {
            if (strcmp(commands[i].name, cmd_name) == 0) {
                commands[i].handler(argc, argv);
                return;
            }
        }
        printf("  未知命令: %s\n", cmd_name);
    }

    char *add_args[] = {"add", "10", "20", "30"};
    dispatch("add", 4, add_args);

    char *mul_args[] = {"multiply", "2", "3", "4"};
    dispatch("multiply", 4, mul_args);

    char *echo_args[] = {"echo", "Hello", "World"};
    dispatch("echo", 3, echo_args);

    printf("\n子命令模式: git add, git commit, git push 就是这种模式\n");
    printf("每个子命令有独立的参数和帮助信息\n");

    printf("\n");
}

void demo_argument_validation(void) {
    printf("=== 参数验证 ===\n");

    int is_valid_integer(const char *s) {
        if (s == NULL || *s == '\0') return 0;
        if (*s == '-' || *s == '+') s++;
        if (*s == '\0') return 0;
        while (*s) {
            if (!isdigit((unsigned char)*s)) return 0;
            s++;
        }
        return 1;
    }

    int is_valid_float(const char *s) {
        if (s == NULL || *s == '\0') return 0;
        char *end;
        strtod(s, &end);
        return *end == '\0';
    }

    int is_valid_range(int val, int min, int max) {
        return val >= min && val <= max;
    }

    __attribute__((used)) int is_valid_filepath(const char *s) {
        if (s == NULL || *s == '\0') return 0;
        if (strlen(s) > 255) return 0;
        return 1;
    }

    const char *int_tests[] = {"42", "-10", "3.14", "abc", ""};
    for (int i = 0; i < 5; i++) {
        printf("  is_valid_integer(\"%s\") = %s\n",
               int_tests[i], is_valid_integer(int_tests[i]) ? "true" : "false");
    }

    printf("\n");
    const char *float_tests[] = {"3.14", "-0.5", "1e10", "abc"};
    for (int i = 0; i < 4; i++) {
        printf("  is_valid_float(\"%s\") = %s\n",
               float_tests[i], is_valid_float(float_tests[i]) ? "true" : "false");
    }

    printf("\n");
    printf("  is_valid_range(5, 1, 10) = %s\n", is_valid_range(5, 1, 10) ? "true" : "false");
    printf("  is_valid_range(15, 1, 10) = %s\n", is_valid_range(15, 1, 10) ? "true" : "false");

    printf("\n验证原则:\n");
    printf("1. 验证所有外部输入\n");
    printf("2. 提供清晰的错误消息\n");
    printf("3. 尽早失败 (fail fast)\n");
    printf("4. 区分必需参数和可选参数\n");

    printf("\n");
}

typedef struct {
    const char *short_name;
    const char *long_name;
    const char *description;
    int has_arg;
    const char *arg_name;
    const char *default_value;
} OptionDef;

void demo_help_generation(void) {
    printf("=== 帮助文本生成 ===\n");

    OptionDef options[] = {
        {"-i", "--input",   "输入文件路径",     1, "FILE",   NULL},
        {"-o", "--output",  "输出文件路径",     1, "FILE",   "output.txt"},
        {"-n", "--count",   "处理次数",         1, "NUM",    "1"},
        {"-v", "--verbose", "详细输出模式",     0, NULL,     NULL},
        {"-q", "--quiet",   "安静模式",         0, NULL,     NULL},
        {"-h", "--help",    "显示帮助信息",     0, NULL,     NULL}
    };
    int opt_count = sizeof(options) / sizeof(options[0]);

    void print_usage(const char *prog_name) {
        printf("用法: %s [选项]\n\n", prog_name);
        printf("选项:\n");
        for (int i = 0; i < opt_count; i++) {
            char opt_str[40];
            if (options[i].has_arg) {
                snprintf(opt_str, sizeof(opt_str), "%s, %s <%s>",
                         options[i].short_name, options[i].long_name,
                         options[i].arg_name);
            } else {
                snprintf(opt_str, sizeof(opt_str), "%s, %s",
                         options[i].short_name, options[i].long_name);
            }
            printf("  %-30s %s", opt_str, options[i].description);
            if (options[i].default_value) {
                printf(" (默认: %s)", options[i].default_value);
            }
            printf("\n");
        }
    }

    print_usage("mytool");

    printf("\n自动生成帮助文本的优势:\n");
    printf("1. 选项定义和帮助文本始终同步\n");
    printf("2. 新增选项只需修改定义数组\n");
    printf("3. 减少手动维护帮助文本的工作量\n");

    printf("\n");
}

void demo_cli_design_principles(void) {
    printf("=== CLI 设计原则 ===\n");

    printf("1. 一致性\n");
    printf("   - 短选项用 -, 长选项用 --\n");
    printf("   - 布尔选项不加参数, 值选项加参数\n");
    printf("   - -h/--help 总是显示帮助\n\n");

    printf("2. 可发现性\n");
    printf("   - 无参数运行时显示帮助或示例\n");
    printf("   - 错误参数时给出正确用法提示\n");
    printf("   - 提供 --version 选项\n\n");

    printf("3. 容错性\n");
    printf("   - 提供合理的默认值\n");
    printf("   - 清晰的错误消息\n");
    printf("   - 建议修正方法\n\n");

    printf("4. 可组合性\n");
    printf("   - 支持管道输入输出\n");
    printf("   - 返回有意义的退出码\n");
    printf("   - 避免交互式输入\n\n");

    printf("5. Unix 哲学\n");
    printf("   - 一个程序做好一件事\n");
    printf("   - 文本流是通用接口\n");
    printf("   - 尽早原型, 迭代改进\n");

    printf("\n");
}

void demo_exit_codes(void) {
    printf("=== 退出码约定 ===\n");

    printf("标准退出码:\n");
    printf("  0    成功\n");
    printf("  1    一般错误\n");
    printf("  2    命令行用法错误\n\n");

    printf("自定义退出码示例:\n");
    printf("  10   配置文件错误\n");
    printf("  11   输入文件不存在\n");
    printf("  12   权限不足\n");
    printf("  20   网络错误\n\n");

    printf("在 shell 中检查退出码:\n");
    printf("  ./program; echo $?\n");
    printf("  ./program && echo \"成功\" || echo \"失败\"\n");

    printf("\n");
}

int main(int argc, char *argv[]) {
    demo_subcommand_pattern();
    demo_argument_validation();
    demo_help_generation();
    demo_cli_design_principles();
    demo_exit_codes();

    (void)argc; (void)argv;
    return 0;
}
