/**
 * @file 02_deep_dive_ranges_patterns.cpp
 * @brief Ranges惰性求值与性能模式
 * @description 对应文档: 02-CPP/24-ranges
 */

#include <iostream>
#include <vector>
#include <ranges>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <string>
#include <sstream>

void demo_lazy_evaluation() {
    std::cout << "\n=== 惰性求值 ===\n";

    std::vector<int> nums = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    int transform_calls = 0;
    auto transformed = nums | std::views::transform([&transform_calls](int n) {
        ++transform_calls;
        std::cout << "  transform(" << n << ")\n";
        return n * n;
    });

    std::cout << "定义视图 - 此时不会调用transform\n";
    std::cout << "transform调用次数: " << transform_calls << "\n";

    std::cout << "开始遍历(只取前3个):\n";
    auto first3 = transformed | std::views::take(3);
    for (int n : first3) {
        std::cout << "  得到: " << n << "\n";
    }
    std::cout << "总调用次数: " << transform_calls << "\n";
    std::cout << "关键: 只计算了需要的元素, 跳过了7-10\n";
}

void demo_composition_patterns() {
    std::cout << "\n=== 组合模式 ===\n";

    std::vector<int> nums = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    auto pipeline = nums
        | std::views::filter([](int n) { return n % 3 == 0; })
        | std::views::transform([](int n) { return n * n; })
        | std::views::take(3)
        | std::views::reverse;
    std::cout << "3的倍数->平方->取3->反转: ";
    for (int n : pipeline) std::cout << n << " ";
    std::cout << "\n";

    std::cout << "chunk(C++23): 每4个一组\n";
    std::cout << "slide(C++23): 滑动窗口(大小3)\n";
    std::cout << "join(C++23): 展平嵌套\n";

    std::cout << "C++20替代方案 - 手动分块:\n";
    std::vector<std::vector<int>> chunks;
    for (size_t i = 0; i < nums.size(); i += 4) {
        chunks.emplace_back(nums.begin() + i, nums.begin() + std::min(i + 4, nums.size()));
    }
    for (const auto& chunk : chunks) {
        std::cout << "[";
        for (int n : chunk) std::cout << n << " ";
        std::cout << "] ";
    }
    std::cout << "\n";
}

void demo_ranges_vs_raw_loops() {
    std::cout << "\n=== Ranges vs 原始循环 ===\n";

    std::vector<int> nums;
    for (int i = 1; i <= 1000000; ++i) nums.push_back(i);

    auto start = std::chrono::high_resolution_clock::now();
    long long sum_raw = 0;
    for (int n : nums) {
        if (n % 2 == 0) {
            int sq = n * n;
            if (sq < 1000000) {
                sum_raw += sq;
            }
        }
    }
    auto end_raw = std::chrono::high_resolution_clock::now();
    auto raw_ms = std::chrono::duration_cast<std::chrono::microseconds>(end_raw - start).count();

    start = std::chrono::high_resolution_clock::now();
    auto view = nums
        | std::views::filter([](int n) { return n % 2 == 0; })
        | std::views::transform([](int n) { return n * n; })
        | std::views::filter([](int sq) { return sq < 1000000; });
    long long sum_ranges = 0;
    for (int n : view) sum_ranges += n;
    auto end_ranges = std::chrono::high_resolution_clock::now();
    auto ranges_ms = std::chrono::duration_cast<std::chrono::microseconds>(end_ranges - start).count();

    std::cout << "原始循环: sum=" << sum_raw << " 耗时=" << raw_ms << "us\n";
    std::cout << "Ranges:   sum=" << sum_ranges << " 耗时=" << ranges_ms << "us\n";
    std::cout << "结果一致: " << std::boolalpha << (sum_raw == sum_ranges) << "\n";
    std::cout << "注意: 优化开启后, 性能差异通常很小\n";
}

void demo_materialization() {
    std::cout << "\n=== 视图物化(Materialization) ===\n";

    auto view = std::views::iota(1, 11)
        | std::views::filter([](int n) { return n % 2 == 0; })
        | std::views::transform([](int n) { return n * n; });

    std::vector<int> materialized;
    std::ranges::copy(view, std::back_inserter(materialized));
    std::cout << "物化到vector: ";
    for (int n : materialized) std::cout << n << " ";
    std::cout << "\n";

    std::cout << "C++23 from_range构造: vector<int>(from_range, view)\n";

    std::string joined;
    auto strs = std::views::iota(1, 6)
        | std::views::transform([](int n) { return std::to_string(n); });
    bool first = true;
    for (const auto& s : strs) {
        if (!first) joined += ",";
        joined += s;
        first = false;
    }
    std::cout << "拼接字符串: " << joined << "\n";
}

void demo_best_practices() {
    std::cout << "\n=== 最佳实践 ===\n";

    std::cout << "1. 优先使用管道操作符 | 组合视图\n";
    std::vector<int> v = {1, 2, 3, 4, 5};
    auto r1 = v | std::views::filter([](int n) { return n > 2; })
                | std::views::transform([](int n) { return n * 10; });
    for (int n : r1) std::cout << n << " ";
    std::cout << "\n";

    std::cout << "\n2. 避免过度组合, 保持可读性\n";
    std::cout << "   好的做法: 每步一个清晰的变换\n";
    std::cout << "   避免: 10+个管道操作链式调用\n";

    std::cout << "\n3. 注意视图的生命周期\n";
    std::cout << "   错误: return std::vector<int>{1,2,3} | views::take(2);\n";
    std::cout << "   正确: 先保存vector, 再取视图\n";

    std::cout << "\n4. 对需要多次遍历的场景, 考虑物化\n";
    auto expensive_view = std::views::iota(1, 1000000)
        | std::views::transform([](int n) { return n * n; })
        | std::views::filter([](int n) { return n % 7 == 0; })
        | std::views::take(100);
    std::vector<int> cached;
    for (int n : expensive_view) cached.push_back(n);
    std::cout << "   缓存后可反复使用, 避免重复计算\n";

    std::cout << "\n5. 使用ranges算法替代<algorithm>中的经典算法\n";
    std::ranges::sort(v);
    std::cout << "   ranges::sort(v) 比 std::sort(v.begin(), v.end()) 更简洁\n";
}

void demo_real_world_pattern() {
    std::cout << "\n=== 实际应用模式 ===\n";

    struct Order {
        std::string product;
        double price;
        int quantity;
    };

    std::vector<Order> orders = {
        {"apple", 5.0, 10}, {"banana", 3.0, 20}, {"cherry", 15.0, 5},
        {"date", 8.0, 15}, {"elderberry", 25.0, 2}
    };

    auto expensive = orders
        | std::views::filter([](const Order& o) { return o.price > 5.0; })
        | std::views::transform([](const Order& o) { return o.price * o.quantity; });
    double total = 0;
    for (double v : expensive) total += v;
    std::cout << "单价>5的订单总额: " << total << "\n";

    auto top_orders = orders
        | std::views::transform([](const Order& o) -> std::pair<std::string, double> {
            return {o.product, o.price * o.quantity};
        });
    std::cout << "订单金额:\n";
    for (const auto& [name, amount] : top_orders) {
        std::cout << "  " << name << ": " << amount << "\n";
    }

    std::vector<int> data = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};
    auto unique_sorted = data;
    std::ranges::sort(unique_sorted);
    auto last = std::unique(unique_sorted.begin(), unique_sorted.end());
    unique_sorted.erase(last, unique_sorted.end());
    std::cout << "排序去重: ";
    for (int n : unique_sorted) std::cout << n << " ";
    std::cout << "\n";
}

int main() {
    std::cout << "========== Ranges惰性求值与性能模式 ==========\n";

    demo_lazy_evaluation();
    demo_composition_patterns();
    demo_ranges_vs_raw_loops();
    demo_materialization();
    demo_best_practices();
    demo_real_world_pattern();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
