/**
 * @file 01_example_cpp20_features.cpp
 * @brief C++20新特性概览
 * @description 对应文档: 02-CPP/25-cpp20
 */

#include <iostream>
#include <vector>
#include <ranges>
#include <algorithm>
#include <compare>
#include <concepts>
#include <string>
#include <array>

void demo_concepts_brief() {
    std::cout << "\n=== Concepts(概念)简介 ===\n";

    auto add = []<typename T>(T a, T b) requires std::integral<T> {
        return a + b;
    };
    std::cout << "add(3, 4) = " << add(3, 4) << "\n";
    std::cout << "add需要整数类型, add(3.0, 4.0)会编译错误\n";

    auto print_size = []<typename T>(const T& v) requires std::ranges::sized_range<T> {
        std::cout << "容器大小: " << std::ranges::size(v) << "\n";
    };
    std::vector<int> vec = {1, 2, 3};
    print_size(vec);

    std::cout << "std::integral<int>: " << std::boolalpha << std::integral<int> << "\n";
    std::cout << "std::integral<double>: " << std::boolalpha << std::integral<double> << "\n";
    std::cout << "std::floating_point<double>: " << std::boolalpha << std::floating_point<double> << "\n";
}

void demo_ranges_brief() {
    std::cout << "\n=== Ranges(范围)简介 ===\n";

    std::vector<int> nums = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto result = nums
        | std::views::filter([](int n) { return n % 2 == 0; })
        | std::views::transform([](int n) { return n * n; });
    std::cout << "偶数平方: ";
    for (int n : result) std::cout << n << " ";
    std::cout << "\n";

    std::ranges::sort(nums, std::greater{});
    std::cout << "降序排序: ";
    for (int n : nums) std::cout << n << " ";
    std::cout << "\n";
}

void demo_three_way_comparison() {
    std::cout << "\n=== 三路比较运算符(<=>) ===\n";

    int a = 10, b = 20;
    auto cmp = (a <=> b);
    std::cout << "10 <=> 20: ";
    if (std::is_lt(cmp)) std::cout << "小于\n";
    else if (std::is_gt(cmp)) std::cout << "大于\n";
    else std::cout << "等于\n";

    std::string s1 = "hello", s2 = "world";
    auto str_cmp = (s1 <=> s2);
    std::cout << "\"hello\" <=> \"world\": ";
    if (std::is_lt(str_cmp)) std::cout << "小于\n";
    else if (std::is_gt(str_cmp)) std::cout << "大于\n";
    else std::cout << "等于\n";

    struct Point {
        int x, y;
        auto operator<=>(const Point&) const = default;
    };
    Point p1{1, 2}, p2{1, 3};
    std::cout << "Point{1,2} <=> Point{1,3}: ";
    auto pt_cmp = (p1 <=> p2);
    if (std::is_lt(pt_cmp)) std::cout << "小于\n";
    else if (std::is_gt(pt_cmp)) std::cout << "大于\n";
    else std::cout << "等于\n";
    std::cout << "p1 < p2: " << std::boolalpha << (p1 < p2) << "\n";
    std::cout << "p1 == p2: " << std::boolalpha << (p1 == p2) << "\n";
}

void demo_designated_initializers() {
    std::cout << "\n=== 指定初始化器 ===\n";

    struct Config {
        int width = 800;
        int height = 600;
        bool fullscreen = false;
        std::string title = "Window";
    };

    Config cfg1{.width = 1920, .height = 1080, .fullscreen = true};
    std::cout << "cfg1: " << cfg1.width << "x" << cfg1.height
              << " fullscreen=" << std::boolalpha << cfg1.fullscreen << "\n";

    Config cfg2{.title = "My App"};
    std::cout << "cfg2: " << cfg2.width << "x" << cfg2.height
              << " title=" << cfg2.title << "\n";

    Config cfg3{.width = 1024, .title = "Custom"};
    std::cout << "cfg3: " << cfg3.width << "x" << cfg3.height
              << " title=" << cfg3.title << "\n";

    struct Point3D {
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
    };
    Point3D p{.x = 1.0, .z = 3.0};
    std::cout << "Point3D: (" << p.x << ", " << p.y << ", " << p.z << ")\n";
}

constexpr int square(int n) {
    return n * n;
}

consteval int cube(int n) {
    return n * n * n;
}

constinit static int global_val = square(5);

void demo_constinit_consteval() {
    std::cout << "\n=== constinit与consteval ===\n";

    std::cout << "constinit全局变量: " << global_val << "\n";
    std::cout << "consteval cube(3): " << cube(3) << "\n";

    std::cout << "constinit: 确保变量在编译期初始化, 避免静态初始化顺序问题\n";
    std::cout << "consteval: 函数必须在编译期执行, 不会产生运行时调用\n";
    std::cout << "constexpr: 可能编译期执行, 也可能运行时执行\n";
}

void demo_template_syntax_improvements() {
    std::cout << "\n=== 模板语法改进 ===\n";

    auto lam = []<typename T>(std::vector<T>& v) {
        std::cout << "向量大小: " << v.size() << " 元素类型大小: " << sizeof(T) << "\n";
    };
    std::vector<int> vi = {1, 2, 3};
    std::vector<double> vd = {1.1, 2.2};
    lam(vi);
    lam(vd);

    auto lam2 = []<typename T, std::size_t N>(std::array<T, N>& arr) {
        std::cout << "数组大小: " << N << " 元素: ";
        for (const auto& e : arr) std::cout << e << " ";
        std::cout << "\n";
    };
    std::array<int, 4> arr = {10, 20, 30, 40};
    lam2(arr);
}

void demo_other_features() {
    std::cout << "\n=== 其他C++20特性 ===\n";

    std::vector<int> v = {1, 2, 3, 4, 5};
    if (auto it = std::ranges::find(v, 3); it != v.end()) {
        std::cout << "初始化语句+范围for: 找到3\n";
    }

    std::cout << "\n聚合体初始化改进:\n";
    struct Pair { int a; int b; };
    Pair p{1, 2};
    std::cout << "Pair: (" << p.a << ", " << p.b << ")\n";

    std::cout << "\nusing enum (C++20):\n";
    enum class Color { Red, Green, Blue };
    Color c = Color::Green;
    switch (c) {
        case Color::Red: std::cout << "红色\n"; break;
        case Color::Green: std::cout << "绿色\n"; break;
        case Color::Blue: std::cout << "蓝色\n"; break;
    }

    std::cout << "\n字符串字面量作为模板参数:\n";
    auto print_str = []<std::size_t N>(const char (&s)[N]) {
        std::cout << "字符串: " << s << " 长度(含\\0): " << N << "\n";
    };
    print_str("hello C++20");
}

int main() {
    std::cout << "========== C++20 新特性概览 ==========\n";

    demo_concepts_brief();
    demo_ranges_brief();
    demo_three_way_comparison();
    demo_designated_initializers();
    demo_constinit_consteval();
    demo_template_syntax_improvements();
    demo_other_features();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
