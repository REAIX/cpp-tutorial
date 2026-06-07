/**
 * @file 01_example_cmake_basics.c
 * @brief CMake基础示例 - 配合CMakeLists.txt演示基本用法
 * @description 对应文档: 23-cmake
 *              本文件是一个简单的C程序，配合同级目录下的 CMakeLists.txt 使用，
 *              演示 CMake 的基本概念和用法
 *
 * 使用方法:
 *   mkdir build && cd build
 *   cmake ..
 *   make
 *   ./cmake_basics
 *
 * CMake 基本概念:
 *   CMake 是一个跨平台的构建系统生成器，它不直接编译代码，
 *   而是生成 Makefile、Ninja 文件、Visual Studio 项目等。
 *
 *   核心文件: CMakeLists.txt
 *   构建流程: 配置(configure) → 生成(generate) → 构建(build)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUFFER 256

typedef struct {
    char name[MAX_BUFFER];
    int age;
    double score;
} student_t;

void student_init(student_t *s, const char *name, int age, double score) {
    strncpy(s->name, name, MAX_BUFFER - 1);
    s->name[MAX_BUFFER - 1] = '\0';
    s->age = age;
    s->score = score;
}

void student_print(const student_t *s) {
    printf("  姓名: %-15s  年龄: %3d  成绩: %6.2f\n", s->name, s->age, s->score);
}

const char *student_grade(const student_t *s) {
    if (s->score >= 90) return "优秀(A)";
    if (s->score >= 80) return "良好(B)";
    if (s->score >= 70) return "中等(C)";
    if (s->score >= 60) return "及格(D)";
    return "不及格(F)";
}

void demo_student_system(void) {
    printf("===== 学生信息管理演示 =====\n\n");

    student_t students[5];
    student_init(&students[0], "张三", 20, 92.5);
    student_init(&students[1], "李四", 21, 85.0);
    student_init(&students[2], "王五", 19, 78.3);
    student_init(&students[3], "赵六", 22, 65.7);
    student_init(&students[4], "钱七", 20, 45.2);

    printf("  %-15s  %-6s  %-8s  %s\n", "姓名", "年龄", "成绩", "等级");
    printf("  %-15s  %-6s  %-8s  %s\n", "---------------", "------", "--------", "--------");
    for (int i = 0; i < 5; i++) {
        printf("  %-15s  %3d     %6.2f   %s\n",
               students[i].name, students[i].age,
               students[i].score, student_grade(&students[i]));
    }
    printf("\n");
}

void demo_cmake_basics(void) {
    printf("===== CMake 基本概念 =====\n\n");

    printf("CMake 三步构建流程:\n");
    printf("  1. 配置 (Configure):\n");
    printf("     mkdir build && cd build\n");
    printf("     cmake ..\n");
    printf("     → 检测编译器、平台、依赖\n");
    printf("     → 处理 CMakeLists.txt 中的命令\n");
    printf("     → 生成 CMakeCache.txt\n\n");

    printf("  2. 生成 (Generate):\n");
    printf("     → 生成构建文件 (Makefile / .sln / build.ninja)\n");
    printf("     → 通常与配置步骤合并\n\n");

    printf("  3. 构建 (Build):\n");
    printf("     cmake --build .    # 跨平台构建命令\n");
    printf("     或 make            # 直接使用生成的Makefile\n\n");

    printf("最基本的 CMakeLists.txt:\n");
    printf("  cmake_minimum_required(VERSION 3.10)\n");
    printf("  project(MyProject C)\n");
    printf("  add_executable(main main.c)\n\n");

    printf("CMake 关键命令:\n");
    printf("  cmake_minimum_required  指定最低CMake版本\n");
    printf("  project                 定义项目名称和语言\n");
    printf("  add_executable          定义可执行目标\n");
    printf("  add_library             定义库目标\n");
    printf("  target_link_libraries   链接库到目标\n");
    printf("  set                     设置变量\n");
    printf("  message                 打印消息\n\n");

    printf("CMake vs 直接写 Makefile:\n");
    printf("  CMake 优点:\n");
    printf("    - 跨平台 (Linux/macOS/Windows)\n");
    printf("    - 自动依赖管理\n");
    printf("    - 开箱即用的构建目录支持\n");
    printf("    - IDE 集成 (VS Code, CLion, Visual Studio)\n");
    printf("  Makefile 优点:\n");
    printf("    - 更简单直接 (小项目)\n");
    printf("    - 无需额外工具\n");
    printf("    - 完全控制构建过程\n\n");
}

int main(void) {
    printf("========== CMake 基础示例 ==========\n\n");

    demo_student_system();
    demo_cmake_basics();

    printf("========== 程序结束 ==========\n");
    return 0;
}
