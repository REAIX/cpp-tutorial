/** @file 01_deep_dive_varargs_patterns.c
 *  @brief 可变参数进阶模式：类型安全模式、哨兵值、格式字符串安全、日志函数实现
 *  @description 对应文档: 11-可变参数与命令行 | 举一反三：可变参数的安全使用
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum { ARG_INT, ARG_DOUBLE, ARG_STRING, ARG_END } ArgType;

static void unsafe_print(int count, ...) {
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++) {
        int val = va_arg(args, int);
        printf("  [%d] = %d\n", i, val);
    }
    va_end(args);
}

static int sum_with_sentinel(int first, ...) {
    va_list args;
    va_start(args, first);

    int total = first;
    int val;
    while ((val = va_arg(args, int)) != -1) {
        total += val;
    }

    va_end(args);
    return total;
}

static void typed_print(int count, ...) {
    va_list args;
    va_start(args, count);

    for (int i = 0; i < count; i++) {
        ArgType type = va_arg(args, ArgType);
        switch (type) {
            case ARG_INT:
                printf("%d", va_arg(args, int));
                break;
            case ARG_DOUBLE:
                printf("%.2f", va_arg(args, double));
                break;
            case ARG_STRING:
                printf("%s", va_arg(args, const char *));
                break;
            case ARG_END:
                goto done;
        }
        if (i < count - 1) printf(", ");
    }
done:
    va_end(args);
    printf("\n");
}

void demo_type_safety_issues(void) {
    printf("=== 可变参数的类型安全问题 ===\n");

    printf("正确使用: 传入 int\n");
    unsafe_print(3, 10, 20, 30);

    printf("\n错误使用: 传入 double 但按 int 读取\n");
    printf("  unsafe_print(1, 3.14);  // 未定义行为!\n");
    printf("  va_arg(args, int) 读取 double 的二进制表示\n");

    printf("\n类型安全问题的根源:\n");
    printf("1. va_arg 需要调用者指定类型, 编译器无法验证\n");
    printf("2. 传入类型和读取类型不匹配 = 未定义行为\n");
    printf("3. printf 的格式字符串和参数不匹配也是同类问题\n");

    printf("\n");
}

void demo_sentinel_value_pattern(void) {
    printf("=== 哨兵值模式 ===\n");

    printf("sum_with_sentinel(10, 20, 30, -1) = %d\n", sum_with_sentinel(10, 20, 30, -1));
    printf("sum_with_sentinel(5, -1) = %d\n", sum_with_sentinel(5, -1));

    printf("\n哨兵值模式: 用特殊值标记参数列表结束\n");
    printf("常见哨兵值:\n");
    printf("  整数: -1, 0, INT_MIN\n");
    printf("  指针: NULL\n");
    printf("  字符串: NULL\n");

    printf("\nGCC/Clang 支持哨兵属性:\n");
    printf("  void func(int first, ...) __attribute__((sentinel));\n");
    printf("  编译器会检查最后一个参数是否为 NULL\n");

    printf("\n");
}

void demo_format_string_security(void) {
    printf("=== 格式字符串安全 ===\n");

    printf("危险: 用户输入作为格式字符串\n");
    printf("  char input[100];\n");
    printf("  fgets(input, sizeof(input), stdin);\n");
    printf("  printf(input);  // 格式化字符串攻击!\n\n");

    printf("安全: 将用户输入作为普通字符串\n");
    printf("  printf(\"%%s\", input);  // 安全\n\n");

    printf("常见格式字符串漏洞:\n");
    printf("  %%x  读取栈上的数据 (信息泄露)\n");
    printf("  %%n  将已输出字节数写入参数指向的地址 (任意写入)\n");
    printf("  %%s  将栈上的值作为地址, 读取内存 (信息泄露)\n\n");

    printf("防御措施:\n");
    printf("1. 永远不要将用户输入直接作为格式字符串\n");
    printf("2. 使用 fputs/puts 输出不需要格式的字符串\n");
    printf("3. 编译器警告: -Wformat -Wformat-security\n");

    printf("\n");
}

typedef enum {
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
} LogLevel;

typedef struct {
    FILE *output;
    LogLevel min_level;
    int show_timestamp;
    int show_file_line;
} Logger;

static Logger g_logger = {NULL, LOG_INFO, 1, 1};

const char *level_names[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

void logger_init(FILE *output, LogLevel min_level) {
    g_logger.output = output ? output : stderr;
    g_logger.min_level = min_level;
}

void logger_log(LogLevel level, const char *file, int line,
                const char *func, const char *fmt, ...) {
    if (level < g_logger.min_level) return;

    if (g_logger.show_timestamp) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char time_buf[20];
        strftime(time_buf, sizeof(time_buf), "%H:%M:%S", t);
        fprintf(g_logger.output, "[%s] ", time_buf);
    }

    fprintf(g_logger.output, "[%s] ", level_names[level]);

    if (g_logger.show_file_line) {
        fprintf(g_logger.output, "%s:%d %s() - ", file, line, func);
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(g_logger.output, fmt, args);
    va_end(args);

    fprintf(g_logger.output, "\n");
    fflush(g_logger.output);
}

#define LOG(level, fmt, ...) \
    logger_log(level, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define LOG_TRACE_MSG(fmt, ...) LOG(LOG_TRACE, fmt, ##__VA_ARGS__)
#define LOG_DEBUG_MSG(fmt, ...) LOG(LOG_DEBUG, fmt, ##__VA_ARGS__)
#define LOG_INFO_MSG(fmt, ...)  LOG(LOG_INFO, fmt, ##__VA_ARGS__)
#define LOG_WARN_MSG(fmt, ...)  LOG(LOG_WARN, fmt, ##__VA_ARGS__)
#define LOG_ERROR_MSG(fmt, ...) LOG(LOG_ERROR, fmt, ##__VA_ARGS__)
#define LOG_FATAL_MSG(fmt, ...) LOG(LOG_FATAL, fmt, ##__VA_ARGS__)

void demo_logging_function(void) {
    printf("=== 实现日志函数 ===\n");

    logger_init(stdout, LOG_DEBUG);

    LOG_TRACE_MSG("这条不会显示 (级别低于 DEBUG)");
    LOG_DEBUG_MSG("调试信息: count = %d", 42);
    LOG_INFO_MSG("程序启动, 版本 %d.%d", 1, 0);
    LOG_WARN_MSG("内存使用率 %d%%", 85);
    LOG_ERROR_MSG("文件打开失败: %s", "data.txt");
    LOG_FATAL_MSG("致命错误, 即将退出");

    printf("\n日志函数设计要点:\n");
    printf("1. 使用宏自动填充文件名、行号、函数名\n");
    printf("2. 支持日志级别过滤\n");
    printf("3. 使用 vsnprintf 防止缓冲区溢出\n");
    printf("4. 线程安全版本需要加锁\n");
    printf("5. 支持输出到文件、网络等\n");

    printf("\n");
}

void demo_type_safe_varargs_pattern(void) {
    printf("=== 类型安全的可变参数模式 ===\n");

    typedef struct {
        ArgType type;
        union {
            int int_val;
            double double_val;
            const char *string_val;
        } value;
    } Arg;

    (void)sizeof(Arg);

    printf("类型安全调用:\n");
    typed_print(4, ARG_INT, 42, ARG_DOUBLE, 3.14, ARG_STRING, "hello", ARG_INT, 99);

    printf("\n每个参数前带类型标签, 运行时根据标签选择正确的 va_arg 类型\n");
    printf("虽然冗长, 但避免了类型不匹配的未定义行为\n");

    printf("\n");
}

void demo_varargs_best_practices(void) {
    printf("=== 可变参数最佳实践 ===\n");
    printf("1. 优先使用类型安全的替代方案 (结构体数组、回调等)\n");
    printf("2. 必须使用时, 确保有可靠的终止条件\n");
    printf("3. 使用 vprintf/vsnprintf 系列函数处理格式化\n");
    printf("4. 不要在可变参数中传递 char/short (会提升为 int)\n");
    printf("5. 不要在可变参数中传递 float (会提升为 double)\n");
    printf("6. 文档中明确说明参数的类型和顺序要求\n");
    printf("7. 使用 __attribute__((format(printf, ...))) 让编译器检查\n");
    printf("8. 使用哨兵值或计数器, 避免越界访问\n");
    printf("\n");
}

int main(void) {
    demo_type_safety_issues();
    demo_sentinel_value_pattern();
    demo_format_string_security();
    demo_logging_function();
    demo_type_safe_varargs_pattern();
    demo_varargs_best_practices();

    return 0;
}
