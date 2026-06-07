/** @file 02_example_chrono_timer.cpp
 *  @brief 计时器实现、性能基准测试、延时、日期格式化(C++20预览)
 *  @description 对应文档: 02-CPP/17-chrono | 实用计时工具和性能测量
 *  编译命令: g++ -std=c++20 02_example_chrono_timer.cpp -o 02_example_chrono_timer
 */

#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <random>

class Timer {
public:
    Timer() : start_(std::chrono::steady_clock::now()) {}

    void reset() { start_ = std::chrono::steady_clock::now(); }

    double elapsed_ms() const {
        auto end = std::chrono::steady_clock::now();
        return std::chrono::duration<double, std::milli>(end - start_).count();
    }

    double elapsed_us() const {
        auto end = std::chrono::steady_clock::now();
        return std::chrono::duration<double, std::micro>(end - start_).count();
    }

    double elapsed_sec() const {
        auto end = std::chrono::steady_clock::now();
        return std::chrono::duration<double>(end - start_).count();
    }

private:
    std::chrono::steady_clock::time_point start_;
};

class ScopedTimer {
public:
    ScopedTimer(const std::string& name)
        : name_(name), start_(std::chrono::steady_clock::now()) {}

    ~ScopedTimer() {
        auto end = std::chrono::steady_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - start_).count();
        std::cout << "  [" << name_ << "] 耗时: " << ms << " ms\n";
    }

    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;
    ScopedTimer(ScopedTimer&&) = delete;
    ScopedTimer& operator=(ScopedTimer&&) = delete;

private:
    std::string name_;
    std::chrono::steady_clock::time_point start_;
};

void demo_basic_timer() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  基本计时器\n";
    std::cout << "═══════════════════════════════════════\n\n";

    Timer timer;

    std::cout << "执行一些计算...\n";
    volatile long long sum = 0;  // volatile 防止优化消除, 生产级基准测试建议用 Google Benchmark
    for (long long i = 0; i < 10000000; i++) {
        sum += i;
    }
    std::cout << "结果: " << sum << "\n";
    std::cout << "耗时: " << timer.elapsed_ms() << " ms\n";

    timer.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "sleep 100ms 实际耗时: " << timer.elapsed_ms() << " ms\n";
}

void demo_scoped_timer() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  RAII 风格作用域计时器\n";
    std::cout << "═══════════════════════════════════════\n\n";

    {
        ScopedTimer t("排序100万元素");
        std::vector<int> v(1000000);
        std::iota(v.begin(), v.end(), 0);
        std::shuffle(v.begin(), v.end(), std::mt19937{std::random_device{}()});
        std::sort(v.begin(), v.end());
    }

    {
        ScopedTimer t("字符串拼接");
        std::string result;
        for (int i = 0; i < 100000; i++) {
            result += std::to_string(i);
        }
    }

    {
        ScopedTimer t("向量push_back");
        std::vector<int> v;
        v.reserve(100000);
        for (int i = 0; i < 100000; i++) {
            v.push_back(i);
        }
    }
}

void demo_benchmark() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  性能基准测试框架\n";
    std::cout << "═══════════════════════════════════════\n\n";

    auto benchmark = [](const std::string& name, auto func, int iterations) {
        std::vector<double> times;
        times.reserve(iterations);

        for (int i = 0; i < iterations; i++) {
            auto start = std::chrono::high_resolution_clock::now();
            func();
            auto end = std::chrono::high_resolution_clock::now();
            times.push_back(std::chrono::duration<double, std::micro>(end - start).count());
        }

        std::sort(times.begin(), times.end());
        double min_t = times.front();
        double max_t = times.back();
        double avg_t = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
        double median = times[times.size() / 2];

        std::cout << "  " << name << " (" << iterations << " 次迭代):\n";
        std::cout << "    最小: " << min_t << " us\n";
        std::cout << "    最大: " << max_t << " us\n";
        std::cout << "    平均: " << avg_t << " us\n";
        std::cout << "    中位: " << median << " us\n\n";
    };

    benchmark("vector排序(1万元素)", []() {
        std::vector<int> v(10000);
        std::iota(v.begin(), v.end(), 0);
        std::shuffle(v.begin(), v.end(), std::mt19937{std::random_device{}()});
        std::sort(v.begin(), v.end());
    }, 10);

    benchmark("string查找", []() {
        std::string s(10000, 'a');
        s[5000] = 'b';
        volatile auto pos = s.find('b');
        (void)pos;
    }, 100);
}

void demo_sleep_delay() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  延时与等待\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. sleep_for —— 固定时长等待:\n";
    auto t1 = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto t2 = std::chrono::steady_clock::now();
    std::cout << "  sleep_for(50ms) 实际: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()
              << " ms\n";

    std::cout << "\n2. sleep_until —— 等到指定时间点:\n";
    auto target = std::chrono::steady_clock::now() + std::chrono::milliseconds(50);
    t1 = std::chrono::steady_clock::now();
    std::this_thread::sleep_until(target);
    t2 = std::chrono::steady_clock::now();
    std::cout << "  sleep_until(+50ms) 实际: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()
              << " ms\n";

    std::cout << "\n3. yield —— 让出CPU时间片:\n";
    std::cout << "  std::this_thread::yield() 提示调度器切换线程\n";
    std::cout << "  适用于忙等待中的优化\n";

    std::cout << "\n注意: sleep 精度受操作系统调度影响，通常为1-15ms\n";
    std::cout << "  Windows默认时间片约15.6ms\n";
    std::cout << "  Linux默认时间片约1-4ms\n";
}

void demo_date_formatting() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  日期时间格式化 (C++17方式)\n";
    std::cout << "═══════════════════════════════════════\n\n";

    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    // 注意: std::localtime 非线程安全! 多线程环境应使用 localtime_r/localtime_s
    std::tm* local_tm = std::localtime(&now_time);

    char buf[64];

    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", local_tm);
    std::cout << "标准格式: " << buf << "\n";

    std::strftime(buf, sizeof(buf), "%Y年%m月%d日 %H时%M分%S秒", local_tm);
    std::cout << "中文格式: " << buf << "\n";

    std::strftime(buf, sizeof(buf), "%A, %B %d, %Y", local_tm);
    std::cout << "英文格式: " << buf << "\n";

    std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", local_tm);
    std::cout << "文件名格式: " << buf << "\n";

    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", local_tm);
    std::cout << "ISO 8601: " << buf << "\n";

    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    std::cout << "带毫秒:   " << buf << "." << std::setfill('0') << std::setw(3) << ms.count() << "\n";

    std::cout << "\nC++20 预览: std::format 与 chrono\n";
    std::cout << "  C++20 可直接: std::format(\"{:%Y-%m-%d %H:%M:%S}\", now)\n";
    std::cout << "  C++20 新增: std::chrono::year_month_day, hh_mm_ss 等\n";
    std::cout << "  C++20 新增: 时区支持 <chrono> tzdb\n";
}

void demo_countdown_timer() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  倒计时器示例\n";
    std::cout << "═══════════════════════════════════════\n\n";

    int countdown_seconds = 3;
    std::cout << "倒计时 " << countdown_seconds << " 秒:\n";

    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(countdown_seconds);
    while (std::chrono::steady_clock::now() < deadline) {
        auto remaining = std::chrono::duration_cast<std::chrono::seconds>(
            deadline - std::chrono::steady_clock::now());
        std::cout << "  剩余: " << remaining.count() << " 秒\r" << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    std::cout << "  时间到!          \n";
}

int main() {
    demo_basic_timer();
    demo_scoped_timer();
    demo_benchmark();
    demo_sleep_delay();
    demo_date_formatting();
    demo_countdown_timer();
    return 0;
}
