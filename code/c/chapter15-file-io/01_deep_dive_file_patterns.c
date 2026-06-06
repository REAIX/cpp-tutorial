/**
 * @file 01_deep_dive_file_patterns.c
 * @brief 文件操作模式深入
 * @description 对应文档: 15-文件操作
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static const char *SRC_FILE = "deep_dive_src.txt";
static const char *DST_FILE = "deep_dive_dst.txt";

static void create_test_file(const char *path, const char *content) {
    FILE *f = fopen(path, "w");
    if (f) { fputs(content, f); fclose(f); }
}

void demo_file_copy(void) {
    printf("=== 文件复制 ===\n");
    create_test_file(SRC_FILE, "第一行: 文件复制测试\n第二行: 数据内容\n第三行: 结束\n");

    FILE *src = fopen(SRC_FILE, "rb");
    FILE *dst = fopen(DST_FILE, "wb");
    if (!src || !dst) {
        perror("  打开文件失败");
        if (src) fclose(src);
        if (dst) fclose(dst);
        return;
    }

    size_t total = 0;
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), src)) > 0) {
        if (fwrite(buf, 1, n, dst) != n) {
            perror("  写入失败");
            break;
        }
        total += n;
    }

    fclose(src);
    fclose(dst);
    printf("  复制完成: %zu 字节\n\n", total);
}

void demo_line_by_line(void) {
    printf("=== 逐行处理 ===\n");
    FILE *f = fopen(SRC_FILE, "r");
    if (!f) return;

    char line[256];
    int line_num = 0;
    while (fgets(line, sizeof(line), f) != NULL) {
        line_num++;
        line[strcspn(line, "\n")] = '\0';
        size_t len = strlen(line);
        printf("  行%2d (len=%2zu): \"%s\"\n", line_num, len, line);
    }
    fclose(f);

    printf("  注意: 行长度超过缓冲区时, fgets会分多次读取\n");
    printf("  检测方法: 如果读取的行末尾没有\\n, 可能被截断\n\n");
}

void demo_csv_parsing(void) {
    printf("=== CSV文件解析 ===\n");
    const char *CSV_FILE = "deep_dive_data.csv";
    create_test_file(CSV_FILE,
        "姓名,年龄,分数\n"
        "Alice,25,89.5\n"
        "Bob,30,95.0\n"
        "Charlie,28,72.3\n");

    FILE *f = fopen(CSV_FILE, "r");
    if (!f) return;

    char line[256];
    int row = 0;
    while (fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\n")] = '\0';
        row++;

        if (row == 1) {
            printf("  表头: %s\n", line);
            continue;
        }

        char *field1 = strtok(line, ",");
        char *field2 = strtok(NULL, ",");
        char *field3 = strtok(NULL, ",");

        if (field1 && field2 && field3) {
            printf("  记录: name=\"%s\", age=%d, score=%.1f\n",
                   field1, atoi(field2), atof(field3));
        }
    }
    fclose(f);

    printf("  CSV解析陷阱:\n");
    printf("    - 字段含逗号时需要引号包围\n");
    printf("    - 字段含引号时需要转义\n");
    printf("    - strtok修改原字符串, 非线程安全\n");
    printf("    - 生产环境建议用专用CSV库\n\n");
    remove(CSV_FILE);
}

void demo_ini_parsing(void) {
    printf("=== INI文件解析(简化版) ===\n");
    const char *INI_FILE = "deep_dive_config.ini";
    create_test_file(INI_FILE,
        "# 配置文件示例\n"
        "[database]\n"
        "host=localhost\n"
        "port=3306\n"
        "name=mydb\n"
        "\n"
        "[server]\n"
        "port=8080\n"
        "debug=true\n");

    FILE *f = fopen(INI_FILE, "r");
    if (!f) return;

    char current_section[64] = "";
    char line[256];
    while (fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\n")] = '\0';

        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0' || *p == '#' || *p == ';') continue;

        if (*p == '[') {
            char *end = strchr(p, ']');
            if (end) {
                *end = '\0';
                snprintf(current_section, sizeof(current_section), "%s", p + 1);
                printf("  [节] %s\n", current_section);
            }
        } else {
            char *eq = strchr(p, '=');
            if (eq) {
                *eq = '\0';
                char *key = p;
                char *val = eq + 1;
                while (*val == ' ') val++;
                printf("    %s.%s = %s\n", current_section, key, val);
            }
        }
    }
    fclose(f);
    printf("\n");
    remove(INI_FILE);
}

void demo_log_rotation(void) {
    printf("=== 日志文件轮转(概念演示) ===\n");
    const char *LOG_FILE = "deep_dive_app.log";
    const long MAX_LOG_SIZE = 100;

    create_test_file(LOG_FILE,
        "2026-05-29 10:00:00 [INFO] 程序启动\n"
        "2026-05-29 10:00:01 [INFO] 加载配置\n"
        "2026-05-29 10:00:02 [WARN] 配置项缺失, 使用默认值\n"
        "2026-05-29 10:00:03 [INFO] 服务就绪\n");

    FILE *f = fopen(LOG_FILE, "rb");
    if (f) {
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fclose(f);

        printf("  当前日志大小: %ld 字节\n", size);
        if (size > MAX_LOG_SIZE) {
            char old_name[256];
            snprintf(old_name, sizeof(old_name), "%s.1", LOG_FILE);
            remove(old_name);
            rename(LOG_FILE, old_name);
            printf("  日志已轮转: %s -> %s\n", LOG_FILE, old_name);
        } else {
            printf("  日志未超过阈值(%ld), 无需轮转\n", MAX_LOG_SIZE);
        }
    }

    printf("  日志轮转策略:\n");
    printf("    - 按大小轮转: 超过阈值时重命名并创建新文件\n");
    printf("    - 按时间轮转: 每天/每小时创建新文件\n");
    printf("    - 保留数量: 只保留最近N个日志文件\n\n");
    remove(LOG_FILE);
    remove("deep_dive_app.log.1");
}

int main(void) {
    printf("========== 文件操作模式深入 ==========\n\n");

    demo_file_copy();
    demo_line_by_line();
    demo_csv_parsing();
    demo_ini_parsing();
    demo_log_rotation();

    remove(SRC_FILE);
    remove(DST_FILE);
    printf("========== 所有演示完成 ==========\n");
    return 0;
}
