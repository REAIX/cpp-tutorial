# CPP工具链实战指南
> 📖 相关章节：[调试技巧](../../04-工程实践/06-调试技巧.md)、[GCC与G++编译器](../../01-C语言/20-GCC与G++编译器.md)

> "工欲善其事，必先利其器"——好的工具链让机器做重复工作，人专注逻辑。

***

### 1. 核心定义

**工具链 = 从代码到可运行程序所需的所有工具集合**。

| 工具类别 | 作用 | 代表工具 |
|----------|------|----------|
| 编译器 | 源码 → 目标文件 | GCC、Clang、MSVC |
| 构建系统 | 编排编译流程 | CMake、Ninja、Make |
| 调试器 | 运行时查错 | GDB、LLDB |
| 分析器 | 静态/动态检查 | clang-tidy、Valgrind、ASan |
| 格式化器 | 统一代码风格 | clang-format |
| 测试框架 | 验证正确性 | Google Test、Catch2 |
| 包管理器 | 管理第三方依赖 | vcpkg、Conan |

**核心原则**：好的工具链 = 让机器做重复工作，人专注逻辑。

***

### 2. 生活类比

**工具链 = 厨房设备**。

| 概念 | 类比 | 说明 |
|------|------|------|
| 编译器 | 刀 | 切菜（源码）的基本工具 |
| 构建系统 | 流水线 | 按顺序安排切菜、炒菜、装盘 |
| 调试器 | 试吃勺 | 做到一半尝一口，看哪里不对 |
| 静态分析 | 食品安全检测仪 | 上菜前检查有没有问题 |
| 格式化器 | 统一餐具 | 所有盘子摆法一样，看着整齐 |
| 测试框架 | 品控流程 | 每道菜出锅前过一遍标准 |

**具体场景**：

- **用对工具**：切菜用刀、炒菜用锅、烤面包用烤箱。用对工具事半功倍。
- **用错工具**：用菜刀烤面包、用烤箱切菜。工具链不对，效率极低且容易出错。

***

### 3. 编译器警告体系

编译器警告是最廉价也最有效的Bug预防手段。

**常用警告选项**：

| 选项 | 作用 | 严格程度 |
|------|------|----------|
| `-Wall` | 常用警告（并非所有警告） | 基础 |
| `-Wextra` | 额外警告（补充-Wall未覆盖的） | 中等 |
| `-Werror` | 将警告视为错误，编译直接失败 | 严格 |
| `-Wpedantic` | 严格遵循ISO标准，不允许任何扩展 | 最严格 |
| `-Wshadow` | 变量遮蔽警告 | 推荐 |
| `-Wconversion` | 隐式类型转换警告 | 推荐 |
| `-Wold-style-cast` | C风格强制转换警告 | 推荐 |

**Sanitizer（消毒器）**：

| 选项 | 作用 | 开销 |
|------|------|------|
| `-fsanitize=address` | 地址消毒器：检测越界、use-after-free、双重释放 | 约2x |
| `-fsanitize=undefined` | 未定义行为检测：整数溢出、空指针解引用、对齐问题 | 约1.5x |
| `-fsanitize=thread` | 线程错误检测：数据竞争 | 约5-15x |
| `-fsanitize=memory` | 内存消毒器：检测未初始化读取 | 约3x |
| `-fsanitize=leak` | 泄漏检测：程序退出时检查内存泄漏 | 约1x |

**推荐的编译选项组合**：

```cmake
# CMakeLists.txt 中的推荐配置

# 开发模式：严格警告 + ASan + UBSan
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
add_compile_options(
    -Wall -Wextra -Werror -Wpedantic
    -Wshadow -Wconversion -Wold-style-cast
    -fsanitize=address,undefined
    -fno-omit-frame-pointer
)

# 发布模式：优化 + 警告
add_compile_options(
    -Wall -Wextra -Werror
    -O2 -DNDEBUG
)

# 测试模式：ASan + UBSan + 覆盖率
add_compile_options(
    -Wall -Wextra -Werror
    -fsanitize=address,undefined
    -fno-omit-frame-pointer
    --coverage
)
```

```bash
# 命令行快速使用
g++ -std=c++17 -Wall -Wextra -Werror -Wpedantic \
    -fsanitize=address,undefined \
    -fno-omit-frame-pointer \
    -g -O0 main.cpp -o main
```

***

### 4. 静态分析工具

#### 1. clang-tidy：C++代码规范和Bug检测

**常用检查项**：

| 类别 | 检查项 | 说明 |
|------|--------|------|
| 现代化 | `modernize-*` | 使用现代C++特性（auto、nullptr、range-for等） |
| Bug预防 | `bugprone-*` | 常见Bug模式（悬空指针、错误比较等） |
| 可读性 | `readability-*` | 命名规范、代码结构 |
| 性能 | `performance-*` | 性能优化建议（避免不必要的拷贝等） |
| C++核心指南 | `cppcoreguidelines-*` | 遵循C++ Core Guidelines |

**.clang-tidy配置示例**：

```yaml
Checks: >
  -*,
  modernize-*,
  bugprone-*,
  readability-*,
  performance-*,
  cppcoreguidelines-*,
  -modernize-use-trailing-return-type,
  -readability-magic-numbers,
  -cppcoreguidelines-avoid-magic-numbers
HeaderFilterRegex: 'src/.*'
WarningsAsErrors: '*'
FormatStyle: file
```

**命令行用法**：

```bash
# 对单个文件检查
clang-tidy src/main.cpp -p build

# 对整个项目检查（需要compile_commands.json）
clang-tidy -p build src/*.cpp

# 自动修复
clang-tidy -fix -p build src/main.cpp
```

#### 2. cppcheck：C/C++静态分析

**常见检测**：内存泄漏、空指针解引用、数组越界、未初始化变量、资源泄漏。

```bash
# 基本用法
cppcheck src/

# 启用所有检查
cppcheck --enable=all src/

# 指定C++标准
cppcheck --std=c++17 src/

# 输出为XML格式（CI集成用）
cppcheck --xml --output-file=report.xml src/

# 抑制特定警告
cppcheck --suppress=unusedFunction src/
```

#### 3. CPPLint：Google风格检查

```bash
# 基本用法
cpplint src/*.cpp src/*.h

# 指定过滤规则
cpplint --filter=-legal/copyright,-readability/streams src/*.cpp

# 设置行宽
cpplint --linelength=120 src/*.cpp
```

**工具对比**：

| 工具 | 侧重 | 速度 | 误报率 | 自动修复 |
|------|------|------|--------|----------|
| clang-tidy | 规范+Bug+现代化 | 中 | 低 | 支持 |
| cppcheck | Bug检测 | 快 | 中 | 不支持 |
| CPPLint | 风格规范 | 快 | 高 | 不支持 |

***

### 5. 代码格式化

#### 1. clang-format：自动格式化

**常用内置风格**：

| 风格 | 缩进 | 行宽 | 大括号位置 | 适用项目 |
|------|------|------|-----------|---------|
| LLVM | 2空格 | 80 | 下一行 | LLVM项目 |
| Google | 2空格 | 80 | 同一行 | Google项目 |
| Chromium | 2空格 | 80 | 同一行 | Chromium项目 |
| Mozilla | 2空格 | 80 | 下一行 | Mozilla项目 |
| WebKit | 4空格 | 80 | 下一行 | WebKit项目 |

**.clang-format配置示例**：

```yaml
BasedOnStyle: Google
IndentWidth: 4
ColumnLimit: 100
AllowShortFunctionsOnASingleLine: Inline
AllowShortIfStatementsOnASingleLine: false
BreakBeforeBraces: Attach
PointerAlignment: Left
ReferenceAlignment: Left
SortIncludes: CaseInsensitive
SpaceAfterCStyleCast: false
SpaceBeforeParens: ControlStatements
Standard: c++17
```

**命令行用法**：

```bash
# 格式化单个文件（原地修改）
clang-format -i src/main.cpp

# 格式化整个项目
find src -name "*.cpp" -o -name "*.h" | xargs clang-format -i

# 检查是否格式正确（不修改，仅报告差异）
clang-format --dry-run --Werror src/main.cpp
```

**与编辑器集成**：

| 编辑器 | 集成方式 |
|--------|---------|
| VS Code | 安装 `clang-format` 扩展，自动读取 `.clang-format` |
| CLion | 内置支持，Settings → Code Style → ClangFormat |
| Vim | `vim-clang-format` 插件 |
| Emacs | `clang-format.el` |

**Git pre-commit hook自动格式化**：

```bash
#!/bin/bash
# .git/hooks/pre-commit

FILES=$(git diff --cached --name-only --diff-filter=ACM \
    | grep -E '\.(cpp|h|cc|cxx)$')

if [ -n "$FILES" ]; then
    clang-format -i $FILES
    git add $FILES
fi
```

***

### 6. 调试工具进阶

#### 1. GDB：Linux标准调试器

**常用命令**：

| 命令 | 缩写 | 作用 |
|------|------|------|
| `break main` | `b main` | 在main函数设断点 |
| `break file.cpp:42` | `b file.cpp:42` | 在指定行设断点 |
| `run` | `r` | 运行程序 |
| `next` | `n` | 单步执行（不进入函数） |
| `step` | `s` | 单步执行（进入函数） |
| `print var` | `p var` | 打印变量值 |
| `backtrace` | `bt` | 打印调用栈 |
| `continue` | `c` | 继续运行 |
| `watch var` | - | 设置观察点（变量变化时暂停） |
| `info breakpoints` | `i b` | 查看所有断点 |
| `delete 1` | `d 1` | 删除断点1 |
| `quit` | `q` | 退出GDB |

**条件断点**：

```gdb
# 在第42行设断点，仅当x > 10时触发
break file.cpp:42 if x > 10

# 修改已有断点的条件
condition 1 x > 10
```

**观察点**：

```gdb
# 变量被修改时暂停
watch global_counter

# 变量被读取时暂停
rwatch config_value

# 变量被读取或修改时暂停
awatch status_flag
```

**多线程调试**：

```gdb
info threads              # 列出所有线程
thread 2                  # 切换到线程2
thread apply all bt       # 所有线程的调用栈
break file.cpp:42 thread 3  # 仅在线程3的该行设断点
```

#### 2. LLDB：macOS/Clang调试器

**GDB与LLDB命令对照**：

| 操作 | GDB | LLDB |
|------|-----|------|
| 运行 | `run` | `run` |
| 断点 | `break main` | `breakpoint set --name main` |
| 单步(过) | `next` | `next` |
| 单步(入) | `step` | `step` |
| 打印 | `print var` | `frame variable var` |
| 调用栈 | `backtrace` | `thread backtrace` |
| 查看线程 | `info threads` | `thread list` |
| 切换线程 | `thread 2` | `thread select 2` |
| 继续运行 | `continue` | `continue` |
| 附加进程 | `attach PID` | `process attach --pid PID` |

#### 3. Valgrind：内存错误检测

**memcheck**：检测内存泄漏、越界访问、use-after-free。

```bash
# 基本用法
valgrind --leak-check=full ./my_program

# 显示泄漏详情 + 追踪来源
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./my_program

# 抑制已知的误报
valgrind --suppressions=suppress.supp ./my_program
```

**输出解读**：

```
==12345== HEAP SUMMARY:
==12345==     in use at exit: 72 bytes in 3 blocks
==12345==   total heap usage: 5 allocs, 2 frees, 72 bytes allocated
==12345==
==12345== 72 bytes in 3 blocks are definitely lost in loss record 1 of 1
==12345==    at 0x4C2FB0F: malloc (in /usr/lib/valgrind/...)
==12345==    by 0x108A68: create_buffer (main.cpp:15)
==12345==    by 0x108ABF: main (main.cpp:30)
==12345==
==12345== LEAK SUMMARY:
==12345==    definitely lost: 72 bytes in 3 blocks   ← 确定泄漏
==12345==    indirectly lost: 0 bytes in 0 blocks
==12345==    possibly lost: 0 bytes in 0 blocks
==12345==    still reachable: 0 bytes in 0 blocks
==12345==    suppressed: 0 bytes in 0 blocks
```

#### 4. AddressSanitizer (ASan)：编译器内置内存检测

**比Valgrind快10-20倍**，是日常开发首选。

```cpp
#include <vector>
#include <iostream>

int main() {
    // 越界访问
    std::vector<int> v = {1, 2, 3};
    std::cout << v[5] << "\n";  // ASan会检测到越界

    // Use-after-free
    int* p = new int(42);
    delete p;
    std::cout << *p << "\n";  // ASan会检测到use-after-free

    // 栈缓冲区溢出
    int arr[3] = {1, 2, 3};
    std::cout << arr[5] << "\n";  // ASan会检测到栈溢出

    return 0;
}
```

编译运行：

```bash
g++ -fsanitize=address -fno-omit-frame-pointer -g main.cpp -o main
./main
# ASan会输出详细的错误报告，包含分配/释放的调用栈
```

**ASan vs Valgrind**：

| | ASan | Valgrind |
|---|---|---|
| 速度 | 约2x慢 | 约20-50x慢 |
| 检测范围 | 越界、use-after-free、双重释放 | 内存泄漏、越界、未初始化读取 |
| 内存泄漏检测 | 需要额外配置 | 默认支持 |
| 使用方式 | 编译时注入 | 运行时检测，无需重编译 |
| 适用场景 | 日常开发 | 无法重编译的第三方库 |

***

### 7. 性能分析

#### 1. perf：Linux性能分析

```bash
# 统计性能事件
perf stat ./my_program

# 记录性能数据
perf record ./my_program

# 分析热点
perf report

# 记录特定事件
perf stat -e cache-misses,branch-misses ./my_program

# 附加到运行中的进程
perf record -p PID sleep 10
```

**常用事件**：

| 事件 | 说明 |
|------|------|
| `cycles` | CPU周期数 |
| `instructions` | 指令数 |
| `cache-misses` | 缓存未命中 |
| `branch-misses` | 分支预测失败 |
| `page-faults` | 页错误 |

#### 2. Callgrind：函数调用分析

```bash
# 记录调用数据
valgrind --tool=callgrind ./my_program

# 用KCachegrind可视化
kcachegrind callgrind.out.*

# 只关注特定函数
valgrind --tool=callgrind --callgrind-out-file=profile.out ./my_program
```

#### 3. gprof：经典性能分析

```bash
# 编译时加入pg选项
g++ -pg -o my_program main.cpp

# 运行程序（自动生成gmon.out）
./my_program

# 分析结果
gprof my_program gmon.out > analysis.txt
```

#### 4. Google Benchmark：C++微基准测试

```cpp
#include <benchmark/benchmark.h>
#include <vector>
#include <algorithm>

static void BM_SortVector(benchmark::State& state) {
    int size = state.range(0);
    for (auto _ : state) {
        state.PauseTiming();
        std::vector<int> v(size);
        for (int i = 0; i < size; i++) v[i] = size - i;
        state.ResumeTiming();

        std::sort(v.begin(), v.end());
        benchmark::DoNotOptimize(v.data());
    }
    state.SetComplexityN(size);
}

BENCHMARK(BM_SortVector)
    ->Arg(100)->Arg(1000)->Arg(10000)
    ->Complexity(benchmark::oNLogN);

static void BM_StringConcat(benchmark::State& state) {
    for (auto _ : state) {
        std::string result;
        for (int i = 0; i < state.range(0); i++) {
            result += "x";
        }
        benchmark::DoNotOptimize(result);
    }
}

BENCHMARK(BM_StringConcat)->Range(8, 8192);

BENCHMARK_MAIN();
```

编译运行：

```bash
g++ -std=c++17 -O2 benchmark_demo.cpp -lbenchmark -lpthread -o benchmark_demo
./benchmark_demo
```

***

### 8. 构建系统最佳实践

#### 1. CMake规范

**现代CMake（target-based）**：

```cmake
cmake_minimum_required(VERSION 3.20)
project(MyProject VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(mylib
    src/core.cpp
    src/utils.cpp
)

target_include_directories(mylib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(mylib PRIVATE
    -Wall -Wextra -Werror -Wpedantic
)

add_executable(myapp src/main.cpp)
target_link_libraries(myapp PRIVATE mylib)
```

**FetchContent vs find_package**：

| 方式 | 用途 | 说明 |
|------|------|------|
| `find_package` | 查找系统已安装的库 | 适合稳定发布的第三方库 |
| `FetchContent` | 下载并构建源码 | 适合未安装的库或子项目 |

```cmake
# find_package方式
find_package(fmt REQUIRED)
target_link_libraries(myapp PRIVATE fmt::fmt)

# FetchContent方式
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)
target_link_libraries(myapp PRIVATE GTest::gtest)
```

**CMakePresets.json**：

```json
{
    "version": 6,
    "cmakeMinimumRequired": { "major": 3, "minor": 20, "patch": 0 },
    "configurePresets": [
        {
            "name": "dev",
            "binaryDir": "${sourceDir}/build",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug",
                "CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
            }
        },
        {
            "name": "release",
            "binaryDir": "${sourceDir}/build-release",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Release"
            }
        },
        {
            "name": "asan",
            "binaryDir": "${sourceDir}/build-asan",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug",
                "CMAKE_CXX_FLAGS": "-fsanitize=address,undefined -fno-omit-frame-pointer"
            }
        }
    ],
    "buildPresets": [
        { "name": "dev", "configurePreset": "dev" },
        { "name": "release", "configurePreset": "release" },
        { "name": "asan", "configurePreset": "asan" }
    ]
}
```

#### 2. vcpkg：C++包管理

```bash
# 安装vcpkg
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg && bootstrap-vcpkg.bat    # Windows
cd vcpkg && ./bootstrap-vcpkg.sh   # Linux/macOS

# 安装库
vcpkg install fmt
vcpkg install spdlog
vcpkg install nlohmann-json

# CMake集成
cmake -B build -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

**manifest模式（vcpkg.json）**：

```json
{
    "name": "myproject",
    "version": "1.0.0",
    "dependencies": [
        "fmt",
        "spdlog",
        "nlohmann-json"
    ]
}
```

#### 3. ccache：编译缓存加速

```bash
# 安装
sudo apt install ccache    # Linux
brew install ccache        # macOS

# CMake集成
set(CMAKE_CXX_COMPILER_LAUNCHER ccache)

# 效果：二次编译速度提升5-10倍
```

#### 4. Ninja：快速构建后端

```bash
# 安装
sudo apt install ninja-build    # Linux
brew install ninja              # macOS

# CMake使用Ninja
cmake -G Ninja -B build
cmake --build build
```

**Ninja vs Make**：

| | Ninja | Make |
|---|---|---|
| 速度 | 快（并行构建优化） | 较慢 |
| 输出 | 简洁 | 冗长 |
| 适用 | 大型项目 | 通用 |

***

### 9. 单元测试

#### 1. Google Test：最流行的C++测试框架

```cpp
#include <gtest/gtest.h>
#include <string>

int add(int a, int b) { return a + b; }
int divide(int a, int b) {
    if (b == 0) throw std::invalid_argument("division by zero");
    return a / b;
}

class CalculatorTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    void TearDown() override {
    }
};

TEST(AddTest, PositiveNumbers) {
    EXPECT_EQ(add(2, 3), 5);
    EXPECT_NE(add(2, 3), 6);
    EXPECT_GT(add(2, 3), 4);
}

TEST(AddTest, NegativeNumbers) {
    EXPECT_EQ(add(-2, -3), -5);
}

TEST_F(CalculatorTest, Division) {
    EXPECT_EQ(divide(10, 2), 5);
    EXPECT_THROW(divide(10, 0), std::invalid_argument);
    EXPECT_ANY_THROW(divide(10, 0));
    EXPECT_NO_THROW(divide(10, 2));
}

TEST_P(ParamTest, VariousInputs) {
    int input = GetParam();
    EXPECT_EQ(add(input, 0), input);
}

INSTANTIATE_TEST_SUITE_P(
    VariousValues, ParamTest,
    ::testing::Values(0, 1, -1, 100, -100)
);
```

**EXPECT vs ASSERT**：

| 宏 | 失败行为 | 适用场景 |
|----|---------|---------|
| `EXPECT_*` | 记录失败，继续执行 | 非致命断言，收集多个错误 |
| `ASSERT_*` | 记录失败，立即停止 | 致命断言，后续代码依赖该条件 |

**CMake集成**：

```cmake
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(mytest test/main_test.cpp)
target_link_libraries(mytest PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(mytest)
```

#### 2. Catch2：轻量级测试框架

```cpp
#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

TEST_CASE("Factorial computes correctly", "[factorial]") {
    REQUIRE(factorial(0) == 1);
    REQUIRE(factorial(1) == 1);
    REQUIRE(factorial(5) == 120);

    SECTION("Negative input throws") {
        REQUIRE_THROWS_AS(factorial(-1), std::invalid_argument);
    }

    SECTION("Large input") {
        REQUIRE(factorial(10) == 3628800);
    }
}

TEST_CASE("BDD style test", "[bdd]") {
    int value = 0;

    GIVEN("An initial value of 0") {
        value = 0;

        WHEN("We add 5") {
            value += 5;

            THEN("The value should be 5") {
                REQUIRE(value == 5);
            }
        }
    }
}
```

#### 3. doctest：最快编译的测试框架

```cpp
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_CASE("Quick test") {
    CHECK(1 + 1 == 2);
    CHECK_FALSE(1 + 1 == 3);
    CHECK_THROWS(throw std::runtime_error("error"));
}
```

**框架对比**：

| 框架 | 编译速度 | 功能丰富度 | C++版本 | 头文件only |
|------|---------|-----------|---------|-----------|
| Google Test | 中 | 丰富 | C++11 | 否 |
| Catch2 | 慢 | 丰富 | C++14 | 是 |
| doctest | 快 | 中等 | C++11 | 是 |

***

### 10. 持续集成

#### 1. GitHub Actions for C++

**多平台构建 + 测试 + 静态分析**：

```yaml
name: C++ CI

on:
  push:
    branches: [main]
  pull_request:
    branches: [main]

jobs:
  build:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, macos-latest, windows-latest]
        build_type: [Debug, Release]
      fail-fast: false

    steps:
      - uses: actions/checkout@v4

      - name: Install dependencies (Linux)
        if: runner.os == 'Linux'
        run: sudo apt install -y libboost-dev

      - name: Configure CMake
        run: |
          cmake -B build -DCMAKE_BUILD_TYPE=${{ matrix.build_type }} \
                -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

      - name: Build
        run: cmake --build build --config ${{ matrix.build_type }}

      - name: Test
        working-directory: build
        run: ctest --build-config ${{ matrix.build_type }} --output-on-failure

  clang-tidy:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Install clang-tidy
        run: sudo apt install -y clang-tidy

      - name: Configure
        run: cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

      - name: Run clang-tidy
        run: clang-tidy -p build src/*.cpp

  coverage:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Configure with coverage
        run: |
          cmake -B build -DCMAKE_BUILD_TYPE=Debug \
                -DCMAKE_CXX_FLAGS="--coverage -fno-omit-frame-pointer"

      - name: Build and Test
        run: |
          cmake --build build
          cd build && ctest --output-on-failure

      - name: Generate coverage report
        run: |
          lcov --capture --directory build --output-file coverage.info
          lcov --remove coverage.info '/usr/*' '*/test/*' --output-file coverage.info
          lcov --list coverage.info

      - name: Upload coverage
        uses: codecov/codecov-action@v3
        with:
          files: coverage.info
```

***

### 11. 工具链速查表

| 需求 | 工具 | 命令/用法 |
|------|------|---------|
| 编译警告 | gcc/clang | `-Wall -Wextra -Werror` |
| 内存检测 | ASan | `-fsanitize=address` |
| UB检测 | UBSan | `-fsanitize=undefined` |
| 线程检测 | TSan | `-fsanitize=thread` |
| 静态分析 | clang-tidy | `clang-tidy -p build` |
| 格式化 | clang-format | `clang-format -i` |
| 内存泄漏 | Valgrind | `valgrind --leak-check=full` |
| 性能分析 | perf | `perf record/report` |
| 调用分析 | Callgrind | `valgrind --tool=callgrind` |
| 微基准 | Google Benchmark | `cmake + benchmark` |
| 单元测试 | Google Test | `cmake + GTest` |
| 轻量测试 | Catch2 | `#include <catch2/catch_test_macros.hpp>` |
| 包管理 | vcpkg | `vcpkg install` |
| 构建系统 | CMake + Ninja | `cmake -G Ninja -B build` |
| 编译缓存 | ccache | `CMAKE_CXX_COMPILER_LAUNCHER=ccache` |
| CI | GitHub Actions | `.github/workflows/ci.yml` |

***

### 12. 极简总结

**工具链 = 让机器做苦力。必用：`-Wall -Wextra -Werror` + ASan + clang-tidy + clang-format。调试用GDB/LLDB + ASan，性能用perf/Valgrind，测试用GTest/Catch2，构建用现代CMake + vcpkg。原则：编译器能检查的不要靠人眼。**

| 阶段 | 必备工具 | 一句话 |
|------|---------|--------|
| 编译 | `-Wall -Wextra -Werror` + ASan | 让编译器帮你找Bug |
| 静态分析 | clang-tidy + cppcheck | 编译前再过一遍 |
| 格式化 | clang-format | 风格统一，不再争论 |
| 调试 | GDB/LLDB + ASan | 运行时精准定位 |
| 性能 | perf + Google Benchmark | 用数据说话 |
| 测试 | GTest/Catch2 | 每个函数都该有测试 |
| 构建 | CMake + Ninja + vcpkg | 现代化构建流水线 |
| CI | GitHub Actions | 每次提交自动验证 |

***

### 相关阅读

- [什么是性能剖析Profiling](09-什么是性能剖析Profiling.md)
- [什么是基准测试Benchmarking](./10-什么是基准测试Benchmarking.md)
- [段错误排查](01-段错误排查.md)

***