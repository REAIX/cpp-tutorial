# 不同平台C与CPP的区别与可移植性
> 📖 相关章节：[编码规范](../../04-工程实践/00-编码规范.md)、[设计模式](../../04-工程实践/03-设计模式.md)、[单元测试](../../04-工程实践/06-单元测试.md)、[代码审查](../../04-工程实践/08-代码审查.md)

> **📖 本文定位**：聚焦不可移植的原因分析（字节序/对齐/路径/API差异）、可移植编码原则、跨平台库推荐，以及 Emscripten/WebAssembly 方案。
>
> **🔗 相关阅读**：
> - [跨平台是什么意思](00-跨平台是什么意思.md) — 聚焦跨平台的概念与实现方法（条件编译/抽象层/CMake/跨平台库），与本文的"原因分析与编码原则"互补

> "可移植性=写一份代码，到哪都能跑——不可移植=换个平台就编译不过。"

***

### 1. 核心定义

- **平台** = CPU架构 + 操作系统 + 编译器的组合
- **可移植性** = 同一份代码在不同平台上都能正确编译和运行
- C/C++标准定义了"通用"部分，各平台有自己的扩展

***

### 2. 生活类比

**C/C++标准 = 国际驾照规则，各平台差异 = 各国交通法规**。

| 概念 | 类比 | 说明 |
|------|------|------|
| C/C++标准 | 国际驾照规则 | 基本规则统一 |
| 各平台差异 | 各国交通法规 | 靠左/靠右行驶、限速不同 |
| 可移植代码 | 遵守国际规则的驾驶 | 到哪国都能开 |
| 不可移植代码 | 只在本国有效的驾照 | 出国就不能用了 |

***

### 3. 主要平台差异

#### 1. CPU架构差异

| 维度 | x86_64 | ARM64 | RISC-V | 32位(x86) |
|------|--------|-------|--------|-----------|
| 字长 | 64位 | 64位 | 32/64位 | 32位 |
| 字节序 | 小端 | 可配置(通常小端) | 小端 | 小端 |
| 指针大小 | 8字节 | 8字节 | 4/8字节 | 4字节 |
| long大小 | 8字节(Linux)/4字节(Windows) | 8字节 | 8字节 | 4字节 |
| 对齐要求 | 较宽松 | 严格(必须对齐) | 较宽松 | 较宽松 |

#### 2. 操作系统差异

| 维度 | Linux | Windows | macOS |
|------|-------|---------|-------|
| 动态库 | .so | .dll | .dylib |
| 可执行文件 | ELF | PE/COFF | Mach-O |
| 线程API | pthread | Windows线程 | pthread |
| 网络API | POSIX socket | Winsock | POSIX socket |
| 文件路径 | / 分隔 | \ 分隔 | / 分隔 |
| 换行符 | LF(\n) | CRLF(\r\n) | LF(\n) |
| API风格 | POSIX | Win32 API | POSIX + Cocoa |

#### 3. 编译器差异

| 维度 | GCC/Clang | MSVC |
|------|-----------|------|
| 名称改编 | Itanium ABI | MSVC ABI |
| 模板支持 | 完整 | 部分限制 |
| 扩展 | __attribute__ | __declspec |
| 预定义宏 | __GNUC__/__clang__ | _MSC_VER |
| C99/C11支持 | 完整 | 部分支持 |

***

### 4. 为什么C/C++不通用

#### 1. 原因1：标准留了"实现定义"的空间

- `int`的大小：标准只要求≥16位，实际16/32/64位
- 字节序：标准未规定
- 结构体对齐：标准未规定填充规则

```cpp
#include <iostream>

struct Data {
    char a;
    int  b;
    short c;
};

int main() {
    std::cout << "sizeof(Data) = " << sizeof(Data) << "\n";
    std::cout << "sizeof(int)  = " << sizeof(int) << "\n";
    std::cout << "sizeof(long) = " << sizeof(long) << "\n";
    std::cout << "sizeof(void*)= " << sizeof(void*) << "\n";
    return 0;
}
```

同一结构体在不同平台的大小：

| 平台 | sizeof(Data) | sizeof(int) | sizeof(long) | sizeof(void*) |
|------|-------------|-------------|--------------|---------------|
| Linux x86_64 (GCC) | 12 | 4 | 8 | 8 |
| Windows x86_64 (MSVC) | 12 | 4 | 4 | 8 |
| Linux ARM64 (GCC) | 12 | 4 | 8 | 8 |
| 32位 x86 (GCC) | 12 | 4 | 4 | 4 |

#### 2. 原因2：各平台API不同

| 功能 | Linux/macOS | Windows |
|------|-------------|---------|
| 网络编程 | POSIX socket | Winsock |
| 多线程 | pthread | Windows线程 |
| 文件映射 | mmap | MapViewOfFile |
| 动态加载 | dlopen | LoadLibrary |

```cpp
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

void* load_library(const char* name) {
#ifdef _WIN32
    return LoadLibraryA(name);
#else
    return dlopen(name, RTLD_LAZY);
#endif
}

void close_library(void* handle) {
#ifdef _WIN32
    FreeLibrary((HMODULE)handle);
#else
    dlclose(handle);
#endif
}
```

#### 3. 原因3：编译器扩展不兼容

```cpp
#ifdef __GNUC__
struct __attribute__((packed)) PackedData {
    char a;
    int  b;
};
#endif

#ifdef _MSC_VER
#pragma pack(push, 1)
struct PackedData {
    char a;
    int  b;
};
#pragma pack(pop)
#endif
```

#### 4. 原因4：ABI不兼容

- 名称改编（Name Mangling）规则不同
- 调用约定（Calling Convention）不同
- 异常处理机制不同
- 虚函数表布局可能不同

```cpp
void foo(int x);

// GCC/Clang改编后: _Z3fooi
// MSVC改编后:      ?foo@@YAXH@Z
```

***

### 5. 通用的是什么

#### 1. C标准库（跨平台通用）

| 头文件 | 核心函数 | 说明 |
|--------|---------|------|
| stdio.h | printf/fopen/fread | 输入输出 |
| stdlib.h | malloc/free/atoi | 内存与工具函数 |
| string.h | strcpy/memcpy/strlen | 字符串与内存操作 |
| math.h | sin/cos/sqrt | 数学函数 |
| time.h | time/clock | 时间函数 |

#### 2. C++标准库（跨平台通用）

| 类别 | 核心组件 | 说明 |
|------|---------|------|
| STL容器 | vector/map/string | 数据存储与管理 |
| 智能指针 | unique_ptr/shared_ptr | 自动内存管理 |
| 线程 | std::thread/mutex/atomic（C++11） | 多线程支持 |
| 文件系统 | std::filesystem（C++17） | 文件目录操作 |
| 网络 | 无标准网络库 | 需用第三方或平台API |

#### 3. POSIX标准（Linux/macOS通用）

| 头文件 | 说明 |
|--------|------|
| unistd.h | POSIX系统调用 |
| pthread.h | POSIX线程 |
| fcntl.h | 文件控制 |
| sys/socket.h | 网络套接字 |

***

### 6. 如何编写可移植代码

**原则1：使用标准库而非平台API**

```cpp
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

void portable_file_ops() {
    fs::path p = fs::current_path() / "data" / "config.txt";
    fs::create_directories(p.parent_path());

    std::ofstream out(p);
    out << "hello\n";
}
```

**原则2：用条件编译处理平台差异**

```cpp
#ifdef _WIN32
    #define PLATFORM_NAME "Windows"
    #include <windows.h>
#elif defined(__linux__)
    #define PLATFORM_NAME "Linux"
    #include <unistd.h>
#elif defined(__APPLE__)
    #define PLATFORM_NAME "macOS"
    #include <unistd.h>
#else
    #define PLATFORM_NAME "Unknown"
#endif
```

**原则3：用typedef定义平台无关的类型**

```cpp
#include <cstdint>

int32_t  value32 = 42;
uint64_t value64 = 100;
int8_t   byte_val = 'A';
```

**原则4：用CMake等构建系统管理平台差异**

```cmake
cmake_minimum_required(VERSION 3.20)
project(PortableApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

if(WIN32)
    target_compile_definitions(myapp PRIVATE WINDOWS_PLATFORM)
    target_link_libraries(myapp PRIVATE ws2_32)
elseif(UNIX)
    target_compile_definitions(myapp PRIVATE UNIX_PLATFORM)
    target_link_libraries(myapp PRIVATE pthread)
endif()
```

**原则5：用跨平台库（Boost/Qt/POCO）**

```cpp
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

void cross_platform_example() {
    namespace asio = boost::asio;
    namespace fs = boost::filesystem;

    asio::io_context io;
    fs::path p = fs::current_path();
}
```

***

### 7. 跨平台库推荐

| 领域 | 库 | 说明 |
|------|---|------|
| 通用 | Boost | C++准标准库 |
| GUI | Qt | 跨平台UI框架 |
| 网络 | libcurl/Boost.Asio | HTTP/TCP/UDP |
| 线程 | std::thread/pthread | C++11标准 |
| 文件系统 | std::filesystem | C++17标准 |
| 日志 | spdlog | 跨平台日志 |
| 序列化 | protobuf | 跨语言跨平台 |

***

### 8. 极简总结

**C/C++不通用因为：标准留了"实现定义"空间、各平台API不同、编译器扩展不兼容、ABI不同。通用的是：C/C++标准库、POSIX（Linux/macOS）、跨平台第三方库。可移植原则：用标准库、用int32_t等固定宽度类型、条件编译处理差异、用CMake管理构建。**

| 要点 | 一句话 |
|------|--------|
| 不通用的原因 | 标准留空间 + API不同 + 编译器扩展不兼容 + ABI不同 |
| 通用部分 | C/C++标准库 + POSIX + 跨平台第三方库 |
| 类型安全 | 用int32_t/uint64_t而非int/long |
| 平台差异 | 用条件编译（#ifdef）处理 |
| 构建管理 | 用CMake管理平台差异 |
| 第三方库 | Boost/Qt/libcurl/spdlog/protobuf |

***

### 相关阅读

- [跨平台是什么意思](./00-跨平台是什么意思.md)
- [什么是字节序Endianness](./28-什么是字节序Endianness.md)
- [操作系统接口编程](../09-系统与安全/06-操作系统接口编程.md)

***