/** @file 02_example_filesystem_operations.cpp
 *  @brief 文件系统操作：创建目录、复制、重命名、删除、文件大小、修改时间、权限
 *  @description 对应文档: 02-CPP/19-filesystem | 演示文件和目录的增删改查操作
 *  编译命令: g++ -std=c++20 02_example_filesystem_operations.cpp -o 02_example_filesystem_operations
 */

#include <iostream>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <string>
#include <vector>

namespace fs = std::filesystem;

const fs::path TEST_DIR = "fs_test_dir";

void setup_test_env() {
    fs::create_directories(TEST_DIR / "subdir1" / "nested");
    fs::create_directories(TEST_DIR / "subdir2");

    {
        std::ofstream(TEST_DIR / "file1.txt") << "Hello, Filesystem!";
        std::ofstream(TEST_DIR / "file2.txt") << "Second file content";
        std::ofstream(TEST_DIR / "subdir1" / "nested" / "deep.txt") << "Deep nested file";
        std::ofstream(TEST_DIR / "subdir2" / "data.bin") << "Binary data here";
    }
    std::cout << "测试环境已创建: " << TEST_DIR << "\n\n";
}

void demo_create_directory() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  创建目录\n";
    std::cout << "═══════════════════════════════════════\n\n";

    fs::path new_dir = TEST_DIR / "new_folder";
    bool created = fs::create_directory(new_dir);
    std::cout << "create_directory(\"new_folder\"): " << (created ? "新建" : "已存在") << "\n";

    fs::path deep_dir = TEST_DIR / "a" / "b" / "c" / "d";
    bool deep_created = fs::create_directories(deep_dir);
    std::cout << "create_directories(\"a/b/c/d\"): " << (deep_created ? "新建" : "已存在") << "\n";

    std::cout << "\n区别:\n";
    std::cout << "  create_directory   —— 只创建一级，父目录必须存在\n";
    std::cout << "  create_directories —— 递归创建所有不存在的父目录\n";
}

void demo_copy_operations() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  复制操作\n";
    std::cout << "═══════════════════════════════════════\n\n";

    fs::copy_file(TEST_DIR / "file1.txt", TEST_DIR / "file1_copy.txt");
    std::cout << "copy_file: file1.txt → file1_copy.txt\n";

    fs::copy(TEST_DIR / "file2.txt", TEST_DIR / "subdir2" / "file2.txt");
    std::cout << "copy: file2.txt → subdir2/file2.txt\n";

    fs::copy_options opts = fs::copy_options::overwrite_existing;
    fs::copy_file(TEST_DIR / "file1.txt", TEST_DIR / "file1_copy.txt", opts);
    std::cout << "copy_file (覆盖模式): 重复复制成功\n";

    std::cout << "\ncopy_options 选项:\n";
    std::cout << "  none              —— 默认，已存在则报错\n";
    std::cout << "  skip_existing     —— 跳过已存在的文件\n";
    std::cout << "  overwrite_existing —— 覆盖已存在的文件\n";
    std::cout << "  update_existing   —— 仅当源文件更新时覆盖\n";
    std::cout << "  recursive         —— 递归复制目录\n";
    std::cout << "  directories_only  —— 只复制目录结构\n";
    std::cout << "  create_symlinks   —— 创建符号链接而非复制\n";
    std::cout << "  create_hard_links —— 创建硬链接而非复制\n";
}

void demo_rename_remove() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  重命名与删除\n";
    std::cout << "═══════════════════════════════════════\n\n";

    fs::rename(TEST_DIR / "file1_copy.txt", TEST_DIR / "renamed.txt");
    std::cout << "rename: file1_copy.txt → renamed.txt\n";

    bool removed = fs::remove(TEST_DIR / "renamed.txt");
    std::cout << "remove: renamed.txt → " << (removed ? "已删除" : "不存在") << "\n";

    std::ofstream(TEST_DIR / "temp1.tmp") << "temp1";
    std::ofstream(TEST_DIR / "temp2.tmp") << "temp2";
    std::ofstream(TEST_DIR / "temp3.tmp") << "temp3";

    int count = 0;
    for (const auto& entry : fs::directory_iterator(TEST_DIR)) {
        if (entry.path().extension() == ".tmp") {
            fs::remove(entry.path());
            count++;
        }
    }
    std::cout << "删除 " << count << " 个 .tmp 文件\n";

    std::cout << "\nremove vs remove_all:\n";
    std::cout << "  remove      —— 删除单个文件或空目录，返回是否成功\n";
    std::cout << "  remove_all  —— 递归删除目录及内容，返回删除数量\n";
}

void demo_file_size_time() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  文件大小与修改时间\n";
    std::cout << "═══════════════════════════════════════\n\n";

    for (const auto& entry : fs::directory_iterator(TEST_DIR)) {
        if (!entry.is_regular_file()) continue;

        auto size = entry.file_size();
        std::cout << entry.path().filename().string() << ": ";

        if (size < 1024) {
            std::cout << size << " B";
        } else if (size < 1024 * 1024) {
            std::cout << size / 1024.0 << " KB";
        } else {
            std::cout << size / (1024.0 * 1024.0) << " MB";
        }

        auto ftime = entry.last_write_time();
        auto sctp = std::chrono::time_point_cast<std::chrono::seconds>(
            ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
        std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
        char buf[32];
        std::tm tm_buf{};
#ifdef _WIN32
        localtime_s(&tm_buf, &cftime);
#else
        localtime_r(&cftime, &tm_buf);
#endif
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_buf);
        std::cout << ", 修改时间: " << buf << "\n";
    }

    std::cout << "\n目录大小(递归):\n";
    std::uintmax_t total_size = 0;
    int file_count = 0;
    for (const auto& entry : fs::recursive_directory_iterator(TEST_DIR)) {
        if (entry.is_regular_file()) {
            total_size += entry.file_size();
            file_count++;
        }
    }
    std::cout << "  " << TEST_DIR << ": " << file_count << " 个文件, "
              << total_size << " 字节\n";

    std::cout << "\n修改文件时间:\n";
    auto now = fs::file_time_type::clock::now();
    fs::last_write_time(TEST_DIR / "file1.txt", now);
    std::cout << "  file1.txt 修改时间已更新为当前时间\n";
}

void demo_permissions() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  文件权限\n";
    std::cout << "═══════════════════════════════════════\n\n";

    fs::path test_file = TEST_DIR / "perm_test.txt";
    std::ofstream(test_file) << "permission test";

    auto perms = fs::status(test_file).permissions();

    auto print_perms = [](fs::perms p) {
        std::cout << ((p & fs::perms::owner_read) != fs::perms::none ? "r" : "-");
        std::cout << ((p & fs::perms::owner_write) != fs::perms::none ? "w" : "-");
        std::cout << ((p & fs::perms::owner_exec) != fs::perms::none ? "x" : "-");
        std::cout << ((p & fs::perms::group_read) != fs::perms::none ? "r" : "-");
        std::cout << ((p & fs::perms::group_write) != fs::perms::none ? "w" : "-");
        std::cout << ((p & fs::perms::group_exec) != fs::perms::none ? "x" : "-");
        std::cout << ((p & fs::perms::others_read) != fs::perms::none ? "r" : "-");
        std::cout << ((p & fs::perms::others_write) != fs::perms::none ? "w" : "-");
        std::cout << ((p & fs::perms::others_exec) != fs::perms::none ? "x" : "-");
    };

    std::cout << "当前权限: ";
    print_perms(perms);
    std::cout << "\n\n";

    fs::permissions(test_file,
        fs::perms::owner_read | fs::perms::owner_write,
        fs::perm_options::replace);

    std::cout << "修改后权限: ";
    print_perms(fs::status(test_file).permissions());
    std::cout << " (仅所有者读写)\n\n";

    fs::permissions(test_file,
        fs::perms::owner_read | fs::perms::owner_write |
        fs::perms::group_read | fs::perms::others_read,
        fs::perm_options::replace);

    std::cout << "恢复权限: ";
    print_perms(fs::status(test_file).permissions());
    std::cout << "\n\n";

    std::cout << "perm_options:\n";
    std::cout << "  replace —— 替换权限\n";
    std::cout << "  add     —— 添加权限\n";
    std::cout << "  remove  —— 移除权限\n";
    std::cout << "  nofollow —— 不跟随符号链接\n";
}

void cleanup_test_env() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  清理测试环境\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::uintmax_t removed = fs::remove_all(TEST_DIR);
    std::cout << "已删除 " << removed << " 个文件/目录\n";
}

int main() {
    setup_test_env();
    demo_create_directory();
    demo_copy_operations();
    demo_rename_remove();
    demo_file_size_time();
    demo_permissions();
    cleanup_test_env();
    return 0;
}
