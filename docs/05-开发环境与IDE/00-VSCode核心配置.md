# VS Code C/C++ 开发环境完全配置指南

> **前置阅读**：如果你还没有安装VS Code和编译器，请先阅读 [FAQ-138：开发环境配置详解](../03-问题解答/01-基础概念/28-开发环境配置.md)，完成"从零搭建环境"的步骤。本文档假设你已经安装好了VS Code和GCC/Clang编译器。

## 1. VS Code 配置文件体系总览

### 1. .vscode 目录下的配置文件

VS Code 的 C/C++ 开发环境核心配置都存放在项目根目录的 `.vscode` 文件夹中，每个文件负责不同的功能：

| 文件名 | 作用 | 是否必需 |
|--------|------|----------|
| `c_cpp_properties.json` | C/C++ 智能提示配置，定义头文件搜索路径、编译器路径、语言标准等 | 推荐 |
| `tasks.json` | 构建任务配置，定义如何编译项目（调用 g++/clang++/cl.exe/CMake 等） | 推荐 |
| `launch.json` | 调试配置，定义如何启动调试器（GDB/LLDB/MSVC调试器） | 推荐 |
| `settings.json` | 工作区级别的设置，覆盖全局设置，针对当前项目生效 | 可选 |
| `extensions.json` | 推荐安装的扩展列表，打开项目时 VS Code 会提示安装 | 可选 |

### 2. 配置文件之间的关联关系

```
┌─────────────────────────────────────────────────────────┐
│                    VS Code 配置体系                       │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  settings.json                                          │
│  ┌──────────────────┐                                   │
│  │ 工作区基础设置     │──→ 影响 c_cpp_properties.json    │
│  │ (编译器路径等)     │    的默认值                      │
│  └──────────────────┘                                   │
│           │                                             │
│           ▼                                             │
│  c_cpp_properties.json                                  │
│  ┌──────────────────┐                                   │
│  │ 智能提示配置       │──→ 决定代码补全、                  │
│  │ (头文件路径、      │    语法检查的行为                  │
│  │  编译器路径)       │                                   │
│  └──────────────────┘                                   │
│                                                         │
│  tasks.json                                             │
│  ┌──────────────────┐                                   │
│  │ 构建任务配置       │──→ 生成可执行文件                  │
│  │ (编译命令和参数)   │                                   │
│  └──────────────────┘                                   │
│           │                                             │
│           │ preLaunchTask 引用                           │
│           ▼                                             │
│  launch.json                                            │
│  ┌──────────────────┐                                   │
│  │ 调试配置          │──→ 启动调试器，                    │
│  │ (调试器路径、      │    加载 tasks.json                │
│  │  程序路径)         │    构建的产物                     │
│  └──────────────────┘                                   │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

关键关联：
- `launch.json` 通过 `preLaunchTask` 字段引用 `tasks.json` 中的任务标签，实现"先编译后调试"
- `settings.json` 中的 `C_Cpp.default.*` 设置会被 `c_cpp_properties.json` 继承作为默认值
- `c_cpp_properties.json` 的 `compilerPath` 会影响智能提示引擎的语法解析行为

### 3. 全局配置 vs 工作区配置 vs 项目配置

| 层级 | 位置 | 作用范围 | 优先级 |
|------|------|----------|--------|
| 全局配置 | `%APPDATA%\Code\User\settings.json`（Windows）<br>`~/.config/Code/User/settings.json`（Linux） | 所有项目 | 最低 |
| 工作区配置 | `.vscode/settings.json` | 当前项目 | 中 |
| 文件夹配置 | 多根工作区中每个文件夹的 `.vscode/settings.json` | 对应文件夹 | 最高 |

优先级规则：项目配置 > 工作区配置 > 全局配置，高优先级的设置会覆盖低优先级的同名设置。

---

## 2. c_cpp_properties.json 详解（智能提示配置）

### 1. 文件结构总览

```json
{
    "configurations": [
        {
            "name": "配置名称",
            "includePath": ["头文件搜索路径"],
            "defines": ["预处理器宏定义"],
            "compilerPath": "编译器路径",
            "cStandard": "C语言标准",
            "cppStandard": "C++语言标准",
            "intelliSenseMode": "智能提示引擎模式",
            "configurationProvider": "配置提供者",
            "browse": {
                "path": ["浏览路径"],
                "limitSymbolsToIncludedHeaders": true,
                "databaseFilename": "符号数据库文件名"
            },
            "forcedInclude": ["强制包含的头文件"],
            "compilerArgs": ["编译器额外参数"],
            "macFrameworkPath": ["macOS框架路径"]
        }
    ],
    "version": 4
}
```

### 2. 每个字段详细说明

#### 1. name
- **作用**：配置的显示名称，在 VS Code 底部状态栏的配置选择器中展示
- **可选值**：自定义字符串，通常使用平台名如 `Linux`、`Win32`、`macOS`
- **示例**：`"name": "Linux"`

#### 2. includePath
- **作用**：指定智能提示引擎搜索头文件的路径列表
- **格式**：支持 glob 模式，`**` 表示递归搜索子目录
- **默认值**：如果设置了 `compilerPath`，扩展会自动查询编译器的默认 include 路径
- **示例**：`"includePath": ["${workspaceFolder}/**", "/usr/include/**"]`

#### 3. defines
- **作用**：预处理器宏定义，等价于编译时的 `-D` 参数
- **示例**：`"defines": ["_DEBUG", "UNICODE", "_UNICODE"]`

#### 4. compilerPath
- **作用**：指定编译器的完整路径，扩展会据此推断系统 include 路径和默认宏定义
- **可选值**：
  - Linux：`/usr/bin/gcc`、`/usr/bin/g++`、`/usr/bin/clang`
  - Windows：`C:/MinGW/bin/g++.exe`、`C:/msys64/mingw64/bin/g++.exe`
  - macOS：`/usr/bin/clang`、`/usr/bin/clang++`
- **注意**：如果不设置，扩展会尝试在系统 PATH 中查找编译器

#### 5. cStandard
- **作用**：指定 C 语言标准版本
- **可选值**：`"c89"`、`"c99"`、`"c11"`、`"c17"`、`"c23"`、`"${default}"`
- **示例**：`"cStandard": "c17"`

#### 6. cppStandard
- **作用**：指定 C++ 语言标准版本
- **可选值**：`"c++98"`、`"c++03"`、`"c++11"`、`"c++14"`、`"c++17"`、`"c++20"`、`"c++23"`、`"${default}"`
- **示例**：`"cppStandard": "c++17"`

#### 7. intelliSenseMode
- **作用**：指定智能提示引擎的模式，决定使用哪种前端解析器
- **可选值**：

| 值 | 说明 |
|----|------|
| `gcc-x64` | 使用 GCC 的语义，64位 |
| `gcc-x86` | 使用 GCC 的语义，32位 |
| `clang-x64` | 使用 Clang 的语义，64位 |
| `clang-x86` | 使用 Clang 的语义，32位 |
| `msvc-x64` | 使用 MSVC 的语义，64位 |
| `msvc-x86` | 使用 MSVC 的语义，32位 |
| `linux-gcc-x64` | Linux 平台 GCC |
| `linux-clang-x64` | Linux 平台 Clang |
| `macos-clang-x64` | macOS 平台 Clang |
| `macos-gcc-x64` | macOS 平台 GCC |
| `windows-gcc-x64` | Windows 平台 GCC/MinGW |
| `windows-msvc-x64` | Windows 平台 MSVC |

- **推荐**：与 `compilerPath` 保持一致，使用 GCC 编译器就选 `gcc-x64`，使用 Clang 就选 `clang-x64`

#### 8. configurationProvider
- **作用**：指定另一个扩展来提供配置信息（如 include 路径）
- **常用值**：`"ms-vscode.cmake-tools"` — 让 CMake Tools 扩展提供配置
- **效果**：设置后，CMake Tools 会自动管理 includePath 和 defines，无需手动配置

#### 9. browse
- **作用**：配置符号浏览器的行为
- **子字段**：
  - `path`：符号浏览器搜索路径，用于"转到定义"等功能
  - `limitSymbolsToIncludedHeaders`：是否仅索引被包含的头文件，`true` 可提升性能
  - `databaseFilename`：符号数据库的存储文件名，默认为空（存储在扩展目录）

#### 10. forcedInclude
- **作用**：强制在所有源文件之前包含指定的头文件
- **示例**：`"forcedInclude": ["${workspaceFolder}/include/pch.h"]`

#### 11. compilerArgs
- **作用**：传递给编译器的额外参数，影响智能提示的解析行为
- **示例**：`"compilerArgs": ["-Wall", "-Wextra"]`

#### 12. macFrameworkPath
- **作用**：macOS 专有，指定框架搜索路径
- **示例**：`"macFrameworkPath": ["/System/Library/Frameworks", "/Library/Frameworks"]`

### 3. 完整示例

#### 1. Linux 示例

```json
{
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/**",
                "/usr/include/**",
                "/usr/local/include/**",
                "${workspaceFolder}/include"
            ],
            "defines": [
                "_DEBUG",
                "LINUX"
            ],
            "compilerPath": "/usr/bin/g++",
            "cStandard": "c17",
            "cppStandard": "c++17",
            "intelliSenseMode": "linux-gcc-x64",
            "browse": {
                "path": [
                    "${workspaceFolder}",
                    "/usr/include",
                    "/usr/local/include"
                ],
                "limitSymbolsToIncludedHeaders": true
            }
        }
    ],
    "version": 4
}
```

#### 2. Windows 示例（MinGW-w64）

```json
{
    "configurations": [
        {
            "name": "Win32",
            "includePath": [
                "${workspaceFolder}/**",
                "C:/msys64/mingw64/include/**",
                "${workspaceFolder}/include"
            ],
            "defines": [
                "_DEBUG",
                "UNICODE",
                "_UNICODE",
                "WIN32"
            ],
            "compilerPath": "C:/msys64/mingw64/bin/g++.exe",
            "cStandard": "c17",
            "cppStandard": "c++17",
            "intelliSenseMode": "windows-gcc-x64",
            "browse": {
                "path": [
                    "${workspaceFolder}",
                    "C:/msys64/mingw64/include"
                ],
                "limitSymbolsToIncludedHeaders": true
            }
        }
    ],
    "version": 4
}
```

#### 3. macOS 示例

```json
{
    "configurations": [
        {
            "name": "macOS",
            "includePath": [
                "${workspaceFolder}/**",
                "/usr/local/include/**",
                "/opt/homebrew/include/**",
                "${workspaceFolder}/include"
            ],
            "defines": [
                "_DEBUG",
                "APPLE"
            ],
            "compilerPath": "/usr/bin/clang++",
            "cStandard": "c17",
            "cppStandard": "c++17",
            "intelliSenseMode": "macos-clang-x64",
            "macFrameworkPath": [
                "/System/Library/Frameworks",
                "/Library/Frameworks"
            ],
            "browse": {
                "path": [
                    "${workspaceFolder}",
                    "/usr/local/include",
                    "/opt/homebrew/include"
                ],
                "limitSymbolsToIncludedHeaders": true
            }
        }
    ],
    "version": 4
}
```

### 4. 常见问题

#### 1. 头文件红色波浪线

**原因分析**：
1. `includePath` 未包含该头文件所在目录
2. `compilerPath` 设置错误，导致自动推断的 include 路径不正确
3. `intelliSenseMode` 与实际编译器不匹配

**解决方法**：
1. 在 `includePath` 中添加头文件目录
2. 确认 `compilerPath` 指向正确的编译器
3. 通过命令面板执行 `C/C++: Reset IntelliSense Database` 重置智能提示数据库
4. 查看输出面板的 `C/C++` 通道，确认扩展检测到的 include 路径

#### 2. 跳转定义失败

**原因分析**：
1. `browse.path` 未包含源文件目录
2. 符号数据库未构建完成
3. 头文件路径未正确配置

**解决方法**：
1. 在 `browse.path` 中添加源文件目录
2. 等待索引完成（状态栏显示扫描进度）
3. 执行 `C/C++: Reset IntelliSense Database` 重建索引

---

## 3. tasks.json 详解（构建任务配置）

### 1. 文件结构总览

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "任务标签",
            "type": "任务类型",
            "command": "执行的命令",
            "args": ["命令参数"],
            "group": "任务分组",
            "problemMatcher": "问题匹配器",
            "presentation": "输出展示配置",
            "options": "运行选项"
        }
    ]
}
```

### 2. 每个字段详细说明

#### 1. label
- **作用**：任务的唯一标识名称，在 `launch.json` 中通过 `preLaunchTask` 引用
- **示例**：`"label": "build"`、`"label": "C/C++: g++ build active file"`

#### 2. type
- **作用**：任务类型
- **可选值**：
  - `"shell"`：在终端中执行命令（推荐）
  - `"process"`：作为进程直接执行，不经过 shell

#### 3. command
- **作用**：要执行的命令
- **示例**：`"command": "g++"`、`"command": "cmake"`、`"command": "make"`

#### 4. args
- **作用**：传递给 command 的参数列表
- **示例**：`"args": ["-g", "-std=c++17", "${file}", "-o", "${fileDirname}/${fileBasenameNoExtension}"]`

#### 5. group
- **作用**：将任务归组，方便快捷键调用
- **可选值**：
  - `"build"`：构建组，可通过 `Ctrl+Shift+B` 快速执行
  - `"test"`：测试组
  - `{"kind": "build", "isDefault": true}`：设为默认构建任务
- **示例**：`"group": {"kind": "build", "isDefault": true}`

#### 6. problemMatcher
- **作用**：解析编译器输出中的错误和警告，在"问题"面板中展示
- **常用值**：
  - `"$gcc"`：匹配 GCC/Clang 的输出格式
  - `"$msCompile"`：匹配 MSVC 的输出格式
  - `[]`：不使用问题匹配器
- **示例**：`"problemMatcher": ["$gcc"]`

#### 7. presentation
- **作用**：控制任务输出在终端中的展示方式
- **子字段**：

| 字段 | 可选值 | 说明 |
|------|--------|------|
| `echo` | `true`/`false` | 是否在终端中回显命令 |
| `reveal` | `"always"`/`"never"`/`"silent"` | 何时显示终端面板 |
| `focus` | `true`/`false` | 是否聚焦到终端面板 |
| `panel` | `"shared"`/`"dedicated"`/`"new"` | 终端面板共享方式 |
| `showReuseMessage` | `true`/`false` | 是否显示"终端将被重用"提示 |
| `clear` | `true`/`false` | 执行前是否清空终端 |

- **示例**：
```json
"presentation": {
    "echo": true,
    "reveal": "always",
    "focus": false,
    "panel": "shared",
    "showReuseMessage": true,
    "clear": true
}
```

#### 8. options
- **作用**：任务运行的环境选项
- **子字段**：
  - `cwd`：工作目录
  - `env`：环境变量
  - `shell`：指定 shell 类型
- **示例**：
```json
"options": {
    "cwd": "${workspaceFolder}/build"
}
```

### 3. args 中常用的编译参数

| 参数 | 说明 | 示例 |
|------|------|------|
| `-g` | 生成调试信息 | `"-g"` |
| `-std=c++17` | 指定 C++ 标准 | `"-std=c++17"` |
| `-Wall` | 开启常见警告 | `"-Wall"` |
| `-Wextra` | 开启额外警告 | `"-Wextra"` |
| `-O0` | 不优化（调试用） | `"-O0"` |
| `-O2` | 二级优化（发布用） | `"-O2"` |
| `-I<dir>` | 添加头文件搜索路径 | `"-I${workspaceFolder}/include"` |
| `-L<dir>` | 添加库文件搜索路径 | `"-L/usr/local/lib"` |
| `-l<lib>` | 链接库文件 | `"-lpthread"` |
| `-D<macro>` | 定义预处理宏 | `"-DDEBUG"` |
| `-o <file>` | 指定输出文件名 | `"-o", "main"` |
| `-c` | 仅编译不链接 | `"-c"` |
| `-fPIC` | 生成位置无关代码 | `"-fPIC"` |

### 4. 完整示例

#### 1. 示例1：单文件编译

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "C/C++: g++ build single file",
            "type": "shell",
            "command": "g++",
            "args": [
                "-g",
                "-std=c++17",
                "-Wall",
                "-Wextra",
                "${file}",
                "-o",
                "${fileDirname}/${fileBasenameNoExtension}"
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
                "showReuseMessage": true,
                "clear": true
            }
        }
    ]
}
```

#### 2. 示例2：多文件联合编译（不用 CMake）

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "C/C++: g++ build multi-file",
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
            },
            "options": {
                "cwd": "${workspaceFolder}"
            }
        }
    ]
}
```

#### 3. 示例3：调用 CMake 构建

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "CMake: Configure",
            "type": "shell",
            "command": "cmake",
            "args": [
                "-B",
                "${workspaceFolder}/build",
                "-S",
                "${workspaceFolder}",
                "-DCMAKE_BUILD_TYPE=Debug"
            ],
            "group": "build",
            "problemMatcher": []
        },
        {
            "label": "CMake: Build",
            "type": "shell",
            "command": "cmake",
            "args": [
                "--build",
                "${workspaceFolder}/build",
                "--config",
                "Debug",
                "-j4"
            ],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": ["$gcc"],
            "dependsOn": ["CMake: Configure"],
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false,
                "panel": "dedicated",
                "clear": true
            }
        }
    ]
}
```

#### 4. 示例4：调用 Make 构建

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Make: Build",
            "type": "shell",
            "command": "make",
            "args": [
                "-C",
                "${workspaceFolder}",
                "-j4"
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
                "panel": "dedicated",
                "clear": true
            }
        }
    ]
}
```

### 5. 变量替换

VS Code 在 `tasks.json` 和 `launch.json` 中支持以下预定义变量：

| 变量 | 说明 | 示例值 |
|------|------|--------|
| `${workspaceFolder}` | 工作区根目录路径 | `/home/user/project` |
| `${workspaceFolderBasename}` | 工作区文件夹名 | `project` |
| `${file}` | 当前活动文件的完整路径 | `/home/user/project/src/main.cpp` |
| `${fileWorkspaceFolder}` | 当前文件所属的工作区文件夹 | `/home/user/project` |
| `${relativeFile}` | 当前文件相对于工作区的相对路径 | `src/main.cpp` |
| `${relativeFileDirname}` | 当前文件所在目录的相对路径 | `src` |
| `${fileBasename}` | 当前文件的文件名（含扩展名） | `main.cpp` |
| `${fileBasenameNoExtension}` | 当前文件的文件名（不含扩展名） | `main` |
| `${fileDirname}` | 当前文件所在的目录的完整路径 | `/home/user/project/src` |
| `${fileExtname}` | 当前文件的扩展名 | `.cpp` |
| `${cwd}` | 任务启动时的工作目录 | `/home/user/project` |
| `${lineNumber}` | 当前活动文件中光标所在行号 | `42` |
| `${selectedText}` | 当前选中的文本 | — |
| `${execPath}` | VS Code 可执行文件路径 | `/usr/bin/code` |
| `${defaultBuildTask}` | 默认构建任务的名称 | — |
| `${env:VAR}` | 环境变量 | `${env:HOME}` → `/home/user` |
| `${config:setting}` | VS Code 设置值 | `${config:editor.fontSize}` |

### 6. 多任务配置（build + clean）

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
                "-I${workspaceFolder}/include",
                "${workspaceFolder}/src/main.cpp",
                "${workspaceFolder}/src/utils.cpp",
                "-o",
                "${workspaceFolder}/build/myapp"
            ],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": ["$gcc"]
        },
        {
            "label": "clean",
            "type": "shell",
            "command": "rm",
            "args": [
                "-rf",
                "${workspaceFolder}/build"
            ],
            "problemMatcher": []
        },
        {
            "label": "rebuild",
            "dependsOn": ["clean", "build"],
            "dependsOrder": "sequence",
            "problemMatcher": []
        }
    ]
}
```

`dependsOrder` 说明：
- `"sequence"`：按顺序依次执行依赖任务
- `"parallel"`：并行执行依赖任务（默认）

---
## 4. settings.json 详解（工作区设置）

### 1. C/C++ 扩展相关设置

#### 1. C_Cpp.default.compilerPath
- **作用**：默认编译器路径，当 `c_cpp_properties.json` 未指定时生效
- **示例**：`"C_Cpp.default.compilerPath": "/usr/bin/g++"`

#### 2. C_Cpp.default.intelliSenseMode
- **作用**：默认智能提示模式
- **示例**：`"C_Cpp.default.intelliSenseMode": "linux-gcc-x64"`

#### 3. C_Cpp.default.cStandard
- **作用**：默认 C 语言标准
- **示例**：`"C_Cpp.default.cStandard": "c17"`

#### 4. C_Cpp.default.cppStandard
- **作用**：默认 C++ 语言标准
- **示例**：`"C_Cpp.default.cppStandard": "c++17"`

#### 5. C_Cpp.errorSquiggles
- **作用**：是否启用错误波浪线提示
- **可选值**：`"enabled"`（启用）、`"disabled"`（禁用）
- **用途**：当波浪线干扰时可以临时关闭

#### 6. C_Cpp.loggingLevel
- **作用**：C/C++ 扩展的日志级别
- **可选值**：`"none"`、`"error"`、`"warning"`、`"information"`、`"debug"`
- **用途**：排查问题时设为 `"debug"` 获取详细日志

#### 7. C_Cpp.autocomplete
- **作用**：自动补全引擎选择
- **可选值**：`"default"`（使用扩展自带引擎）、`"tagParser"`（使用旧版标签解析器）

#### 8. C_Cpp.formatting
- **作用**：代码格式化工具
- **可选值**：`"clangFormat"`（使用 clang-format）、`"vcFormat"`（使用 MSVC 格式化器）、`"none"`

### 2. CMake Tools 扩展相关设置

#### 1. cmake.buildDirectory
- **作用**：CMake 构建目录
- **示例**：`"cmake.buildDirectory": "${workspaceFolder}/build"`

#### 2. cmake.generator
- **作用**：CMake 生成器
- **可选值**：`"Unix Makefiles"`、`"Ninja"`、`"MinGW Makefiles"`、`"Visual Studio 17 2022"` 等
- **示例**：`"cmake.generator": "Ninja"`

#### 3. cmake.configureArgs
- **作用**：CMake 配置阶段的额外参数
- **示例**：`"cmake.configureArgs": ["-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"]`

#### 4. cmake.debugConfig
- **作用**：CMake 调试配置，会合并到 `launch.json` 的配置中
- **示例**：
```json
"cmake.debugConfig": {
    "args": ["--test"],
    "environment": [
        {"name": "LD_LIBRARY_PATH", "value": "${workspaceFolder}/build"}
    ]
}
```

#### 5. cmake.buildArgs
- **作用**：CMake 构建阶段的额外参数
- **示例**：`"cmake.buildArgs": ["--verbose"]`

#### 6. cmake.configureOnOpen
- **作用**：打开项目时是否自动执行 CMake 配置
- **可选值**：`true`、`false`
- **建议**：大型项目设为 `false`，避免每次打开都重新配置

### 3. 文件关联设置

```json
"files.associations": {
    "*.h": "cpp",
    "*.hpp": "cpp",
    "*.cpp": "cpp",
    "*.c": "c",
    "*.tcc": "cpp",
    "*.cc": "cpp",
    "*.cxx": "cpp",
    "*.hxx": "cpp",
    "CMakeLists.txt": "cmake",
    "*.cmake": "cmake"
}
```

### 4. 终端设置

```json
"terminal.integrated.defaultProfile.windows": "Command Prompt",
"terminal.integrated.profiles.windows": {
    "Command Prompt": {
        "path": "C:\\Windows\\System32\\cmd.exe",
        "args": []
    },
    "Developer Command Prompt": {
        "path": "C:\\Windows\\System32\\cmd.exe",
        "args": ["/k", "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvarsall.bat", "x64"]
    },
    "Git Bash": {
        "path": "C:\\Program Files\\Git\\bin\\bash.exe",
        "args": []
    }
}
```

### 5. 完整示例

```json
{
    "C_Cpp.default.compilerPath": "/usr/bin/g++",
    "C_Cpp.default.intelliSenseMode": "linux-gcc-x64",
    "C_Cpp.default.cStandard": "c17",
    "C_Cpp.default.cppStandard": "c++17",
    "C_Cpp.errorSquiggles": "enabled",
    "C_Cpp.loggingLevel": "warning",
    "C_Cpp.formatting": "clangFormat",

    "cmake.buildDirectory": "${workspaceFolder}/build",
    "cmake.generator": "Unix Makefiles",
    "cmake.configureArgs": [
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
    ],
    "cmake.configureOnOpen": false,
    "cmake.debugConfig": {
        "args": [],
        "environment": []
    },

    "files.associations": {
        "*.h": "cpp",
        "*.hpp": "cpp",
        "*.tcc": "cpp",
        "CMakeLists.txt": "cmake",
        "*.cmake": "cmake"
    },

    "editor.formatOnSave": true,
    "editor.tabSize": 4,
    "editor.insertSpaces": true,

    "terminal.integrated.defaultProfile.linux": "bash"
}
```

---

## 5. 必须安装的插件及其关联关系

### 1. C/C++（Microsoft）

- **扩展ID**：`ms-vscode.cpptools`
- **作用**：
  - 提供 C/C++ 语言的智能提示（代码补全、参数提示）
  - 提供代码导航（转到定义、查找所有引用）
  - 提供代码格式化（clang-format 集成）
  - 提供调试功能（GDB/LLDB/MSVC 调试适配器）
- **配置要点**：
  - 安装后需要配置 `c_cpp_properties.json` 或通过 `settings.json` 设置默认值
  - 首次使用时会自动检测系统编译器

### 2. C/C++ Extension Pack

- **扩展ID**：`ms-vscode.cpptools-extension-pack`
- **包含内容**：
  - **C/C++**（核心扩展）
  - **C/C++ Themes**（C/C++ 主题）
  - **CMake Tools**（CMake 集成）
  - **CMake**（CMake 语法高亮）
- **作用**：一键安装 C/C++ 开发所需的全部扩展

### 3. CMake Tools

- **扩展ID**：`ms-vscode.cmake-tools`
- **作用**：
  - 提供 CMake 项目的配置、构建、调试一体化工作流
  - 自动检测 CMakeLists.txt 并提供配置界面
  - 管理构建目录、生成器、构建类型
  - 与 C/C++ 扩展联动，自动提供 include 路径和编译器信息
  - 底部状态栏显示 CMake 操作按钮
- **与 C/C++ 扩展的关联**：
  - 在 `c_cpp_properties.json` 中设置 `"configurationProvider": "ms-vscode.cmake-tools"` 后，CMake Tools 会自动管理智能提示配置
  - CMake Tools 可以直接调用 C/C++ 扩展的调试功能

### 4. CMake

- **扩展ID**：`twxs.cmake`
- **作用**：
  - 提供 CMake 语言的语法高亮
  - 提供 CMake 命令的代码补全
- **注意**：仅提供语法支持，不提供构建功能（构建由 CMake Tools 负责）

### 5. Code Runner

- **扩展ID**：`formulahendry.code-runner`
- **作用**：
  - 一键运行代码（点击右上角播放按钮）
  - 支持多种语言
  - 可自定义运行命令
- **配置要点**：
```json
{
    "code-runner.executorMap": {
        "c": "cd $dir && gcc $fileName -o $fileNameWithoutExt && $dir$fileNameWithoutExt",
        "cpp": "cd $dir && g++ $fileName -o $fileNameWithoutExt && $dir$fileNameWithoutExt"
    },
    "code-runner.runInTerminal": true,
    "code-runner.saveFileBeforeRun": true,
    "code-runner.clearPreviousOutput": true
}
```
- **局限**：不支持调试，仅适合快速运行验证

### 6. 插件之间的协作关系图

```
┌──────────────────────────────────────────────────────────────┐
│                    插件协作关系                                │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌─────────────────────┐                                     │
│  │   C/C++ (Microsoft) │                                     │
│  │                     │                                     │
│  │  ✅ 智能提示         │──→ 代码补全、参数提示、错误检查      │
│  │  ✅ 代码导航         │──→ 转到定义、查找引用                │
│  │  ✅ 调试适配器       │──→ GDB/LLDB/MSVC 调试               │
│  │  ✅ 代码格式化       │──→ clang-format 集成                │
│  └─────────┬───────────┘                                     │
│            │                                                 │
│            │ configurationProvider                            │
│            │ 提供智能提示配置                                  │
│            ▼                                                 │
│  ┌─────────────────────┐     ┌─────────────────────┐        │
│  │   CMake Tools       │     │     CMake           │        │
│  │                     │     │   (语法高亮)         │        │
│  │  ✅ CMake配置        │     │                     │        │
│  │  ✅ CMake构建        │     │  ✅ 语法高亮         │        │
│  │  ✅ CMake调试        │     │  ✅ 命令补全         │        │
│  │  ✅ 提供include路径  │     │                     │        │
│  └─────────────────────┘     └─────────────────────┘        │
│                                                              │
│  ┌─────────────────────┐                                     │
│  │   Code Runner       │                                     │
│  │                     │                                     │
│  │  ✅ 一键运行         │──→ 仅运行，不调试                    │
│  │  ❌ 不参与智能提示   │                                     │
│  │  ❌ 不参与调试       │                                     │
│  └─────────────────────┘                                     │
│                                                              │
│  职责划分：                                                   │
│  ┌──────────┬──────────────┬──────────────┬────────────┐    │
│  │ 功能     │ C/C++        │ CMake Tools  │ Code Runner│    │
│  ├──────────┼──────────────┼──────────────┼────────────┤    │
│  │ 智能提示 │ ✅ 主要负责   │ ✅ 提供路径   │ ❌         │    │
│  │ 构建     │ ❌           │ ✅ 主要负责   │ ⚠️ 简单运行│    │
│  │ 调试     │ ✅ 主要负责   │ ✅ 触发调试   │ ❌         │    │
│  │ 格式化   │ ✅ 主要负责   │ ❌           │ ❌         │    │
│  └──────────┴──────────────┴──────────────┴────────────┘    │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

### 7. 插件冲突和注意事项

1. **C/C++ 与 Clangd 扩展冲突**：
   - `clangd` 扩展（`llvm-vs-code-extensions.vscode-clangd`）与 C/C++ 扩展功能重叠
   - 同时安装会导致智能提示冲突，建议只安装其中一个
   - 如果使用 clangd，需要在 C/C++ 扩展设置中禁用智能提示：`"C_Cpp.intelliSenseEngine": "disabled"`

2. **CMake Tools 与手动 tasks.json 冲突**：
   - 使用 CMake Tools 时，不建议再手动配置 CMake 相关的 `tasks.json`
   - CMake Tools 会自动管理构建任务，手动配置可能导致重复构建

3. **Code Runner 与 tasks.json**：
   - Code Runner 使用自己的执行配置，不走 `tasks.json`
   - 如果需要调试，不要使用 Code Runner，应使用 `launch.json`

4. **多版本编译器冲突**：
   - 系统中安装多个编译器时，需明确指定 `compilerPath`
   - Windows 上 MinGW 和 MSVC 共存时，注意 `intelliSenseMode` 的选择

---
