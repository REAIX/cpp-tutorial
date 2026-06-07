# CMake 与构建系统实战配置

> **前置阅读**：如果你还没有安装CMake和编译器，请先阅读 [FAQ-138：开发环境配置详解](../../03-问题解答/01-基础概念/33-开发环境配置.md)。本文档假设你已经安装好了CMake和编译器，需要学习如何编写CMakeLists.txt和配置构建系统。

> **相关教程**：CMake基础概念见 [CMake构建系统](../../01-C语言/22-CMake构建系统.md)，Makefile基础见 [Makefile与构建系统](../../01-C语言/21-Makefile与构建系统.md)，静态库/动态库见 [静态库](../../01-C语言/18-静态库.md) 和 [动态库与共享库](../../01-C语言/19-动态库与共享库.md)。

## 1. 使用CMake vs 不使用CMake：核心区别

### 1. 对比表格

| 特性 | 不使用CMake | 使用CMake |
|------|------------|-----------|
| 配置方式 | 手动编写编译命令或tasks.json | 编写CMakeLists.txt，自动生成构建文件 |
| 多文件编译 | 手动列出每个源文件 | 自动收集或用GLOB匹配 |
| 外部库 | 手动指定include和lib路径 | find_package / FetchContent自动查找或下载 |
| 跨平台 | 需要为每个平台写不同配置 | 一份CMakeLists.txt生成各平台构建文件 |
| 调试配置 | 手动在launch.json中指定程序路径 | CMake Tools自动定位构建产物 |
| 构建类型切换 | 手动修改编译选项 | 修改CMAKE_BUILD_TYPE即可 |
| IDE支持 | 需要手动配置各IDE | 自动生成VS/Xcode/Makefile等项目文件 |

### 2. 不使用CMake时，VS Code的tasks.json

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "build",
            "type": "cppbuild",
            "command": "g++",
            "args": [
                "-g",
                "-std=c++17",
                "-I${workspaceFolder}/include",
                "-I/usr/local/include/boost",
                "-L/usr/local/lib",
                "-lboost_system",
                "-lboost_filesystem",
                "${workspaceFolder}/src/main.cpp",
                "${workspaceFolder}/src/utils.cpp",
                "${workspaceFolder}/src/logger.cpp",
                "-o",
                "${workspaceFolder}/build/myapp"
            ],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": ["$gcc"]
        }
    ]
}
```

### 3. 使用CMake时，VS Code的tasks.json

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "cmake-configure",
            "type": "shell",
            "command": "cmake",
            "args": [
                "-B",
                "${workspaceFolder}/build",
                "-S",
                "${workspaceFolder}",
                "-DCMAKE_BUILD_TYPE=Debug"
            ]
        },
        {
            "label": "cmake-build",
            "type": "shell",
            "command": "cmake",
            "args": [
                "--build",
                "${workspaceFolder}/build",
                "--config",
                "Debug"
            ],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "dependsOn": "cmake-configure"
        }
    ]
}
```

### 4. 配置文件写法对比

不使用CMake时，你需要手动管理所有编译参数：

```bash
# 手动编译单个文件
g++ -std=c++17 -g -O0 -I./include -c src/main.cpp -o build/main.o
g++ -std=c++17 -g -O0 -I./include -c src/utils.cpp -o build/utils.o
g++ build/main.o build/utils.o -o build/myapp

# 或者一行搞定（文件少时）
g++ -std=c++17 -g -I./include src/main.cpp src/utils.cpp -o build/myapp
```

使用CMake时，只需编写CMakeLists.txt：

```cmake
cmake_minimum_required(VERSION 3.20)
project(MyApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(myapp
    src/main.cpp
    src/utils.cpp
)

target_include_directories(myapp PRIVATE include)
```

### 5. 什么时候必须用CMake，什么时候可以不用

**必须用CMake的场景：**
- 项目源文件超过5个以上
- 需要链接外部库（Boost、OpenSSL、Qt等）
- 需要跨平台支持（Windows/Linux/macOS）
- 团队协作，不同成员使用不同IDE
- 项目需要生成动态库/静态库供其他项目使用
- 需要自动下载和管理第三方依赖

**可以不用CMake的场景：**
- 单文件或2-3个文件的简单练习项目
- 快速验证某个语法特性的临时代码
- 只用一种IDE且不需要跨平台

---

## 2. CMakeLists.txt 完全写法指南

### 1. cmake_minimum_required — 版本选择

```cmake
# 指定最低CMake版本
cmake_minimum_required(VERSION 3.20)

# 也可以指定一个版本范围（3.20最低，3.27策略使用3.27的）
cmake_minimum_required(VERSION 3.20...3.27)
```

**版本选择建议：**
- 个人项目：使用你安装的最新版本
- 开源项目：使用3.16+（Ubuntu 20.04默认版本）
- 企业项目：根据CI环境最低版本决定

### 2. project() — 项目定义，语言指定

```cmake
# 基本用法
project(MyApp)

# 指定版本号
project(MyApp VERSION 1.0.0)

# 指定语言（CXX = C++）
project(MyApp LANGUAGES CXX)

# 同时指定版本和语言
project(MyApp VERSION 2.1.0 LANGUAGES C CXX)

# 禁用汇编语言步骤（加速配置）
project(MyApp LANGUAGES NONE)
enable_language(CXX)
```

project()之后可使用的变量：

```cmake
# 以下变量由project()自动设置
message(STATUS "项目名称: ${PROJECT_NAME}")        # MyApp
message(STATUS "项目版本: ${PROJECT_VERSION}")      # 2.1.0
message(STATUS "源码目录: ${PROJECT_SOURCE_DIR}")   # CMakeLists.txt所在目录
message(STATUS "二进制目录: ${PROJECT_BINARY_DIR}")  # 构建目录
message(STATUS "C++编译器: ${CMAKE_CXX_COMPILER}")  # 编译器路径
```

### 3. set() — 变量设置

```cmake
# 设置普通变量
set(MY_SOURCES src/main.cpp src/utils.cpp)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)   # 标准必须满足，否则报错
set(CMAKE_CXX_EXTENSIONS OFF)         # 关闭编译器扩展，使用纯标准

# 设置构建类型
set(CMAKE_BUILD_TYPE Debug)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

# 设置列表变量
set(MY_LIBS math utils logger)

# 追加到列表
list(APPEND MY_LIBS network)

# 设置缓存变量（可被命令行覆盖）
set(MY_OPTION ON CACHE BOOL "这是一个可选开关")

# 设置环境变量
set(ENV{PATH} "$ENV{PATH};C:/mylibs/bin")
```

### 4. add_executable() — 可执行目标

```cmake
# 基本用法
add_executable(myapp main.cpp)

# 多个源文件
add_executable(myapp
    src/main.cpp
    src/utils.cpp
    src/logger.cpp
)

# 使用变量指定源文件
set(APP_SOURCES
    src/main.cpp
    src/utils.cpp
)
add_executable(myapp ${APP_SOURCES})

# 创建Win32 GUI应用（Windows下使用WinMain而非main入口）
add_executable(myapp WIN32 main.cpp)
```

### 5. add_library() — 库目标

```cmake
# 静态库（.a / .lib）
add_library(mylib STATIC
    src/mylib.cpp
    src/helper.cpp
)

# 动态库/共享库（.so / .dll / .dylib）
add_library(mylib SHARED
    src/mylib.cpp
    src/helper.cpp
)

# 模块库（运行时加载的插件，.so / .dll）
add_library(myplugin MODULE
    src/plugin.cpp
)

# 对象库（只编译不归档，用于合并到其他目标）
add_library(mylib_objects OBJECT
    src/mylib.cpp
)
add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE mylib_objects)

# 不指定类型时，根据BUILD_SHARED_LIBS变量决定
# BUILD_SHARED_LIBS=ON → SHARED，OFF → STATIC
add_library(mylib
    src/mylib.cpp
)

# 头文件only的库（接口库）
add_library(mylib_header INTERFACE)
target_include_directories(mylib_header INTERFACE include/)
```

### 6. target_include_directories() — 头文件路径

```cmake
# PRIVATE：仅本目标使用，依赖本目标的其他目标看不到
target_include_directories(myapp PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/internal
)

# PUBLIC：本目标和依赖本目标的其他目标都能使用
target_include_directories(mylib PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

# INTERFACE：仅依赖本目标的其他目标使用，本目标不使用
# 通常用于头文件only的接口库
target_include_directories(mylib_header INTERFACE
    ${PROJECT_SOURCE_DIR}/include
)

# 三种可见性的区别总结：
# PRIVATE   → 自己用，别人不能用
# PUBLIC    → 自己用，别人也能用
# INTERFACE → 自己不用，别人能用

# 构建时和安装时使用不同路径
target_include_directories(mylib PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
```

### 7. target_link_libraries() — 链接库

```cmake
# 链接自定义库
target_link_libraries(myapp PRIVATE mylib)

# 链接多个库
target_link_libraries(myapp PRIVATE
    mylib
    mathlib
    logger
)

# 可见性传播
# PRIVATE：仅本目标链接，依赖本目标的目标不会自动链接
target_link_libraries(myapp PRIVATE boost_system)

# PUBLIC：本目标链接，依赖本目标的目标也会自动链接
target_link_libraries(mylib PUBLIC openssl)

# INTERFACE：本目标不链接，仅传播给依赖本目标的目标
target_link_libraries(mylib_header INTERFACE fmt)

# 链接系统库（不需要find_package）
target_link_libraries(myapp PRIVATE pthread)
target_link_libraries(myapp PRIVATE dl)

# 链接全路径库文件
target_link_libraries(myapp PRIVATE /usr/local/lib/libmylib.a)

# 链接时使用调试/发布优化选项
target_link_libraries(myapp PRIVATE
    $<$<CONFIG:Debug>:mylib_debug>
    $<$<CONFIG:Release>:mylib_release>
)
```

### 8. target_compile_options() — 编译选项

```cmake
# 添加警告选项
target_compile_options(myapp PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# GCC/Clang特定选项
target_compile_options(myapp PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wno-unused-parameter>
    $<$<CXX_COMPILER_ID:Clang>:-Wno-unused-parameter>
)

# MSVC特定选项
target_compile_options(myapp PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
)

# 不同构建类型使用不同选项
target_compile_options(myapp PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O2>
)
```

### 9. target_compile_features() — C++标准

```cmake
# 要求C++17特性
target_compile_features(myapp PUBLIC cxx_std_17)

# 要求C++20特性
target_compile_features(myapp PUBLIC cxx_std_20)

# 要求特定语言特性（而非整个标准）
target_compile_features(myapp PRIVATE cxx_lambdas)
target_compile_features(myapp PRIVATE cxx_constexpr)
target_compile_features(myapp PRIVATE cxx_filesystem)
```

### 10. add_subdirectory() — 多目录项目

项目结构：

```
project/
├── CMakeLists.txt          # 顶层
├── src/
│   ├── CMakeLists.txt      # 子目录
│   ├── main.cpp
│   └── utils.cpp
├── lib/
│   ├── CMakeLists.txt      # 子目录
│   ├── mylib.cpp
│   └── mylib.h
└── tests/
    ├── CMakeLists.txt      # 子目录
    └── test_main.cpp
```

顶层CMakeLists.txt：

```cmake
cmake_minimum_required(VERSION 3.20)
project(MyProject VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(tests)
```

lib/CMakeLists.txt：

```cmake
add_library(mylib STATIC
    mylib.cpp
)

target_include_directories(mylib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
```

src/CMakeLists.txt：

```cmake
add_executable(myapp main.cpp utils.cpp)

target_link_libraries(myapp PRIVATE mylib)
```

tests/CMakeLists.txt：

```cmake
add_executable(test_app test_main.cpp)

target_link_libraries(test_app PRIVATE mylib)
```

### 11. aux_source_directory() — 自动收集源文件

```cmake
# 收集当前目录下所有.cpp文件到SRCS变量
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRCS)

# 收集多个目录的源文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/core SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/utils SRCS)

add_executable(myapp ${SRCS})
```

> **注意**：aux_source_directory不会递归搜索子目录，新增文件后需要重新运行CMake配置。

### 12. file(GLOB) — 文件匹配

```cmake
# 匹配所有.cpp文件
file(GLOB SOURCES "src/*.cpp")

# 递归匹配所有子目录中的.cpp文件
file(GLOB_RECURSE SOURCES "src/**/*.cpp")

# 匹配多种扩展名
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.cxx"
    "src/*.cc"
)

add_executable(myapp ${SOURCES})
```

> **注意**：CMake官方不推荐使用GLOB收集源文件，因为新增/删除文件时不会自动触发重新配置。如果使用GLOB，添加新文件后需要手动重新运行CMake。

### 13. configure_file() — 配置头文件

创建模板文件 `config.h.in`：

```c
#pragma once

#define PROJECT_NAME "@PROJECT_NAME@"
#define PROJECT_VERSION "@PROJECT_VERSION@"

#cmakedefine USE_FEATURE_A
#cmakedefine USE_FEATURE_B
#cmakedefine01 ENABLE_LOGGING

#define DATA_PATH "@DATA_PATH@"
```

在CMakeLists.txt中使用：

```cmake
# 设置选项
option(USE_FEATURE_A "启用功能A" ON)
option(USE_FEATURE_B "启用功能B" OFF)
option(ENABLE_LOGGING "启用日志" ON)

set(DATA_PATH "${CMAKE_INSTALL_PREFIX}/data")

# 生成配置头文件
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    @ONLY
)

# 让目标能找到生成的头文件
add_executable(myapp main.cpp)
target_include_directories(myapp PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
```

生成的 `config.h`：

```c
#pragma once

#define PROJECT_NAME "MyApp"
#define PROJECT_VERSION "1.0.0"

#define USE_FEATURE_A
/* #undef USE_FEATURE_B */
#define ENABLE_LOGGING 1

#define DATA_PATH "/usr/local/data"
```

### 14. install() — 安装规则

```cmake
# 安装可执行文件
install(TARGETS myapp
    RUNTIME DESTINATION bin
)

# 安装库文件
install(TARGETS mylib
    ARCHIVE DESTINATION lib       # 静态库
    LIBRARY DESTINATION lib       # 共享库（Linux）
    RUNTIME DESTINATION bin       # DLL文件（Windows）
)

# 安装头文件
install(FILES
    include/mylib.h
    include/helper.h
    DESTINATION include
)

# 安装整个目录
install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# 安装时导出CMake配置（让其他项目通过find_package找到）
install(TARGETS mylib
    EXPORT mylibTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(EXPORT mylibTargets
    FILE mylibTargets.cmake
    NAMESPACE mylib::
    DESTINATION lib/cmake/mylib
)
```

### 15. 完整的多文件多目录项目示例

项目结构：

```
myproject/
├── CMakeLists.txt
├── include/
│   └── myapp/
│       ├── calculator.h
│       └── printer.h
├── src/
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── calculator.cpp
│   └── printer.cpp
├── libs/
│   └── mathutils/
│       ├── CMakeLists.txt
│       ├── mathutils.h
│       └── mathutils.cpp
└── tests/
    ├── CMakeLists.txt
    └── test_calculator.cpp
```

顶层 CMakeLists.txt：

```cmake
cmake_minimum_required(VERSION 3.20)
project(MyProject VERSION 1.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

option(BUILD_TESTS "是否构建测试" ON)

add_subdirectory(libs/mathutils)
add_subdirectory(src)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
```

libs/mathutils/CMakeLists.txt：

```cmake
add_library(mathutils STATIC mathutils.cpp)

target_include_directories(mathutils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(mathutils PRIVATE -Wall -Wextra)
```

src/CMakeLists.txt：

```cmake
set(APP_SOURCES
    main.cpp
    calculator.cpp
    printer.cpp
)

add_executable(myapp ${APP_SOURCES})

target_include_directories(myapp PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(myapp PRIVATE mathutils)

target_compile_options(myapp PRIVATE -Wall -Wextra -Wpedantic)
```

tests/CMakeLists.txt：

```cmake
add_executable(test_calculator test_calculator.cpp)

target_include_directories(test_calculator PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(test_calculator PRIVATE mathutils)

add_test(
    NAME test_calculator
    COMMAND test_calculator
)
```

---
