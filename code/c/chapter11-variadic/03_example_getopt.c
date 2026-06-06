/** @file 03_example_getopt.c
 *  @brief 命令行选项解析：getopt、getopt_long、选项解析
 *  @description 对应文档: 11-可变参数与命令行
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32

typedef struct {
    const char *input_file;
    const char *output_file;
    int verbose;
    int count;
    int show_help;
} Options;

static Options parse_args(int argc, char *argv[]) {
    Options opts = {NULL, NULL, 0, 1, 0};

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0) {
            if (i + 1 < argc) opts.input_file = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (i + 1 < argc) opts.output_file = argv[++i];
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            opts.verbose = 1;
        } else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--count") == 0) {
            if (i + 1 < argc) opts.count = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            opts.show_help = 1;
        }
    }
    return opts;
}

void demo_getopt_basic(void) {
    printf("=== getopt 基础 (POSIX, Windows 下需自行实现) ===\n");
    printf("getopt 是 POSIX 标准函数, 用于解析短选项 (如 -a, -b value)\n\n");

    printf("函数签名:\n");
    printf("  int getopt(int argc, char *const argv[], const char *optstring);\n\n");

    printf("参数说明:\n");
    printf("  argc, argv: main 的参数\n");
    printf("  optstring:  合法的选项字符, 后跟 : 表示需要参数\n");
    printf("    \"ab:c\" 表示 -a, -b <value>, -c\n\n");

    printf("全局变量:\n");
    printf("  optarg: 当前选项的参数值\n");
    printf("  optind: 下一个要处理的 argv 索引\n");
    printf("  opterr: 设为 0 可抑制错误消息\n");
    printf("  optopt: 最后一个未知选项字符\n\n");

    printf("返回值:\n");
    printf("  成功: 选项字符\n");
    printf("  结束: -1\n");
    printf("  错误: '?' (未知选项或缺参数)\n");

    printf("\n");
}

void demo_getopt_long(void) {
    printf("=== getopt_long (GNU 扩展) ===\n");
    printf("getopt_long 支持长选项 (如 --help, --output=file)\n\n");

    printf("函数签名:\n");
    printf("  int getopt_long(int argc, char *const argv[],\n");
    printf("                  const char *optstring,\n");
    printf("                  const struct option *longopts,\n");
    printf("                  int *longindex);\n\n");

    printf("struct option {\n");
    printf("    const char *name;    // 长选项名\n");
    printf("    int has_arg;         // no_argument(0), required_argument(1), optional_argument(2)\n");
    printf("    int *flag;           // 如何返回结果\n");
    printf("    int val;             // 返回的值\n");
    printf("};\n\n");

    printf("Windows 下 getopt_long 不可用, 可使用第三方库或自行实现\n");

    printf("\n");
}

void demo_manual_option_parser(void) {
    printf("=== 跨平台手动选项解析器 ===\n");

    char *test_argv[] = {"program", "-i", "input.txt", "--output", "output.txt", "-v", "-n", "5"};
    Options opts = parse_args(8, test_argv);

    printf("解析结果:\n");
    printf("  input_file  = %s\n", opts.input_file ? opts.input_file : "(无)");
    printf("  output_file = %s\n", opts.output_file ? opts.output_file : "(无)");
    printf("  verbose     = %d\n", opts.verbose);
    printf("  count       = %d\n", opts.count);

    printf("\n");
}

#else

#include <unistd.h>
#include <getopt.h>

static int simulate_getopt(void) {
    char *argv[] = {"program", "-a", "-b", "hello", "-c", "world", NULL};
    int argc = 6;

    optind = 1;
    opterr = 0;

    printf("模拟命令行: program -a -b hello -c world\n\n");

    int opt;
    while ((opt = getopt(argc, argv, "ab:c:")) != -1) {
        switch (opt) {
            case 'a':
                printf("  选项 -a (无参数)\n");
                break;
            case 'b':
                printf("  选项 -b 参数: %s\n", optarg);
                break;
            case 'c':
                printf("  选项 -c 参数: %s\n", optarg);
                break;
            case '?':
                printf("  未知选项: -%c\n", optopt);
                break;
        }
    }
    return 0;
}

static void simulate_getopt_long(void) {
    char *argv[] = {"program", "--input=data.txt", "--verbose", "--count=3", NULL};
    int argc = 4;

    static struct option long_options[] = {
        {"input",   required_argument, 0, 'i'},
        {"output",  required_argument, 0, 'o'},
        {"verbose", no_argument,       0, 'v'},
        {"count",   required_argument, 0, 'n'},
        {"help",    no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    optind = 1;

    printf("模拟命令行: program --input=data.txt --verbose --count=3\n\n");

    int opt;
    int long_index = 0;
    while ((opt = getopt_long(argc, argv, "i:o:vn:h", long_options, &long_index)) != -1) {
        switch (opt) {
            case 'i':
                printf("  --input = %s\n", optarg);
                break;
            case 'o':
                printf("  --output = %s\n", optarg);
                break;
            case 'v':
                printf("  --verbose\n");
                break;
            case 'n':
                printf("  --count = %s\n", optarg);
                break;
            case 'h':
                printf("  --help\n");
                break;
        }
    }
}

void demo_getopt_basic(void) {
    printf("=== getopt 基础 ===\n");

    simulate_getopt();

    printf("\noptstring \"ab:c:\" 含义:\n");
    printf("  a  - 无参数选项\n");
    printf("  b: - 需要参数的选项\n");
    printf("  c: - 需要参数的选项\n");

    printf("\n");
}

void demo_getopt_long(void) {
    printf("=== getopt_long ===\n");

    simulate_getopt_long();

    printf("\n");
}

void demo_manual_option_parser(void) {
    printf("=== 手动选项解析器 (备用方案) ===\n");
    printf("当 getopt 不可用时, 手动解析也是一种选择\n");
    printf("参见 Windows 版本的实现\n\n");
}

#endif

void demo_option_parsing_patterns(void) {
    printf("=== 选项解析模式 ===\n");

    printf("常见选项格式:\n");
    printf("  短选项: -a, -b value, -bvalue, -abc (合并)\n");
    printf("  长选项: --help, --output=file, --output file\n\n");

    printf("特殊标记:\n");
    printf("  --  表示选项结束, 后面都是位置参数\n");
    printf("  rm -- -f  删除名为 -f 的文件\n\n");

    printf("设计原则:\n");
    printf("1. 短选项用于常用选项, 长选项用于不常用的\n");
    printf("2. 必需参数用 required_argument\n");
    printf("3. 提供合理的默认值\n");
    printf("4. 总是提供 --help\n");

    printf("\n");
}

int main(int argc, char *argv[]) {
    demo_getopt_basic();
    demo_getopt_long();
    demo_manual_option_parser();
    demo_option_parsing_patterns();

    (void)argc; (void)argv;
    return 0;
}
