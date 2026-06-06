/**
 * @file 01_deep_dive_cmake_patterns.c
 * @brief CMake深入剖析 - Modern CMake、接口库、生成器表达式、CMake Presets
 * @description 对应文档: 23-cmake
 *              本文件以独立可运行代码演示Modern CMake的设计模式，
 *              包括接口库、生成器表达式、CMake Presets等高级特性
 *
 * 编译: gcc -Wall -Wextra -std=c11 01_deep_dive_cmake_patterns.c -o deep_dive_cmake
 * 运行: ./deep_dive_cmake
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========================================================================
 * 第一部分: Modern CMake 模式
 * ======================================================================== */

void demo_modern_cmake(void) {
    printf("===== Modern CMake 模式 =====\n\n");

    printf("Modern CMake (3.x+) 的核心思想: 基于目标的属性传播\n\n");

    printf("Old CMake (不推荐):\n");
    printf("  include_directories(include/)       # 全局影响\n");
    printf("  add_definitions(-DDEBUG)             # 全局影响\n");
    printf("  link_libraries(m pthread)            # 全局影响\n\n");

    printf("Modern CMake (推荐):\n");
    printf("  target_include_directories(myapp PRIVATE include/)\n");
    printf("  target_compile_definitions(myapp PRIVATE DEBUG=1)\n");
    printf("  target_link_libraries(myapp PRIVATE m pthread)\n\n");

    printf("Modern CMake 的原则:\n");
    printf("  1. 永远不要使用全局命令 (include_directories 等)\n");
    printf("  2. 一切通过 target 属性传播\n");
    printf("  3. 使用 PRIVATE/PUBLIC/INTERFACE 控制可见性\n");
    printf("  4. 使用导入目标 (Imported Targets) 链接外部库\n");
    printf("  5. 让 CMake 自动推导依赖关系\n\n");

    printf("可见性传播规则:\n");
    printf("  ┌──────────┬──────────────┬──────────────┬──────────────┐\n");
    printf("  │ 可见性   │ 本目标       │ 依赖本目标者 │ 说明         │\n");
    printf("  ├──────────┼──────────────┼──────────────┼──────────────┤\n");
    printf("  │ PRIVATE  │ ✓            │ ✗            │ 仅内部使用   │\n");
    printf("  │ INTERFACE│ ✗            │ ✓            │ 仅对外传播   │\n");
    printf("  │ PUBLIC   │ ✓            │ ✓            │ 内外都可用   │\n");
    printf("  └──────────┴──────────────┴──────────────┴──────────────┘\n\n");

    printf("示例 - 库的公共头文件传播:\n");
    printf("  add_library(mylib mylib.c)\n");
    printf("  target_include_directories(mylib\n");
    printf("      PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include   # 用户需要\n");
    printf("      PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src       # 仅内部\n");
    printf("  )\n");
    printf("  # 链接 mylib 的目标自动获得 include/ 目录\n\n");
}

/* ========================================================================
 * 第二部分: 接口库 (Interface Library)
 * ======================================================================== */

void demo_interface_library(void) {
    printf("===== 接口库 (Interface Library) =====\n\n");

    printf("接口库没有源文件，只传播编译属性。\n");
    printf("非常适合封装编译选项、警告设置、项目约定等。\n\n");

    printf("1. 项目约定库:\n");
    printf("   add_library(project_warnings INTERFACE)\n");
    printf("   target_compile_options(project_warnings INTERFACE\n");
    printf("       -Wall -Wextra -Wpedantic -Wshadow\n");
    printf("       -Wconversion -Werror=return-type\n");
    printf("   )\n\n");

    printf("   # 所有目标链接此库即获得警告选项\n");
    printf("   target_link_libraries(myapp PRIVATE project_warnings)\n\n");

    printf("2. 头文件库 (Header-only):\n");
    printf("   add_library(myheaderlib INTERFACE)\n");
    printf("   target_include_directories(myheaderlib INTERFACE\n");
    printf("       ${CMAKE_CURRENT_SOURCE_DIR}/include\n");
    printf("   )\n\n");

    printf("3. 特定C标准的约定:\n");
    printf("   add_library(project_c_standard INTERFACE)\n");
    printf("   target_compile_features(project_c_standard INTERFACE c_std_11)\n");
    printf("   # 或\n");
    printf("   set_target_properties(project_c_standard PROPERTIES\n");
    printf("       C_STANDARD 11\n");
    printf("       C_STANDARD_REQUIRED ON\n");
    printf("   )\n\n");

    printf("4. 完整的项目约定示例:\n");
    printf("   add_library(project_conventions INTERFACE)\n");
    printf("   target_compile_options(project_conventions INTERFACE\n");
    printf("       -Wall -Wextra -Wpedantic -Wshadow\n");
    printf("   )\n");
    printf("   target_compile_features(project_conventions INTERFACE c_std_11)\n\n");

    printf("   add_executable(myapp main.c)\n");
    printf("   target_link_libraries(myapp PRIVATE\n");
    printf("       project_conventions  # 自动获得所有约定\n");
    printf("       mylib\n");
    printf("   )\n\n");

    printf("举一反三 - 接口库的典型应用:\n");
    printf("  - 封装第三方库的使用方式\n");
    printf("  - 统一项目的编译选项\n");
    printf("  - 提供头文件库的导入目标\n");
    printf("  - 实现条件编译选项的组合\n\n");
}

/* ========================================================================
 * 第三部分: 生成器表达式 (Generator Expression)
 * ======================================================================== */

void demo_generator_expressions(void) {
    printf("===== 生成器表达式 (Generator Expression) =====\n\n");

    printf("生成器表达式在构建时(而非配置时)求值，\n");
    printf("允许根据构建配置、目标属性等动态选择值。\n\n");

    printf("语法: $<condition:value>  或  $<expression>\n\n");

    printf("1. 构建类型条件:\n");
    printf("   target_compile_options(myapp PRIVATE\n");
    printf("       $<$<CONFIG:Debug>:-g -O0>\n");
    printf("       $<$<CONFIG:Release>:-O2 -DNDEBUG>\n");
    printf("   )\n");
    printf("   # Debug时: -g -O0\n");
    printf("   # Release时: -O2 -DNDEBUG\n\n");

    printf("2. 编译器条件:\n");
    printf("   target_compile_options(myapp PRIVATE\n");
    printf("       $<$<C_COMPILER_ID:GNU>:-Wall>\n");
    printf("       $<$<C_COMPILER_ID:MSVC>:/W4>\n");
    printf("       $<$<C_COMPILER_ID:Clang>:-Weverything>\n");
    printf("   )\n\n");

    printf("3. 目标属性查询:\n");
    printf("   $<TARGET_FILE:myapp>          # 目标输出文件路径\n");
    printf("   $<TARGET_PROPERTY:myapp,TYPE> # 目标类型\n");
    printf("   $<TARGET_INCLUDE_DIR:mylib>   # 目标包含目录\n\n");

    printf("4. 布尔表达式:\n");
    printf("   $<BOOL:0>      → 0 (假)\n");
    printf("   $<BOOL:1>      → 1 (真)\n");
    printf("   $<BOOL:\"\">     → 0 (假)\n");
    printf("   $<BOOL:\"hi\">   → 1 (真)\n\n");

    printf("5. 条件链接:\n");
    printf("   target_link_libraries(myapp PRIVATE\n");
    printf("       core_lib\n");
    printf("       $<$<BOOL:${ENABLE_TESTS}>:test_lib>\n");
    printf("   )\n\n");

    printf("6. 实际应用 - 跨平台编译选项:\n");
    printf("   target_compile_options(myapp PRIVATE\n");
    printf("       $<$<AND:$<C_COMPILER_ID:GNU>,$<CONFIG:Release>>:-O3>\n");
    printf("       $<$<PLATFORM_ID:Windows>:-DWIN32_LEAN_AND_MEAN>\n");
    printf("       $<$<PLATFORM_ID:Linux>:-fPIC>\n");
    printf("   )\n\n");

    printf("注意事项:\n");
    printf("  - 生成器表达式不能用于 add_custom_command 的 COMMENT 中\n");
    printf("  - 复杂的生成器表达式难以调试\n");
    printf("  - 简单的条件判断优先使用 if() 命令\n\n");
}

/* ========================================================================
 * 第四部分: CMake Presets
 * ======================================================================== */

void demo_cmake_presets(void) {
    printf("===== CMake Presets (CMakePresets.json) =====\n\n");

    printf("CMake Presets (3.19+) 允许在 JSON 文件中预定义配置，\n");
    printf("用户无需记忆复杂的 cmake 命令行参数。\n\n");

    printf("CMakePresets.json 示例:\n");
    printf("  {\n");
    printf("    \"version\": 3,\n");
    printf("    \"cmakeMinimumRequired\": { \"major\": 3, \"minor\": 20 },\n");
    printf("    \"configurePresets\": [\n");
    printf("      {\n");
    printf("        \"name\": \"debug\",\n");
    printf("        \"binaryDir\": \"${sourceDir}/build/debug\",\n");
    printf("        \"cacheVariables\": {\n");
    printf("          \"CMAKE_BUILD_TYPE\": \"Debug\",\n");
    printf("          \"CMAKE_C_FLAGS\": \"-g -O0 -fsanitize=address\"\n");
    printf("        }\n");
    printf("      },\n");
    printf("      {\n");
    printf("        \"name\": \"release\",\n");
    printf("        \"binaryDir\": \"${sourceDir}/build/release\",\n");
    printf("        \"cacheVariables\": {\n");
    printf("          \"CMAKE_BUILD_TYPE\": \"Release\",\n");
    printf("          \"CMAKE_C_FLAGS\": \"-O2 -DNDEBUG\"\n");
    printf("        }\n");
    printf("      }\n");
    printf("    ],\n");
    printf("    \"buildPresets\": [\n");
    printf("      { \"name\": \"debug\", \"configurePreset\": \"debug\" },\n");
    printf("      { \"name\": \"release\", \"configurePreset\": \"release\" }\n");
    printf("    ]\n");
    printf("  }\n\n");

    printf("使用 Presets:\n");
    printf("  cmake --list-presets          # 列出可用预设\n");
    printf("  cmake --preset debug          # 使用 debug 预设配置\n");
    printf("  cmake --build --preset debug  # 使用 debug 预设构建\n\n");

    printf("Presets 的优势:\n");
    printf("  1. 统一团队构建配置\n");
    printf("  2. 版本控制 (CMakePresets.json 提交到仓库)\n");
    printf("  3. IDE 集成 (VS Code, CLion 自动识别)\n");
    printf("  4. 无需记忆命令行参数\n\n");

    printf("CMakeUserPresets.json:\n");
    printf("  个人预设文件，不提交到仓库 (.gitignore)\n");
    printf("  可以 include 共享的 CMakePresets.json\n\n");
}

/* ========================================================================
 * 第五部分: CMake 项目结构最佳实践
 * ======================================================================== */

void demo_project_structure(void) {
    printf("===== CMake 项目结构最佳实践 =====\n\n");

    printf("推荐的项目目录结构:\n");
    printf("  myproject/\n");
    printf("  ├── CMakeLists.txt           # 顶层 CMakeLists\n");
    printf("  ├── CMakePresets.json        # 预设配置\n");
    printf("  ├── cmake/                   # 自定义 CMake 模块\n");
    printf("  │   ├── CompilerWarnings.cmake\n");
    printf("  │   └── FindMyLib.cmake\n");
    printf("  ├── include/                 # 公共头文件\n");
    printf("  │   └── myproject/\n");
    printf("  │       └── api.h\n");
    printf("  ├── src/                     # 源文件\n");
    printf("  │   ├── CMakeLists.txt\n");
    printf("  │   ├── api.c\n");
    printf("  │   └── internal.h\n");
    printf("  ├── tests/                   # 测试\n");
    printf("  │   ├── CMakeLists.txt\n");
    printf("  │   └── test_api.c\n");
    printf("  └── examples/               # 示例程序\n");
    printf("      ├── CMakeLists.txt\n");
    printf("      └── demo.c\n\n");

    printf("顶层 CMakeLists.txt 模板:\n");
    printf("  cmake_minimum_required(VERSION 3.20)\n\n");
    printf("  project(MyProject\n");
    printf("      VERSION 1.0.0\n");
    printf("      LANGUAGES C\n");
    printf("  )\n\n");
    printf("  # 约定库\n");
    printf("  add_library(project_warnings INTERFACE)\n");
    printf("  target_compile_options(project_warnings INTERFACE\n");
    printf("      -Wall -Wextra -Wpedantic -Wshadow\n");
    printf("  )\n\n");
    printf("  # 子目录\n");
    printf("  add_subdirectory(src)\n\n");
    printf("  option(BUILD_TESTS \"Build tests\" ON)\n");
    printf("  if(BUILD_TESTS)\n");
    printf("      enable_testing()\n");
    printf("      add_subdirectory(tests)\n");
    printf("  endif()\n\n");

    printf("src/CMakeLists.txt:\n");
    printf("  add_library(myproject api.c)\n");
    printf("  target_include_directories(myproject PUBLIC\n");
    printf("      ${CMAKE_CURRENT_SOURCE_DIR}/../include\n");
    printf("  )\n");
    printf("  target_link_libraries(myproject PRIVATE project_warnings)\n\n");
}

/* ========================================================================
 * 第六部分: CMake 常见陷阱与调试
 * ======================================================================== */

void demo_cmake_pitfalls(void) {
    printf("===== CMake 常见陷阱与调试 =====\n\n");

    printf("陷阱1: 使用全局命令\n");
    printf("  include_directories()   → 用 target_include_directories()\n");
    printf("  add_definitions()       → 用 target_compile_definitions()\n");
    printf("  link_libraries()        → 用 target_link_libraries()\n\n");

    printf("陷阱2: 变量作用域误解\n");
    printf("  function() 内修改变量不影响外部\n");
    printf("  需要 PARENT_SCOPE 或使用缓存变量\n\n");

    printf("陷阱3: 列表与字符串混淆\n");
    printf("  set(VAR \"a b c\")    # 一个字符串\n");
    printf("  set(VAR a b c)      # 三个元素的列表\n");
    printf("  列表在内部用分号分隔: \"a;b;c\"\n\n");

    printf("陷阱4: find_package 顺序\n");
    printf("  Module模式先于Config模式\n");
    printf("  可以强制: find_package(Foo CONFIG REQUIRED)\n\n");

    printf("陷阱5: 构建目录污染\n");
    printf("  修改CMakeLists.txt后应重新配置\n");
    printf("  有时需要清空构建目录: rm -rf build/*\n\n");

    printf("调试技巧:\n");
    printf("  1. message() 输出变量:\n");
    printf("     message(STATUS \"VAR = ${VAR}\")\n");
    printf("     message(STATUS \"type = ${CMAKE_BUILD_TYPE}\")\n\n");

    printf("  2. 查看缓存:\n");
    printf("     cmake -LH ..          # 列出所有缓存变量\n");
    printf("     cat CMakeCache.txt    # 直接查看缓存文件\n\n");

    printf("  3. 图形化配置:\n");
    printf("     ccmake ..             # 终端GUI\n");
    printf("     cmake-gui ..          # 桌面GUI\n\n");

    printf("  4. 追踪模式:\n");
    printf("     cmake --trace ..      # 追踪CMake执行\n");
    printf("     cmake --trace-expand  # 追踪并展开变量\n\n");

    printf("  5. 查看目标属性:\n");
    printf("     cmake --build . --target help   # 列出所有目标\n\n");

    printf("举一反三 - CMake 学习路线:\n");
    printf("  入门: cmake_minimum_required + project + add_executable\n");
    printf("  进阶: target_* + add_library + find_package\n");
    printf("  高级: 接口库 + 生成器表达式 + Presets\n");
    printf("  专家: 自定义Find模块 + Toolchain + CPack\n");
    printf("\n");
}

int main(void) {
    printf("================================================\n");
    printf("  CMake深入剖析 - Modern CMake/接口库/生成器表达式\n");
    printf("================================================\n\n");

    demo_modern_cmake();
    demo_interface_library();
    demo_generator_expressions();
    demo_cmake_presets();
    demo_project_structure();
    demo_cmake_pitfalls();

    printf("================================================\n");
    printf("  演示结束\n");
    printf("================================================\n");
    return 0;
}
