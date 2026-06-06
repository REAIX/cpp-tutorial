/** @file 02_example_command_line.c
 *  @brief 命令行参数：argc、argv、命令行解析、环境变量(getenv)
 *  @description 对应文档: 11-可变参数与命令行
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void simulate_parsing(int argc, char *argv[]) {
    const char *input_file = NULL;
    const char *output_file = NULL;
    int verbose = 0;
    int count = 1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0) {
            if (i + 1 < argc) {
                input_file = argv[++i];
            } else {
                printf("  错误: %s 需要参数\n", argv[i]);
            }
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (i + 1 < argc) {
                output_file = argv[++i];
            } else {
                printf("  错误: %s 需要参数\n", argv[i]);
            }
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--count") == 0) {
            if (i + 1 < argc) {
                count = atoi(argv[++i]);
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("  用法: program [选项]\n");
            printf("  -i, --input   输入文件\n");
            printf("  -o, --output  输出文件\n");
            printf("  -v, --verbose 详细输出\n");
            printf("  -n, --count   计数\n");
            printf("  -h, --help    显示帮助\n");
            return;
        } else {
            printf("  未知参数: %s\n", argv[i]);
        }
    }

    printf("  解析结果:\n");
    printf("    input_file  = %s\n", input_file ? input_file : "(未指定)");
    printf("    output_file = %s\n", output_file ? output_file : "(未指定)");
    printf("    verbose     = %s\n", verbose ? "是" : "否");
    printf("    count       = %d\n", count);
}

static const char *get_config(const char *env_name, const char *default_val) {
    const char *val = getenv(env_name);
    return val ? val : default_val;
}

void demo_argc_argv(void) {
    printf("=== argc 和 argv 基础 ===\n");

    printf("main 函数的两种标准签名:\n");
    printf("  int main(void)\n");
    printf("  int main(int argc, char *argv[])\n\n");

    printf("argc: 参数个数 (包括程序名本身)\n");
    printf("argv: 参数字符串数组, argv[0] 是程序名\n");
    printf("argv[argc] 是 NULL (哨兵值)\n\n");

    printf("示例: ./program hello world 42\n");
    printf("  argc = 4\n");
    printf("  argv[0] = \"./program\"\n");
    printf("  argv[1] = \"hello\"\n");
    printf("  argv[2] = \"world\"\n");
    printf("  argv[3] = \"42\"\n");
    printf("  argv[4] = NULL\n");

    printf("\n");
}

void demo_parse_arguments(void) {
    printf("=== 手动解析命令行参数 ===\n");

    char *sim_argv[] = {"program", "-i", "data.txt", "-o", "result.txt", "-v", "-n", "10"};
    int sim_argc = 8;
    simulate_parsing(sim_argc, sim_argv);

    printf("\n");
}

void demo_environment_variables(void) {
    printf("=== 环境变量 (getenv) ===\n");

    const char *path = getenv("PATH");
    if (path) {
        printf("PATH = %.80s...\n", path);
    } else {
        printf("PATH 未设置\n");
    }

    const char *home = getenv("HOME");
    if (home) {
        printf("HOME = %s\n", home);
    } else {
        const char *userprofile = getenv("USERPROFILE");
        if (userprofile) {
            printf("USERPROFILE = %s\n", userprofile);
        } else {
            printf("HOME/USERPROFILE 未设置\n");
        }
    }

    const char *lang = getenv("LANG");
    printf("LANG = %s\n", lang ? lang : "(未设置)");

    printf("\ngetenv 返回指向环境变量的指针, 不要修改!\n");
    printf("修改环境变量使用 setenv/putenv (POSIX)\n");

    printf("\n");
}

void demo_env_for_config(void) {
    printf("=== 环境变量用于配置 ===\n");

    printf("数据库配置:\n");
    printf("  DB_HOST = %s\n", get_config("DB_HOST", "localhost"));
    printf("  DB_PORT = %s\n", get_config("DB_PORT", "5432"));
    printf("  DB_NAME = %s\n", get_config("DB_NAME", "mydb"));

    printf("\n环境变量配置的优势:\n");
    printf("1. 不需要修改代码或配置文件\n");
    printf("2. 适合容器化部署 (Docker, K8s)\n");
    printf("3. 敏感信息不硬编码在代码中\n");
    printf("4. 12-Factor App 推荐方式\n");

    printf("\n");
}

int main(int argc, char *argv[]) {
    printf("实际 argc = %d\n", argc);
    for (int i = 0; i < argc && i < 5; i++) {
        printf("  argv[%d] = \"%s\"\n", i, argv[i]);
    }
    printf("\n");

    demo_argc_argv();
    demo_parse_arguments();
    demo_environment_variables();
    demo_env_for_config();

    return 0;
}
