/** @file 01_deep_dive_chrono_patterns.cpp
 *  @brief 类型安全的时间算术、自定义duration、取整、日历(C++20预览)
 *  @description 对应文档: 02-CPP/17-chrono | 举一反三：chrono的类型系统深度、自定义时间单位、取整策略
 *  编译命令: g++ -std=c++20 01_deep_dive_chrono_patterns.cpp -o 01_deep_dive_chrono_patterns
 */

#include <iostream>
#include <chrono>
#include <thread>
#include <ratio>
#include <type_traits>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>

void demo_type_safe_arithmetic() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  类型安全的时间算术\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "chrono 的核心设计: 编译期防止时间单位混淆\n\n";

    std::chrono::seconds sec(5);
    std::chrono::milliseconds ms(500);

    auto result1 = sec + ms;
    std::cout << "seconds(5) + milliseconds(500) 的类型: milliseconds\n";
    std::cout << "  结果: " << result1.count() << " ms\n\n";

    auto result2 = ms + sec;
    std::cout << "milliseconds(500) + seconds(5) 的类型: milliseconds\n";
    std::cout << "  结果: " << result2.count() << " ms\n\n";

    std::cout << "隐式转换规则:\n";
    std::cout << "  精度高的 → 精度低的: 需要显式 duration_cast\n";
    std::cout << "  精度低的 → 精度高的: 隐式转换(安全)\n\n";

    std::chrono::milliseconds ms2 = std::chrono::seconds(3);
    std::cout << "  seconds → milliseconds: 隐式, " << ms2.count() << " ms\n";

    auto sec2 = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::milliseconds(3500));
    std::cout << "  milliseconds → seconds: 显式, " << sec2.count() << " s (截断)\n";

    std::cout << "\n编译期类型检查:\n";
    std::cout << "  以下操作会在编译期报错:\n";
    std::cout << "    time_point + time_point  → 非法\n";
    std::cout << "    duration + time_point    → 非法\n";
    std::cout << "    time_point - duration    → 合法(返回time_point)\n";
    std::cout << "    time_point - time_point  → 合法(返回duration)\n";

    auto tp1 = std::chrono::steady_clock::now();
    auto tp2 = tp1 + std::chrono::seconds(10);
    auto diff = tp2 - tp1;
    std::cout << "\n  time_point + duration = time_point: 合法\n";
    std::cout << "  time_point - time_point = duration: "
              << std::chrono::duration_cast<std::chrono::seconds>(diff).count() << " s\n";
}

void demo_custom_duration() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  自定义 duration 类型\n";
    std::cout << "═══════════════════════════════════════\n\n";

    using frames = std::chrono::duration<double, std::ratio<1, 60>>;
    using days = std::chrono::duration<int, std::ratio<86400>>;
    using weeks = std::chrono::duration<int, std::ratio<604800>>;
    using years = std::chrono::duration<double, std::ratio<31556952>>;
    using deci_seconds = std::chrono::duration<int, std::deci>;

    frames f(1.0);
    auto f_ms = std::chrono::duration_cast<std::chrono::milliseconds>(f);
    std::cout << "1帧(1/60秒) = " << f_ms.count() << " ms\n";

    days d(365);
    auto d_hours = std::chrono::duration_cast<std::chrono::hours>(d);
    std::cout << "365天 = " << d_hours.count() << " 小时\n";

    weeks w(1);
    auto w_days = std::chrono::duration_cast<days>(w);
    std::cout << "1周 = " << w_days.count() << " 天\n";

    years y(1.0);
    auto y_days = std::chrono::duration_cast<days>(y);
    std::cout << "1年 ≈ " << y_days.count() << " 天\n";

    deci_seconds ds(5);
    auto ds_ms = std::chrono::duration_cast<std::chrono::milliseconds>(ds);
    std::cout << "5分秒(0.1秒) = " << ds_ms.count() << " ms\n";

    std::cout << "\n自定义字面量(C++14起):\n";
    using namespace std::chrono_literals;
    auto time = 1h + 30min + 45s + 500ms;
    auto total_sec = std::chrono::duration_cast<std::chrono::seconds>(time);
    std::cout << "1h + 30min + 45s + 500ms = " << total_sec.count() << " 秒\n";
}

void demo_rounding() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  时间取整策略\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::chrono::milliseconds ms(2750);

    auto trunc_sec = std::chrono::duration_cast<std::chrono::seconds>(ms);
    std::cout << "2750ms 截断取整: " << trunc_sec.count() << " s\n";

    auto ceil_sec = std::chrono::ceil<std::chrono::seconds>(ms);
    std::cout << "2750ms 向上取整: " << ceil_sec.count() << " s\n";

    auto floor_sec = std::chrono::floor<std::chrono::seconds>(ms);
    std::cout << "2750ms 向下取整: " << floor_sec.count() << " s\n";

    auto round_sec = std::chrono::round<std::chrono::seconds>(ms);
    std::cout << "2750ms 四舍五入: " << round_sec.count() << " s\n";

    std::chrono::milliseconds ms2(2500);
    auto round2 = std::chrono::round<std::chrono::seconds>(ms2);
    std::cout << "2500ms 四舍五入: " << round2.count() << " s (银行家舍入)\n";

    std::cout << "\ntime_point 取整:\n";
    auto tp = std::chrono::system_clock::now();
    auto tp_ceil = std::chrono::ceil<std::chrono::seconds>(tp);
    auto tp_floor = std::chrono::floor<std::chrono::seconds>(tp);
    auto tp_round = std::chrono::round<std::chrono::seconds>(tp);
    std::cout << "  当前时间点取整到秒级\n";
    std::cout << "  ceil:  " << std::chrono::system_clock::to_time_t(tp_ceil) << "\n";
    std::cout << "  floor: " << std::chrono::system_clock::to_time_t(tp_floor) << "\n";
    std::cout << "  round: " << std::chrono::system_clock::to_time_t(tp_round) << "\n";
}

void demo_calendar_cpp20_preview() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  C++20 日历库预览\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "C++20 新增的日历类型:\n\n";

    std::cout << "1. year_month_day —— 日期表示\n";
    std::cout << "   auto ymd = 2024y/January/15d;\n";
    std::cout << "   auto ymd = year{2024}/month{1}/day{15};\n\n";

    std::cout << "2. hh_mm_ss —— 时分秒表示\n";
    std::cout << "   auto time = hh_mm_ss{3h + 25min + 45s};\n";
    std::cout << "   time.hours() → 3, time.minutes() → 25\n\n";

    std::cout << "3. year_month_day_last —— 月末日期\n";
    std::cout << "   auto last_day = 2024y/February/last;\n";
    std::cout << "   可自动处理闰年: 2024年2月有29天\n\n";

    std::cout << "4. weekday —— 星期几\n";
    std::cout << "   auto wd = weekday{year_month_day{sys_days{now}}};\n\n";

    std::cout << "5. 时区支持\n";
    std::cout << "   auto now = zoned_time{current_zone(), system_clock::now()};\n";
    std::cout << "   auto ny = zoned_time{\"America/New_York\", now};\n\n";

    std::cout << "在C++17中模拟部分功能:\n";
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm* local_tm = std::localtime(&now_time);

    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", local_tm);
    std::cout << "  当前日期: " << buf << "\n";
    std::strftime(buf, sizeof(buf), "%H:%M:%S", local_tm);
    std::cout << "  当前时间: " << buf << "\n";
    std::strftime(buf, sizeof(buf), "%A", local_tm);
    std::cout << "  星期: " << buf << "\n";
    std::strftime(buf, sizeof(buf), "%B", local_tm);
    std::cout << "  月份: " << buf << "\n";
}

void demo_duration_common_pitfalls() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  chrono 常见陷阱\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. 整数溢出:\n";
    std::chrono::hours long_time(250000);
    auto in_sec = std::chrono::duration_cast<std::chrono::seconds>(long_time);
    std::cout << "  250000小时 = " << in_sec.count() << " 秒\n";
    std::cout << "  注意: seconds::rep 是 long long，足够大\n";
    std::cout << "  但自定义小类型可能溢出\n\n";

    std::cout << "2. duration_cast 截断:\n";
    std::chrono::milliseconds ms(999);
    auto sec = std::chrono::duration_cast<std::chrono::seconds>(ms);
    std::cout << "  999ms → " << sec.count() << "s (截断为0!)\n";
    std::cout << "  解决: 使用 ceil/floor/round\n\n";

    std::cout << "3. system_clock 不适合测量间隔:\n";
    auto sys1 = std::chrono::system_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    auto sys2 = std::chrono::system_clock::now();
    auto sys_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(sys2 - sys1);

    auto steady1 = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    auto steady2 = std::chrono::steady_clock::now();
    auto steady_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(steady2 - steady1);

    std::cout << "  system_clock 测量sleep(10ms): " << sys_elapsed.count() << " ms\n";
    std::cout << "  steady_clock 测量sleep(10ms): " << steady_elapsed.count() << " ms\n";
    std::cout << "  system_clock 可能受NTP校时影响\n\n";

    std::cout << "4. 比较不同类型的 duration:\n";
    std::chrono::seconds s1(1);
    std::chrono::milliseconds ms1(1000);
    std::cout << "  seconds(1) == milliseconds(1000): " << (s1 == ms1) << "\n";
    std::cout << "  不同类型可直接比较，自动转换为公共类型\n";
}

int main() {
    demo_type_safe_arithmetic();
    demo_custom_duration();
    demo_rounding();
    demo_calendar_cpp20_preview();
    demo_duration_common_pitfalls();
    return 0;
}
