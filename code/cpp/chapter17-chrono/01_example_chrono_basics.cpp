/** @file 01_example_chrono_basics.cpp
 *  @brief C++ chrono库基础：duration, time_point, clock类型
 *  @description 对应文档: 02-CPP/17-chrono | 演示时间库的核心概念和基本用法
 *  编译命令: g++ -std=c++20 01_example_chrono_basics.cpp -o 01_example_chrono_basics
 */

#include <iostream>
#include <chrono>
#include <thread>
#include <string>

void demo_duration() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  duration —— 时间段\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::chrono::seconds sec(5);
    std::chrono::milliseconds ms(500);
    std::chrono::microseconds us(1000);
    std::chrono::nanoseconds ns(1000);
    std::chrono::minutes min(2);
    std::chrono::hours hr(1);

    std::cout << "常用 duration 类型:\n";
    std::cout << "  seconds(5)      = " << sec.count() << " 秒\n";
    std::cout << "  milliseconds(500) = " << ms.count() << " 毫秒\n";
    std::cout << "  microseconds(1000) = " << us.count() << " 微秒\n";
    std::cout << "  nanoseconds(1000) = " << ns.count() << " 纳秒\n";
    std::cout << "  minutes(2)      = " << min.count() << " 分钟\n";
    std::cout << "  hours(1)        = " << hr.count() << " 小时\n";

    std::cout << "\nduration 算术运算:\n";
    auto sum = std::chrono::seconds(3) + std::chrono::milliseconds(500);
    std::cout << "  3秒 + 500毫秒 = " << sum.count() << " 毫秒\n";

    auto diff = std::chrono::minutes(5) - std::chrono::seconds(30);
    std::cout << "  5分钟 - 30秒 = " << diff.count() << " 秒\n";

    auto scaled = std::chrono::seconds(2) * 3;
    std::cout << "  2秒 * 3 = " << scaled.count() << " 秒\n";

    auto divided = std::chrono::seconds(10) / 3;
    std::cout << "  10秒 / 3 = " << divided.count() << " 秒\n";
}

void demo_duration_cast() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  duration_cast —— 时间单位转换\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::chrono::milliseconds ms(2500);
    auto sec = std::chrono::duration_cast<std::chrono::seconds>(ms);
    std::cout << "2500毫秒 → " << sec.count() << " 秒 (截断)\n";

    std::chrono::seconds s(3661);
    auto h = std::chrono::duration_cast<std::chrono::hours>(s);
    auto m = std::chrono::duration_cast<std::chrono::minutes>(s % std::chrono::hours(1));
    auto remain_sec = s % std::chrono::minutes(1);
    std::cout << "3661秒 → " << h.count() << "小时 " << m.count() << "分 "
              << remain_sec.count() << "秒\n";

    std::cout << "\n隐式转换(小单位→大单位，仅当精确时允许):\n";
    std::chrono::milliseconds ms2 = std::chrono::seconds(3);
    std::cout << "  seconds(3) → milliseconds: " << ms2.count() << " ms\n";

    std::cout << "\n自定义 duration 类型:\n";
    using frame_duration = std::chrono::duration<double, std::ratio<1, 60>>;
    frame_duration frame(1.0);
    auto frame_ms = std::chrono::duration_cast<std::chrono::milliseconds>(frame);
    std::cout << "  1帧(1/60秒) = " << frame_ms.count() << " 毫秒\n";

    using days = std::chrono::duration<int, std::ratio<86400>>;
    days d(7);
    auto d_hours = std::chrono::duration_cast<std::chrono::hours>(d);
    std::cout << "  7天 = " << d_hours.count() << " 小时\n";
}

void demo_time_point() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  time_point —— 时间点\n";
    std::cout << "═══════════════════════════════════════\n\n";

    auto now = std::chrono::system_clock::now();
    auto epoch = std::chrono::system_clock::time_point{};
    auto since_epoch = now - epoch;
    auto secs = std::chrono::duration_cast<std::chrono::seconds>(since_epoch);

    std::cout << "time_point = clock 的 epoch + duration\n\n";
    std::cout << "当前时间距 epoch 的秒数: " << secs.count() << "\n";

    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::cout << "当前时间(ctime): " << std::ctime(&now_time);

    auto future = now + std::chrono::hours(24);
    std::time_t future_time = std::chrono::system_clock::to_time_t(future);
    std::cout << "24小时后: " << std::ctime(&future_time);

    std::cout << "time_point 算术:\n";
    auto tp1 = std::chrono::system_clock::now();
    auto tp2 = tp1 + std::chrono::minutes(30);
    auto diff = tp2 - tp1;
    std::cout << "  时间点差: " << std::chrono::duration_cast<std::chrono::seconds>(diff).count()
              << " 秒\n";
}

void demo_clock_types() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  三种时钟类型\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. system_clock —— 系统挂钟时间\n";
    std::cout << "   - 可与 time_t 互转，用于获取当前日期时间\n";
    std::cout << "   - 可能受系统时间调整影响(如NTP校时)\n";
    auto sys_now = std::chrono::system_clock::now();
    std::time_t sys_time = std::chrono::system_clock::to_time_t(sys_now);
    std::cout << "   当前: " << std::ctime(&sys_time);

    std::cout << "2. steady_clock —— 单调递增时钟\n";
    std::cout << "   - 保证后续调用不会返回更早的时间\n";
    std::cout << "   - 不受系统时间调整影响\n";
    std::cout << "   - 适合测量时间间隔\n";
    auto steady1 = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    auto steady2 = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(steady2 - steady1);
    std::cout << "   sleep 10ms 实际耗时: " << elapsed.count() << " ms\n";

    std::cout << "3. high_resolution_clock —— 最高精度时钟\n";
    std::cout << "   - 通常是 steady_clock 或其别名\n";
    std::cout << "   - 用于微基准测试\n";
    auto hr1 = std::chrono::high_resolution_clock::now();
    volatile int sum = 0;  // volatile 防止优化消除, 生产级基准测试建议用 Google Benchmark
    for (int i = 0; i < 1000; i++) sum += i;
    auto hr2 = std::chrono::high_resolution_clock::now();
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(hr2 - hr1);
    std::cout << "   1000次加法耗时: " << ns.count() << " 纳秒\n";

    std::cout << "\n时钟选择建议:\n";
    std::cout << "  获取日期时间 → system_clock\n";
    std::cout << "  测量时间间隔 → steady_clock\n";
    std::cout << "  高精度计时   → high_resolution_clock\n";
}

void demo_literals() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  chrono 字面量 (C++14起)\n";
    std::cout << "═══════════════════════════════════════\n\n";

    using namespace std::chrono_literals;

    auto d1 = 5s;
    auto d2 = 250ms;
    auto d3 = 100us;
    auto d4 = 500ns;
    auto d5 = 30min;
    auto d6 = 2h;

    std::cout << "5s   = " << d1.count() << " 秒\n";
    std::cout << "250ms = " << d2.count() << " 毫秒\n";
    std::cout << "100us = " << d3.count() << " 微秒\n";
    std::cout << "500ns = " << d4.count() << " 纳秒\n";
    std::cout << "30min = " << d5.count() << " 分钟\n";
    std::cout << "2h   = " << d6.count() << " 小时\n";

    auto total = 1h + 30min + 45s;
    auto total_sec = std::chrono::duration_cast<std::chrono::seconds>(total);
    std::cout << "\n1h + 30min + 45s = " << total_sec.count() << " 秒\n";
}

int main() {
    demo_duration();
    demo_duration_cast();
    demo_time_point();
    demo_clock_types();
    demo_literals();
    return 0;
}
