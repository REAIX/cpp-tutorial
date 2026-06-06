# VS Code 调试与优化配置

> **前置阅读**：如果你还没有配置VS Code开发环境，请先阅读 [00-VSCode核心配置](./00-VSCode核心配置.md) 完成基础配置。本文档假设你已经掌握了VS Code的基本配置，需要学习调试和优化配置。

## 1. launch.json 详解（调试配置）

### 1. 文件结构总览

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "配置名称",
            "type": "调试器类型",
            "request": "请求类型",
            "program": "程序路径",
            "args": ["命令行参数"],
            "cwd": "工作目录",
            "environment": ["环境变量"],
            "externalConsole": false,
            "MIMode": "MI模式",
            "miDebuggerPath": "调试器路径",
            "setupCommands": ["初始化命令"],
            "preLaunchTask": "前置任务",
            "stopAtEntry": false
        }
    ]
}
```

### 2. 每个字段详细说明

#### 1. name
- **作用**：调试配置的显示名称，在调试面板的下拉列表中展示
- **示例**：`"name": "GDB Debug"`

#### 2. type
- **作用**：指定调试器类型
- **可选值**：`"cppdbg"`（C/C++ 扩展提供）、`"lldb"`（CodeLLDB 扩展提供）、`"cppvsdbg"`（MSVC 调试器）

#### 3. request
- **作用**：调试请求类型
- **可选值**：
  - `"launch"`：启动新程序进行调试
  - `"attach"`：附加到已运行的进程

#### 4. program
- **作用**：要调试的可执行文件路径
- **示例**：`"program": "${workspaceFolder}/build/myapp"`、`"program": "${fileDirname}/${fileBasenameNoExtension}"`

#### 5. args
- **作用**：传递给被调试程序的命令行参数（即 main 函数的参数）
- **格式**：字符串数组，每个元素对应一个参数
- **示例**：`"args": ["--verbose", "input.txt", "--count", "10"]`

#### 6. cwd
- **作用**：调试时程序的工作目录
- **示例**：`"cwd": "${workspaceFolder}"`

#### 7. environment
- **作用**：设置调试时的环境变量
- **格式**：对象数组，每个对象包含 `name` 和 `value`
- **示例**：
```json
"environment": [
    {"name": "PATH", "value": "/usr/local/bin:${env:PATH}"},
    {"name": "LD_LIBRARY_PATH", "value": "${workspaceFolder}/lib"}
]
```

#### 8. externalConsole
- **作用**：是否使用外部终端窗口
- **可选值**：`true`（弹出独立控制台窗口）、`false`（使用 VS Code 内置终端）
- **注意**：Windows 上调试需要控制台输入的程序时，建议设为 `true`

#### 9. MIMode
- **作用**：指定 MI（Machine Interface）调试器模式
- **可选值**：`"gdb"`、`"lldb"`
- **平台对应**：Linux/Windows(MinGW) 用 `"gdb"`，macOS 用 `"lldb"`

#### 10. miDebuggerPath
- **作用**：调试器的完整路径
- **示例**：
  - Linux：`"/usr/bin/gdb"`
  - Windows MinGW：`"C:/msys64/mingw64/bin/gdb.exe"`
  - macOS：`"/usr/bin/lldb"`

#### 11. setupCommands
- **作用**：调试器启动后执行的初始化命令
- **常用命令**：
  - `"text": "-enable-pretty-printing"`：启用 STL 容器的美化打印
  - `"text": "-gdb-set print elements 0"`：不限制打印的元素数量
  - `"text": "-gdb-set print object on"`：启用多态对象的正确打印
- **示例**：
```json
"setupCommands": [
    {
        "description": "为 gdb 启用整齐打印",
        "text": "-enable-pretty-printing",
        "ignoreFailures": true
    },
    {
        "description": "将反汇编风格设置为 Intel",
        "text": "-gdb-set disassembly-flavor intel",
        "ignoreFailures": true
    }
]
```

#### 12. preLaunchTask
- **作用**：调试前自动执行的任务，对应 `tasks.json` 中的 `label`
- **示例**：`"preLaunchTask": "build"` — 调试前先编译
- **注意**：如果设为空字符串 `""`，则不执行任何前置任务

#### 13. stopAtEntry
- **作用**：是否在程序入口（main 函数）处自动暂停
- **可选值**：`true`（自动在 main 处设断点）、`false`（不自动暂停）
- **用途**：调试时快速定位程序入口

### 3. 完整示例

#### 1. 示例1：GDB 调试单文件（Linux/MinGW）

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "GDB Debug Single File",
            "type": "cppdbg",
            "request": "launch",
            "program": "${fileDirname}/${fileBasenameNoExtension}",
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
                },
                {
                    "description": "将反汇编风格设置为 Intel",
                    "text": "-gdb-set disassembly-flavor intel",
                    "ignoreFailures": true
                }
            ],
            "preLaunchTask": "C/C++: g++ build single file"
        }
    ]
}
```

#### 2. 示例2：GDB 调试 CMake 项目

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "GDB Debug CMake Project",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/myapp",
            "args": ["--config", "debug.cfg"],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [
                {"name": "LD_LIBRARY_PATH", "value": "${workspaceFolder}/build/lib"}
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
            "preLaunchTask": "CMake: Build"
        }
    ]
}
```

#### 3. 示例3：LLDB 调试（macOS）

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "LLDB Debug macOS",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/myapp",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "lldb",
            "miDebuggerPath": "/usr/bin/lldb",
            "preLaunchTask": "build"
        }
    ]
}
```

#### 4. 示例4：MSVC 调试（Windows + cl.exe）

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "MSVC Debug Windows",
            "type": "cppvsdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/Debug/myapp.exe",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "console": "integratedTerminal",
            "preLaunchTask": "CMake: Build"
        }
    ]
}
```

**MSVC 调试注意事项**：
- `type` 必须使用 `"cppvsdbg"`
- 不需要 `MIMode` 和 `miDebuggerPath` 字段
- `console` 字段可选值：`"internalConsole"`、`"integratedTerminal"`、`"externalTerminal"`
- 需要从 Visual Studio Developer Command Prompt 启动 VS Code，或配置 `tasks.json` 中的 `vcvarsall.bat` 调用

#### 5. 示例5：附加到进程

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Attach to Process",
            "type": "cppdbg",
            "request": "attach",
            "program": "/path/to/target/executable",
            "MIMode": "gdb",
            "miDebuggerPath": "/usr/bin/gdb",
            "processId": "${command:pickProcess}"
        }
    ]
}
```

`${command:pickProcess}` 会在调试启动时弹出进程选择器，让你选择要附加的进程。

### 4. 条件断点与日志断点

#### 1. 条件断点

在代码行号左侧右键 → "添加条件断点"，支持以下类型：

1. **表达式条件**：当表达式为 `true` 时命中
   ```
   i == 100
   ```
   ```
   strcmp(name, "target") == 0
   ```

2. **命中次数**：执行到第 N 次时命中
   ```
   5
   ```
   表示第5次执行到该行时暂停

3. **日志消息**：不暂停执行，仅输出日志
   ```
   i = {i}, sum = {sum}
   ```

#### 2. 在 launch.json 中配置断点

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug with breakpoints",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/myapp",
            "stopAtEntry": true,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "设置条件断点示例（在GDB命令行中）",
                    "text": "break main.cpp:42 if i > 10",
                    "ignoreFailures": true
                }
            ]
        }
    ]
}
```

### 5. main 函数参数的配置方法

通过 `args` 字段传递参数给 main 函数：

```cpp
// main.cpp
#include <iostream>
int main(int argc, char* argv[]) {
    std::cout << "参数个数: " << argc << std::endl;
    for (int i = 0; i < argc; ++i) {
        std::cout << "argv[" << i << "]: " << argv[i] << std::endl;
    }
    return 0;
}
```

对应的 `launch.json`：

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug with args",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/myapp",
            "args": [
                "--input", "data.txt",
                "--verbose",
                "--count", "100"
            ],
            "cwd": "${workspaceFolder}",
            "MIMode": "gdb"
        }
    ]
}
```

运行后 `argc` 为 6，`argv` 内容为：
```
argv[0]: myapp
argv[1]: --input
argv[2]: data.txt
argv[3]: --verbose
argv[4]: --count
argv[5]: 100
```

### 6. 环境变量的配置方法

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug with env",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/myapp",
            "args": [],
            "cwd": "${workspaceFolder}",
            "environment": [
                {
                    "name": "MY_CONFIG_PATH",
                    "value": "${workspaceFolder}/config"
                },
                {
                    "name": "LD_LIBRARY_PATH",
                    "value": "/usr/local/lib:${env:LD_LIBRARY_PATH}"
                },
                {
                    "name": "DEBUG_LEVEL",
                    "value": "3"
                }
            ],
            "MIMode": "gdb"
        }
    ]
}
```

---
## 2. 常见问题排查

### 1. 头文件找不到

**症状**：`#include` 语句下方出现红色波浪线，提示 `cannot open source file`

**排查步骤**：

1. **确认头文件实际位置**：
```bash
# Linux 查找头文件
find /usr -name "header.h" 2>/dev/null
locate header.h

# macOS 查找头文件
find /usr/local -name "header.h"
find /opt/homebrew -name "header.h"
```

2. **检查 c_cpp_properties.json 的 includePath**：
   - 确认路径是否正确
   - 确认是否使用了正确的变量（如 `${workspaceFolder}`）
   - 确认 glob 模式是否匹配（`**` 表示递归）

3. **检查 compilerPath**：
   - 如果设置了 `compilerPath`，扩展会自动查询编译器的默认 include 路径
   - 如果编译器路径错误，自动查询会失败

4. **查看扩展检测到的 include 路径**：
   - 打开命令面板 → `C/C++: Log Diagnostics`
   - 在输出面板查看 `C/C++` 通道，检查 `includePath` 是否包含所需路径

5. **使用 CMake Tools 时**：
   - 确认 `configurationProvider` 设置为 `"ms-vscode.cmake-tools"`
   - 确认 CMake 配置已成功执行
   - 确认 `CMAKE_EXPORT_COMPILE_COMMANDS` 设为 `ON`

### 2. 智能提示不工作

**症状**：代码没有自动补全、没有语法高亮、没有错误提示

**排查步骤**：

1. **确认 C/C++ 扩展已安装并启用**：
   - 打开扩展面板，搜索 `C/C++`，确认已安装且未禁用

2. **检查 intelliSenseMode**：
   - 确认与实际编译器匹配
   - 使用 GCC 编译器就选 `gcc-x64`，不要选 `msvc-x64`

3. **检查是否与 clangd 冲突**：
   - 如果同时安装了 clangd 扩展，两者会冲突
   - 解决方案：卸载 clangd，或在 C/C++ 扩展设置中禁用智能提示

4. **重置智能提示数据库**：
   - 命令面板 → `C/C++: Reset IntelliSense Database`

5. **查看扩展日志**：
   - 输出面板选择 `C/C++` 通道
   - 设置 `"C_Cpp.loggingLevel": "debug"` 获取详细日志

6. **检查文件关联**：
   - 确认 `.cpp` 文件被识别为 C++ 语言
   - 查看右下角的语言模式指示器

### 3. 调试时断点不命中

**症状**：设置了断点但调试时程序不暂停，断点显示为灰色空心圆

**排查步骤**：

1. **确认编译时添加了调试信息**：
   - 检查 `tasks.json` 中的 `args` 是否包含 `"-g"`
   - 检查是否开启了优化（`-O2` 会导致断点偏移），调试时应使用 `-O0`

2. **确认 program 路径正确**：
   - `launch.json` 中的 `program` 必须指向编译生成的可执行文件
   - 路径错误会导致调试器加载错误的二进制文件

3. **确认源文件路径匹配**：
   - 编译时的源文件路径与调试时打开的源文件路径必须一致
   - 软链接或映射路径可能导致路径不匹配

4. **CMake 项目确认构建类型**：
   - 确认 `CMAKE_BUILD_TYPE` 为 `Debug`
   - Release 模式默认不生成调试信息

5. **Windows MSVC 特殊检查**：
   - 确认使用 `"type": "cppvsdbg"` 而非 `"cppdbg"`
   - 确认 PDB 文件与可执行文件在同一目录

6. **GDB 特殊检查**：
   - 尝试在 `setupCommands` 中添加 `"text": "set breakpoint pending on"`
   - 检查是否有地址空间随机化影响：`"text": "disable-randomization off"`

### 4. 编译成功但运行报错

**症状**：编译无错误，但运行时出现段错误、找不到库等

**常见原因及解决**：

1. **动态库找不到**：
```
error while loading shared libraries: libxxx.so: cannot open shared object file
```
解决：设置 `LD_LIBRARY_PATH` 环境变量
```json
"environment": [
    {"name": "LD_LIBRARY_PATH", "value": "/usr/local/lib:${env:LD_LIBRARY_PATH}"}
]
```

2. **段错误（Segmentation Fault）**：
   - 使用 GDB 调试定位崩溃位置
   - 在 `launch.json` 中设置 `"stopAtEntry": false`，让程序运行到崩溃点
   - 使用 `bt` 命令查看调用栈

3. **未定义引用（链接错误）**：
```
undefined reference to `function_name'
```
解决：检查 `tasks.json` 中是否遗漏了源文件或库文件
```json
// 确保所有源文件都被包含
"args": ["main.cpp", "utils.cpp", "logger.cpp", ...]
// 确保链接了所需的库
"args": [..., "-lpthread", "-lcurl"]
```

4. **权限问题**：
```
Permission denied
```
解决：检查可执行文件是否有执行权限
```bash
chmod +x build/myapp
```

### 5. CMake Configure 失败

**症状**：CMake 配置阶段报错

**常见原因及解决**：

1. **编译器未找到**：
```
No CMAKE_CXX_COMPILER could be found.
```
解决：
- 安装编译器（g++/clang++）
- 或在 CMake 配置参数中指定编译器：
```json
"cmake.configureArgs": [
    "-DCMAKE_CXX_COMPILER=/usr/bin/g++"
]
```

2. **Kit 选择错误**：
   - 命令面板 → `CMake: Select a Kit` 选择正确的编译器工具链

3. **依赖库未找到**：
```
Could NOT find OpenSSL
```
解决：
- 安装缺失的库：`sudo apt install libssl-dev`
- 或指定库的搜索路径：
```cmake
set(OPENSSL_ROOT_DIR /usr/local/ssl)
find_package(OpenSSL REQUIRED)
```

4. **生成器不兼容**：
```
Could not find CMAKE_C_COMPILER
```
解决：
- 更换 CMake 生成器
- 命令面板 → `CMake: Select a Kit` 重新选择
- 或在 settings.json 中指定：
```json
"cmake.generator": "Unix Makefiles"
```

5. **缓存冲突**：
   - 删除构建目录重新配置：
```bash
rm -rf build/
cmake -B build
```
   - 或命令面板 → `CMake: Delete Cache and Reconfigure`

### 6. 插件冲突

#### 1. C/C++ 与 clangd 冲突

**症状**：智能提示异常、重复提示、卡顿

**解决方案**（二选一）：

方案一：保留 C/C++ 扩展，卸载 clangd

方案二：保留 clangd，禁用 C/C++ 扩展的智能提示
```json
{
    "C_Cpp.intelliSenseEngine": "disabled",
    "C_Cpp.autocomplete": "disabled",
    "C_Cpp.errorSquiggles": "disabled"
}
```

#### 2. CMake Tools 与手动 tasks.json 冲突

**症状**：构建时执行两次编译

**解决方案**：
- 使用 CMake Tools 时，删除 `tasks.json` 中的 CMake 相关任务
- 或在 `launch.json` 中不设置 `preLaunchTask`，改用 CMake Tools 的调试功能

#### 3. 多个调试扩展冲突

**症状**：调试时弹出多个调试器选择

**解决方案**：
- 在 `launch.json` 中明确指定 `type` 字段
- 卸载不需要的调试扩展

#### 4. Code Runner 与 C/C++ 扩展冲突

**症状**：Code Runner 运行时编译参数与预期不符

**解决方案**：
- Code Runner 使用自己的配置，不走 `tasks.json`
- 需要在 `settings.json` 中单独配置 `code-runner.executorMap`
- 调试时不要使用 Code Runner，使用 `launch.json`

---

## 3. 附录：快速配置模板

### 1. Linux + GCC 快速配置

```json
// c_cpp_properties.json
{
    "configurations": [{
        "name": "Linux",
        "includePath": ["${workspaceFolder}/**"],
        "defines": ["_DEBUG"],
        "compilerPath": "/usr/bin/g++",
        "cStandard": "c17",
        "cppStandard": "c++17",
        "intelliSenseMode": "linux-gcc-x64"
    }],
    "version": 4
}
```

```json
// tasks.json
{
    "version": "2.0.0",
    "tasks": [{
        "label": "build",
        "type": "shell",
        "command": "g++",
        "args": ["-g", "-std=c++17", "-Wall", "${file}", "-o", "${fileDirname}/${fileBasenameNoExtension}"],
        "group": {"kind": "build", "isDefault": true},
        "problemMatcher": ["$gcc"]
    }]
}
```

```json
// launch.json
{
    "version": "0.2.0",
    "configurations": [{
        "name": "Debug",
        "type": "cppdbg",
        "request": "launch",
        "program": "${fileDirname}/${fileBasenameNoExtension}",
        "args": [],
        "cwd": "${workspaceFolder}",
        "MIMode": "gdb",
        "miDebuggerPath": "/usr/bin/gdb",
        "setupCommands": [{
            "description": "为 gdb 启用整齐打印",
            "text": "-enable-pretty-printing",
            "ignoreFailures": true
        }],
        "preLaunchTask": "build"
    }]
}
```

### 2. Windows + MinGW-w64 快速配置

```json
// c_cpp_properties.json
{
    "configurations": [{
        "name": "Win32",
        "includePath": ["${workspaceFolder}/**"],
        "defines": ["_DEBUG", "UNICODE", "_UNICODE"],
        "compilerPath": "C:/msys64/mingw64/bin/g++.exe",
        "cStandard": "c17",
        "cppStandard": "c++17",
        "intelliSenseMode": "windows-gcc-x64"
    }],
    "version": 4
}
```

```json
// tasks.json
{
    "version": "2.0.0",
    "tasks": [{
        "label": "build",
        "type": "shell",
        "command": "C:/msys64/mingw64/bin/g++.exe",
        "args": ["-g", "-std=c++17", "-Wall", "${file}", "-o", "${fileDirname}\\${fileBasenameNoExtension}.exe"],
        "group": {"kind": "build", "isDefault": true},
        "problemMatcher": ["$gcc"]
    }]
}
```

```json
// launch.json
{
    "version": "0.2.0",
    "configurations": [{
        "name": "Debug",
        "type": "cppdbg",
        "request": "launch",
        "program": "${fileDirname}\\${fileBasenameNoExtension}.exe",
        "args": [],
        "cwd": "${workspaceFolder}",
        "environment": [],
        "externalConsole": true,
        "MIMode": "gdb",
        "miDebuggerPath": "C:/msys64/mingw64/bin/gdb.exe",
        "setupCommands": [{
            "description": "为 gdb 启用整齐打印",
            "text": "-enable-pretty-printing",
            "ignoreFailures": true
        }],
        "preLaunchTask": "build"
    }]
}
```

### 3. macOS + Clang 快速配置

```json
// c_cpp_properties.json
{
    "configurations": [{
        "name": "macOS",
        "includePath": ["${workspaceFolder}/**", "/usr/local/include/**", "/opt/homebrew/include/**"],
        "defines": ["_DEBUG"],
        "compilerPath": "/usr/bin/clang++",
        "cStandard": "c17",
        "cppStandard": "c++17",
        "intelliSenseMode": "macos-clang-x64",
        "macFrameworkPath": ["/System/Library/Frameworks", "/Library/Frameworks"]
    }],
    "version": 4
}
```

```json
// tasks.json
{
    "version": "2.0.0",
    "tasks": [{
        "label": "build",
        "type": "shell",
        "command": "/usr/bin/clang++",
        "args": ["-g", "-std=c++17", "-Wall", "${file}", "-o", "${fileDirname}/${fileBasenameNoExtension}"],
        "group": {"kind": "build", "isDefault": true},
        "problemMatcher": ["$gcc"]
    }]
}
```

```json
// launch.json
{
    "version": "0.2.0",
    "configurations": [{
        "name": "Debug",
        "type": "cppdbg",
        "request": "launch",
        "program": "${fileDirname}/${fileBasenameNoExtension}",
        "args": [],
        "cwd": "${workspaceFolder}",
        "MIMode": "lldb",
        "miDebuggerPath": "/usr/bin/lldb",
        "preLaunchTask": "build"
    }]
}
```
