/** @file 02_deep_dive_stream_advanced.cpp
 *  @brief 流架构、自定义streambuf、locale与编码、内存流
 *  @description 对应文档: 02-CPP/18-file-io | 举一反三：深入理解C++流架构，掌握高级I/O技巧
 *  编译命令: g++ -std=c++20 02_deep_dive_stream_advanced.cpp -o 02_deep_dive_stream_advanced
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <locale>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <iomanip>

const std::string TEST_DIR = "test_io_data/";

void demo_stream_architecture() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  C++ 流架构\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "流类层次结构:\n\n";
    std::cout << "  ios_base\n";
    std::cout << "    └── ios\n";
    std::cout << "          ├── istream  ──→ ifstream, istringstream\n";
    std::cout << "          ├── ostream  ──→ ofstream, ostringstream\n";
    std::cout << "          └── iostream ──→ fstream, stringstream\n\n";

    std::cout << "核心组件:\n";
    std::cout << "  1. 流类 (istream/ostream) —— 格式化/解析逻辑\n";
    std::cout << "  2. 流缓冲 (streambuf)     —— 实际I/O操作\n";
    std::cout << "  3. locale                 —— 区域化格式\n\n";

    std::cout << "streambuf 缓冲机制:\n";
    std::cout << "  get区域: eback ── gptr ── egptr\n";
    std::cout << "  put区域: pbase ── pptr ── epptr\n";
    std::cout << "  读取: gptr移动，到egptr时调用underflow()\n";
    std::cout << "  写入: pptr移动，到epptr时调用overflow()\n\n";

    std::cout << "rdbuf() 访问底层缓冲:\n";
    std::ifstream in(TEST_DIR + "hello.txt");
    if (in) {
        std::cout << "  流缓冲类型: " << typeid(*in.rdbuf()).name() << "\n";
    }
}

class CounterBuf : public std::streambuf {
public:
    CounterBuf(std::streambuf* original) : original_(original), char_count_(0), line_count_(0) {}

    size_t char_count() const { return char_count_; }
    size_t line_count() const { return line_count_; }

protected:
    int_type overflow(int_type c) override {
        if (c != EOF) {
            char_count_++;
            if (c == '\n') line_count_++;
            if (original_) original_->sputc(c);
        }
        return c;
    }

    int sync() override {
        if (original_) return original_->pubsync();
        return 0;
    }

private:
    std::streambuf* original_;
    size_t char_count_;
    size_t line_count_;
};

void demo_custom_streambuf() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  自定义 streambuf —— 字符计数器\n";
    std::cout << "═══════════════════════════════════════\n\n";

    CounterBuf counter(std::cout.rdbuf());
    std::ostream counted_out(&counter);

    counted_out << "Hello, World!\n";
    counted_out << "这是一行中文\n";
    counted_out << "第三行内容\n";

    std::cout << "\n统计结果:\n";
    std::cout << "  字符数: " << counter.char_count() << "\n";
    std::cout << "  行数: " << counter.line_count() << "\n";

    std::cout << "\n自定义 streambuf 的典型应用:\n";
    std::cout << "  1. 日志分流 —— 同时写文件和控制台\n";
    std::cout << "  2. 字符计数 —— 统计输出量\n";
    std::cout << "  3. 加密流 —— 写入时加密\n";
    std::cout << "  4. 压缩流 —— 写入时压缩\n";
    std::cout << "  5. 网络流 —— 通过socket传输\n";
}

class LineBuf : public std::streambuf {
public:
    LineBuf(std::streambuf* dest, const std::string& prefix)
        : dest_(dest), prefix_(prefix), at_line_start_(true) {}

protected:
    int_type overflow(int_type c) override {
        if (c != EOF) {
            if (at_line_start_) {
                std::string timestamp = get_timestamp();
                dest_->sputn(timestamp.c_str(), timestamp.size());
                dest_->sputn(prefix_.c_str(), prefix_.size());
                at_line_start_ = false;
            }
            dest_->sputc(c);
            if (c == '\n') at_line_start_ = true;
        }
        return c;
    }

    int sync() override {
        return dest_->pubsync();
    }

private:
    std::string get_timestamp() {
        auto now = std::chrono::system_clock::now();
        auto now_time = std::chrono::system_clock::to_time_t(now);
        // 注意: std::localtime 非线程安全! 多线程环境应使用 localtime_r/localtime_s
        std::tm* tm = std::localtime(&now_time);
        char buf[32];
        std::strftime(buf, sizeof(buf), "[%H:%M:%S] ", tm);
        return buf;
    }

    std::streambuf* dest_;
    std::string prefix_;
    bool at_line_start_;
};

void demo_line_prefix_buf() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  自定义 streambuf —— 行前缀(时间戳)\n";
    std::cout << "═══════════════════════════════════════\n\n";

    LineBuf line_buf(std::cout.rdbuf(), "LOG: ");
    std::ostream log_out(&line_buf);

    log_out << "应用启动\n";
    log_out << "加载配置文件\n";
    log_out << "连接数据库\n";
    log_out << "就绪\n";
}

void demo_locale_encoding() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  Locale 与编码\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. 默认 locale:\n";
    std::cout << "  当前 locale: " << std::locale("").name() << "\n";
    std::cout << "  C locale: " << std::locale("C").name() << "\n\n";

    std::cout << "2. locale 对数字格式的影响:\n";
    double val = 1234567.89;
    std::cout << "  默认(C):    " << val << "\n";

    try {
        std::locale::global(std::locale(""));
        std::cout << "  系统locale: " << val << "\n";
        std::locale::global(std::locale("C"));
    } catch (const std::exception& e) {
        std::cout << "  系统locale不可用: " << e.what() << "\n";
    }

    std::cout << "\n3. 编码注意事项:\n";
    std::cout << "  - C++ 流不处理编码转换\n";
    std::cout << "  - 源文件编码应与运行环境一致\n";
    std::cout << "  - Windows 默认 GBK，Linux 默认 UTF-8\n";
    std::cout << "  - 跨平台建议统一使用 UTF-8\n";
    std::cout << "  - C++20 引入 <codecvt> 替代方案(已弃用codecvt)\n";
    std::cout << "  - 编码转换建议使用 ICU 或 iconv 库\n";
}

void demo_memory_stream() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  内存流 (stringstream)\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. 字符串构建器:\n";
    std::ostringstream builder;
    builder << "SELECT * FROM users WHERE id = " << 42
            << " AND name = '" << "张三" << "'"
            << " AND score > " << 90.5;
    std::cout << "  SQL: " << builder.str() << "\n\n";

    std::cout << "2. 类型安全的数据包:\n";
    std::ostringstream packet;
    packet << "HEAD|";
    packet << std::setw(4) << std::setfill('0') << 1024;
    packet << "|";
    packet << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << 0xDEADBEEF;
    packet << "|TAIL";
    std::cout << "  数据包: " << packet.str() << "\n\n";

    std::cout << "3. 数据解析器:\n";
    std::istringstream data("100 200 300 400 500");
    int sum = 0;
    int val;
    while (data >> val) {
        sum += val;
    }
    std::cout << "  数据和: " << sum << "\n\n";

    std::cout << "4. stringstream 性能提示:\n";
    std::cout << "  - str() 返回副本，频繁调用有开销\n";
    std::cout << "  - 清空内容用 str(\"\") 而不是新建对象\n";
    std::cout << "  - 清空状态用 clear()\n";
    std::cout << "  - 预分配空间: oss.str().reserve(size)\n\n";

    std::ostringstream oss;
    for (int i = 0; i < 5; i++) {
        oss << "item" << i << ",";
    }
    std::string result = oss.str();
    if (!result.empty() && result.back() == ',') {
        result.pop_back();
    }
    std::cout << "5. 去尾逗号: \"" << result << "\"\n";
}

int main() {
    demo_stream_architecture();
    demo_custom_streambuf();
    demo_line_prefix_buf();
    demo_locale_encoding();
    demo_memory_stream();
    return 0;
}
