# CMake构建系统

> 掌握CMake的使用与现代项目构建

***

> 💡 **通俗理解 - 什么是CMake？**

想象你要建一座房子：

- **手动写Makefile**：就像手写每一份建筑图纸
- **用CMake**：就像用"建筑建模软件"，自动生成图纸

**CMake就是"自动生成Makefile的工具"！**

- 用更简单的语法描述项目
- 自动生成不同平台的Makefile
- 支持复杂的项目结构

***

> 🔬 **抽象理解 - CMake的本质**：
>
> - **CMake**：是"跨平台的构建系统生成器"
> - **CMakeLists.txt**：是CMake的配置文件，描述项目结构
> - **CMake**：读取CMakeLists.txt，生成平台相关的构建文件
> - **Generator**：是"生成器"，生成Makefile、Ninja、Visual Studio项目等

***

> **🎯 君子生非异也，善假于物也。善用CMake，事半功倍。**
>
> （君子不是因为生来与别人不同，而是善于借助外力；善于使用CMake，可以让编译效率翻倍。）

## 前置知识
- [Makefile与构建系统](21-Makefile与构建系统.md)
## 后续内容
- [进程与线程](23-进程与线程.md)
***

## 目录

- [1. CMake基础](#1-cmake基础)
- [2. 变量与函数](#2-变量与函数)
- [3. 目标与链接](#3-目标与链接)
- [4. 条件与循环](#4-条件与循环)
- [5. 实战CMake项目](#5-实战cmake项目)

***

## 1. CMake基础

### 1. 基本结构

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(MyApp)

# 添加可执行文件
add_executable(myapp main.cpp)
```

### 2. 使用CMake

```bash
# 1. 创建build目录
mkdir build
cd build

# 2. 配置（生成Makefile）
cmake ..

# 3. 编译
cmake --build .

# 或者
make
```

### 3. 项目结构

```
project/
├── CMakeLists.txt
├── src/
│   ├── CMakeLists.txt
│   ├── main.cpp
│   └── utils.cpp
├── include/
│   └── utils.h
└── build/
```

***

## 2. 变量与函数

### 1. 变量

```cmake
# 定义变量
set(MY_VAR "hello")
set(SOURCES main.cpp utils.cpp)

# 使用变量
add_executable(myapp ${SOURCES})

# 列表
set(PRIVATE_LIBS pthread curl)
set(PUBLIC_LIBS json)
```

### 2. 内置变量

```cmake
# 常用内置变量
CMAKE_SOURCE_DIR        # 源代码根目录
CMAKE_BINARY_DIR        # 构建目录
CMAKE_CURRENT_SOURCE_DIR# 当前CMakeLists.txt目录
CMAKE_CXX_COMPILER     # C++编译器
CMAKE_CXX_STANDARD     # C++标准
```

### 3. 字符串操作

```cmake
# 字符串拼接
set(ALL_SOURCES "${SRC_DIR}/main.cpp")

# 字符串替换
string(REPLACE ".cpp" ".o" OBJECTS "${SOURCES}")
```

***

## 3. 目标与链接

### 1. 添加可执行文件

```cmake
# 添加可执行文件
add_executable(myapp main.cpp)
add_executable(myapp main.cpp utils.cpp)
```

### 2. 添加库

```cmake
# 添加静态库
add_library(mylib STATIC utils.cpp)

# 添加动态库
add_library(mylib SHARED utils.cpp)

# 添加对象库（只编译不链接）
add_library(mylib OBJECT utils.cpp)
```

### 3. 链接库

```cmake
# 链接库
target_link_libraries(myapp pthread)
target_link_libraries(myapp mylib)

# 链接多个库
target_link_libraries(myapp pthread curl json)
```

### 4. 添加头文件目录

```cmake
# 添加头文件搜索路径
target_include_directories(myapp PUBLIC ${CMAKE_SOURCE_DIR}/include)
```

***

## 4. 条件与循环

### 1. 条件判断

```cmake
# 判断操作系统
if(WIN32)
    message(STATUS "Windows")
elseif(UNIX)
    message(STATUS "Linux/Unix")
endif()

# 判断编译器
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "GCC")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "Clang")
endif()

# 判断选项
option(ENABLE_TESTS "Enable tests" ON)
if(ENABLE_TESTS)
    enable_testing()
endif()
```

### 2. 循环

```cmake
# 遍历列表
foreach(src ${SOURCES})
    message(STATUS "Source: ${src}")
endforeach()

# 遍历目录
foreach(dir ${DIRS})
    add_subdirectory(${dir})
endforeach()
```

***

## 5. 实战CMake项目

### 1. 单目录项目

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(MyApp LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找源文件
file(GLOB SOURCES "*.cpp")

# 添加可执行文件
add_executable(myapp ${SOURCES})

# 链接库
target_link_libraries(myapp pthread)
```

### 2. 多目录项目

```
project/
├── CMakeLists.txt           # 根目录
├── src/
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── utils.cpp
│   └── utils.h
├── lib/
│   ├── CMakeLists.txt
│   └── mylib.cpp
└── include/
    └── mylib.h
```

**根目录CMakeLists.txt：**

```cmake
cmake_minimum_required(VERSION 3.16)
project(MyApp)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加子目录
add_subdirectory(src)
add_subdirectory(lib)

# 链接库
target_link_libraries(myapp mylib)
```

**src/CMakeLists.txt：**

```cmake
# 查找源文件
file(GLOB SOURCES "*.cpp")

# 添加可执行文件
add_executable(myapp ${SOURCES})

# 链接库
target_link_libraries(myapp mylib pthread)

# 添加头文件目录
target_include_directories(myapp 
    PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
)
```

**lib/CMakeLists.txt：**

```cmake
# 查找源文件
file(GLOB SOURCES "*.cpp")

# 添加库
add_library(mylib STATIC ${SOURCES})

# 添加头文件目录
target_include_directories(mylib 
    PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
)
```

### 3. 使用外部库

CMake管理第三方依赖有两大王牌手段：**find_package** 和 **FetchContent**。

#### 1. find_package —— 查找系统已安装的库

```cmake
# 查找系统自带的线程库（必须找到，否则报错）
find_package(Threads REQUIRED)
```

**它是干什么的？**

去**系统目录、环境变量**里搜索：
- 已经提前安装好的库
- 系统自带库（线程、OpenGL、Python、Boost等）

**作用：**
告诉CMake："去系统里帮我找到Threads库，找不到就直接报错停止！"

**常见用法：**
```cmake
find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED)
find_package(Threads REQUIRED)
```

**特点：**
- 库**必须提前安装**在系统里
- 速度快
- 适合系统库、大型预编译库

#### 2. FetchContent —— 自动下载源码依赖

```cmake
# 自动下载、编译、集成第三方库
include(FetchContent)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)

FetchContent_MakeAvailable(json)
```

**它是干什么的？**

**不需要你手动下载、不需要安装！**
CMake自动帮你：
1. 从GitHub下载源码
2. 编译
3. 直接集成到你的项目

**超级优势：**
- **一行代码引入第三方库**
- 不用装、不用配、不用管路径
- 全平台自动编译
- 版本锁定，不会乱

**现代C++最推荐的方式！**

#### 3. target_link_libraries —— 链接库

```cmake
target_link_libraries(myapp nlohmann_json::nlohmann_json)
```

把**下载好的库**链接到你的可执行文件。

#### 4. 核心对比

| 方式 | 库在哪 | 要不要提前安装 |
|------|--------|----------------|
| **find_package** | **系统里已安装** | ✅ **必须先安装** |
| **FetchContent** | **自动从网上下载** | ❌ **不用安装** |

#### 5. 终极总结

**find_package = 找电脑里已经装好的库**

**FetchContent = 自动下载新库**

**一句话记住：**
- **先安装，才能 find_package**
- **不安装，就用 FetchContent 自动下！**

#### 6. 完整示例

```cmake
# 查找系统库
find_package(Threads REQUIRED)

# 使用FetchContent下载依赖
include(FetchContent)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

# 链接
target_link_libraries(myapp nlohmann_json::nlohmann_json)
target_link_libraries(myapp Threads::Threads)
```

### 4. 安装与打包

#### 1. 一、install() 安装规则

```cmake
# 安装可执行程序到 bin 目录
install(TARGETS myapp
    RUNTIME DESTINATION bin
)

# 安装头文件到 include 目录
install(FILES mylib.h
    DESTINATION include
)
```

**这是干什么？**

当你执行：
```bash
make install
# 或者 Windows
ninja install
```

CMake 会自动把你的**程序、库、头文件**复制到**系统目录/安装目录**：

- **myapp.exe** → 复制到 `bin/`
- **mylib.h** → 复制到 `include/`
- **动态库 .dll/.so** → 复制到 `lib/`

**最终安装目录结构：**
```
prefix/
├── bin/
│   └── myapp.exe
├── include/
│   └── mylib.h
└── lib/
    └── mylib.dll
```

#### 2. 二、生成 CMake 配置包（给别人用）

```cmake
include(CMakePackageConfigHelpers)

configure_package_config_file(
    cmake/MyAppConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/MyAppConfig.cmake
)

install(EXPORT MyAppTargets
    FILE MyAppConfig.cmake
    DESTINATION lib/cmake/MyApp
)
```

**这是干什么？**

**让别人可以用一句 find_package(MyApp) 直接使用你的库！**

例如别人用你的库，只需要：
```cmake
find_package(MyApp REQUIRED)
target_link_libraries(xxx MyApp::mylib)
```

背后就是靠你生成的：
- **MyAppConfig.cmake**
- **MyAppTargets.cmake**

这是 **C++ 开源项目、SDK、库发布的标准方式**。

#### 3. 三、总结

**install()**：
- 把你的 **exe、dll、头文件** 复制到安装目录
- 让程序可以**被系统找到、运行**

**EXPORT + Config.cmake**：
- 生成 **CMake 配置文件**
- 让别人 **find_package** 就能用你的库
- 这是 **C/C++ 工程化、SDK 发布的标准**

**整体一句话总结：**
- **install = 安装程序到系统**
- **Config.cmake = 让别人能轻松 find_package 引用你的库**

这已经是 **企业级 C++ 项目的完整发布流程**！

***

## 6. 常用CMake命令速查

```cmake
# 项目初始化
cmake_minimum_required(VERSION 3.16)
project(MyApp LANGUAGES CXX)

# 添加可执行文件
add_executable(name source.cpp)

# 添加库
add_library(name STATIC source.cpp)  # 生成 静态库 .a / .lib
add_library(name SHARED source.cpp)   # 生成 动态库 .so / .dll
add_library(name OBJECT source.cpp)  # 生成 目标文件 .o（不打包成库）

# 链接库
target_link_libraries(target library)

# 头文件目录
target_include_directories(target PUBLIC/PRIVATE path)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找包
find_package(Threads REQUIRED)

# 添加子目录
add_subdirectory(path)

# 安装
install(TARGETS target DESTINATION bin)
install(FILES file DESTINATION include)
```

***

## 7. 常见问题

**Q：CMake找不到头文件？**
A：

```cmake
target_include_directories(myapp PRIVATE ${CMAKE_SOURCE_DIR}/include)
```

**Q：CMake找不到库？**
A：

```cmake
# 方法1：find_package
find_package(Threads REQUIRED)
target_link_libraries(myapp Threads::Threads)

# 方法2：手动指定
target_link_libraries(myapp /path/to/libxxx.a)
```

**Q：如何指定编译器？**
A：

```bash
cmake -DCMAKE_CXX_COMPILER=g++ ..
cmake -DCMAKE_C_COMPILER=gcc ..
```

**Q：如何生成调试版本？**
A：

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake -DCMAKE_BUILD_TYPE=Release ..
```

***

## 8. Modern CMake理念

### 1. target-based设计：告别全局变量

旧式CMake最大的问题：**用全局变量影响所有目标**。

```cmake
# 旧式写法（不推荐）
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_STANDARD 17)
include_directories(${CMAKE_SOURCE_DIR}/include)
add_definitions(-DDEBUG)
```

**问题**：这些设置影响项目中**所有**目标，包括第三方库！你只想给自己项目加`-Wall`，结果连依赖库也被影响了。

**Modern CMake的核心思想**：一切围绕**目标（target）**，而非全局变量。

```cmake
# Modern CMake写法（推荐）
add_executable(myapp main.cpp)
target_compile_options(myapp PRIVATE -Wall -Wextra)
target_compile_features(myapp PRIVATE cxx_std_17)
target_include_directories(myapp PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(myapp PRIVATE DEBUG)
```

**好处**：设置只影响`myapp`这一个目标，不会"污染"其他目标。

### 2. target_compile_features代替set(CMAKE_CXX_STANDARD)

```cmake
# 旧式：全局设置，影响所有目标
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Modern：只对特定目标生效
target_compile_features(myapp PRIVATE cxx_std_17)
target_compile_features(mylib PUBLIC cxx_std_20)
```

**`target_compile_features`的优势**：
- 精确控制每个目标使用的C++标准
- 通过`PUBLIC/INTERFACE`可以传播给依赖方
- 可以指定具体语言特性，如`cxx_auto_type`、`cxx_lambdas`

```cmake
# 只要求特定特性，而非整个标准
target_compile_features(myapp PRIVATE
    cxx_auto_type
    cxx_lambdas
    cxx_range_for
)
```

### 3. target_compile_options / target_compile_definitions

```cmake
# 编译选项
target_compile_options(myapp
    PRIVATE
        -Wall -Wextra -Werror
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O2>
)

# 编译定义
target_compile_definitions(myapp
    PRIVATE
        VERSION="1.0"
        $<$<CONFIG:Debug>:DEBUG_MODE>
)
```

**关键区别**：
- `target_compile_options`：添加编译器选项（如`-Wall`）
- `target_compile_definitions`：添加预处理器宏（如`-DVERSION="1.0"`）
- 两者都支持`PRIVATE/PUBLIC/INTERFACE`传播规则

### 4. 接口库（INTERFACE library）：管理纯头文件库

纯头文件库没有`.cpp`文件，无法编译成`.a`或`.so`。CMake用`INTERFACE`库来管理它们：

```cmake
# 创建接口库
add_library(myheader_lib INTERFACE)

# 设置接口属性（注意：只能用INTERFACE关键字）
target_include_directories(myheader_lib INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(myheader_lib INTERFACE cxx_std_17)
target_compile_definitions(myheader_lib INTERFACE MY_LIB_HEADER_ONLY)
```

**使用接口库**：

```cmake
add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE myheader_lib)
# myapp自动获得myheader_lib的所有INTERFACE属性：
# - 头文件搜索路径
# - C++17标准
# - MY_LIB_HEADER_ONLY宏定义
```

**典型场景**：nlohmann/json、spdlog等纯头文件库。

### 5. PRIVATE / PUBLIC / INTERFACE 传播规则

这是Modern CMake最重要的概念之一：

| 关键字 | 当前目标 | 依赖方 | 说明 |
|--------|---------|--------|------|
| `PRIVATE` | ✅ 使用 | ❌ 不传播 | 只给自己用 |
| `PUBLIC` | ✅ 使用 | ✅ 传播 | 自己用，也给别人用 |
| `INTERFACE` | ❌ 不使用 | ✅ 传播 | 只给别人用（接口库专用） |

**直观理解**：

```cmake
# mylib是一个库
add_library(mylib utils.cpp)

# PRIVATE：mylib自己需要这个头文件路径，但链接mylib的人不需要
target_include_directories(mylib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# PUBLIC：mylib自己需要，链接mylib的人也需要（因为头文件里暴露了这些类型）
target_include_directories(mylib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# INTERFACE：mylib自己不需要，但链接mylib的人需要（纯头文件库场景）
target_include_directories(mylib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
```

**传播链示例**：

```cmake
# 库A：使用PUBLIC传播
add_library(A a.cpp)
target_compile_definitions(A PUBLIC LIB_A_ENABLED)

# 库B：链接A，使用PUBLIC传播
add_library(B b.cpp)
target_link_libraries(B PUBLIC A)
target_compile_definitions(B PUBLIC LIB_B_ENABLED)

# 可执行文件：链接B
add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE B)
# myapp同时获得：LIB_A_ENABLED 和 LIB_B_ENABLED
# 因为A的PUBLIC定义通过B的PUBLIC链接传播过来了
```

### 6. 对比：旧式CMake vs Modern CMake

```cmake
# ============================================
# 旧式CMake（不推荐）
# ============================================
cmake_minimum_required(VERSION 3.16)
project(MyApp)

# 全局设置，影响所有目标
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
include_directories(${CMAKE_SOURCE_DIR}/include)
add_definitions(-DDEBUG)
link_libraries(pthread)

add_executable(myapp main.cpp)
add_library(mylib utils.cpp)
```

```cmake
# ============================================
# Modern CMake（推荐）
# ============================================
cmake_minimum_required(VERSION 3.16)
project(MyApp LANGUAGES CXX)

add_library(mylib utils.cpp)
target_compile_features(mylib PUBLIC cxx_std_17)
target_compile_options(mylib PRIVATE -Wall -Wextra)
target_include_directories(mylib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_definitions(mylib PRIVATE DEBUG)

add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE mylib)
# myapp自动从mylib获得C++17标准和include路径
```

**Modern CMake的优势**：
- **隔离性**：每个目标的设置互不干扰
- **传播性**：依赖关系自动传递编译选项和头文件路径
- **可维护性**：修改一个库的设置不会意外影响其他目标
- **可复用性**：库可以被其他项目直接使用，自带所有必要配置

***

## 9. Generator Expressions

### 1. 什么是Generator Expressions？

Generator Expressions（生成器表达式）是在**生成构建系统时**求值的表达式，而不是在CMake配置时求值。

简单理解：普通CMake变量在`cmake ..`时就确定了，而生成器表达式在`cmake --build .`时才求值。这让同一个CMakeLists.txt可以根据不同配置产生不同行为。

### 2. $<CONFIG:Debug> 条件表达式

最常用的生成器表达式——根据构建配置（Debug/Release）选择不同值：

```cmake
add_executable(myapp main.cpp)

# Debug模式加调试选项，Release模式加优化选项
target_compile_options(myapp PRIVATE
    $<$<CONFIG:Debug>:-g -O0 -fsanitize=address>
    $<$<CONFIG:Release>:-O2 -DNDEBUG>
)

# 根据配置链接不同库
target_link_libraries(myapp PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=address>
)
```

**语法**：`$<condition:true_value>`，条件为真时展开为`true_value`，否则为空。

**多配置生成器**（Visual Studio、Ninja Multi-Config）可以同时支持Debug和Release：

```cmake
# 设置不同配置的输出目录
set_target_properties(myapp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/release
)
```

### 3. $<TARGET_FILE:target> 获取目标文件路径

获取目标编译后的文件完整路径：

```cmake
add_executable(myapp main.cpp)

# 打印可执行文件路径
add_custom_command(TARGET myapp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "生成文件: $<TARGET_FILE:myapp>"
)

# 复制可执行文件到指定目录
add_custom_command(TARGET myapp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:myapp>
        ${CMAKE_BINARY_DIR}/dist/
)
```

**相关的生成器表达式**：

| 表达式 | 说明 |
|--------|------|
| `$<TARGET_FILE:target>` | 目标文件的完整路径 |
| `$<TARGET_FILE_NAME:target>` | 目标文件名（含扩展名） |
| `$<TARGET_FILE_DIR:target>` | 目标文件所在目录 |
| `$<TARGET_LINKER_FILE:target>` | 链接器使用的文件路径（.a/.lib） |

### 4. $<IF:cond,true,false> 条件选择

CMake 3.8+支持的三元表达式：

```cmake
# 根据条件选择不同值
target_compile_definitions(myapp PRIVATE
    $<IF:$<CONFIG:Debug>,DEBUG_BUILD,RELEASE_BUILD>
)

# 结合option使用
option(ENABLE_LOGGING "启用日志" ON)
target_compile_definitions(myapp PRIVATE
    $<IF:${ENABLE_LOGGING},LOG_ENABLED,LOG_DISABLED>
)
```

### 5. 在install和自定义命令中使用

**install中使用生成器表达式**：

```cmake
install(TARGETS myapp
    RUNTIME DESTINATION $<IF:$<CONFIG:Debug>,debug/bin,bin>
)

# 根据配置安装不同的库
install(FILES
    $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/config/debug.conf>
    $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/config/release.conf>
    DESTINATION etc
)
```

**自定义命令中使用**：

```cmake
# 构建后自动运行（仅Debug模式）
add_custom_command(TARGET myapp POST_BUILD
    COMMAND $<TARGET_FILE:myapp> --test
    COMMENT "运行测试（仅Debug模式）"
    $<$<NOT:$<CONFIG:Debug>>:COMMAND_ECHO>  # 非Debug模式跳过
)
```

**生成器表达式常用模式汇总**：

```cmake
# 编译选项：根据编译器和配置选择
target_compile_options(myapp PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O2>
)

# 编译定义：根据平台和配置
target_compile_definitions(myapp PRIVATE
    $<$<PLATFORM_ID:Windows>:WINDOWS_BUILD>
    $<$<PLATFORM_ID:Linux>:LINUX_BUILD>
    $<$<CONFIG:Debug>:DEBUG_MODE>
)

# 链接选项
target_link_options(myapp PRIVATE
    $<$<CONFIG:Release>:-s>  # Release模式去除符号表
)
```

***

## 10. CMake Presets

### 1. CMakePresets.json的作用

`CMakePresets.json`是CMake 3.19+引入的功能，用于**预定义配置和构建参数**。

**为什么需要Presets？**
- 不用每次手动输入长长的`cmake -D... -D...`命令
- 团队成员使用统一的构建配置
- CI/CD脚本更简洁
- 支持多配置一键切换

**文件位置**：放在项目根目录，与`CMakeLists.txt`同级

```
project/
├── CMakeLists.txt
├── CMakePresets.json    # 团队共享的预设（提交到版本控制）
├── CMakeUserPresets.json # 个人预设（不提交，加入.gitignore）
└── src/
```

### 2. configurePreset和buildPreset

**configurePreset**：定义`cmake ..`阶段的参数

```json
{
    "version": 7,
    "configurePresets": [
        {
            "name": "debug",
            "binaryDir": "${sourceDir}/build/debug",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug",
                "CMAKE_CXX_COMPILER": "g++"
            }
        },
        {
            "name": "release",
            "binaryDir": "${sourceDir}/build/release",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Release"
            }
        }
    ]
}
```

**buildPreset**：定义`cmake --build`阶段的参数

```json
{
    "version": 7,
    "configurePresets": [
        {
            "name": "debug",
            "binaryDir": "${sourceDir}/build/debug",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug"
            }
        },
        {
            "name": "release",
            "binaryDir": "${sourceDir}/build/release",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Release"
            }
        }
    ],
    "buildPresets": [
        {
            "name": "debug",
            "configurePreset": "debug",
            "configuration": "Debug",
            "jobs": 8
        },
        {
            "name": "release",
            "configurePreset": "release",
            "configuration": "Release"
        }
    ]
}
```

**使用方式**：

```bash
# 用debug预设配置
cmake --preset debug

# 用debug预设构建
cmake --build --preset debug

# 用release预设配置和构建
cmake --preset release
cmake --build --preset release
```

### 3. 多配置支持（Debug/Release）

使用多配置生成器（如Ninja Multi-Config、Visual Studio）时，可以在一个构建目录中同时支持Debug和Release：

```json
{
    "version": 6,
    "configurePresets": [
        {
            "name": "multi-config",
            "binaryDir": "${sourceDir}/build",
            "generator": "Ninja Multi-Config",
            "cacheVariables": {
                "CMAKE_CONFIGURATION_TYPES": "Debug;Release"
            }
        }
    ],
    "buildPresets": [
        {
            "name": "debug",
            "configurePreset": "multi-config",
            "configuration": "Debug"
        },
        {
            "name": "release",
            "configurePreset": "multi-config",
            "configuration": "Release"
        }
    ]
}
```

**使用方式**：

```bash
# 配置一次
cmake --preset multi-config

# 构建Debug版本
cmake --build --preset debug

# 构建Release版本
cmake --build --preset release
```

### 4. 示例CMakePresets.json

一个完整的、适合实际项目的`CMakePresets.json`：

```json
{
    "version": 6,
    "cmakeMinimumRequired": {
        "major": 3,
        "minor": 25,
        "patch": 0
    },
    "configurePresets": [
        {
            "name": "base",
            "hidden": true,
            "binaryDir": "${sourceDir}/build/${presetName}",
            "cacheVariables": {
                "CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
            }
        },
        {
            "name": "debug",
            "displayName": "调试配置",
            "inherits": "base",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug",
                "ENABLE_TESTS": "ON",
                "ENABLE_SANITIZER": "ON"
            }
        },
        {
            "name": "release",
            "displayName": "发布配置",
            "inherits": "base",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Release",
                "ENABLE_TESTS": "OFF"
            }
        },
        {
            "name": "msvc",
            "displayName": "MSVC配置",
            "inherits": "base",
            "generator": "Visual Studio 17 2022",
            "cacheVariables": {
                "CMAKE_CONFIGURATION_TYPES": "Debug;Release"
            }
        }
    ],
    "buildPresets": [
        {
            "name": "debug",
            "configurePreset": "debug",
            "jobs": 8
        },
        {
            "name": "release",
            "configurePreset": "release",
            "jobs": 8
        },
        {
            "name": "msvc-debug",
            "configurePreset": "msvc",
            "configuration": "Debug"
        },
        {
            "name": "msvc-release",
            "configurePreset": "msvc",
            "configuration": "Release"
        }
    ],
    "testPresets": [
        {
            "name": "debug",
            "configurePreset": "debug",
            "output": {
                "outputOnFailure": true
            },
            "execution": {
                "jobs": 4
            }
        }
    ]
}
```

**使用方式**：

```bash
# 列出所有可用预设
cmake --list-presets

# 配置
cmake --preset debug

# 构建
cmake --build --preset debug

# 运行测试
ctest --preset debug
```

**关键特性说明**：
- `hidden: true`：隐藏预设，不直接使用，只用于被其他预设继承
- `inherits`：继承其他预设的配置，避免重复
- `CMAKE_EXPORT_COMPILE_COMMANDS`：生成`compile_commands.json`，供IDE和工具使用
- `testPresets`：预定义测试配置，配合`ctest`使用

***

## 11. 相关链接

**上一章：** [第22章：Makefile与构建系统](21-Makefile与构建系统.md)\
**下一章：** [第24章：进程与线程](23-进程与线程.md)

***

### 1. 相关章节

- [CMake与构建系统实战配置](../04-工程实践/开发环境/03-CMake基础入门.md) — CMakeLists.txt完全写法、外部库4种配置方式
- [VS Code开发环境完全配置指南](../04-工程实践/开发环境/00-VSCode核心配置.md) — VS Code + CMake Tools完整工作流
- [CLion开发环境配置](../04-工程实践/开发环境/06-CLion开发环境配置.md) — CLion + CMake配合使用
- [Visual Studio开发环境配置指南](../04-工程实践/开发环境/05-VisualStudio开发环境配置.md) — VS打开CMake项目

