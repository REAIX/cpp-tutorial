/**
 * @file 02_example_cmake_advanced.c
 * @brief CMake高级特性示例 - 变量、目标、find_package
 * @description 对应文档: 23-cmake
 *              演示 CMake 变量系统、目标属性、find_package 等高级特性
 *
 * 使用方法:
 *   mkdir build && cd build
 *   cmake .. -DCMAKE_BUILD_TYPE=Debug
 *   cmake --build .
 *   ./cmake_advanced
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
    double *data;
    int size;
    int capacity;
} dynamic_array_t;

void array_init(dynamic_array_t *arr, int initial_capacity) {
    arr->capacity = initial_capacity > 0 ? initial_capacity : 8;
    arr->data = (double *)malloc(arr->capacity * sizeof(double));
    arr->size = 0;
}

void array_push(dynamic_array_t *arr, double value) {
    if (arr->size >= arr->capacity) {
        int new_cap = arr->capacity * 2;
        double *new_data = (double *)realloc(arr->data, new_cap * sizeof(double));
        if (!new_data) return;  // realloc 失败时保留原数据, 拒绝 push
        arr->data = new_data;
        arr->capacity = new_cap;
    }
    arr->data[arr->size++] = value;
}

void array_free(dynamic_array_t *arr) {
    free(arr->data);
    arr->data = NULL;
    arr->size = 0;
    arr->capacity = 0;
}

double array_mean(const dynamic_array_t *arr) {
    if (arr->size == 0) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < arr->size; i++) {
        sum += arr->data[i];
    }
    return sum / arr->size;
}

double array_stddev(const dynamic_array_t *arr) {
    if (arr->size < 2) return 0.0;
    double mean = array_mean(arr);
    double sum_sq = 0.0;
    for (int i = 0; i < arr->size; i++) {
        double diff = arr->data[i] - mean;
        sum_sq += diff * diff;
    }
    return sqrt(sum_sq / (arr->size - 1));
}

void demo_statistics(void) {
    printf("===== 统计计算演示 =====\n\n");

    dynamic_array_t arr;
    array_init(&arr, 8);

    double values[] = {85.5, 92.3, 78.1, 95.7, 88.9, 76.4, 91.2, 83.6};
    for (int i = 0; i < 8; i++) {
        array_push(&arr, values[i]);
    }

    printf("  数据: ");
    for (int i = 0; i < arr.size; i++) {
        printf("%.1f", arr.data[i]);
        if (i < arr.size - 1) printf(", ");
    }
    printf("\n");
    printf("  数量: %d\n", arr.size);
    printf("  均值: %.2f\n", array_mean(&arr));
    printf("  标准差: %.2f\n\n", array_stddev(&arr));

    array_free(&arr);
}

void demo_cmake_variables(void) {
    printf("===== CMake 变量系统 =====\n\n");

    printf("1. 变量类型:\n");
    printf("   普通变量:  set(MY_VAR \"hello\")\n");
    printf("   缓存变量:  set(MY_CACHE_VAR \"default\" CACHE STRING \"description\")\n");
    printf("   环境变量:  $ENV{PATH}\n\n");

    printf("2. 变量作用域:\n");
    printf("   CMake 变量有作用域 (类似函数栈帧)\n");
    printf("   add_subdirectory() 创建子作用域\n");
    printf("   function() 创建子作用域\n");
    printf("   PARENT_SCOPE 关键字修改父作用域变量\n\n");

    printf("3. 常用内置变量:\n");
    printf("   CMAKE_SOURCE_DIR       源码根目录\n");
    printf("   CMAKE_BINARY_DIR       构建根目录\n");
    printf("   CMAKE_CURRENT_SOURCE_DIR 当前 CMakeLists.txt 目录\n");
    printf("   CMAKE_C_COMPILER       C编译器路径\n");
    printf("   CMAKE_C_FLAGS          C编译器标志\n");
    printf("   CMAKE_BUILD_TYPE       构建类型 (Debug/Release)\n");
    printf("   CMAKE_INSTALL_PREFIX   安装路径前缀\n");
    printf("   PROJECT_NAME           项目名称\n");
    printf("   PROJECT_VERSION        项目版本\n\n");

    printf("4. 列表变量:\n");
    printf("   set(SOURCES a.c b.c c.c)    # 分号分隔的列表\n");
    printf("   list(APPEND SOURCES d.c)    # 追加元素\n");
    printf("   list(LENGTH SOURCES len)    # 获取长度\n");
    printf("   list(GET SOURCES 0 1 sub)   # 获取子列表\n\n");

    printf("5. 条件判断:\n");
    printf("   if(BUILD_TESTS)\n");
    printf("       add_subdirectory(tests)\n");
    printf("   endif()\n\n");

    printf("   if(CMAKE_BUILD_TYPE STREQUAL \"Debug\")\n");
    printf("       target_compile_definitions(myapp PRIVATE DEBUG_MODE)\n");
    printf("   endif()\n\n");
}

void demo_cmake_targets(void) {
    printf("===== CMake 目标 (Target) =====\n\n");

    printf("CMake 的核心概念是\"目标\" (target)，分为:\n\n");

    printf("1. 可执行目标 (Executable):\n");
    printf("   add_executable(myapp main.c)\n\n");

    printf("2. 库目标 (Library):\n");
    printf("   add_library(mylib STATIC mylib.c)    # 静态库\n");
    printf("   add_library(mylib SHARED mylib.c)    # 动态库\n");
    printf("   add_library(mylib INTERFACE)         # 接口库(无源文件)\n\n");

    printf("3. 目标属性 (Target Properties):\n");
    printf("   target_include_directories(myapp PRIVATE include/)\n");
    printf("   target_compile_options(myapp PRIVATE -Wall -Wextra)\n");
    printf("   target_compile_definitions(myapp PRIVATE DEBUG=1)\n");
    printf("   target_link_libraries(myapp PRIVATE mylib m pthread)\n\n");

    printf("4. 可见性 (Visibility):\n");
    printf("   PRIVATE    - 仅本目标使用\n");
    printf("   INTERFACE  - 仅依赖本目标的其他目标使用\n");
    printf("   PUBLIC     - 本目标和依赖目标都使用\n\n");

    printf("   示例:\n");
    printf("   target_include_directories(mylib\n");
    printf("       PUBLIC  include/          # 头文件对使用者可见\n");
    printf("       PRIVATE src/              # 内部头文件不暴露\n");
    printf("   )\n\n");

    printf("5. 目标间依赖:\n");
    printf("   add_executable(app main.c)\n");
    printf("   target_link_libraries(app PRIVATE mylib)\n");
    printf("   # CMake自动处理: 包含目录、编译选项、链接顺序\n\n");
}

void demo_cmake_find_package(void) {
    printf("===== find_package 机制 =====\n\n");

    printf("find_package 用于查找并加载外部库的配置:\n\n");

    printf("基本用法:\n");
    printf("  find_package(Threads REQUIRED)\n");
    printf("  target_link_libraries(myapp PRIVATE Threads::Threads)\n\n");

    printf("  find_package(OpenSSL REQUIRED)\n");
    printf("  target_link_libraries(myapp PRIVATE OpenSSL::SSL OpenSSL::Crypto)\n\n");

    printf("  find_package(SQLite3 REQUIRED)\n");
    printf("  target_link_libraries(myapp PRIVATE SQLite::SQLite3)\n\n");

    printf("查找模式:\n");
    printf("  Module模式: 查找 Find<Package>.cmake 模块\n");
    printf("    → 设置 <Package>_FOUND, <Package>_INCLUDE_DIRS, <Package>_LIBRARIES\n\n");

    printf("  Config模式: 查找 <Package>Config.cmake 文件\n");
    printf("    → 提供导入目标 (Imported Target)\n");
    printf("    → 如 OpenSSL::SSL, Threads::Threads\n\n");

    printf("推荐: 始终使用导入目标而非原始变量\n");
    printf("  旧式: target_link_libraries(app ${OPENSSL_LIBRARIES})\n");
    printf("  新式: target_link_libraries(app OpenSSL::SSL)\n\n");

    printf("REQUIRED 选项: 找不到包时报错停止\n");
    printf("  find_package(OpenSSL REQUIRED)\n\n");

    printf("可选依赖:\n");
    printf("  find_package(CURL)\n");
    printf("  if(CURL_FOUND)\n");
    printf("      target_link_libraries(app PRIVATE CURL::libcurl)\n");
    printf("      target_compile_definitions(app PRIVATE HAS_CURL)\n");
    printf("  endif()\n\n");
}

int main(void) {
    printf("========== CMake 高级特性示例 ==========\n\n");

    demo_statistics();
    demo_cmake_variables();
    demo_cmake_targets();
    demo_cmake_find_package();

    printf("========== 程序结束 ==========\n");
    return 0;
}
