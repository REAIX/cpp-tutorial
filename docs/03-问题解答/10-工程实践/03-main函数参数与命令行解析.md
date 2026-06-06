# main函数参数与命令行解析
> 📖 相关章节：[编码规范](../../04-工程实践/00-编码规范.md)、[设计模式](../../04-工程实践/03-设计模式.md)、[单元测试](../../04-工程实践/06-单元测试.md)、[代码审查](../../04-工程实践/08-代码审查.md)

### 1. 要义概览

**main(argc, argv)是命令行参数的入口。简单场景手动遍历argv即可，复杂场景用getopt(C)或cxxopts/CLI11(C++)。核心原则：先检查帮助/版本，再解析选项，最后处理位置参数。**

***

### 2. 核心定义

| | argc | argv |
|---|---|---|
| 全称 | argument count | argument vector |
| 是什么 | 参数个数（包括程序名） | 参数字符串数组 |
| 类型 | int | char* []（字符串指针数组） |
| 最小值 | 1（至少有程序名） | 至少包含argv[0] |
| 结尾标志 | 无 | argv[argc] == NULL |

**本质**：

```cpp
// main 的两个参数就是命令行传入的所有内容
int main(int argc, char* argv[]) {
    // argc = 参数个数（包括程序名本身）
    // argv = 参数字符串数组
    //   argv[0]        = 程序名本身
    //   argv[1] ~ argv[argc-1] = 实际参数
    //   argv[argc]     = NULL（哨兵）
    return 0;
}
```

***

### 3. 生活类比

| | 命令行参数 | 餐厅点菜 |
|---|---|---|
| 程序 | 厨师 | 厨师 |
| argc | 点了几个菜（包括厨师工号） | 点了3个菜 |
| argv | 菜单列表 | ["厨师01号", "宫保鸡丁", "鱼香肉丝"] |
| argv[0] | 程序名 | 厨师工号（必须带上，知道谁在做） |
| argv[1+] | 实际参数 | 具体菜品 |
| 厨师根据菜单做菜 | 程序根据参数执行不同逻辑 | 点宫保鸡丁就做宫保鸡丁 |

**具体场景**：

- **命令行参数**：你走进餐厅跟厨师说"来份宫保鸡丁、微辣、加饭"。厨师（程序）根据你说的内容（参数）做不同的菜（执行不同逻辑）。argc告诉你一共说了几句话，argv是每句话的具体内容。
- **没有参数**：你走进餐厅啥也没说，厨师就做默认的今日套餐（程序执行默认行为）。

***

### 4. argc和argv详解

**argc的值**：至少为1（只有程序名）

**argv的结构**：字符串数组，以NULL结尾

```cpp
#include <iostream>
using namespace std;

int main(int argc, char* argv[]) {
    cout << "参数个数: " << argc << "\n";
    for (int i = 0; i < argc; i++) {
        cout << "argv[" << i << "] = " << argv[i] << "\n";
    }
    cout << "argv[" << argc << "] = "
         << (argv[argc] == nullptr ? "NULL" : "非NULL") << "\n";
    return 0;
}
```

**常见命令行格式**：

| 命令行 | argc | argv内容 |
|--------|------|----------|
| `./program` | 1 | ["./program"] |
| `./program -h` | 2 | ["./program", "-h"] |
| `./program input.txt` | 2 | ["./program", "input.txt"] |
| `./program -o out.txt -v 3` | 5 | ["./program", "-o", "out.txt", "-v", "3"] |
| `./program --input file.txt -v` | 4 | ["./program", "--input", "file.txt", "-v"] |

**argv的内存布局**：

```
argv[0] ──→ "./program\0"
argv[1] ──→ "-o\0"
argv[2] ──→ "out.txt\0"
argv[3] ──→ "-v\0"
argv[4] ──→ "3\0"
argv[5] ──→ NULL          ← 哨兵，标志数组结束
```

***

### 5. 手动解析参数（基础方式）

逐个遍历argv，用strcmp判断参数。

```cpp
#include <iostream>
#include <cstring>
using namespace std;

int main(int argc, char* argv[]) {
    bool verbose = false;
    const char* output_file = nullptr;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            cout << "Usage: " << argv[0] << " [options]\n";
            cout << "  -h, --help     Show help\n";
            cout << "  -v, --version  Show version\n";
            cout << "  -o, --output   Output file\n";
            return 0;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            cout << "v1.0.0\n";
            return 0;
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (i + 1 < argc) {
                output_file = argv[++i];
            } else {
                cerr << "Error: -o requires an argument\n";
                return 1;
            }
        } else if (strcmp(argv[i], "--verbose") == 0) {
            verbose = true;
        } else {
            cerr << "Unknown option: " << argv[i] << "\n";
            return 1;
        }
    }

    cout << "verbose=" << verbose << ", output=" << (output_file ? output_file : "none") << "\n";
    return 0;
}
```

**手动解析的问题**：

| 问题 | 说明 |
|------|------|
| 代码冗长 | 每个选项都要写strcmp判断 |
| 容易出错 | 忘记检查i+1越界、拼写错误 |
| 不支持短选项合并 | `-abc`无法自动拆分为`-a -b -c` |
| 不支持参数值绑定 | `-ofile`无法识别为`-o file` |
| 不支持`--option=value` | 需要手动拆分等号 |

***

### 6. 常见参数模式

| 模式 | 示例 | 说明 |
|------|------|------|
| 短选项 | `-h`, `-v`, `-o file` | 单字母，用`-`前缀 |
| 长选项 | `--help`, `--output=file` | 完整单词，用`--`前缀 |
| 位置参数 | `./program input.txt` | 不带前缀的参数 |
| 标志选项 | `-v`, `--verbose` | 不带值的开关 |
| 带值选项 | `-o output.txt` | 选项后面跟一个值 |
| 合并短选项 | `-abc` 等同 `-a -b -c` | 多个短选项合并 |
| 等号赋值 | `--output=file.txt` | 长选项用等号绑定值 |

**参数解析顺序原则**：

```
1. 先检查 -h/--help（帮助信息，立即退出）
2. 再检查 -v/--version（版本信息，立即退出）
3. 然后解析选项（-o file, --verbose 等）
4. 最后处理位置参数（剩余的非选项参数）
```

***

### 7. 使用getopt解析（C语言标准方式）

POSIX标准函数`getopt()`，用于解析短选项。

```cpp
#include <iostream>
#include <unistd.h>
#include <cstring>
extern char* optarg;
extern int optind, opterr, optopt;

int main(int argc, char* argv[]) {
    bool verbose = false;
    const char* output_file = nullptr;
    int opt;

    // "vo:h" → -v无参数, -o需参数, -h无参数
    while ((opt = getopt(argc, argv, "vo:h")) != -1) {
        switch (opt) {
            case 'v':
                verbose = true;
                break;
            case 'o':
                output_file = optarg;
                break;
            case 'h':
                std::cout << "Usage: " << argv[0] << " [-v] [-o file] [-h]\n";
                return 0;
            case '?':
                std::cerr << "Unknown option: -" << (char)optopt << "\n";
                return 1;
            case ':':
                std::cerr << "Missing argument for -" << (char)optopt << "\n";
                return 1;
        }
    }

    // optind 指向第一个非选项参数
    for (int i = optind; i < argc; i++) {
        std::cout << "Positional arg: " << argv[i] << "\n";
    }

    std::cout << "verbose=" << verbose
              << ", output=" << (output_file ? output_file : "none") << "\n";
    return 0;
}
```

**getopt全局变量**：

| 变量 | 说明 |
|------|------|
| `optarg` | 当前选项的参数值 |
| `optind` | 下一个待处理的argv索引 |
| `opterr` | 设为0可抑制错误信息 |
| `optopt` | 最后一个未知选项字符 |

**选项字符串规则**：

| 字符串 | 含义 |
|--------|------|
| `"abc"` | -a -b -c 都无参数 |
| `"a:b:c"` | -a需参数，-b需参数，-c无参数 |
| `"a::b"` | -a可选参数，-b无参数 |

**优点**：标准、POSIX系统通用、支持短选项合并
**缺点**：Windows不原生支持、不支持长选项

***

### 8. 使用getopt_long（GNU扩展）

支持长选项的GNU扩展，同时兼容短选项。

```cpp
#include <iostream>
#include <getopt.h>
#include <cstring>

int main(int argc, char* argv[]) {
    bool verbose = false;
    const char* input_file = nullptr;
    const char* output_file = nullptr;

    static struct option long_options[] = {
        {"verbose", no_argument,       0, 'v'},
        {"input",   required_argument, 0, 'i'},
        {"output",  required_argument, 0, 'o'},
        {"help",    no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "vi:o:h",
                              long_options, &option_index)) != -1) {
        switch (opt) {
            case 'v':
                verbose = true;
                break;
            case 'i':
                input_file = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            case 'h':
                std::cout << "Usage: " << argv[0] << " [options]\n"
                          << "  -v, --verbose       Verbose mode\n"
                          << "  -i, --input FILE    Input file\n"
                          << "  -o, --output FILE   Output file\n"
                          << "  -h, --help          Show help\n";
                return 0;
            case '?':
                return 1;
        }
    }

    for (int i = optind; i < argc; i++) {
        std::cout << "Positional arg: " << argv[i] << "\n";
    }

    std::cout << "verbose=" << verbose
              << ", input=" << (input_file ? input_file : "none")
              << ", output=" << (output_file ? output_file : "none") << "\n";
    return 0;
}
```

**option结构体字段**：

| 字段 | 说明 |
|------|------|
| `name` | 长选项名称（不含`--`） |
| `has_arg` | `no_argument` / `required_argument` / `optional_argument` |
| `flag` | 设为0则返回val，否则将val存入*flag |
| `val` | 返回值或存入*flag的值 |

***

### 9. C++方式：第三方库

**cxxopts**：轻量级C++命令行解析库（头文件only）

```cpp
#include <iostream>
#include <cxxopts.hpp>

int main(int argc, char* argv[]) {
    cxxopts::Options options("mytool", "A file processing tool");

    options.add_options()
        ("v,verbose", "Verbose output", cxxopts::value<bool>()->default_value("false"))
        ("i,input", "Input file", cxxopts::value<std::string>())
        ("o,output", "Output file", cxxopts::value<std::string>())
        ("h,help", "Show help")
        ("n,count", "Process count", cxxopts::value<int>()->default_value("1"));

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << "\n";
        return 0;
    }

    bool verbose = result["verbose"].as<bool>();
    int count = result["count"].as<int>();
    std::string input = result.count("input") ? result["input"].as<std::string>() : "";
    std::string output = result.count("output") ? result["output"].as<std::string>() : "";

    std::cout << "verbose=" << verbose
              << ", count=" << count
              << ", input=" << input
              << ", output=" << output << "\n";
    return 0;
}
```

**CLI11**：功能更丰富的C++命令行解析库

```cpp
#include <iostream>
#include <CLI/App.hpp>
#include <CLI/Formatter.hpp>
#include <CLI/Config.hpp>

int main(int argc, char* argv[]) {
    CLI::App app{"A file processing tool", "mytool"};

    bool verbose = false;
    std::string input_file;
    std::string output_file;
    int count = 1;

    app.add_flag("-v,--verbose", verbose, "Verbose output");
    app.add_option("-i,--input", input_file, "Input file")->required()->check(CLI::ExistingFile);
    app.add_option("-o,--output", output_file, "Output file");
    app.add_option("-n,--count", count, "Process count")->default_val(1)->check(CLI::PositiveNumber);

    CLI11_PARSE(app, argc, argv);

    std::cout << "verbose=" << verbose
              << ", count=" << count
              << ", input=" << input_file
              << ", output=" << output_file << "\n";
    return 0;
}
```

**第三方库对比**：

| 库 | 大小 | 功能 | 易用性 | C++版本 | 头文件only |
|---|---|---|---|---|---|
| **cxxopts** | 小 | 中等 | 高 | C++11 | 是 |
| **CLI11** | 中 | 丰富 | 高 | C++11 | 是 |
| **argparse** | 小 | 中等 | 高（Python风格） | C++17 | 是 |
| **gflags** | 大 | 丰富（Google风格） | 中 | C++11 | 否 |
| **boost.program_options** | 大 | 非常丰富 | 中 | C++11 | 否 |

**选择建议**：

| 场景 | 推荐 |
|------|------|
| 小项目、快速上手 | cxxopts |
| 中大型项目、需要验证 | CLI11 |
| Python风格偏好 | argparse |
| Google生态项目 | gflags |
| 已用Boost的项目 | boost.program_options |

***

### 10. 实战：实现一个完整的命令行工具

需求：实现一个文件处理工具，支持：
- `./tool -h` 显示帮助
- `./tool -v` 显示版本
- `./tool -i input.txt -o output.txt --verbose` 处理文件
- `./tool --count 10 input.txt` 指定次数处理

**手动解析版本**：

```cpp
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
using namespace std;

void showHelp(const char* prog) {
    cout << "Usage: " << prog << " [options] [input_file]\n"
         << "Options:\n"
         << "  -h, --help          Show this help\n"
         << "  -v, --version       Show version\n"
         << "  -i, --input FILE    Input file\n"
         << "  -o, --output FILE   Output file\n"
         << "  -n, --count N       Process count (default: 1)\n"
         << "      --verbose       Verbose output\n";
}

void showVersion() {
    cout << "mytool v1.0.0\n";
}

int main(int argc, char* argv[]) {
    string input_file;
    string output_file;
    int count = 1;
    bool verbose = false;

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            showHelp(argv[0]);
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            showVersion();
            return 0;
        } else if (arg == "-i" || arg == "--input") {
            if (i + 1 >= argc) {
                cerr << "Error: " << arg << " requires an argument\n";
                return 1;
            }
            input_file = argv[++i];
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 >= argc) {
                cerr << "Error: " << arg << " requires an argument\n";
                return 1;
            }
            output_file = argv[++i];
        } else if (arg == "-n" || arg == "--count") {
            if (i + 1 >= argc) {
                cerr << "Error: " << arg << " requires an argument\n";
                return 1;
            }
            count = stoi(argv[++i]);
            if (count <= 0) {
                cerr << "Error: count must be positive\n";
                return 1;
            }
        } else if (arg == "--verbose") {
            verbose = true;
        } else if (arg[0] == '-') {
            cerr << "Error: unknown option " << arg << "\n";
            return 1;
        } else {
            input_file = arg;
        }
    }

    if (input_file.empty()) {
        cerr << "Error: input file is required\n";
        showHelp(argv[0]);
        return 1;
    }

    if (verbose) {
        cout << "Input:  " << input_file << "\n";
        cout << "Output: " << (output_file.empty() ? "stdout" : output_file) << "\n";
        cout << "Count:  " << count << "\n";
    }

    for (int i = 0; i < count; i++) {
        ifstream fin(input_file);
        if (!fin) {
            cerr << "Error: cannot open " << input_file << "\n";
            return 1;
        }

        if (output_file.empty()) {
            cout << fin.rdbuf();
        } else {
            ofstream fout(output_file, i > 0 ? ios::app : ios::out);
            fout << fin.rdbuf();
        }

        if (verbose) {
            cout << "Pass " << (i + 1) << "/" << count << " done\n";
        }
    }

    return 0;
}
```

**cxxopts版本**：

```cpp
#include <iostream>
#include <fstream>
#include <string>
#include <cxxopts.hpp>
using namespace std;

int main(int argc, char* argv[]) {
    cxxopts::Options options("mytool", "A file processing tool");

    options.add_options()
        ("h,help", "Show help")
        ("v,version", "Show version")
        ("i,input", "Input file", cxxopts::value<string>())
        ("o,output", "Output file", cxxopts::value<string>())
        ("n,count", "Process count", cxxopts::value<int>()->default_value("1"))
        ("verbose", "Verbose output");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        cout << options.help() << "\n";
        return 0;
    }
    if (result.count("version")) {
        cout << "mytool v1.0.0\n";
        return 0;
    }

    if (!result.count("input")) {
        cerr << "Error: input file is required\n";
        cout << options.help() << "\n";
        return 1;
    }

    string input_file = result["input"].as<string>();
    string output_file = result.count("output") ? result["output"].as<string>() : "";
    int count = result["count"].as<int>();
    bool verbose = result.count("verbose") > 0;

    if (verbose) {
        cout << "Input:  " << input_file << "\n";
        cout << "Output: " << (output_file.empty() ? "stdout" : output_file) << "\n";
        cout << "Count:  " << count << "\n";
    }

    for (int i = 0; i < count; i++) {
        ifstream fin(input_file);
        if (!fin) {
            cerr << "Error: cannot open " << input_file << "\n";
            return 1;
        }

        if (output_file.empty()) {
            cout << fin.rdbuf();
        } else {
            ofstream fout(output_file, i > 0 ? ios::app : ios::out);
            fout << fin.rdbuf();
        }

        if (verbose) {
            cout << "Pass " << (i + 1) << "/" << count << " done\n";
        }
    }

    return 0;
}
```

**两个版本对比**：

| | 手动解析 | cxxopts |
|---|---|---|
| 代码量 | 多（~80行解析逻辑） | 少（~20行解析逻辑） |
| 错误处理 | 需手动检查每个参数 | 自动生成错误信息 |
| 帮助信息 | 手动编写 | 自动生成 |
| 可维护性 | 差（加选项要改多处） | 好（加一行add_options） |
| 依赖 | 无 | 需引入头文件 |
| 适用场景 | 简单工具、教学 | 生产项目 |

***

### 11. Windows下的命令行参数

**Windows的宽字符问题**：Windows控制台默认使用UTF-16编码，`argv`是窄字符（ANSI），可能丢失非ASCII字符。

**方式1：wmain()获取宽字符参数**

```cpp
#include <iostream>
#include <windows.h>
using namespace std;

int wmain(int argc, wchar_t* argv[]) {
    wcout << L"参数个数: " << argc << "\n";
    for (int i = 0; i < argc; i++) {
        wcout << L"argv[" << i << L"] = " << argv[i] << "\n";
    }

    // 宽字符转窄字符（如果需要传给窄字符API）
    for (int i = 0; i < argc; i++) {
        int len = WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, nullptr, 0, nullptr, nullptr);
        char* buf = new char[len];
        WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, buf, len, nullptr, nullptr);
        cout << "UTF-8: " << buf << "\n";
        delete[] buf;
    }

    return 0;
}
```

**方式2：GetCommandLineW()获取原始命令行**

```cpp
#include <iostream>
#include <windows.h>
#include <shellapi.h>
using namespace std;

int main() {
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    wcout << L"参数个数: " << argc << "\n";
    for (int i = 0; i < argc; i++) {
        wcout << L"argv[" << i << L"] = " << argv[i] << "\n";
    }

    LocalFree(argv);
    return 0;
}
```

**Windows入口函数对比**：

| 入口函数 | 字符集 | 用途 |
|----------|--------|------|
| `main()` | ANSI（窄字符） | 控制台程序 |
| `wmain()` | Unicode（宽字符） | 控制台程序（宽字符版） |
| `WinMain()` | ANSI | GUI程序（无控制台窗口） |
| `wWinMain()` | Unicode | GUI程序（宽字符版） |
| `_tmain()` | 根据UNICODE宏决定 | 兼容写法（TCHAR映射） |

**跨平台建议**：

| 平台 | 推荐 | 说明 |
|------|------|------|
| Linux/macOS | `main(int argc, char* argv[])` | 标准方式，UTF-8编码 |
| Windows | `main(int argc, char* argv[])` + `_setmode` | 简单项目可用窄字符 |
| Windows（需Unicode） | `wmain()` 或 `GetCommandLineW()` | 处理中文路径等场景 |
| 跨平台 | `main(int argc, char* argv[])` | 统一接口，Windows下用UTF-8代码页 |

***

### 12. 极简总结

**main(argc, argv)是命令行参数的入口。简单场景手动遍历argv即可，复杂场景用getopt(C)或cxxopts/CLI11(C++)。核心原则：先检查帮助/版本，再解析选项，最后处理位置参数。**

| 方式 | 适用场景 | 复杂度 | 平台 |
|------|---------|--------|------|
| 手动遍历argv | 1-3个选项的简单工具 | 低 | 全平台 |
| getopt | C语言项目、POSIX系统 | 中 | Linux/macOS |
| getopt_long | C语言项目、需长选项 | 中 | Linux/macOS |
| cxxopts | C++项目、轻量需求 | 低 | 全平台 |
| CLI11 | C++项目、丰富功能 | 中 | 全平台 |

***

### 相关阅读

- [程序中执行命令与文件目录操作](./10-程序中执行命令与文件目录操作.md)
- [CPP编码规范与命名约定](./04-CPP编码规范与命名约定.md)
- [程序自我更新与删除卸载](./09-程序自我更新与删除卸载.md)

***