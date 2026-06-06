/** @file 02_deep_dive_filesystem_advanced.cpp
 *  @brief 文件系统错误处理、符号链接、空间信息、临时目录、权限深入
 *  @description 对应文档: 02-CPP/19-filesystem | 举一反三：高级文件系统操作和健壮性处理
 *  编译命令: g++ -std=c++20 02_deep_dive_filesystem_advanced.cpp -o 02_deep_dive_filesystem_advanced
 */

#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <iomanip>

namespace fs = std::filesystem;

const fs::path TEST_DIR = "fs_advanced_test";

void setup() {
    fs::create_directories(TEST_DIR / "dir1");
    fs::create_directories(TEST_DIR / "dir2");
    std::ofstream(TEST_DIR / "file1.txt") << "content1";
    std::ofstream(TEST_DIR / "file2.txt") << "content2";
    std::ofstream(TEST_DIR / "dir1" / "nested.txt") << "nested content";
}

void demo_error_handling() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  文件系统错误处理\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. filesystem_error 异常:\n";
    try {
        fs::copy_file(TEST_DIR / "nonexistent.txt", TEST_DIR / "dest.txt");
    } catch (const fs::filesystem_error& e) {
        std::cout << "  捕获异常: " << e.what() << "\n";
        std::cout << "  path1: " << e.path1() << "\n";
        std::cout << "  path2: " << e.path2() << "\n";
        std::cout << "  code: " << e.code().message() << "\n";
    }

    std::cout << "\n2. 错误码版本(不抛异常):\n";
    std::error_code ec;
    fs::copy_file(TEST_DIR / "nonexistent.txt", TEST_DIR / "dest.txt", ec);
    if (ec) {
        std::cout << "  错误码: " << ec.value() << "\n";
        std::cout << "  错误信息: " << ec.message() << "\n";
        std::cout << "  分类: " << ec.category().name() << "\n";
    }

    std::cout << "\n3. 两套API的选择:\n";
    std::cout << "  抛异常版本: 适合错误是意外的情况\n";
    std::cout << "  错误码版本: 适合错误是预期的情况\n";
    std::cout << "  示例: 检查文件是否存在用错误码版\n";

    std::error_code ec2;
    bool exists = fs::exists(TEST_DIR / "maybe.txt", ec2);
    std::cout << "  exists(maybe.txt): " << exists << " (无异常)\n";

    std::cout << "\n4. 健壮的目录遍历:\n";
    int count = 0;
    for (const auto& entry : fs::recursive_directory_iterator(
            TEST_DIR, fs::directory_options::skip_permission_denied)) {
        count++;
    }
    std::cout << "  skip_permission_denied: 遍历了 " << count << " 项\n";
}

void demo_symlink_handling() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  符号链接处理\n";
    std::cout << "═══════════════════════════════════════\n\n";

    fs::create_directory_symlink(TEST_DIR / "dir1", TEST_DIR / "link_to_dir1");
    std::cout << "创建目录符号链接: link_to_dir1 → dir1\n";

    fs::create_symlink(TEST_DIR / "file1.txt", TEST_DIR / "link_to_file1");
    std::cout << "创建文件符号链接: link_to_file1 → file1.txt\n";

    std::cout << "\n读取符号链接目标:\n";
    std::cout << "  link_to_dir1 → " << fs::read_symlink(TEST_DIR / "link_to_dir1") << "\n";
    std::cout << "  link_to_file1 → " << fs::read_symlink(TEST_DIR / "link_to_file1") << "\n";

    std::cout << "\nstatus vs symlink_status:\n";
    std::cout << "  status(链接) —— 获取目标文件的状态\n";
    std::cout << "  symlink_status(链接) —— 获取链接本身的状态\n\n";

    auto st = fs::status(TEST_DIR / "link_to_file1");
    auto slst = fs::symlink_status(TEST_DIR / "link_to_file1");
    std::cout << "  status(link_to_file1): "
              << (st.type() == fs::file_type::regular ? "regular" : "other") << "\n";
    std::cout << "  symlink_status(link_to_file1): "
              << (slst.type() == fs::file_type::symlink ? "symlink" : "other") << "\n";

    std::cout << "\n符号链接注意事项:\n";
    std::cout << "  - Windows 需要管理员权限或开发者模式创建符号链接\n";
    std::cout << "  - 悬空链接(目标不存在)不会报错，但status为not_found\n";
    std::cout << "  - 目录遍历默认不跟随符号链接\n";
    std::cout << "  - fs::copy 默认跟随符号链接复制目标内容\n";

    fs::create_symlink(TEST_DIR / "nonexistent", TEST_DIR / "dangling_link");
    auto dst = fs::symlink_status(TEST_DIR / "dangling_link");
    std::cout << "\n  悬空链接类型: "
              << (dst.type() == fs::file_type::symlink ? "symlink" : "other") << "\n";
    std::cout << "  目标是否存在: " << fs::exists(TEST_DIR / "dangling_link") << "\n";
}

void demo_space_info() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  磁盘空间信息\n";
    std::cout << "═══════════════════════════════════════\n\n";

    auto format_size = [](std::uintmax_t bytes) -> std::string {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unit_idx = 0;
        double size = static_cast<double>(bytes);
        while (size >= 1024.0 && unit_idx < 4) {
            size /= 1024.0;
            unit_idx++;
        }
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << size << " " << units[unit_idx];
        return oss.str();
    };

    try {
        fs::space_info si = fs::space(fs::current_path());
        std::cout << "当前磁盘空间:\n";
        std::cout << "  总容量:   " << format_size(si.capacity) << "\n";
        std::cout << "  可用空间: " << format_size(si.available) << "\n";
        std::cout << "  剩余空间: " << format_size(si.free) << "\n";
        std::cout << "\n  available vs free:\n";
        std::cout << "    free      —— 未使用的磁盘空间\n";
        std::cout << "    available —— 普通用户可用的空间(考虑配额)\n";
    } catch (const fs::filesystem_error& e) {
        std::cout << "  无法获取空间信息: " << e.what() << "\n";
    }
}

void demo_temp_directory() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  临时目录与临时文件\n";
    std::cout << "═══════════════════════════════════════\n\n";

    fs::path temp_dir = fs::temp_directory_path();
    std::cout << "系统临时目录: " << temp_dir << "\n\n";

    auto create_temp_file = [](const std::string& prefix = "tmp") -> fs::path {
        auto now = std::chrono::high_resolution_clock::now();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()).count();
        std::string filename = prefix + "_" + std::to_string(ns) + ".tmp";
        fs::path temp_path = fs::temp_directory_path() / filename;
        std::ofstream(temp_path) << "temporary data";
        return temp_path;
    };

    fs::path temp1 = create_temp_file("myapp");
    fs::path temp2 = create_temp_file("myapp");
    std::cout << "创建临时文件:\n";
    std::cout << "  " << temp1 << "\n";
    std::cout << "  " << temp2 << "\n";

    fs::remove(temp1);
    fs::remove(temp2);
    std::cout << "临时文件已清理\n";

    std::cout << "\n临时文件最佳实践:\n";
    std::cout << "  1. 使用唯一文件名避免冲突(时间戳/UUID)\n";
    std::cout << "  2. 使用 RAII 确保文件被清理\n";
    std::cout << "  3. 考虑使用 tmpnam() 或自定义命名\n";
    std::cout << "  4. 注意临时目录可能被系统定期清理\n";
    std::cout << "  5. C++17 没有标准临时文件API，需自行实现\n";
}

class TempFile {
public:
    explicit TempFile(const std::string& prefix = "tmp")
        : path_(create_temp_path(prefix)) {}

    ~TempFile() {
        std::error_code ec;
        fs::remove(path_, ec);
    }

    const fs::path& path() const { return path_; }
    std::ofstream open_write() { return std::ofstream(path_); }
    std::ifstream open_read() { return std::ifstream(path_); }

    TempFile(const TempFile&) = delete;
    TempFile& operator=(const TempFile&) = delete;

private:
    static fs::path create_temp_path(const std::string& prefix) {
        auto now = std::chrono::high_resolution_clock::now();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()).count();
        return fs::temp_directory_path() / (prefix + "_" + std::to_string(ns) + ".tmp");
    }

    fs::path path_;
};

void demo_raii_temp_file() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  RAII 临时文件管理\n";
    std::cout << "═══════════════════════════════════════\n\n";

    {
        TempFile tf("raii_test");
        {
            auto out = tf.open_write();
            out << "临时数据写入测试\n";
        }
        std::cout << "临时文件: " << tf.path() << "\n";
        std::cout << "文件存在: " << fs::exists(tf.path()) << "\n";

        {
            auto in = tf.open_read();
            std::string line;
            std::getline(in, line);
            std::cout << "读取内容: " << line << "\n";
        }
    }

    std::cout << "RAII析构后，文件自动清理\n";
}

void demo_permissions_advanced() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  权限深入\n";
    std::cout << "═══════════════════════════════════════\n\n";

    fs::path test_file = TEST_DIR / "perm_test.txt";
    std::ofstream(test_file) << "permission test content";

    std::cout << "权限位说明:\n";
    std::cout << "  owner_read  (0400) —— 所有者读\n";
    std::cout << "  owner_write (0200) —— 所有者写\n";
    std::cout << "  owner_exec  (0100) —— 所有者执行\n";
    std::cout << "  group_read  (0040) —— 组读\n";
    std::cout << "  group_write (0020) —— 组写\n";
    std::cout << "  group_exec  (0010) —— 组执行\n";
    std::cout << "  others_read (0004) —— 其他读\n";
    std::cout << "  others_write(0002) —— 其他写\n";
    std::cout << "  others_exec (0001) —— 其他执行\n";
    std::cout << "  set_uid     (4000) —— 设置用户ID\n";
    std::cout << "  set_gid     (2000) —— 设置组ID\n";
    std::cout << "  sticky_bit  (1000) —— 粘滞位\n\n";

    std::cout << "Windows vs Linux 权限差异:\n";
    std::cout << "  Windows: 权限模型基于ACL，fs::permissions映射有限\n";
    std::cout << "  Linux:   权限模型基于rwx，fs::permissions完整映射\n";
    std::cout << "  跨平台:  建议只使用基本的读写权限\n";
}

void cleanup() {
    fs::remove_all(TEST_DIR);
}

int main() {
    setup();
    demo_error_handling();
    demo_symlink_handling();
    demo_space_info();
    demo_temp_directory();
    demo_raii_temp_file();
    demo_permissions_advanced();
    cleanup();
    return 0;
}
