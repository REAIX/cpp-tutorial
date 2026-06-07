/** @file 01_deep_dive_cpp14_patterns.cpp
 *  @brief C++14实战、弃用特性、改进的constexpr、sized deallocation
 *  @description 对应文档: 02-CPP/21-cpp14 | 举一反三：C++14在实践中的应用和注意事项
 *  编译命令: g++ -std=c++20 01_deep_dive_cpp14_patterns.cpp -o 01_deep_dive_cpp14_patterns
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <functional>
#include <utility>
#include <type_traits>
#include <random>

constexpr int gcd(int a, int b) {
    while (b != 0) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

constexpr int lcm(int a, int b) {
    return a / gcd(a, b) * b;
}

constexpr bool is_prime(int n) {
    if (n < 2) return false;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return false;
    }
    return true;
}

void demo_cpp14_in_practice() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  C++14 实战模式\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. 泛型lambda实现策略模式:\n";
    auto strategies = std::make_tuple(
        [](auto x) { return x * 2; },
        [](auto x) { return x + 10; },
        [](auto x) { return x * x; }
    );

    int val = 5;
    std::cout << "  原始值: " << val << "\n";
    std::cout << "  策略1(×2): " << std::get<0>(strategies)(val) << "\n";
    std::cout << "  策略2(+10): " << std::get<1>(strategies)(val) << "\n";
    std::cout << "  策略3(²): " << std::get<2>(strategies)(val) << "\n\n";

    std::cout << "2. 初始化捕获(广义lambda捕获):\n";
    auto ptr = std::make_unique<int>(42);
    auto lambda = [p = std::move(ptr)]() {
        return p ? *p : -1;
    };
    std::cout << "  初始化捕获移动unique_ptr: " << lambda() << "\n";
    std::cout << "  移动后 ptr=" << (ptr ? "非空" : "空") << "\n\n";

    std::cout << "3. lambda 中的初始化捕获:\n";
    double factor = 2.5;
    auto scale = [factor_copy = factor](auto x) {
        return x * factor_copy;
    };
    std::cout << "  scale(10) = " << scale(10) << "\n";
    std::cout << "  scale(3.14) = " << scale(3.14) << "\n\n";

    std::cout << "4. 返回auto的递归函数:\n";
    std::function<int(int)> fib = [&fib](int n) -> int {
        if (n <= 1) return n;
        return fib(n - 1) + fib(n - 2);
    };
    std::cout << "  fib(10) = " << fib(10) << "\n";
}

void demo_constexpr_advanced() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  C++14 constexpr 高级用法\n";
    std::cout << "═══════════════════════════════════════\n\n";

    constexpr int g = gcd(12, 18);
    constexpr int l = lcm(4, 6);
    std::cout << "gcd(12, 18) = " << g << " (编译期)\n";
    std::cout << "lcm(4, 6) = " << l << " (编译期)\n\n";

    std::cout << "编译期素数判断:\n";
    for (int i = 2; i <= 20; i++) {
        if (is_prime(i)) std::cout << "  " << i;
    }
    std::cout << "\n\n";

    std::cout << "constexpr 函数的双重性质:\n";
    std::cout << "  - 参数为常量表达式时 → 编译期计算\n";
    std::cout << "  - 参数为运行时值时 → 运行时计算\n\n";

    int runtime_val = 15;
    int runtime_gcd = gcd(runtime_val, 25);
    std::cout << "  运行时 gcd(15, 25) = " << runtime_gcd << "\n";
}

void demo_sized_deallocation() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  Sized Deallocation (C++14)\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "C++14 新增带大小的 delete 运算符:\n";
    std::cout << "  void operator delete(void* ptr, std::size_t size) noexcept;\n";
    std::cout << "  void operator delete[](void* ptr, std::size_t size) noexcept;\n\n";

    std::cout << "优势:\n";
    std::cout << "  - 内存分配器可以利用大小信息优化回收\n";
    std::cout << "  - 减少内存碎片\n";
    std::cout << "  - 提升多线程内存分配性能\n\n";

    std::cout << "示例: tcmalloc/jemalloc 等高性能分配器\n";
    std::cout << "  知道要释放的大小，无需查找元数据\n\n";

    std::cout << "对用户代码的影响:\n";
    std::cout << "  - 大多数代码无需修改\n";
    std::cout << "  - 自定义 operator delete 需要更新\n";
    std::cout << "  - 编译器自动传递大小参数\n";
}

void demo_deprecated_features() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  C++14 弃用特性\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. std::random_shuffle —— 已弃用\n";
    std::cout << "   原因: 依赖 C 的 rand()，不安全\n";
    std::cout << "   替代: std::shuffle + 随机数引擎\n\n";

    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(v.begin(), v.end(), g);
    std::cout << "   std::shuffle 结果: ";
    for (int i = 0; i < 5; i++) std::cout << v[i] << " ";
    std::cout << "...\n\n";

    std::cout << "2. [[deprecated]] 属性(C++14新增):\n";
    std::cout << "   [[deprecated]] —— 标记弃用\n";
    std::cout << "   [[deprecated(\"原因\")]] —— 带原因\n\n";

    std::cout << "3. 其他C++14变化:\n";
    std::cout << "   - std::gets() 从C++14中移除(不安全)\n";
    std::cout << "   - std::is_literal_type 弃用(C++17移除)\n";
    std::cout << "   - std::result_of 弃用(C++17用invoke_result替代)\n";
}

void demo_cpp14_tips() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  C++14 实用技巧总结\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. 用泛型lambda减少代码重复:\n";
    auto less_than = [](auto threshold) {
        return [threshold](const auto& x) {
            return x < threshold;
        };
    };
    auto lt5 = less_than(5);
    std::cout << "   less_than(5)(3) = " << lt5(3) << "\n";
    std::cout << "   less_than(5)(7) = " << lt5(7) << "\n\n";

    std::cout << "2. 用数字分隔符提高可读性:\n";
    std::cout << "   1'000'000 比 1000000 更易读\n";
    std::cout << "   0xFF'FF 比 0xFFFF 更清晰\n\n";

    std::cout << "3. 用 make_unique 替代 new:\n";
    auto p = std::make_unique<std::vector<int>>(10, 42);
    std::cout << "   make_unique<vector<int>>(10, 42) 大小=" << p->size() << "\n\n";

    std::cout << "4. constexpr 更多场景:\n";
    std::cout << "   C++14 的 constexpr 可以包含循环和局部变量\n";
    std::cout << "   可以实现更复杂的编译期计算\n\n";

    std::cout << "5. [[deprecated]] 标记旧API:\n";
    std::cout << "   [[deprecated(\"请使用 new_func()\")]]\n";
    std::cout << "   void old_func();\n";
    std::cout << "   编译器会对调用者发出警告\n";
}

int main() {
    demo_cpp14_in_practice();
    demo_constexpr_advanced();
    demo_sized_deallocation();
    demo_deprecated_features();
    demo_cpp14_tips();
    return 0;
}
