/**
 * @file 02_example_makefile_advanced.c
 * @brief Makefile高级特性示例 - 配合高级Makefile演示
 * @description 对应文档: 22-makefile
 *              演示模式规则、自动变量、伪目标、变量等高级特性
 *              本文件包含多个模块，模拟多文件项目
 *
 * 使用方法:
 *   make              # 编译程序
 *   make debug        # 编译调试版本
 *   make release      # 编译发布版本
 *   make clean        # 清理
 *   make vars         # 显示Makefile变量
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* ========================================================================
 * 模拟多个模块的功能
 * ======================================================================== */

typedef struct {
    double x;
    double y;
} point_t;

double point_distance(point_t a, point_t b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);
}

void point_translate(point_t *p, double dx, double dy) {
    p->x += dx;
    p->y += dy;
}

typedef struct {
    point_t center;
    double radius;
} circle_t;

double circle_area(circle_t *c) {
    return 3.14159265358979323846 * c->radius * c->radius;
}

int circle_contains(circle_t *c, point_t *p) {
    return point_distance(c->center, *p) <= c->radius;
}

/* ========================================================================
 * 演示函数
 * ======================================================================== */

void demo_geometry(void) {
    printf("===== 几何计算演示 =====\n\n");

    point_t a = {0.0, 0.0};
    point_t b = {3.0, 4.0};
    printf("点 A = (%.1f, %.1f), 点 B = (%.1f, %.1f)\n", a.x, a.y, b.x, b.y);
    printf("距离 AB = %.2f\n\n", point_distance(a, b));

    circle_t c = {{0.0, 0.0}, 5.0};
    printf("圆心=(%.1f, %.1f), 半径=%.1f\n", c.center.x, c.center.y, c.radius);
    printf("面积 = %.2f\n", circle_area(&c));
    printf("点A在圆内: %s\n", circle_contains(&c, &a) ? "是" : "否");
    printf("点B在圆内: %s\n\n", circle_contains(&c, &b) ? "是" : "否");
}

void demo_makefile_advanced_concepts(void) {
    printf("===== Makefile 高级概念 =====\n\n");

    printf("1. 自动变量 (Automatic Variables):\n");
    printf("   $@  - 目标文件名\n");
    printf("   $<  - 第一个依赖文件名\n");
    printf("   $^  - 所有依赖文件名(去重)\n");
    printf("   $+  - 所有依赖文件名(不去重)\n");
    printf("   $?  - 比目标新的依赖文件\n");
    printf("   $*  - 匹配模式规则中 %% 的部分\n\n");

    printf("   示例:\n");
    printf("   %%.o: %%.c\n");
    printf("       $(CC) $(CFLAGS) -c -o $@ $<\n");
    printf("       # $@ = 目标.o文件, $< = 源.c文件\n\n");

    printf("2. 模式规则 (Pattern Rules):\n");
    printf("   %%.o: %%.c\n");
    printf("       $(CC) $(CFLAGS) -c -o $@ $<\n\n");
    printf("   %% 是通配符，匹配任意非空子串\n");
    printf("   这条规则将所有 .c 文件编译为 .o 文件\n\n");

    printf("3. 变量与赋值:\n");
    printf("   =   递归展开 (使用时才求值)\n");
    printf("   :=  简单展开 (定义时求值，推荐)\n");
    printf("   ?=  条件赋值 (未定义时才赋值)\n");
    printf("   +=  追加赋值\n\n");

    printf("   示例:\n");
    printf("   CC := gcc\n");
    printf("   CFLAGS := -Wall\n");
    printf("   CFLAGS += -O2          # CFLAGS = -Wall -O2\n");
    printf("   DEBUG ?= 0             # 如果未定义 DEBUG，则设为 0\n\n");

    printf("4. 条件判断:\n");
    printf("   ifeq ($(DEBUG),1)\n");
    printf("       CFLAGS += -g -DDEBUG\n");
    printf("   else\n");
    printf("       CFLAGS += -O2 -DNDEBUG\n");
    printf("   endif\n\n");

    printf("5. 函数:\n");
    printf("   $(wildcard *.c)           # 匹配所有 .c 文件\n");
    printf("   $(patsubst %%.c,%%.o,$(SRC))  # .c 替换为 .o\n");
    printf("   $(shell date)             # 执行shell命令\n");
    printf("   $(strip text)             # 去除首尾空格\n");
    printf("   $(addprefix src/,a.c b.c) # 添加前缀\n\n");

    printf("6. 常用内置变量:\n");
    printf("   $(MAKE)     - make 命令路径\n");
    printf("   $(CC)       - C编译器 (默认 cc)\n");
    printf("   $(CXX)      - C++编译器 (默认 g++)\n");
    printf("   $(AR)       - 归档工具 (默认 ar)\n");
    printf("   $(RM)       - 删除命令 (默认 rm -f)\n\n");
}

int main(void) {
    printf("========== Makefile 高级特性示例 ==========\n\n");

    demo_geometry();
    demo_makefile_advanced_concepts();

    printf("========== 程序结束 ==========\n");
    return 0;
}
