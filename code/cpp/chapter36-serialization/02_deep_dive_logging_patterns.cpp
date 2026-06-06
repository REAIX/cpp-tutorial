/**
 * @file 02_deep_dive_logging_patterns.cpp
 * @brief 高级日志模式: 异步日志, 结构化日志, 日志聚合, 性能考量
 * @description 对应文档: 02-CPP/36-序列化与日志
 */

#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <atomic>
#include <chrono>
#include <sstream>
#include <map>
#include <iomanip>
#include <ctime>

enum class LogLevel { TRACE, DEBUG, INFO, WARN, ERROR, FATAL };

std::string level_name(LogLevel level) {
    static const char* names[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
    return names[static_cast<int>(level)];
}

void demo_async_logging() {
    std::cout << "\n=== demo_async_logging ===\n";
    std::cout << "异步日志: 日志写入不阻塞业务线程\n\n";

    class AsyncLogger {
        struct LogEntry {
            LogLevel level;
            std::string message;
            std::chrono::system_clock::time_point timestamp;
        };

        std::queue<LogEntry> queue_;
        mutable std::mutex queue_mutex_;
        std::condition_variable cv_;
        std::atomic<bool> running_{true};
        std::thread writer_thread_;
        std::atomic<size_t> dropped_{0};

        void writer_loop() {
            while (running_ || !queue_.empty()) {
                std::vector<LogEntry> batch;
                {
                    std::unique_lock<std::mutex> lock(queue_mutex_);
                    cv_.wait_for(lock, std::chrono::milliseconds(10), [this]() {
                        return !queue_.empty() || !running_;
                    });
                    while (!queue_.empty() && batch.size() < 100) {
                        batch.push_back(std::move(queue_.front()));
                        queue_.pop();
                    }
                }

                for (const auto& entry : batch) {
                    auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
                    std::tm tm_buf;
#ifdef _WIN32
                    localtime_s(&tm_buf, &time_t);
#else
                    localtime_r(&time_t, &tm_buf);
#endif
                    std::cout << "  [异步] "
                              << std::put_time(&tm_buf, "%H:%M:%S")
                              << " [" << level_name(entry.level) << "] "
                              << entry.message << "\n";
                }
            }
        }

    public:
        AsyncLogger() : writer_thread_(&AsyncLogger::writer_loop, this) {}

        ~AsyncLogger() {
            running_ = false;
            cv_.notify_one();
            if (writer_thread_.joinable()) {
                writer_thread_.join();
            }
            if (dropped_ > 0) {
                std::cout << "  [异步日志] 丢弃了 " << dropped_ << " 条日志\n";
            }
        }

        void log(LogLevel level, const std::string& message) {
            {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                if (queue_.size() < 10000) {
                    queue_.push({level, message, std::chrono::system_clock::now()});
                } else {
                    dropped_.fetch_add(1);
                }
            }
            cv_.notify_one();
        }
    };

    AsyncLogger async_logger;

    std::cout << "异步日志写入测试:\n";
    for (int i = 0; i < 20; ++i) {
        async_logger.log(LogLevel::INFO, "异步日志消息 #" + std::to_string(i));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    std::cout << "\n异步日志架构:\n";
    std::cout << "  业务线程 -> 日志队列 -> 写入线程 -> 目标(Sink)\n";
    std::cout << "  1. 业务线程只负责入队 (微秒级)\n";
    std::cout << "  2. 写入线程批量写入 (减少I/O次数)\n";
    std::cout << "  3. 队列满时丢弃或阻塞 (策略可配)\n\n";

    std::cout << "异步日志注意事项:\n";
    std::cout << "  1. 队列大小限制, 防止内存耗尽\n";
    std::cout << "  2. 程序退出时刷新队列\n";
    std::cout << "  3. FATAL日志应同步写入 (确保记录)\n";
    std::cout << "  4. 队列满策略: 丢弃/阻塞/覆盖最旧\n";
}

void demo_structured_logging() {
    std::cout << "\n=== demo_structured_logging ===\n";
    std::cout << "结构化日志: 机器可读的日志格式\n\n";

    class StructuredLogger {
    public:
        struct LogContext {
            std::map<std::string, std::string> fields;

            LogContext& with(const std::string& key, const std::string& value) {
                fields[key] = value;
                return *this;
            }

            LogContext& with(const std::string& key, int value) {
                fields[key] = std::to_string(value);
                return *this;
            }

            LogContext& with(const std::string& key, double value) {
                std::ostringstream oss;
                oss << value;
                fields[key] = oss.str();
                return *this;
            }
        };

    private:
        LogContext global_context_;
        std::mutex mutex_;

        std::string format_json(LogLevel level, const std::string& msg,
                                const LogContext& ctx) {
            std::ostringstream oss;
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) % 1000;

            oss << "{\"timestamp\":\"";
            oss << std::put_time(std::localtime(&time_t), "%Y-%m-%dT%H:%M:%S");
            oss << "." << std::setfill('0') << std::setw(3) << ms.count() << "\"";
            oss << ",\"level\":\"" << level_name(level) << "\"";
            oss << ",\"message\":\"" << msg << "\"";

            auto all_fields = global_context_.fields;
            for (const auto& [k, v] : ctx.fields) {
                all_fields[k] = v;
            }
            for (const auto& [k, v] : all_fields) {
                oss << ",\"" << k << "\":\"" << v << "\"";
            }
            oss << "}";
            return oss.str();
        }

    public:
        void set_global_context(const std::string& key, const std::string& value) {
            std::lock_guard<std::mutex> lock(mutex_);
            global_context_.with(key, value);
        }

        void log(LogLevel level, const std::string& msg,
                 const LogContext& ctx = LogContext{}) {
            std::lock_guard<std::mutex> lock(mutex_);
            std::cout << format_json(level, msg, ctx) << "\n";
        }
    };

    StructuredLogger slogger;
    slogger.set_global_context("service", "user-api");
    slogger.set_global_context("version", "1.2.3");

    std::cout << "结构化日志输出 (JSON格式):\n";
    slogger.log(LogLevel::INFO, "用户登录",
                StructuredLogger::LogContext()
                    .with("user_id", 12345)
                    .with("ip", "192.168.1.100"));

    slogger.log(LogLevel::WARN, "请求超时",
                StructuredLogger::LogContext()
                    .with("endpoint", "/api/users")
                    .with("timeout_ms", 5000));

    slogger.log(LogLevel::ERROR, "数据库错误",
                StructuredLogger::LogContext()
                    .with("db", "primary")
                    .with("error_code", 2003)
                    .with("retry_count", 3));

    std::cout << "\n结构化日志优势:\n";
    std::cout << "  1. 机器可读: 便于ELK/Splunk等系统处理\n";
    std::cout << "  2. 上下文丰富: 每条日志携带结构化字段\n";
    std::cout << "  3. 可搜索: 按字段精确查询\n";
    std::cout << "  4. 可聚合: 统计分析\n";
    std::cout << "  5. 可追踪: 关联request_id等\n";
}

void demo_log_aggregation() {
    std::cout << "\n=== demo_log_aggregation ===\n";
    std::cout << "日志聚合与集中管理\n\n";

    std::cout << "日志聚合架构:\n";
    std::cout << "  应用 -> 日志Agent -> 日志收集器 -> 存储 -> 搜索/可视化\n\n";

    std::cout << "常见方案:\n\n";

    std::cout << "1. ELK Stack:\n";
    std::cout << "   Elasticsearch: 存储和搜索\n";
    std::cout << "   Logstash: 日志收集和处理\n";
    std::cout << "   Kibana: 可视化\n";
    std::cout << "   最流行的开源方案\n\n";

    std::cout << "2. EFK Stack:\n";
    std::cout << "   用Fluentd替代Logstash\n";
    std::cout << "   Kubernetes默认方案\n\n";

    std::cout << "3. Loki + Grafana:\n";
    std::cout << "   轻量级, 只索引标签\n";
    std::cout << "   适合容器化环境\n\n";

    std::cout << "4. 商业方案:\n";
    std::cout << "   Datadog, Splunk, New Relic\n";
    std::cout << "   开箱即用, 功能丰富\n\n";

    std::cout << "日志聚合最佳实践:\n";
    std::cout << "  1. 统一日志格式 (JSON)\n";
    std::cout << "  2. 包含追踪ID (trace_id, request_id)\n";
    std::cout << "  3. 设置合理的日志级别\n";
    std::cout << "  4. 控制日志量 (避免日志风暴)\n";
    std::cout << "  5. 敏感信息脱敏\n";
    std::cout << "  6. 设置日志保留策略\n";
}

void demo_logging_performance() {
    std::cout << "\n=== demo_logging_performance ===\n";
    std::cout << "日志性能考量\n\n";

    std::cout << "性能影响因素:\n\n";

    std::cout << "1. 格式化开销:\n";
    std::cout << "   字符串拼接是最耗时的操作\n";
    std::cout << "   解决: 延迟格式化 (只在需要时格式化)\n";
    std::cout << "   例: if (logger.is_enabled(INFO)) logger.info(...)\n\n";

    std::cout << "2. I/O开销:\n";
    std::cout << "   磁盘I/O是最慢的环节\n";
    std::cout << "   解决: 异步写入, 批量flush\n\n";

    std::cout << "3. 锁竞争:\n";
    std::cout << "   多线程写日志时的锁竞争\n";
    std::cout << "   解决: 每线程缓冲, 无锁队列\n\n";

    std::cout << "4. 内存分配:\n";
    std::cout << "   每条日志分配字符串内存\n";
    std::cout << "   解决: 内存池, 预分配缓冲区\n\n";

    auto benchmark_logging = [](const std::string& name, int count, auto log_func) {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < count; ++i) {
            log_func(i);
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::cout << "  " << name << ": " << count << "条日志, "
                  << us << " us, "
                  << (double)us / count << " us/条\n";
    };

    constexpr int n = 10000;

    std::cout << "\n性能对比 (" << n << "条日志):\n";

    std::ostringstream null_oss;
    benchmark_logging("直接输出到ostringstream", n, [&null_oss](int i) {
        null_oss.str("");
        null_oss << "日志消息 #" << i << " value=" << i * 2;
    });

    std::string pre_allocated;
    pre_allocated.reserve(256);
    benchmark_logging("预分配字符串", n, [&pre_allocated](int i) {
        pre_allocated.clear();
        pre_allocated += "日志消息 #";
        pre_allocated += std::to_string(i);
        pre_allocated += " value=";
        pre_allocated += std::to_string(i * 2);
    });

    std::cout << "\n日志性能优化总结:\n";
    std::cout << "  1. 使用异步日志 (spdlog的async模式)\n";
    std::cout << "  2. 避免在热路径写DEBUG/TRACE日志\n";
    std::cout << "  3. 使用fmt而非stringstream (更快)\n";
    std::cout << "  4. 批量写入, 减少flush次数\n";
    std::cout << "  5. 使用二进制日志格式 (如需要)\n";
    std::cout << "  6. 生产环境用INFO级别\n";
}

int main() {
    std::cout << "高级日志模式深入\n";

    demo_async_logging();
    demo_structured_logging();
    demo_log_aggregation();
    demo_logging_performance();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
