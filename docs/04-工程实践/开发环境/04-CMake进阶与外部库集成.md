# CMake 进阶与外部库集成

> **前置阅读**：如果你还不熟悉CMake基础语法，请先阅读 [CMake基础入门](./03-CMake基础入门.md)。本文档假设你已经掌握了CMake的基本用法，需要学习如何集成外部库。

## 1. CMake配置外部库的4种方式

### 1. 方式1：find_package() — 系统安装的库

#### 1. find_package的查找机制

find_package有两种查找模式：

**Module模式**：查找 `FindXXX.cmake` 文件
- 搜索路径：`CMAKE_MODULE_PATH` 中的目录，然后是CMake自带的 `/usr/share/cmake/Modules/`
- 文件命名：`FindBoost.cmake`、`FindOpenSSL.cmake` 等
- 这是传统方式，CMake自带了很多Find模块

**Config模式**：查找 `XXXConfig.cmake` 或 `xxx-config.cmake` 文件
- 搜索路径：`CMAKE_PREFIX_PATH`、标准安装路径等
- 文件命名：`BoostConfig.cmake`、`openssl-config.cmake` 等
- 这是现代方式，由库的作者提供，更可靠

```cmake
# 优先使用Config模式
find_package(Boost CONFIG REQUIRED)

# 指定查找组件
find_package(Boost REQUIRED COMPONENTS filesystem system)

# 指定最低版本
find_package(OpenSSL 1.1.0 REQUIRED)

# 可选依赖（找不到不报错）
find_package(spdlog QUIET)
if(spdlog_FOUND)
    target_link_libraries(myapp PRIVATE spdlog::spdlog)
endif()
```

#### 2. 使用示例：find_package(Boost REQUIRED)

```cmake
cmake_minimum_required(VERSION 3.20)
project(BoostDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# 查找Boost，需要filesystem和system组件
find_package(Boost REQUIRED COMPONENTS filesystem system)

add_executable(boost_demo main.cpp)

# 链接Boost（使用导入目标方式，推荐）
target_link_libraries(boost_demo PRIVATE
    Boost::filesystem
    Boost::system
)

# 如果Boost安装在非标准路径，指定前缀
# set(CMAKE_PREFIX_PATH "C:/boost_1_84_0")
# 或者
# set(BOOST_ROOT "C:/boost_1_84_0")
```

main.cpp：

```cpp
#include <iostream>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

int main() {
    fs::path p = fs::current_path();
    std::cout << "当前路径: " << p.string() << std::endl;
    return 0;
}
```

#### 3. 使用示例：find_package(OpenSSL REQUIRED)

```cmake
cmake_minimum_required(VERSION 3.20)
project(OpenSSLDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenSSL REQUIRED)

add_executable(openssl_demo main.cpp)

target_link_libraries(openssl_demo PRIVATE OpenSSL::SSL OpenSSL::Crypto)

# 查看找到的OpenSSL版本信息
message(STATUS "OpenSSL版本: ${OPENSSL_VERSION}")
message(STATUS "OpenSSL包含目录: ${OPENSSL_INCLUDE_DIR}")
```

### 2. 方式2：FetchContent — 自动下载

FetchContent是CMake 3.14+引入的功能，可以在配置阶段自动下载和构建第三方库。

```cmake
cmake_minimum_required(VERSION 3.20)
project(FetchContentDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
```

#### 1. FetchContent_Declare + FetchContent_MakeAvailable

```cmake
# 声明要下载的内容
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)

# 下载并使其可用（如果已经下载过则跳过）
FetchContent_MakeAvailable(googletest)

# 现在可以直接使用googletest的导入目标
enable_testing()
add_executable(my_test test.cpp)
target_link_libraries(my_test PRIVATE GTest::gtest_main)
add_test(NAME my_test COMMAND my_test)
```

#### 2. 使用示例：FetchContent下载googletest

```cmake
cmake_minimum_required(VERSION 3.20)
project(GTestDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)

# Windows上防止gtest覆盖编译器优化设置
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(calculator_test
    tests/calculator_test.cpp
    src/calculator.cpp
)

target_include_directories(calculator_test PRIVATE include)

target_link_libraries(calculator_test PRIVATE GTest::gtest_main)

add_test(NAME calculator_test COMMAND calculator_test)
```

#### 3. 使用示例：FetchContent下载nlohmann/json

```cmake
cmake_minimum_required(VERSION 3.20)
project(JsonDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

# nlohmann/json是header-only库，下载很快
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)

FetchContent_MakeAvailable(nlohmann_json)

add_executable(json_demo main.cpp)

# nlohmann_json提供了导入目标 nlohmann_json::nlohmann_json
target_link_libraries(json_demo PRIVATE nlohmann_json::nlohmann_json)
```

main.cpp：

```cpp
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main() {
    json j = {{"name", "张三"}, {"age", 25}, {"skills", {"C++", "Python"}}};
    std::cout << j.dump(4) << std::endl;
    return 0;
}
```

### 3. 方式3：add_subdirectory() — 本地源码

将第三方库源码放在项目目录中，直接用add_subdirectory包含。

项目结构：

```
myproject/
├── CMakeLists.txt
├── src/
│   └── main.cpp
└── third_party/
    └── fmt/
        ├── CMakeLists.txt
        ├── include/
        │   └── fmt/
        │       ├── core.h
        │       └── format.h
        └── src/
            └── format.cpp
```

```cmake
cmake_minimum_required(VERSION 3.20)
project(ThirdPartyDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# 包含第三方库的CMakeLists.txt
add_subdirectory(third_party/fmt)

add_executable(myapp src/main.cpp)

# 直接使用fmt的导入目标
target_link_libraries(myapp PRIVATE fmt::fmt)
```

> **提示**：可以使用git submodule来管理third_party目录中的第三方库源码。

```bash
# 添加子模块
git submodule add https://github.com/fmtlib/fmt.git third_party/fmt

# 克隆项目时自动拉取子模块
git clone --recurse-submodules <repo-url>
```

### 4. 方式4：手动指定路径

当库没有CMake支持，或者只需要链接预编译的二进制文件时使用。

```cmake
cmake_minimum_required(VERSION 3.20)
project(ManualLibDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# 手动指定头文件路径
include_directories(
    "C:/mylibs/boost/include"
    "C:/mylibs/openssl/include"
)

# 手动指定库文件路径
link_directories(
    "C:/mylibs/boost/lib"
    "C:/mylibs/openssl/lib"
)

add_executable(myapp main.cpp)

# 手动链接库文件
target_link_libraries(myapp PRIVATE
    boost_system
    boost_filesystem
    ssl
    crypto
)
```

更推荐的方式（使用target_*代替全局命令）：

```cmake
cmake_minimum_required(VERSION 3.20)
project(ManualLibDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# 定义库的根路径变量
set(MYLIB_ROOT "C:/mylibs/mylib")

# 创建一个导入库目标
add_library(mylib::mylib STATIC IMPORTED)

# 设置导入库的属性
set_target_properties(mylib::mylib PROPERTIES
    IMPORTED_LOCATION "${MYLIB_ROOT}/lib/mylib.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${MYLIB_ROOT}/include"
)

add_executable(myapp main.cpp)

target_link_libraries(myapp PRIVATE mylib::mylib)
```

### 5. 四种方式对比表格

| 特性 | find_package | FetchContent | add_subdirectory | 手动指定 |
|------|-------------|-------------|-----------------|---------|
| 库来源 | 系统已安装 | 自动下载 | 项目内源码 | 预编译文件 |
| 需要预安装 | 是 | 否 | 否 | 否 |
| 网络要求 | 否 | 首次需要 | 否 | 否 |
| 跨平台 | 好 | 好 | 好 | 差 |
| 配置复杂度 | 低 | 中 | 低 | 高 |
| 版本控制 | 依赖系统 | GIT_TAG控制 | git submodule | 手动管理 |
| 推荐场景 | 系统库 | 小型依赖 | 子模块管理 | 无CMake的库 |

---

## 2. CMake与VS Code的配合

### 1. CMake Tools插件的作用

CMake Tools是VS Code中C++开发的核心插件，提供以下功能：

- 自动检测CMakeLists.txt并配置项目
- 一键配置、构建、调试、运行
- 底部状态栏快速切换构建配置
- 代码补全和语法高亮
- CMake命令的智能提示

安装方式：在VS Code扩展商店搜索 "CMake Tools" 安装。

### 2. 底部状态栏的功能说明

安装CMake Tools后，VS Code底部状态栏会显示以下按钮：

| 状态栏项 | 功能 | 快捷键 |
|---------|------|--------|
| Build | 构建项目 | F7 |
| 🎯 目标 | 选择构建目标 | - |
| Debug | 调试当前目标 | Shift+F5 |
| Run | 运行当前目标 | Shift+F6 |
| 🔧 配置 | 选择构建类型（Debug/Release） | - |
| CMake | 打开CMake命令面板 | - |
| 📁 文件夹 | 选择CMake项目目录 | - |

### 3. CMake: Configure → Build → Debug 完整流程

**第一步：配置（Configure）**

```bash
# 命令面板中执行：CMake: Configure
# 或按 Ctrl+Shift+P → 输入 "CMake: Configure"
# 这会运行：cmake -B build -S .
```

**第二步：构建（Build）**

```bash
# 命令面板中执行：CMake: Build
# 或按 F7
# 这会运行：cmake --build build --config Debug
```

**第三步：调试（Debug）**

```bash
# 命令面板中执行：CMake: Debug
# 或按 Shift+F5
# 这会启动调试器并附加到构建产物
```

VS Code的settings.json中可以配置CMake Tools的行为：

```json
{
    "cmake.buildDirectory": "${workspaceFolder}/build",
    "cmake.configureOnOpen": true,
    "cmake.defaultTarget": "myapp",
    "cmake.buildBeforeRun": true,
    "cmake.debugConfig": {
        "cwd": "${workspaceFolder}",
        "environment": [
            {"name": "PATH", "value": "${env:PATH};C:/mylibs/bin"}
        ]
    },
    "cmake.environment": {
        "MY_VAR": "my_value"
    }
}
```

### 4. CMake Presets（CMakePresets.json）

CMakePresets.json是CMake 3.19+引入的预设配置文件，可以定义多套构建配置，与团队成员和CI共享。

在项目根目录创建 `CMakePresets.json`：

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
                "CMAKE_CXX_STANDARD": "17",
                "CMAKE_CXX_STANDARD_REQUIRED": "ON",
                "CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
            }
        },
        {
            "name": "debug",
            "displayName": "Debug配置",
            "inherits": "base",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug"
            }
        },
        {
            "name": "release",
            "displayName": "Release配置",
            "inherits": "base",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Release"
            }
        },
        {
            "name": "debug-clang",
            "displayName": "Clang Debug配置",
            "inherits": "debug",
            "cacheVariables": {
                "CMAKE_CXX_COMPILER": "clang++"
            }
        },
        {
            "name": "debug-msvc",
            "displayName": "MSVC Debug配置",
            "inherits": "debug",
            "generator": "Visual Studio 17 2022",
            "cacheVariables": {
                "CMAKE_CXX_COMPILER": "cl"
            }
        }
    ],
    "buildPresets": [
        {
            "name": "debug",
            "configurePreset": "debug",
            "configuration": "Debug"
        },
        {
            "name": "release",
            "configurePreset": "release",
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
                "noTestsAction": "error",
                "stopOnFailure": false
            }
        }
    ]
}
```

使用预设：

```bash
# 配置
cmake --preset debug

# 构建
cmake --build --preset debug

# 测试
ctest --preset debug
```

### 5. CMake Variants（.vscode/cmake-variants.json）

CMake Variants是CMake Tools插件提供的旧版配置方式（推荐使用CMakePresets.json替代）：

```json
{
    "buildType": {
        "default": "debug",
        "description": "构建类型",
        "choices": {
            "debug": {
                "short": "Debug",
                "long": "启用调试信息，关闭优化",
                "buildType": "Debug"
            },
            "release": {
                "short": "Release",
                "long": "关闭调试信息，启用优化",
                "buildType": "Release"
            },
            "relwithdebinfo": {
                "short": "RelWithDebInfo",
                "long": "启用优化同时保留调试信息",
                "buildType": "RelWithDebInfo"
            }
        }
    },
    "sanitizer": {
        "default": "none",
        "description": "Sanitizer选项",
        "choices": {
            "none": {
                "short": "无",
                "long": "不启用Sanitizer"
            },
            "address": {
                "short": "ASan",
                "long": "启用Address Sanitizer",
                "settings": {
                    "environment": {
                        "ENABLE_ASAN": "ON"
                    }
                }
            }
        }
    }
}
```

### 6. launch.json中引用CMake构建产物

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "调试CMake项目",
            "type": "cppdbg",
            "request": "launch",
            "program": "${command:cmake.launchTargetPath}",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [
                {
                    "name": "PATH",
                    "value": "${env:PATH};${command:cmake.getLaunchTargetDirectory}"
                }
            ],
            "console": "integratedTerminal",
            "MIMode": "gdb",
            "miDebuggerPath": "/usr/bin/gdb"
        },
        {
            "name": "调试CMake项目（Windows）",
            "type": "cppvsdbg",
            "request": "launch",
            "program": "${command:cmake.launchTargetPath}",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "console": "integratedTerminal"
        }
    ]
}
```

关键变量说明：

| 变量 | 含义 |
|------|------|
| `${command:cmake.launchTargetPath}` | 当前启动目标的完整路径 |
| `${command:cmake.getLaunchTargetDirectory}` | 启动目标所在目录 |
| `${command:cmake.launchTargetFilename}` | 启动目标的文件名 |

---

## 3. CMake与Visual Studio的配合

### 1. 用CMake打开项目

Visual Studio 2017+原生支持CMake项目，无需生成.sln文件。

**方式1：File → Open → CMake**

1. 打开Visual Studio
2. 选择 `文件 → 打开 → CMake`
3. 选择项目根目录的 `CMakeLists.txt`
4. Visual Studio会自动配置项目

**方式2：命令行打开**

```bash
# 使用Visual Studio打开CMake项目
devenv CMakeLists.txt
```

**方式3：从文件夹打开**

1. `文件 → 打开 → 文件夹`
2. 选择包含CMakeLists.txt的目录
3. Visual Studio自动检测并配置

### 2. CMakeSettings.json的写法

在项目根目录创建 `CMakeSettings.json`（Visual Studio专用配置）：

```json
{
    "configurations": [
        {
            "name": "x64-Debug",
            "generator": "Ninja",
            "configurationType": "Debug",
            "inheritEnvironments": [ "msvc_x64_x64" ],
            "buildRoot": "${projectDir}\\out\\build\\${name}",
            "installRoot": "${projectDir}\\out\\install\\${name}",
            "cmakeCommandArgs": "",
            "buildCommandArgs": "",
            "ctestCommandArgs": "",
            "variables": [
                {
                    "name": "CMAKE_CXX_STANDARD",
                    "value": "17",
                    "type": "STRING"
                },
                {
                    "name": "BUILD_TESTS",
                    "value": "ON",
                    "type": "BOOL"
                }
            ]
        },
        {
            "name": "x64-Release",
            "generator": "Ninja",
            "configurationType": "Release",
            "inheritEnvironments": [ "msvc_x64_x64" ],
            "buildRoot": "${projectDir}\\out\\build\\${name}",
            "installRoot": "${projectDir}\\out\\install\\${name}",
            "variables": [
                {
                    "name": "CMAKE_CXX_STANDARD",
                    "value": "17",
                    "type": "STRING"
                }
            ]
        },
        {
            "name": "Linux-Debug",
            "generator": "Unix Makefiles",
            "configurationType": "Debug",
            "inheritEnvironments": [ "linux_x64" ],
            "buildRoot": "${projectDir}\\out\\build\\${name}",
            "installRoot": "${projectDir}\\out\\install\\${name}",
            "cmakeExecutable": "/usr/bin/cmake",
            "remoteMachineName": "user@192.168.1.100",
            "variables": []
        }
    ]
}
```

### 3. 调试配置

在Visual Studio中调试CMake项目：

1. 在解决方案资源管理器中右键点击可执行目标
2. 选择"设为启动项"
3. 按F5启动调试

也可以在 `.vs/launch.vs.json` 中自定义调试配置：

```json
{
    "version": "0.2.1",
    "defaults": {},
    "configurations": [
        {
            "type": "default",
            "name": "调试 myapp",
            "project": "CMakeLists.txt",
            "projectTarget": "myapp.exe",
            "args": ["--verbose", "--config=test.json"],
            "currentDir": "${workspaceRoot}",
            "env": [
                {
                    "name": "MY_ENV_VAR",
                    "value": "debug_value"
                }
            ]
        }
    ]
}
```

---

## 4. CMake与CLion的配合

### 1. CLion自动识别CMakeLists.txt

CLion是JetBrains出品的C/C++ IDE，对CMake有最原生的支持：

1. 打开项目目录（包含CMakeLists.txt的目录）
2. CLion自动检测CMakeLists.txt并运行配置
3. 项目结构、代码索引、跳转、补全自动就绪
4. 修改CMakeLists.txt后自动重新加载

### 2. CMake配置界面说明

在CLion中通过 `File → Settings → Build, Execution, Deployment → CMake` 打开配置：

| 配置项 | 说明 |
|-------|------|
| Build options | 传递给cmake --build的选项，如 -j8 |
| Build directory | 构建目录路径 |
| Build type | Debug / Release / RelWithDebInfo / MinSizeRel |
| CMake options | 传递给cmake命令的额外选项，如 -DCMAKE_PREFIX_PATH=... |
| Toolchain | 使用的工具链（编译器、调试器等） |
| Profile | 配置方案，可创建多个 |

### 3. Profile配置（Debug/Release/RelWithDebInfo）

CLion支持同时配置多个Profile，每个Profile独立构建：

```
File → Settings → Build, Execution, Deployment → CMake → 点击 + 号
```

配置示例：

| Profile名称 | Build type | Build目录 | 用途 |
|-------------|-----------|----------|------|
| Debug | Debug | cmake-build-debug | 日常开发调试 |
| Release | Release | cmake-build-release | 性能测试 |
| RelWithDebInfo | RelWithDebInfo | cmake-build-relwithdebinfo | 线上问题排查 |
| Debug-Clang | Debug | cmake-build-debug-clang | Clang编译器测试 |

每个Profile可以指定不同的CMake选项：

```
-DCMAKE_CXX_COMPILER=clang++
-DCMAKE_PREFIX_PATH=/usr/local/clang-libs
-DBUILD_TESTS=ON
```

### 4. 工具链配置

在 `File → Settings → Build, Execution, Deployment → Toolchains` 中配置：

**添加MinGW工具链：**

1. 点击 + 号 → MinGW
2. 设置MinGW主目录，如 `C:\msys64\mingw64`
3. CLion自动检测编译器（gcc/g++）和调试器（gdb）

**添加MSVC工具链：**

1. 点击 + 号 → Visual Studio
2. 选择已安装的Visual Studio版本
3. 选择架构（amd64 / x86）

**添加远程工具链（Linux开发）：**

1. 点击 + 号 → Remote Host
2. 配置SSH连接信息（主机、端口、用户名、密码/密钥）
3. 指定远程CMake路径和编译器路径
4. 配置文件同步目录

**添加WSL工具链：**

1. 点击 + 号 → WSL
2. 选择WSL发行版
3. CLion自动检测WSL中的编译器和CMake

---

## 5. CMake调试配置

### 1. 在CMakeLists.txt中添加调试信息（-g -O0）

```cmake
# 方式1：设置CMAKE_BUILD_TYPE为Debug（推荐）
# Debug模式默认包含 -g -O0
set(CMAKE_BUILD_TYPE Debug)

# 方式2：使用target_compile_options精确控制
target_compile_options(myapp PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O2 -DNDEBUG>
)

# 方式3：MSVC编译器的调试选项
target_compile_options(myapp PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:
        $<$<CONFIG:Debug>:/Od /Zi /DEBUG>
        $<$<CONFIG:Release>:/O2 /DNDEBUG>
    >
)

# 方式4：全局设置编译标志
set(CMAKE_C_FLAGS_DEBUG "-g -O0" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0" CACHE STRING "" FORCE)
```

### 2. 在CMakeLists.txt中启用Sanitizer

```cmake
cmake_minimum_required(VERSION 3.20)
project(SanitizerDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# 创建Sanitizer选项
option(ENABLE_ASAN "启用Address Sanitizer" OFF)
option(ENABLE_TSAN "启用Thread Sanitizer" OFF)
option(ENABLE_UBSAN "启用Undefined Behavior Sanitizer" OFF)
option(ENABLE_MSAN "启用Memory Sanitizer" OFF)

add_executable(myapp main.cpp)

# Address Sanitizer：检测内存错误（越界、use-after-free等）
if(ENABLE_ASAN)
    target_compile_options(myapp PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(myapp PRIVATE -fsanitize=address)
endif()

# Thread Sanitizer：检测数据竞争
if(ENABLE_TSAN)
    target_compile_options(myapp PRIVATE -fsanitize=thread)
    target_link_options(myapp PRIVATE -fsanitize=thread)
endif()

# Undefined Behavior Sanitizer：检测未定义行为
if(ENABLE_UBSAN)
    target_compile_options(myapp PRIVATE -fsanitize=undefined)
    target_link_options(myapp PRIVATE -fsanitize=undefined)
endif()

# Memory Sanitizer：检测未初始化内存读取（仅Clang支持）
if(ENABLE_MSAN)
    target_compile_options(myapp PRIVATE -fsanitize=memory -fno-omit-frame-pointer)
    target_link_options(myapp PRIVATE -fsanitize=memory)
endif()
```

使用方法：

```bash
# 启用Address Sanitizer
cmake -B build -DENABLE_ASAN=ON
cmake --build build

# 运行程序，如果有内存错误会自动报告
./build/myapp
```

### 3. 在CMakeLists.txt中设置调试工作目录

```cmake
# 设置运行时工作目录（通过环境变量或配置文件）

# 方式1：通过CMake配置生成运行脚本
configure_file(
    run.sh.in
    run.sh
    @ONLY
)

# 方式2：使用CMAKE_RUNTIME_OUTPUT_DIRECTORY控制输出位置
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/run)

# 方式3：在VS Code的settings.json中配置
# "cmake.debugConfig": { "cwd": "${workspaceFolder}/data" }
```

创建 `run.sh.in` 模板：

```bash
#!/bin/bash
cd @CMAKE_SOURCE_DIR@/data
@CMAKE_BINARY_DIR@/bin/myapp "$@"
```

### 4. CMAKE_BUILD_TYPE的作用

CMAKE_BUILD_TYPE控制构建类型，影响编译和链接选项：

| 构建类型 | 优化级别 | 调试信息 | 典型用途 |
|---------|---------|---------|---------|
| Debug | -O0 | -g | 日常开发调试 |
| Release | -O2/-O3 | 无 | 正式发布 |
| RelWithDebInfo | -O2 | -g | 线上调试 |
| MinSizeRel | -Os | 无 | 最小体积发布 |

```cmake
# 设置构建类型
set(CMAKE_BUILD_TYPE Debug CACHE STRING "构建类型" FORCE)

# 提供选项让用户选择
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "构建类型" FORCE)
endif()

# 验证构建类型
set(ALLOWED_BUILD_TYPES Debug Release RelWithDebInfo MinSizeRel)
if(NOT CMAKE_BUILD_TYPE IN_LIST ALLOWED_BUILD_TYPES)
    message(FATAL_ERROR "无效的构建类型: ${CMAKE_BUILD_TYPE}，"
                        "可选值: ${ALLOWED_BUILD_TYPES}")
endif()

message(STATUS "构建类型: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++编译标志: ${CMAKE_CXX_FLAGS}")
message(STATUS "Debug编译标志: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "Release编译标志: ${CMAKE_CXX_FLAGS_RELEASE}")
```

### 5. Generator Expressions在调试配置中的应用

Generator Expressions（生成器表达式）在构建系统生成阶段求值，非常适合条件化配置：

```cmake
# 基本语法：$<条件:值>
# 条件为真时展开为"值"，为假时展开为空

# 根据构建类型设置不同编译选项
target_compile_options(myapp PRIVATE
    $<$<CONFIG:Debug>:-g -O0 -DDEBUG>
    $<$<CONFIG:Release>:-O2 -DNDEBUG>
    $<$<CONFIG:RelWithDebInfo>:-O2 -g -DNDEBUG>
)

# 根据编译器设置不同选项
target_compile_options(myapp PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wno-unused>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /utf-8>
)

# 条件链接库
target_link_libraries(myapp PRIVATE
    $<$<PLATFORM_ID:Linux>:pthread>
    $<$<PLATFORM_ID:Windows>:ws2_32>
    $<$<PLATFORM_ID:Darwin>:objc>
)

# 常用生成器表达式
# $<BOOL:值>              → 值为真时为1，为假时为0
# $<AND:条件1,条件2>      → 逻辑与
# $<OR:条件1,条件2>       → 逻辑或
# $<NOT:条件>             → 逻辑非
# $<STREQUAL:a,b>        → 字符串相等
# $<VERSION_LESS:a,b>    → 版本比较
# $<TARGET_FILE:目标>     → 目标的输出文件路径
# $<TARGET_FILE_NAME:目标> → 目标的输出文件名
# $<JOIN:列表,分隔符>     → 用分隔符连接列表
# $<TARGET_PROPERTY:目标,属性> → 获取目标属性

# 实际应用：根据编译器和构建类型组合条件
target_compile_options(myapp PRIVATE
    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:Debug>>:-g -O0 -fno-omit-frame-pointer>
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/Od /Zi>
)

# 在install中使用生成器表达式
install(TARGETS myapp
    RUNTIME DESTINATION $<IF:$<CONFIG:Debug>,debug-bin,bin>
)
```

---

## 6. CMake常见问题

### 1. CMake Cache问题

CMake会将配置结果缓存到 `CMakeCache.txt` 中。当修改了CMakeLists.txt但配置似乎没有生效时，通常是缓存问题。

**症状：**
- 修改了CMakeLists.txt但编译行为没变
- 修改了编译器路径但仍在用旧编译器
- find_package找不到刚安装的库

**解决方案：**

```bash
# 方案1：删除整个构建目录（最彻底）
rm -rf build/
cmake -B build

# 方案2：只删除缓存文件
rm build/CMakeCache.txt
cmake -B build

# 方案3：使用CMake命令清除缓存
cmake --build build --target clean

# 方案4：强制重新配置
cmake -B build --fresh
```

在CMakeLists.txt中也可以强制刷新特定缓存：

```cmake
# 强制更新缓存变量
set(MY_OPTION ON CACHE BOOL "选项说明" FORCE)

# 取消缓存变量
unset(MY_OPTION CACHE)
```

### 2. 找不到库

**问题：find_package找不到已安装的库**

```cmake
# 方案1：设置CMAKE_PREFIX_PATH（推荐）
set(CMAKE_PREFIX_PATH "C:/libs/boost_1_84_0;C:/libs/openssl")

# 也可以在命令行指定
# cmake -B build -DCMAKE_PREFIX_PATH="C:/libs/boost_1_84_0;C:/libs/openssl"

# 方案2：设置库特定的根路径变量
set(BOOST_ROOT "C:/boost_1_84_0")
set(OpenSSL_DIR "C:/OpenSSL-Win64")
set(fmt_DIR "C:/fmt/lib/cmake/fmt")

# 方案3：添加自定义搜索路径
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
# 然后在项目中的cmake/目录下放置自定义的FindXXX.cmake

# 方案4：指定Config文件路径
find_package(Boost REQUIRED
    PATHS "C:/boost_1_84_0/lib/cmake/Boost"
    NO_DEFAULT_PATH
)
```

**自定义Find模块示例**（`cmake/FindMyLib.cmake`）：

```cmake
# 查找MyLib头文件
find_path(MYLIB_INCLUDE_DIR
    NAMES mylib/mylib.h
    PATHS /usr/local/include /opt/mylib/include
          "C:/mylib/include"
)

# 查找MyLib库文件
find_library(MYLIB_LIBRARY
    NAMES mylib mylib_static
    PATHS /usr/local/lib /opt/mylib/lib
          "C:/mylib/lib"
)

# 处理标准参数
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MyLib
    REQUIRED_VARS MYLIB_LIBRARY MYLIB_INCLUDE_DIR
)

# 创建导入目标
if(MyLib_FOUND AND NOT TARGET MyLib::MyLib)
    add_library(MyLib::MyLib UNKNOWN IMPORTED)
    set_target_properties(MyLib::MyLib PROPERTIES
        IMPORTED_LOCATION "${MYLIB_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${MYLIB_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(MYLIB_INCLUDE_DIR MYLIB_LIBRARY)
```

### 3. 交叉编译配置

交叉编译是在一个平台上生成另一个平台可执行代码的过程。

**示例：在x86_64主机上编译ARM64程序**

创建工具链文件 `toolchain-arm64.cmake`：

```cmake
# 目标系统
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# 交叉编译器
set(CMAKE_C_COMPILER /usr/bin/aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER /usr/bin/aarch64-linux-gnu-g++)

# 目标系统的根路径
set(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu)

# 搜索规则调整
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
```

使用工具链文件：

```bash
cmake -B build-arm64 \
    -DCMAKE_TOOLCHAIN_FILE=toolchain-arm64.cmake \
    -DCMAKE_BUILD_TYPE=Release

cmake --build build-arm64
```

**Windows交叉编译到Linux（使用WSL）：**

```cmake
# toolchain-wsl.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_C_COMPILER /usr/bin/gcc)
set(CMAKE_CXX_COMPILER /usr/bin/g++)
```

### 4. CMake版本兼容性

**问题：本地CMake版本与CI/团队成员版本不一致**

```cmake
# 方案1：在cmake_minimum_required中指定版本范围
cmake_minimum_required(VERSION 3.16...3.27)

# 方案2：使用cmake_policy控制行为
cmake_minimum_required(VERSION 3.20)

# 设置策略：使用新行为
cmake_policy(SET CMP0091 NEW)  # MSVC运行时库选择
cmake_policy(SET CMP0135 NEW)  # FetchContent下载时间戳

# 方案3：检查CMake版本并给出提示
if(CMAKE_VERSION VERSION_LESS 3.20)
    message(WARNING "建议使用CMake 3.20+，当前版本: ${CMAKE_VERSION}")
endif()

# 方案4：使用CMakePresets.json指定最低版本
# 在CMakePresets.json中设置cmakeMinimumRequired
```

**常用功能与最低版本对照：**

| 功能 | 最低CMake版本 |
|------|-------------|
| FetchContent | 3.11 |
| target_link_options | 3.13 |
| FetchContent_MakeAvailable | 3.14 |
| file(GLOB CONFIGURE_DEPENDS) | 3.12 |
| CMakePresets.json | 3.19 |
| cmake --fresh | 3.24 |
| CMAKE_EXPORT_COMPILE_COMMANDS | 2.8.12 |

**检查项目所需CMake版本的技巧：**

```cmake
# 打印当前CMake版本
message(STATUS "CMake版本: ${CMAKE_VERSION}")

# 检查特定功能是否可用
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.19)
    message(STATUS "支持CMakePresets.json")
else()
    message(STATUS "不支持CMakePresets.json，请升级CMake")
endif()
```

### 5. 相关章节

- [CI-CD与DevOps实践](../03-问题解答/10-工程实践/23-CI-CD与DevOps实践.md) — CI/CD流水线、自动化构建与部署
