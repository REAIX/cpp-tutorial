/** @file 01_deep_dive_filesystem_patterns.cpp
 *  @brief 目录遍历模式、文件搜索、路径操作、跨平台路径处理
 *  @description 对应文档: 02-CPP/19-filesystem | 举一反三：文件系统常用模式和跨平台技巧
 *  编译命令: g++ -std=c++20 01_deep_dive_filesystem_patterns.cpp -o 01_deep_dive_filesystem_patterns
 */

#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <regex>
#include <chrono>
#include <iomanip>
#include <ctime>

namespace fs = std::filesystem;

const fs::path TEST_DIR = "fs_patterns_test";

void setup() {
    fs::create_directories(TEST_DIR / "src" / "utils");
    fs::create_directories(TEST_DIR / "include");
    fs::create_directories(TEST_DIR / "docs");
    fs::create_directories(TEST_DIR / "build");

    std::ofstream(TEST_DIR / "src" / "main.cpp") << "int main() {}";
    std::ofstream(TEST_DIR / "src" / "app.cpp") << "// app";
    std::ofstream(TEST_DIR / "src" / "utils" / "helper.cpp") << "// helper";
    std::ofstream(TEST_DIR / "src" / "utils" / "math.cpp") << "// math";
    std::ofstream(TEST_DIR / "include" / "app.h") << "#pragma once";
    std::ofstream(TEST_DIR / "include" / "utils.h") << "#pragma once";
    std::ofstream(TEST_DIR / "docs" / "README.md") << "# README";
    std::ofstream(TEST_DIR / "docs" / "CHANGELOG.md") << "# Changelog";
    std::ofstream(TEST_DIR / "CMakeLists.txt") << "cmake_minimum_required(VERSION 3.10)";
    std::ofstream(TEST_DIR / "config.json") << "{}";
    std::ofstream(TEST_DIR / "build" / "output.log") << "Build log";
}

void demo_traversal_patterns() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  目录遍历模式\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. 按类型遍历:\n";
    std::vector<fs::path> files, dirs;
    for (const auto& entry : fs::recursive_directory_iterator(TEST_DIR)) {
        if (entry.is_regular_file()) files.push_back(entry.path());
        else if (entry.is_directory()) dirs.push_back(entry.path());
    }
    std::cout << "  文件(" << files.size() << "): ";
    for (const auto& f : files) std::cout << f.filename().string() << " ";
    std::cout << "\n  目录(" << dirs.size() << "): ";
    for (const auto& d : dirs) std::cout << d.filename().string() << " ";
    std::cout << "\n\n";

    std::cout << "2. 按扩展名过滤:\n";
    auto find_by_extension = [](const fs::path& dir, const std::string& ext) {
        std::vector<fs::path> result;
        for (const auto& entry : fs::recursive_directory_iterator(dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ext) {
                result.push_back(entry.path());
            }
        }
        return result;
    };

    auto cpp_files = find_by_extension(TEST_DIR, ".cpp");
    std::cout << "  .cpp 文件: ";
    for (const auto& f : cpp_files) std::cout << f.filename().string() << " ";
    std::cout << "\n";

    auto h_files = find_by_extension(TEST_DIR, ".h");
    std::cout << "  .h 文件: ";
    for (const auto& f : h_files) std::cout << f.filename().string() << " ";
    std::cout << "\n\n";

    std::cout << "3. 按大小排序:\n";
    std::vector<std::pair<std::uintmax_t, fs::path>> sized_files;
    for (const auto& entry : fs::recursive_directory_iterator(TEST_DIR)) {
        if (entry.is_regular_file()) {
            sized_files.emplace_back(entry.file_size(), entry.path());
        }
    }
    std::sort(sized_files.begin(), sized_files.end());
    for (const auto& [size, path] : sized_files) {
        std::cout << "  " << size << " B  " << path.filename().string() << "\n";
    }

    std::cout << "\n4. 按修改时间排序:\n";
    std::vector<std::pair<fs::file_time_type, fs::path>> timed_files;
    for (const auto& entry : fs::recursive_directory_iterator(TEST_DIR)) {
        if (entry.is_regular_file()) {
            timed_files.emplace_back(entry.last_write_time(), entry.path());
        }
    }
    std::sort(timed_files.begin(), timed_files.end());
    for (const auto& [ftime, path] : timed_files) {
        std::cout << "  " << path.filename().string() << "\n";
    }
}

void demo_file_search() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  文件搜索\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. 按名称搜索:\n";
    auto search_by_name = [](const fs::path& dir, const std::string& name) {
        std::vector<fs::path> result;
        for (const auto& entry : fs::recursive_directory_iterator(dir)) {
            if (entry.path().filename().string().find(name) != std::string::npos) {
                result.push_back(entry.path());
            }
        }
        return result;
    };

    auto found = search_by_name(TEST_DIR, "main");
    std::cout << "  包含\"main\"的文件: ";
    for (const auto& f : found) std::cout << f.filename().string() << " ";
    std::cout << "\n\n";

    std::cout << "2. 正则表达式搜索:\n";
    auto search_by_regex = [](const fs::path& dir, const std::string& pattern) {
        std::vector<fs::path> result;
        std::regex re(pattern);
        for (const auto& entry : fs::recursive_directory_iterator(dir)) {
            if (std::regex_search(entry.path().filename().string(), re)) {
                result.push_back(entry.path());
            }
        }
        return result;
    };

    auto regex_found = search_by_regex(TEST_DIR, R"(\.(cpp|h)$)");
    std::cout << "  C++源文件: ";
    for (const auto& f : regex_found) std::cout << f.filename().string() << " ";
    std::cout << "\n\n";

    std::cout << "3. 按大小范围搜索:\n";
    auto search_by_size = [](const fs::path& dir, std::uintmax_t min_size, std::uintmax_t max_size) {
        std::vector<fs::path> result;
        for (const auto& entry : fs::recursive_directory_iterator(dir)) {
            if (entry.is_regular_file()) {
                auto size = entry.file_size();
                if (size >= min_size && size <= max_size) {
                    result.push_back(entry.path());
                }
            }
        }
        return result;
    };

    auto sized = search_by_size(TEST_DIR, 0, 100);
    std::cout << "  0-100字节的文件(" << sized.size() << "个)\n";
}

void demo_path_manipulation() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  路径操作技巧\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. 获取相对路径:\n";
    fs::path base = TEST_DIR / "src";
    fs::path target = TEST_DIR / "include" / "app.h";
    try {
        fs::path rel = fs::relative(target, base);
        std::cout << "  relative(" << target << ", " << base << ") = " << rel << "\n";
    } catch (const fs::filesystem_error& e) {
        std::cout << "  错误: " << e.what() << "\n";
    }

    std::cout << "\n2. 获取规范路径:\n";
    fs::path messy = TEST_DIR / "src" / ".." / "include" / "app.h";
    try {
        fs::path canonical = fs::canonical(messy);
        std::cout << "  canonical(" << messy << ") = " << canonical << "\n";
    } catch (const fs::filesystem_error& e) {
        std::cout << "  (路径可能不存在: " << e.what() << ")\n";
    }

    fs::path weak = TEST_DIR / "src" / ".." / "include" / "app.h";
    std::cout << "  weakly_canonical: " << fs::weakly_canonical(weak) << "\n";

    std::cout << "\n3. 路径拼接方式对比:\n";
    fs::path p1 = fs::path("/home") / "user" / "file.txt";
    fs::path p2 = fs::path("/home"); p2 += "/user/file.txt";
    std::cout << "  / 运算符: " << p1 << "\n";
    std::cout << "  += 运算符: " << p2 << "\n";
    std::cout << "  / 会自动添加分隔符，+= 直接拼接字符串\n";

    std::cout << "\n4. 路径迭代:\n";
    fs::path p = "/home/user/docs/readme.txt";
    std::cout << "  " << p << " 的各部分: ";
    for (const auto& part : p) {
        std::cout << "[" << part << "] ";
    }
    std::cout << "\n";
}

void demo_cross_platform_paths() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  跨平台路径处理\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. 首选分隔符:\n";
    std::cout << "  fs::path::preferred_separator = '"
              << static_cast<char>(fs::path::preferred_separator) << "'\n\n";

    std::cout << "2. 使用 / 运算符构建路径(自动适配平台):\n";
    fs::path p = fs::path("folder") / "subfolder" / "file.txt";
    std::cout << "  fs::path(\"folder\") / \"subfolder\" / \"file.txt\" = " << p << "\n\n";

    std::cout << "3. 通用路径 vs 原生路径:\n";
    std::cout << "  generic_string(): " << p.generic_string() << "\n";
    std::cout << "  string():         " << p.string() << "\n\n";

    std::cout << "4. 跨平台注意事项:\n";
    std::cout << "  - 使用 fs::path 而非字符串拼接路径\n";
    std::cout << "  - 使用 / 运算符而非手动拼接分隔符\n";
    std::cout << "  - 文件名避免使用 <>:\"/\\|?* 等特殊字符\n";
    std::cout << "  - Windows 不区分大小写，Linux 区分\n";
    std::cout << "  - 使用 fs::relative() 处理相对路径\n";
    std::cout << "  - 使用 fs::canonical() 获取唯一路径\n";

    std::cout << "\n5. 特殊目录:\n";
    std::cout << "  当前目录: " << fs::current_path() << "\n";
    std::cout << "  临时目录: " << fs::temp_directory_path() << "\n";
}

void cleanup() {
    fs::remove_all(TEST_DIR);
    std::cout << "\n测试环境已清理\n";
}

int main() {
    setup();
    demo_traversal_patterns();
    demo_file_search();
    demo_path_manipulation();
    demo_cross_platform_paths();
    cleanup();
    return 0;
}
