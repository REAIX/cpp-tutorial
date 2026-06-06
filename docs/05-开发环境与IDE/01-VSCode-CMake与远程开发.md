# VS Code CMake 集成与远程开发

> **前置阅读**：如果你还没有配置VS Code开发环境，请先阅读 [00-VSCode核心配置](./00-VSCode核心配置.md) 完成基础配置。本文档假设你已经掌握了VS Code的基本配置，需要学习CMake集成和远程开发。

> **相关教程**：CMake基础入门见 [CMake基础入门](./03-CMake基础入门.md)，VSCode调试配置见 [VSCode调试与优化](./02-VSCode调试与优化.md)。

## 1. 不使用CMake的完整工作流

### 1. 项目结构

```
my_project/
├── .vscode/
│   ├── c_cpp_properties.json
│   ├── tasks.json
│   ├── launch.json
│   └── settings.json
├── include/
│   ├── utils.h
│   └── logger.h
├── src/
│   ├── main.cpp
│   ├── utils.cpp
│   └── logger.cpp
└── build/
    └── (编译产物)
```

### 2. 配置文件写法

#### 1. c_cpp_properties.json

```json
{
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/include",
                "${workspaceFolder}/src"
            ],
            "defines": ["_DEBUG"],
            "compilerPath": "/usr/bin/g++",
            "cStandard": "c17",
            "cppStandard": "c++17",
            "intelliSenseMode": "linux-gcc-x64",
            "browse": {
                "path": [
                    "${workspaceFolder}/include",
                    "${workspaceFolder}/src"
                ],
                "limitSymbolsToIncludedHeaders": true
            }
        }
    ],
    "version": 4
}
```

#### 2. tasks.json

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "build",
            "type": "shell",
            "command": "g++",
            "args": [
                "-g",
                "-std=c++17",
                "-Wall",
                "-Wextra",
                "-I${workspaceFolder}/include",
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
            "problemMatcher": ["$gcc"],
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false,
                "panel": "shared",
                "clear": true
            }
        },
        {
            "label": "clean",
            "type": "shell",
            "command": "rm",
            "args": [
                "-f",
                "${workspaceFolder}/build/myapp"
            ],
            "problemMatcher": []
        }
    ]
}
```

#### 3. launch.json

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug myapp",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/myapp",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "miDebuggerPath": "/usr/bin/gdb",
            "setupCommands": [
                {
                    "description": "为 gdb 启用整齐打印",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ],
            "preLaunchTask": "build"
        }
    ]
}
```

### 3. 多文件编译

当项目源文件较多时，可以分两步编译：先编译为目标文件，再链接：

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "compile objects",
            "type": "shell",
            "command": "g++",
            "args": [
                "-g", "-std=c++17", "-Wall",
                "-I${workspaceFolder}/include",
                "-c",
                "${workspaceFolder}/src/utils.cpp",
                "-o", "${workspaceFolder}/build/utils.o"
            ],
            "problemMatcher": ["$gcc"]
        },
        {
            "label": "compile main",
            "type": "shell",
            "command": "g++",
            "args": [
                "-g", "-std=c++17", "-Wall",
                "-I${workspaceFolder}/include",
                "-c",
                "${workspaceFolder}/src/main.cpp",
                "-o", "${workspaceFolder}/build/main.o"
            ],
            "problemMatcher": ["$gcc"]
        },
        {
            "label": "link",
            "type": "shell",
            "command": "g++",
            "args": [
                "${workspaceFolder}/build/main.o",
                "${workspaceFolder}/build/utils.o",
                "-o",
                "${workspaceFolder}/build/myapp"
            ],
            "problemMatcher": ["$gcc"],
            "dependsOn": ["compile objects", "compile main"]
        },
        {
            "label": "build",
            "dependsOn": ["link"],
            "dependsOrder": "sequence",
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": []
        }
    ]
}
```

### 4. 外部库的使用

在 `tasks.json` 中通过 `-I`、`-L`、`-l` 参数配置外部库：

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "build with libs",
            "type": "shell",
            "command": "g++",
            "args": [
                "-g", "-std=c++17", "-Wall",
                "-I${workspaceFolder}/include",
                "-I/usr/local/include",
                "${workspaceFolder}/src/main.cpp",
                "${workspaceFolder}/src/utils.cpp",
                "-L/usr/local/lib",
                "-lpthread",
                "-lcurl",
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

参数说明：
- `-I/usr/local/include`：添加头文件搜索路径
- `-L/usr/local/lib`：添加库文件搜索路径
- `-lpthread`：链接 pthread 库（等价于链接 `libpthread.so`）
- `-lcurl`：链接 curl 库（等价于链接 `libcurl.so`）

### 5. 调试配置

调试时如果程序依赖动态库，需要设置 `LD_LIBRARY_PATH`：

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug with libs",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/myapp",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [
                {
                    "name": "LD_LIBRARY_PATH",
                    "value": "/usr/local/lib:${env:LD_LIBRARY_PATH}"
                }
            ],
            "externalConsole": false,
            "MIMode": "gdb",
            "miDebuggerPath": "/usr/bin/gdb",
            "setupCommands": [
                {
                    "description": "为 gdb 启用整齐打印",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ],
            "preLaunchTask": "build with libs"
        }
    ]
}
```

---

## 2. 使用CMake的完整工作流

### 1. 项目结构

```
my_cmake_project/
├── .vscode/
│   ├── c_cpp_properties.json
│   ├── settings.json
│   └── launch.json
├── CMakeLists.txt
├── include/
│   ├── utils.h
│   └── logger.h
├── src/
│   ├── main.cpp
│   ├── utils.cpp
│   └── logger.cpp
└── build/
    └── (CMake构建产物)
```

### 2. CMakeLists.txt 写法

```cmake
cmake_minimum_required(VERSION 3.20)
project(myapp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")

add_executable(myapp ${SOURCES})

target_include_directories(myapp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(myapp PRIVATE
    -Wall -Wextra -Wpedantic
)

target_compile_definitions(myapp PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
)
```

### 3. VS Code 配置文件写法

#### 1. c_cpp_properties.json（与 CMake Tools 配合）

```json
{
    "configurations": [
        {
            "name": "Linux",
            "configurationProvider": "ms-vscode.cmake-tools",
            "cStandard": "c17",
            "cppStandard": "c++17",
            "intelliSenseMode": "linux-gcc-x64"
        }
    ],
    "version": 4
}
```

设置 `configurationProvider` 后，CMake Tools 会自动管理 `includePath`、`defines` 和 `compilerPath`，无需手动配置。

#### 2. settings.json

```json
{
    "cmake.buildDirectory": "${workspaceFolder}/build",
    "cmake.generator": "Unix Makefiles",
    "cmake.configureArgs": [
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
    ],
    "cmake.configureOnOpen": true,
    "cmake.debugConfig": {
        "args": [],
        "environment": []
    },
    "C_Cpp.default.cppStandard": "c++17",
    "files.associations": {
        "*.h": "cpp",
        "CMakeLists.txt": "cmake",
        "*.cmake": "cmake"
    }
}
```

#### 3. launch.json

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "CMake Debug",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/myapp",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "miDebuggerPath": "/usr/bin/gdb",
            "setupCommands": [
                {
                    "description": "为 gdb 启用整齐打印",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ]
        }
    ]
}
```

**注意**：使用 CMake Tools 时，可以直接点击状态栏的调试按钮，CMake Tools 会自动处理构建和调试，无需手动配置 `preLaunchTask`。

### 4. CMake 配置、构建、调试的完整流程

#### 1. 方式一：使用 CMake Tools 状态栏（推荐）

1. **配置**：点击状态栏的 `Build` 旁边的 Kit 选择器，选择编译器工具链
2. **配置**：点击状态栏的 `Configure` 执行 CMake 配置
3. **构建**：点击状态栏的 `Build` 执行编译
4. **调试**：点击状态栏的 `Debug` 启动调试

#### 2. 方式二：使用命令面板

1. `Ctrl+Shift+P` → `CMake: Configure` — 执行 CMake 配置
2. `Ctrl+Shift+P` → `CMake: Build` — 执行构建
3. `Ctrl+Shift+P` → `CMake: Debug` — 启动调试

#### 3. 方式三：使用快捷键

| 快捷键 | 功能 |
|--------|------|
| `F7` | CMake 构建 |
| `Shift+F7` | CMake 选择构建目标 |
| `Ctrl+F5` | CMake 运行（不调试） |
| `Shift+F5` | CMake 调试 |

### 5. 与不使用 CMake 的对比

| 对比项 | 不使用 CMake | 使用 CMake |
|--------|-------------|-----------|
| 构建配置 | 手动编写 `tasks.json` | CMakeLists.txt + CMake Tools 自动管理 |
| 智能提示 | 手动配置 `c_cpp_properties.json` | CMake Tools 自动提供 |
| 多文件管理 | 手动列出所有源文件 | `file(GLOB_RECURSE ...)` 自动收集 |
| 外部库 | 手动写 `-I -L -l` 参数 | `find_package` / `target_link_libraries` |
| 跨平台 | 需要为每个平台写不同配置 | CMake 自动适配平台 |
| 构建类型 | 手动管理 Debug/Release | `-DCMAKE_BUILD_TYPE=Debug` |
| 可移植性 | 低，依赖特定环境 | 高，CMake 生成对应平台的构建文件 |
| 学习成本 | 低 | 中等 |
| 适用场景 | 小型项目、单文件 | 中大型项目、跨平台项目 |

---

## 3. 外部库配置详解

### 1. 不使用 CMake 时配置外部库

在 `tasks.json` 中通过编译参数配置：

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "build with external libs",
            "type": "shell",
            "command": "g++",
            "args": [
                "-g",
                "-std=c++17",
                "-Wall",
                "-I${workspaceFolder}/include",
                "-I/usr/local/include",
                "-I/opt/homebrew/include",
                "${workspaceFolder}/src/main.cpp",
                "-L/usr/local/lib",
                "-L/opt/homebrew/lib",
                "-lpthread",
                "-lcurl",
                "-lssl",
                "-lcrypto",
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

参数详解：
- `-I<path>`：告诉编译器在 `<path>` 中搜索头文件，等价于 `#include` 的搜索路径
- `-L<path>`：告诉链接器在 `<path>` 中搜索库文件（`.a`、`.so`、`.lib`、`.dll`）
- `-l<name>`：链接名为 `lib<name>.so` 或 `lib<name>.a` 的库，不需要写 `lib` 前缀和扩展名

### 2. 使用 CMake 时配置外部库

#### 1. 方式一：target_link_libraries（直接链接）

```cmake
# 链接系统已安装的库
target_link_libraries(myapp PRIVATE
    pthread
    curl
)
```

#### 2. 方式二：find_package（查找已安装的库）

```cmake
# 查找 OpenSSL 库
find_package(OpenSSL REQUIRED)

if(OpenSSL_FOUND)
    message(STATUS "OpenSSL 版本: ${OPENSSL_VERSION}")
    target_include_directories(myapp PRIVATE ${OPENSSL_INCLUDE_DIR})
    target_link_libraries(myapp PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()
```

`find_package` 的工作原理：
1. CMake 在 `CMAKE_MODULE_PATH` 和自带的 Module 目录中搜索 `Find<Package>.cmake`
2. 执行该模块，设置 `<Package>_FOUND`、`<Package>_INCLUDE_DIRS`、`<Package>_LIBRARIES` 等变量
3. 现代库通常提供 `<Package>Config.cmake`，支持导入目标（如 `OpenSSL::SSL`）

#### 3. 方式三：FetchContent（自动下载和集成）

```cmake
include(FetchContent)

# 下载并集成 nlohmann/json（header-only 库）
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

FetchContent_MakeAvailable(json)

target_link_libraries(myapp PRIVATE nlohmann_json::nlohmann_json)
```

```cmake
# 下载并集成 fmt 库
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 10.2.1
)

FetchContent_MakeAvailable(fmt)

target_link_libraries(myapp PRIVATE fmt::fmt)
```

### 3. 在 c_cpp_properties.json 中配置头文件路径

当使用外部库时，智能提示需要知道头文件的位置：

```json
{
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/include",
                "/usr/local/include",
                "/usr/include/SDL2",
                "/usr/include/boost",
                "/opt/homebrew/include"
            ],
            "defines": ["_DEBUG"],
            "compilerPath": "/usr/bin/g++",
            "cStandard": "c17",
            "cppStandard": "c++17",
            "intelliSenseMode": "linux-gcc-x64"
        }
    ],
    "version": 4
}
```

### 4. 常见外部库配置示例

#### 1. SDL2

**不使用 CMake**：

```json
// tasks.json 中的 args
"args": [
    "-g", "-std=c++17",
    "-I/usr/include/SDL2",
    "${workspaceFolder}/src/main.cpp",
    "-L/usr/lib/x86_64-linux-gnu",
    "-lSDL2", "-lSDL2_image", "-lSDL2_ttf",
    "-o", "${workspaceFolder}/build/game"
]
```

**使用 CMake**：

```cmake
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)

target_link_libraries(game PRIVATE
    SDL2::SDL2
    SDL2::SDL2main
    SDL2_image::SDL2_image
    SDL2_ttf::SDL2_ttf
)
```

#### 2. OpenSSL

**不使用 CMake**：

```json
"args": [
    "-g", "-std=c++17",
    "-I/usr/include/openssl",
    "${workspaceFolder}/src/main.cpp",
    "-lssl", "-lcrypto",
    "-o", "${workspaceFolder}/build/myapp"
]
```

**使用 CMake**：

```cmake
find_package(OpenSSL REQUIRED)
target_link_libraries(myapp PRIVATE OpenSSL::SSL OpenSSL::Crypto)
```

#### 3. Boost

**不使用 CMake**：

```json
"args": [
    "-g", "-std=c++17",
    "-I/usr/include/boost",
    "-I/usr/local/include",
    "${workspaceFolder}/src/main.cpp",
    "-L/usr/local/lib",
    "-lboost_system", "-lboost_filesystem", "-lboost_thread",
    "-lpthread",
    "-o", "${workspaceFolder}/build/myapp"
]
```

**使用 CMake**：

```cmake
find_package(Boost REQUIRED COMPONENTS system filesystem thread)
target_link_libraries(myapp PRIVATE
    Boost::system
    Boost::filesystem
    Boost::thread
)
```

#### 4. nlohmann/json

**不使用 CMake**（header-only 库，只需头文件路径）：

```json
"args": [
    "-g", "-std=c++17",
    "-I/usr/include/nlohmann",
    "-I${workspaceFolder}/third_party/json/include",
    "${workspaceFolder}/src/main.cpp",
    "-o", "${workspaceFolder}/build/myapp"
]
```

**使用 CMake**（FetchContent 方式）：

```cmake
include(FetchContent)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)
target_link_libraries(myapp PRIVATE nlohmann_json::nlohmann_json)
```

### 5. vcpkg 集成

vcpkg 是微软开源的 C++ 包管理器，可以与 CMake 无缝集成。

#### 1. 安装 vcpkg

```bash
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh        # Linux/macOS
bootstrap-vcpkg.bat          # Windows
```

#### 2. 安装库

```bash
./vcpkg install fmt
./vcpkg install nlohmann-json
./vcpkg install openssl
```

#### 3. CMake 集成方式一：通过 CMAKE_TOOLCHAIN_FILE

```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

或在 `settings.json` 中配置：

```json
{
    "cmake.configureArgs": [
        "-DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"
    ]
}
```

#### 4. CMake 集成方式二：在 CMakeLists.txt 中配置

```cmake
set(CMAKE_TOOLCHAIN_FILE "/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")

find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

target_link_libraries(myapp PRIVATE fmt::fmt nlohmann_json::nlohmann_json)
```

#### 5. CMake 集成方式三：使用 vcpkg.json 清单文件

在项目根目录创建 `vcpkg.json`：

```json
{
    "name": "myapp",
    "version": "1.0.0",
    "dependencies": [
        "fmt",
        "nlohmann-json",
        "openssl"
    ]
}
```

当使用 vcpkg 工具链时，CMake 会自动读取 `vcpkg.json` 并安装依赖。

---
