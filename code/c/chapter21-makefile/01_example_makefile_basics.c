/**
 * @file 01_example_makefile_basics.c
 * @brief Makefile基础示例 - 简单C程序配合Makefile演示
 * @description 对应文档: 22-makefile
 *              本文件是一个简单的C程序，配合同级目录下的 Makefile 使用，
 *              演示 Makefile 的基本语法和概念
 *
 * 使用方法:
 *   make          # 编译程序
 *   make run      # 编译并运行
 *   make clean    # 清理生成文件
 *
 * Makefile 基本概念:
 *   Makefile 由一系列规则(rule)组成，每条规则的格式:
 *
 *   目标(target): 依赖(prerequisites)
 *   	命令(recipe)        # 注意: 必须用 Tab 缩进，不能用空格!
 *
 *   目标:     要生成的文件名或伪目标名
 *   依赖:     生成目标所需的文件
 *   命令:     生成目标时执行的shell命令
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ITEMS 100

typedef struct {
    int id;
    char name[32];
    double value;
} item_t;

static item_t items[MAX_ITEMS];
static int item_count = 0;

int item_add(int id, const char *name, double value) {
    if (item_count >= MAX_ITEMS) {
        return -1;
    }
    items[item_count].id = id;
    strncpy(items[item_count].name, name, sizeof(items[item_count].name) - 1);
    items[item_count].name[sizeof(items[item_count].name) - 1] = '\0';
    items[item_count].value = value;
    item_count++;
    return 0;
}

void item_print_all(void) {
    printf("  %-5s %-20s %10s\n", "ID", "Name", "Value");
    printf("  %-5s %-20s %10s\n", "-----", "--------------------", "----------");
    for (int i = 0; i < item_count; i++) {
        printf("  %-5d %-20s %10.2f\n", items[i].id, items[i].name, items[i].value);
    }
}

double item_total_value(void) {
    double total = 0.0;
    for (int i = 0; i < item_count; i++) {
        total += items[i].value;
    }
    return total;
}

void demo_basic_program(void) {
    printf("===== Makefile 基础示例程序 =====\n\n");

    item_add(1, "Widget Alpha", 29.99);
    item_add(2, "Gadget Beta", 49.50);
    item_add(3, "Tool Gamma", 15.75);
    item_add(4, "Device Delta", 89.99);

    item_print_all();
    printf("\n  总价值: %.2f\n\n", item_total_value());
}

void demo_makefile_concepts(void) {
    printf("===== Makefile 基本概念 =====\n\n");

    printf("1. 规则 (Rule):\n");
    printf("   target: prerequisites\n");
    printf("   \tcommand          # Tab缩进!\n\n");

    printf("2. 伪目标 (Phony Target):\n");
    printf("   不对应实际文件的目标，如 clean, install\n");
    printf("   .PHONY: clean install\n\n");

    printf("3. 默认目标:\n");
    printf("   Makefile 中的第一个目标是默认目标\n");
    printf("   直接运行 make 时执行\n\n");

    printf("4. 依赖关系:\n");
    printf("   make 会自动判断依赖是否需要重新编译\n");
    printf("   只重新编译修改过的文件及其依赖者\n\n");

    printf("5. 变量:\n");
    printf("   CC = gcc              # 编译器\n");
    printf("   CFLAGS = -Wall -g     # 编译选项\n");
    printf("   $(CC) $(CFLAGS) ...   # 使用变量\n\n");

    printf("6. 常见目标命名约定:\n");
    printf("   all      - 编译所有内容\n");
    printf("   clean    - 清理生成文件\n");
    printf("   install  - 安装程序\n");
    printf("   uninstall- 卸载程序\n");
    printf("   test     - 运行测试\n");
    printf("\n");
}

int main(void) {
    printf("========== Makefile 基础示例 ==========\n\n");

    demo_basic_program();
    demo_makefile_concepts();

    printf("========== 程序结束 ==========\n");
    return 0;
}
