/**
 * @file 03_example_logging.cpp
 * @brief 日志框架设计: 日志级别, 格式化, 文件轮转概念
 * @description 对应文档: 02-CPP/36-序列化与日志
 */

#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <ctime>
#include <atomic>
#include <memory>

enum class LogLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
};

std::string level_to_string(LogLevel level) {
    switch (level) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

class LogFormatter {
public:
    static std::string format(LogLevel level, const std::string& message,
                              const char* file, int line) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::tm tm_buf;
#ifdef _WIN32
        localtime_s(&tm_buf, &time_t);
#else
        localtime_r(&time_t, &tm_buf);
#endif

        std::ostringstream oss;
        oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S")
            << "." << std::setfill('0') << std::setw(3) << ms.count()
            << " [" << std::setw(5) << level_to_string(level) << "]"
            << " [" << file << ":" << line << "]"
            << " " << message;
        return oss.str();
    }
};

class LogSink {
public:
    virtual ~LogSink() = default;
    virtual void write(const std::string& formatted_message, LogLevel level) = 0;
};

class ConsoleSink : public LogSink {
public:
    void write(const std::string& formatted_message, LogLevel level) override {
        if (level >= LogLevel::ERROR) {
            std::cerr << formatted_message << "\n";
        } else {
            std::cout << formatted_message << "\n";
        }
    }
};

class FileSink : public LogSink {
    std::string base_path_;
    std::ofstream file_;
    size_t max_size_;
    size_t current_size_ = 0;
    int file_index_ = 0;
    std::mutex mutex_;

    void rotate() {
        if (current_size_ >= max_size_) {
            file_.close();
            ++file_index_;
            open_file();
            current_size_ = 0;
        }
    }

    void open_file() {
        std::string path = base_path_ + "." + std::to_string(file_index_) + ".log";
        file_.open(path, std::ios::app);
    }

public:
    FileSink(const std::string& path, size_t max_size = 1024 * 1024)
        : base_path_(path), max_size_(max_size) {
        open_file();
    }

    void write(const std::string& formatted_message, LogLevel level) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!file_.is_open()) return;
        file_ << formatted_message << "\n";
        file_.flush();
        current_size_ += formatted_message.size() + 1;
        rotate();
    }
};

class Logger {
    LogLevel min_level_ = LogLevel::INFO;
    std::vector<std::unique_ptr<LogSink>> sinks_;
    std::mutex mutex_;

public:
    void set_level(LogLevel level) { min_level_ = level; }

    void add_sink(std::unique_ptr<LogSink> sink) {
        std::lock_guard<std::mutex> lock(mutex_);
        sinks_.push_back(std::move(sink));
    }

    void log(LogLevel level, const std::string& message,
             const char* file = __FILE__, int line = __LINE__) {
        if (level < min_level_) return;

        std::string formatted = LogFormatter::format(level, message, file, line);

        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& sink : sinks_) {
            sink->write(formatted, level);
        }
    }
};

#define LOG_TRACE(logger, msg) (logger).log(LogLevel::TRACE, msg, __FILE__, __LINE__)
#define LOG_DEBUG(logger, msg) (logger).log(LogLevel::DEBUG, msg, __FILE__, __LINE__)
#define LOG_INFO(logger, msg)  (logger).log(LogLevel::INFO,  msg, __FILE__, __LINE__)
#define LOG_WARN(logger, msg)  (logger).log(LogLevel::WARN,  msg, __FILE__, __LINE__)
#define LOG_ERROR(logger, msg) (logger).log(LogLevel::ERROR, msg, __FILE__, __LINE__)
#define LOG_FATAL(logger, msg) (logger).log(LogLevel::FATAL, msg, __FILE__, __LINE__)

void demo_log_levels() {
    std::cout << "\n=== demo_log_levels ===\n";
    std::cout << "日志级别\n\n";

    Logger logger;
    logger.add_sink(std::make_unique<ConsoleSink>());

    std::cout << "默认级别(INFO), 只输出INFO及以上:\n";
    LOG_TRACE(logger, "这条TRACE不会输出");
    LOG_DEBUG(logger, "这条DEBUG不会输出");
    LOG_INFO(logger, "这是一条INFO日志");
    LOG_WARN(logger, "这是一条WARN日志");
    LOG_ERROR(logger, "这是一条ERROR日志");
    LOG_FATAL(logger, "这是一条FATAL日志");

    std::cout << "\n设置为DEBUG级别:\n";
    logger.set_level(LogLevel::DEBUG);
    LOG_DEBUG(logger, "现在DEBUG也会输出");
    LOG_TRACE(logger, "TRACE仍然不输出");

    std::cout << "\n日志级别选择指南:\n";
    std::cout << "  TRACE: 最详细, 仅开发调试时使用\n";
    std::cout << "  DEBUG: 调试信息, 开发环境使用\n";
    std::cout << "  INFO:  关键业务信息, 生产环境默认\n";
    std::cout << "  WARN:  警告, 可恢复的异常\n";
    std::cout << "  ERROR: 错误, 需要关注\n";
    std::cout << "  FATAL: 致命错误, 程序即将退出\n";
}

void demo_log_formatting() {
    std::cout << "\n=== demo_log_formatting ===\n";
    std::cout << "日志格式化\n\n";

    Logger logger;
    logger.add_sink(std::make_unique<ConsoleSink>());
    logger.set_level(LogLevel::TRACE);

    std::cout << "格式: 时间戳.毫秒 [级别] [文件:行号] 消息\n\n";

    LOG_INFO(logger, "应用启动");
    LOG_DEBUG(logger, "加载配置文件成功");
    LOG_WARN(logger, "缓存未命中, 回源查询");
    LOG_ERROR(logger, "数据库连接超时");

    std::cout << "\n常见日志格式:\n";
    std::cout << "  1. 纯文本: 可读性好, 不易解析\n";
    std::cout << "  2. 结构化(JSON): 易解析, 便于日志系统处理\n";
    std::cout << "  3. 键值对: 折中方案, 如 k1=v1 k2=v2\n";
}

void demo_file_rotation_concept() {
    std::cout << "\n=== demo_file_rotation_concept ===\n";
    std::cout << "文件轮转概念\n\n";

    std::cout << "文件轮转策略:\n\n";

    std::cout << "1. 按大小轮转:\n";
    std::cout << "   文件达到指定大小时创建新文件\n";
    std::cout << "   app.log.0, app.log.1, app.log.2, ...\n";
    std::cout << "   保留固定数量的历史文件\n\n";

    std::cout << "2. 按时间轮转:\n";
    std::cout << "   每天创建新文件\n";
    std::cout << "   app-2024-01-15.log, app-2024-01-16.log\n";
    std::cout << "   便于按日期查找日志\n\n";

    std::cout << "3. 按大小+时间:\n";
    std::cout << "   结合两者, 先到先轮转\n\n";

    std::cout << "4. 压缩归档:\n";
    std::cout << "   旧日志文件压缩存储\n";
    std::cout << "   节省磁盘空间\n\n";

    Logger logger;
    logger.add_sink(std::make_unique<FileSink>("demo_log", 2048));
    logger.set_level(LogLevel::TRACE);

    std::cout << "写入日志到文件 (2KB轮转):\n";
    for (int i = 0; i < 50; ++i) {
        LOG_INFO(logger, "日志条目 #" + std::to_string(i) + " - 这是一条测试日志消息");
    }
    std::cout << "  日志已写入文件 (查看demo_log.*.log)\n";

    std::cout << "\nC++日志库推荐:\n";
    std::cout << "  spdlog: 最快, header-only, 异步支持\n";
    std::cout << "  glog: Google出品, 稳定可靠\n";
    std::cout << "  log4cxx: Apache出品, 功能丰富\n";
    std::cout << "  fmtlog: 基于fmt, 极致性能\n";
}

int main() {
    std::cout << "日志框架设计演示\n";

    demo_log_levels();
    demo_log_formatting();
    demo_file_rotation_concept();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
