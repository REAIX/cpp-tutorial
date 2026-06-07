/** @file 01_example_filesystem_basics.cpp
 *  @brief 文件系统基础：path, directory_entry, directory_iterator, file status
 *  @description 对应文档: 02-CPP/19-filesystem | 演示C++17文件系统库的基本概念
 *  编译命令: g++ -std=c++20 01_example_filesystem_basics.cpp -o 01_example_filesystem_basics
 */

#include <iostream>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

void demo_path_basics() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  path —— 路径操作\n";
    std::cout << "═══════════════════════════════════════\n\n";

    fs::path p1 = "/home/user/docs/readme.txt";
    fs::path p2 = R"(C:\Users\文档\报告.docx)";
    fs::path p3 = "../src/main.cpp";

    std::cout << "路径分解:\n";
    std::cout << "  p1 = " << p1 << "\n";
    std::cout << "  root_name:   " << p1.root_name() << "\n";
    std::cout << "  root_directory: " << p1.root_directory() << "\n";
    std::cout << "  root_path:   " << p1.root_path() << "\n";
    std::cout << "  relative_path: " << p1.relative_path() << "\n";
    std::cout << "  parent_path: " << p1.parent_path() << "\n";
    std::cout << "  filename:    " << p1.filename() << "\n";
    std::cout << "  stem:        " << p1.stem() << "\n";
    std::cout << "  extension:   " << p1.extension() << "\n";

    std::cout << "\nWindows路径:\n";
    std::cout << "  p2 = " << p2 << "\n";
    std::cout << "  root_name:   " << p2.root_name() << "\n";
    std::cout << "  filename:    " << p2.filename() << "\n";
    std::cout << "  extension:   " << p2.extension() << "\n";

    std::cout << "\n路径拼接:\n";
    fs::path base = "/home/user";
    fs::path full = base / "docs" / "readme.txt";
    std::cout << "  base / \"docs\" / \"readme.txt\" = " << full << "\n";

    fs::path concat = fs::path("file") += ".txt";
    std::cout << "  path(\"file\") += \".txt\" = " << concat << "\n";

    std::cout << "\n路径修改:\n";
    fs::path modified = p1;
    modified.replace_extension(".md");
    std::cout << "  replace_extension(.md): " << modified << "\n";

    modified = p1;
    modified.replace_filename("new_file.txt");
    std::cout << "  replace_filename: " << modified << "\n";

    modified = p1;
    modified.remove_filename();
    std::cout << "  remove_filename: " << modified << "\n";
}

void demo_path_queries() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  路径查询\n";
    std::cout << "═══════════════════════════════════════\n\n";

    fs::path p = "/home/user/docs/readme.txt";

    std::cout << "路径: " << p << "\n";
    std::cout << "  empty():          " << p.empty() << "\n";
    std::cout << "  has_root_path():  " << p.has_root_path() << "\n";
    std::cout << "  has_root_name():  " << p.has_root_name() << "\n";
    std::cout << "  has_filename():   " << p.has_filename() << "\n";
    std::cout << "  has_extension():  " << p.has_extension() << "\n";
    std::cout << "  has_parent_path(): " << p.has_parent_path() << "\n";
    std::cout << "  is_absolute():    " << p.is_absolute() << "\n";
    std::cout << "  is_relative():    " << p.is_relative() << "\n";

    fs::path rel = "../src/main.cpp";
    std::cout << "\n路径: " << rel << "\n";
    std::cout << "  is_absolute(): " << rel.is_absolute() << "\n";
    std::cout << "  is_relative(): " << rel.is_relative() << "\n";

    std::cout << "\n路径比较:\n";
    fs::path a = "/home/user/docs";
    fs::path b = "/home/user/docs";
    fs::path c = "/home/user/images";
    std::cout << "  " << a << " == " << b << ": " << (a == b) << "\n";
    std::cout << "  " << a << " < " << c << ": " << (a < c) << "\n";
}

void demo_directory_iterator() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  目录迭代器\n";
    std::cout << "═══════════════════════════════════════\n\n";

    fs::path current_dir = fs::current_path();
    std::cout << "当前目录: " << current_dir << "\n\n";

    std::cout << "1. directory_iterator (非递归):\n";
    int count = 0;
    try {
        for (const auto& entry : fs::directory_iterator(current_dir)) {
            if (count < 15) {
                std::cout << "  " << entry.path().filename().string();
                if (entry.is_directory()) std::cout << "/";
                std::cout << "\n";
            }
            count++;
        }
        if (count > 15) std::cout << "  ... (共 " << count << " 项)\n";
    } catch (const fs::filesystem_error& e) {
        std::cout << "  错误: " << e.what() << "\n";
    }

    std::cout << "\n2. recursive_directory_iterator (递归):\n";
    count = 0;
    try {
        for (auto it = fs::recursive_directory_iterator(
                current_dir, fs::directory_options::skip_permission_denied);
             it != fs::recursive_directory_iterator(); ++it) {
            if (count < 10) {
                std::cout << "  " << std::string(it.depth() * 2, ' ')
                          << it->path().filename().string();
                if (it->is_directory()) std::cout << "/";
                std::cout << "\n";
            }
            count++;
        }
        if (count > 10) std::cout << "  ... (共 " << count << " 项)\n";
    } catch (const fs::filesystem_error& e) {
        std::cout << "  错误: " << e.what() << "\n";
    }
}

void demo_directory_entry() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  directory_entry —— 目录项信息\n";
    std::cout << "═══════════════════════════════════════\n\n";

    fs::path current_dir = fs::current_path();
    int shown = 0;
    for (const auto& entry : fs::directory_iterator(current_dir)) {
        if (shown >= 8) break;
        shown++;

        std::cout << "名称: " << entry.path().filename().string() << "\n";
        std::cout << "  is_regular_file: " << entry.is_regular_file() << "\n";
        std::cout << "  is_directory:    " << entry.is_directory() << "\n";
        std::cout << "  is_symlink:      " << entry.is_symlink() << "\n";

        if (entry.is_regular_file()) {
            try {
                std::cout << "  文件大小: " << entry.file_size() << " 字节\n";
            } catch (...) {
                std::cout << "  文件大小: 无法获取\n";
            }
        }

        try {
            auto ftime = entry.last_write_time();
            auto sctp = std::chrono::time_point_cast<std::chrono::seconds>(
                ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
            std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
            std::cout << "  修改时间: " << std::ctime(&cftime);
        } catch (...) {
            std::cout << "  修改时间: 无法获取\n";
        }
        std::cout << "\n";
    }
}

void demo_file_status() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  file_status —— 文件状态\n";
    std::cout << "═══════════════════════════════════════\n\n";

    auto print_type = [](fs::file_type ft) {
        switch (ft) {
            case fs::file_type::none:       return "none";
            case fs::file_type::not_found:  return "not_found";
            case fs::file_type::regular:    return "regular";
            case fs::file_type::directory:  return "directory";
            case fs::file_type::symlink:    return "symlink";
            case fs::file_type::block:      return "block";
            case fs::file_type::character:  return "character";
            case fs::file_type::fifo:       return "fifo";
            case fs::file_type::socket:     return "socket";
            case fs::file_type::unknown:    return "unknown";
            default:                        return "other";
        }
    };

    fs::path current_dir = fs::current_path();

    std::cout << "status() vs symlink_status():\n";
    std::cout << "  status() —— 跟随符号链接获取目标状态\n";
    std::cout << "  symlink_status() —— 获取链接本身状态\n\n";

    for (const auto& entry : fs::directory_iterator(current_dir)) {
        try {
            fs::file_status st = fs::status(entry.path());
            std::cout << "  " << entry.path().filename().string()
                      << " → " << print_type(st.type()) << "\n";
        } catch (const fs::filesystem_error& e) {
            std::cout << "  " << entry.path().filename().string()
                      << " → 错误: " << e.what() << "\n";
        }
    }

    std::cout << "\n快捷判断函数:\n";
    std::cout << "  fs::exists(p)           —— 路径是否存在\n";
    std::cout << "  fs::is_regular_file(p)  —— 是否为普通文件\n";
    std::cout << "  fs::is_directory(p)     —— 是否为目录\n";
    std::cout << "  fs::is_symlink(p)       —— 是否为符号链接\n";
    std::cout << "  fs::is_empty(p)         —— 文件/目录是否为空\n";
}

int main() {
    demo_path_basics();
    demo_path_queries();
    demo_directory_iterator();
    demo_directory_entry();
    demo_file_status();
    return 0;
}
