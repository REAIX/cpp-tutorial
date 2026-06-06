/** @file 02_deep_dive_chrono_advanced.cpp
 *  @brief 高精度基准测试、单调时钟vs挂钟、时区处理概念
 *  @description 对应文档: 02-CPP/17-chrono | 举一反三：深入理解时钟特性、精确计时、时区概念
 *  编译命令: g++ -std=c++20 02_deep_dive_chrono_advanced.cpp -o 02_deep_dive_chrono_advanced
 */

#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <unordered_map>
#include <ctime>
#include <iomanip>

void demo_high_resolution_benchmark() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  高精度基准测试\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. 避免优化消除:\n";
    std::cout << "   volatile 防止编译器消除\"无用\"计算\n\n";

    auto bench_volatile = []() -> double {
        auto start = std::chrono::high_resolution_clock::now();
        volatile int result = 0;
        for (int i = 0; i < 1000000; i++) {
            result = result + i;
        }
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::nano>(end - start).count() / 1000000.0;
    };

    double ns_per_op = bench_volatile();
    std::cout << "  volatile 加法: " << ns_per_op << " ns/op\n\n";

    std::cout << "2. 多次运行取统计值:\n";

    auto run_benchmark = [](const std::string& name, auto func, int runs) {
        std::vector<double> times;
        times.reserve(runs);

        for (int i = 0; i < runs; i++) {
            auto start = std::chrono::high_resolution_clock::now();
            func();
            auto end = std::chrono::high_resolution_clock::now();
            times.push_back(std::chrono::duration<double, std::micro>(end - start).count());
        }

        std::sort(times.begin(), times.end());

        double sum = std::accumulate(times.begin(), times.end(), 0.0);
        double mean = sum / times.size();
        double sq_sum = 0;
        for (double t : times) sq_sum += (t - mean) * (t - mean);
        double stddev = std::sqrt(sq_sum / times.size());

        std::cout << "  " << name << ":\n";
        std::cout << "    平均: " << mean << " us\n";
        std::cout << "    标准差: " << stddev << " us\n";
        std::cout << "    最小: " << times.front() << " us\n";
        std::cout << "    中位: " << times[times.size() / 2] << " us\n";
        std::cout << "    P95: " << times[times.size() * 95 / 100] << " us\n";
        std::cout << "    最大: " << times.back() << " us\n\n";
    };

    run_benchmark("vector排序(1万元素)", []() {
        std::vector<int> v(10000);
        for (int i = 0; i < 10000; i++) v[i] = 10000 - i;
        std::sort(v.begin(), v.end());
    }, 20);

    run_benchmark("unordered_map查找", []() {
        std::unordered_map<int, int> m;
        for (int i = 0; i < 1000; i++) m[i] = i * 2;
        volatile int val = m[500];
        (void)val;
    }, 50);

    std::cout << "3. 预热(Warm-up):\n";
    std::cout << "   首次运行可能因缓存未命中而偏慢\n";
    std::cout << "   建议: 丢弃前几次结果，或先运行几次预热\n";
}

void demo_monotonic_vs_wall_clock() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  单调时钟 vs 挂钟\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "system_clock (挂钟):\n";
    std::cout << "  - 反映实际日期时间\n";
    std::cout << "  - 可能被NTP、手动调整等修改\n";
    std::cout << "  - 可能出现时间\"倒流\"\n";
    std::cout << "  - 用途: 日志时间戳、文件修改时间\n\n";

    std::cout << "steady_clock (单调时钟):\n";
    std::cout << "  - 保证单调递增，永不倒流\n";
    std::cout << "  - 不受系统时间调整影响\n";
    std::cout << "  - 用途: 测量时间间隔、超时控制\n\n";

    auto sys_now = std::chrono::system_clock::now();
    auto steady_now = std::chrono::steady_clock::now();

    std::cout << "时钟精度:\n";
    auto sys_period = std::chrono::system_clock::period();
    auto steady_period = std::chrono::steady_clock::period();
    std::cout << "  system_clock 精度: 1/" << (sys_period.den / sys_period.num) << " 秒\n";
    std::cout << "  steady_clock 精度: 1/" << (steady_period.den / steady_period.num) << " 秒\n";

    std::cout << "\nis_steady 属性:\n";
    std::cout << "  system_clock::is_steady = " << std::chrono::system_clock::is_steady << "\n";
    std::cout << "  steady_clock::is_steady = " << std::chrono::steady_clock::is_steady << "\n";
    std::cout << "  high_resolution_clock::is_steady = " << std::chrono::high_resolution_clock::is_steady << "\n";

    std::cout << "\n实际影响演示:\n";
    auto t1 = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto t2 = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
    std::cout << "  steady_clock 测量 sleep(50ms): " << elapsed.count() << " ms\n";
    std::cout << "  即使期间系统时间被修改，测量结果也不受影响\n";
}

void demo_time_zone_concept() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  时区处理概念\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "C++17 中的时区处理(手动方式):\n\n";

    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    std::cout << "1. UTC 时间:\n";
    std::tm* utc_tm = std::gmtime(&now_time);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", utc_tm);
    std::cout << "  UTC: " << buf << "\n\n";

    std::cout << "2. 本地时间:\n";
    std::tm* local_tm = std::localtime(&now_time);
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S %Z", local_tm);
    std::cout << "  本地: " << buf << "\n\n";

    std::cout << "3. 计算时区偏移:\n";
    auto utc_offset = local_tm->tm_hour - utc_tm->tm_hour;
    if (utc_offset > 12) utc_offset -= 24;
    if (utc_offset < -12) utc_offset += 24;
    std::cout << "  UTC" << (utc_offset >= 0 ? "+" : "") << utc_offset << ":00\n\n";

    std::cout << "4. C++20 时区支持:\n";
    std::cout << "  #include <chrono> (C++20扩展)\n";
    std::cout << "  auto now = std::chrono::system_clock::now();\n";
    std::cout << "  auto local = std::chrono::zoned_time{std::chrono::current_zone(), now};\n";
    std::cout << "  auto ny = std::chrono::zoned_time{\"America/New_York\", now};\n";
    std::cout << "  auto tokyo = std::chrono::zoned_time{\"Asia/Tokyo\", now};\n\n";

    std::cout << "5. C++20 日历与时区结合:\n";
    std::cout << "  auto meeting = 2024y/March/15d + 14h + 30min;\n";
    std::cout << "  auto ny_time = zoned_time{\"America/New_York\", meeting};\n";
    std::cout << "  auto bj_time = zoned_time{\"Asia/Shanghai\", meeting};\n";
    std::cout << "  // 同一时刻在不同时区的本地表示\n\n";

    std::cout << "6. 夏令时(DST)注意事项:\n";
    std::cout << "  - 夏令时切换时存在\"不存在的时间\"和\"重复的时间\"\n";
    std::cout << "  - C++20 的 time_zone 库自动处理 DST\n";
    std::cout << "  - C++17 需要手动处理，建议使用操作系统API\n";
}

void demo_timer_patterns() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  高级计时模式\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. 超时控制模式:\n";
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(100);
    int iterations = 0;
    while (std::chrono::steady_clock::now() < deadline) {
        volatile int x = 0;
        for (int i = 0; i < 1000; i++) x += i;
        iterations++;
    }
    std::cout << "  100ms内执行了 " << iterations << " 次迭代\n\n";

    std::cout << "2. 限速模式(固定帧率):\n";
    using namespace std::chrono_literals;
    auto frame_time = 33ms;
    for (int frame = 0; frame < 5; frame++) {
        auto frame_start = std::chrono::steady_clock::now();

        volatile int work = 0;
        for (int i = 0; i < 100000; i++) work += i;

        auto frame_end = std::chrono::steady_clock::now();
        auto elapsed = frame_end - frame_start;
        if (elapsed < frame_time) {
            std::this_thread::sleep_for(frame_time - elapsed);
        }
        auto actual = std::chrono::steady_clock::now() - frame_start;
        std::cout << "  帧 " << frame << ": "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(actual).count()
                  << " ms\n";
    }

    std::cout << "\n3. 滑动窗口计时:\n";
    std::vector<std::chrono::steady_clock::time_point> timestamps;
    auto window = std::chrono::seconds(1);
    for (int i = 0; i < 10; i++) {
        timestamps.push_back(std::chrono::steady_clock::now());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        auto cutoff = std::chrono::steady_clock::now() - window;
        auto it = std::remove_if(timestamps.begin(), timestamps.end(),
            [cutoff](const auto& tp) { return tp < cutoff; });
        timestamps.erase(it, timestamps.end());
    }
    std::cout << "  最近1秒内的采样数: " << timestamps.size() << "\n";
}

int main() {
    demo_high_resolution_benchmark();
    demo_monotonic_vs_wall_clock();
    demo_time_zone_concept();
    demo_timer_patterns();
    return 0;
}
