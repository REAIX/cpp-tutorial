/** @file 01_deep_dive_cpp17_patterns.cpp
 *  @brief C++17实战、并行算法、保证拷贝消除、inline变量、嵌套命名空间、__has_include
 *  @description 对应文档: 02-CPP/22-cpp17 | 举一反三：C++17高级特性和实战模式
 *  编译命令: g++ -std=c++20 01_deep_dive_cpp17_patterns.cpp -o 01_deep_dive_cpp17_patterns
 */

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <numeric>
#include <optional>
#include <variant>
#include <memory>
#include <chrono>
#include <type_traits>
#include <map>
#include <cmath>

void demo_cpp17_in_practice() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  C++17 实战模式\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. 结构化绑定 + if初始化器:\n";
    std::map<std::string, int> scores = {{"语文", 95}, {"数学", 87}};

    if (auto [it, inserted] = scores.emplace("英语", 92); inserted) {
        std::cout << "  插入成功: 英语=" << it->second << "\n";
    }

    if (auto [it, inserted] = scores.emplace("语文", 100); !inserted) {
        std::cout << "  插入失败: 语文已存在=" << it->second << "\n";
    }

    std::cout << "\n2. optional 链式调用:\n";
    auto get_config = []() -> std::optional<std::string> { return "config_value"; };
    auto parse_int = [](const std::string& s) -> std::optional<int> {
        try { return std::stoi(s); }
        catch (...) { return std::nullopt; }
    };

    if (auto cfg = get_config(); cfg.has_value()) {
        std::cout << "  配置值: " << cfg.value() << "\n";
    }

    std::cout << "\n3. variant + visit 实现多态:\n";
    struct ShapeArea {
        double operator()(double radius) const { return 3.14159 * radius * radius; }
        double operator()(const std::pair<double, double>& dims) const { return dims.first * dims.second; }
        double operator()(const std::tuple<double, double, double>& dims) const {
            return 0.5 * std::get<0>(dims) * std::get<1>(dims) * sin(std::get<2>(dims));
        }
    };

    using Shape = std::variant<double, std::pair<double, double>, std::tuple<double, double, double>>;
    std::vector<Shape> shapes = {
        5.0,
        std::make_pair(4.0, 6.0),
        std::make_tuple(3.0, 4.0, 1.5707963)
    };

    for (const auto& shape : shapes) {
        std::cout << "  面积: " << std::visit(ShapeArea{}, shape) << "\n";
    }
}

void demo_guaranteed_copy_elision() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  保证拷贝消除 (Guaranteed Copy Elision)\n";
    std::cout << "═══════════════════════════════════════\n\n";

    struct BigObject {
        int data[1000];
        BigObject() { std::cout << "  BigObject() 构造\n"; }
        BigObject(const BigObject&) { std::cout << "  BigObject(const&) 拷贝\n"; }
        BigObject(BigObject&&) { std::cout << "  BigObject(&&) 移动\n"; }
    };

    auto make_big = []() -> BigObject {
        return BigObject();
    };

    std::cout << "C++17 保证:\n";
    BigObject obj = make_big();
    (void)obj;

    std::cout << "\nC++17 之前:\n";
    std::cout << "  拷贝/移动可能被消除(但不是保证)\n";
    std::cout << "  返回类型必须可移动构造\n\n";

    std::cout << "C++17:\n";
    std::cout << "  返回值直接在调用方构造(保证零拷贝)\n";
    std::cout << "  类型甚至不需要移动构造函数\n\n";

    struct NonMovable {
        NonMovable() = default;
        NonMovable(const NonMovable&) = delete;
        NonMovable(NonMovable&&) = delete;
    };

    auto make_nonmovable = []() -> NonMovable {
        return NonMovable();
    };

    NonMovable nm = make_nonmovable();
    (void)nm;
    std::cout << "  不可移动类型也能通过返回值构造!\n";
}

void demo_inline_variables() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  inline 变量与嵌套命名空间\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "inline 变量:\n";
    std::cout << "  C++17前: 头文件中定义常量需用 constexpr 或 inline 函数\n";
    std::cout << "  C++17: inline 变量保证只有一个定义\n\n";

    std::cout << "  // 头文件中\n";
    std::cout << "  inline constexpr int VERSION = 17;\n";
    std::cout << "  inline const std::string APP_NAME = \"MyApp\";\n\n";

    std::cout << "  优势:\n";
    std::cout << "  - 头文件中定义全局常量不再有ODR问题\n";
    std::cout << "  - constexpr 变量隐式 inline\n";
    std::cout << "  - 适合定义跨编译单元的常量\n\n";

    std::cout << "嵌套命名空间:\n";
    std::cout << "  C++14: namespace A { namespace B { namespace C { } } }\n";
    std::cout << "  C++17: namespace A::B::C { }\n\n";

    std::cout << "__has_include:\n";
    std::cout << "  检测头文件是否可用:\n";
    std::cout << "  #if __has_include(<filesystem>)\n";
    std::cout << "    #include <filesystem>\n";
    std::cout << "  #endif\n\n";

#if __has_include(<filesystem>)
    std::cout << "  <filesystem> 可用\n";
#else
    std::cout << "  <filesystem> 不可用\n";
#endif
}

void demo_parallel_algorithms() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  并行算法 (Parallel Algorithms)\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::vector<int> v(1'000'000);
    std::iota(v.begin(), v.end(), 0);

    auto start = std::chrono::high_resolution_clock::now();
    std::sort(v.begin(), v.end());
    auto end = std::chrono::high_resolution_clock::now();
    double seq_ms = std::chrono::duration<double, std::milli>(end - start).count();

    std::cout << "排序100万元素(串行): " << seq_ms << " ms\n\n";

    std::cout << "C++17 并行算法(需要 <execution> 头文件和TBB库):\n";
    std::cout << "  std::sort(std::execution::par, v.begin(), v.end());\n\n";

    std::cout << "执行策略:\n";
    std::cout << "  std::execution::seq     —— 顺序执行\n";
    std::cout << "  std::execution::par     —— 并行执行\n";
    std::cout << "  std::execution::par_unseq —— 并行+向量化\n\n";

    std::cout << "支持并行的算法:\n";
    std::cout << "  sort, stable_sort, for_each, transform,\n";
    std::cout << "  reduce, count, find, copy, ... 等60+算法\n\n";

    std::cout << "注意事项:\n";
    std::cout << "  - 需要链接 TBB 库(GCC) 或 /Qpar(Intel)\n";
    std::cout << "  - 并行有开销，小数据集可能更慢\n";
    std::cout << "  - 函数对象必须是线程安全的\n";
    std::cout << "  - 注意数据竞争和死锁\n";
}

void demo_other_cpp17_features() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  C++17 其他重要特性\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. if constexpr:\n";
    auto print_type = [](auto x) {
        if constexpr (std::is_integral_v<decltype(x)>) {
            std::cout << "  整数: " << x << "\n";
        } else if constexpr (std::is_floating_point_v<decltype(x)>) {
            std::cout << "  浮点: " << x << "\n";
        } else {
            std::cout << "  其他类型\n";
        }
    };
    print_type(42);
    print_type(3.14);
    print_type("hello");

    std::cout << "\n2. std::invoke:\n";
    std::cout << "  统一调用: 函数指针、成员函数、函数对象\n";

    std::cout << "\n3. [[nodiscard]] 属性:\n";
    std::cout << "  [[nodiscard]] int get_value();\n";
    std::cout << "  忽略返回值时编译器发出警告\n";

    std::cout << "\n4. [[maybe_unused]] 属性:\n";
    std::cout << "  [[maybe_unused]] int x = compute();\n";
    std::cout << "  抑制未使用变量的警告\n";

    std::cout << "\n5. [[fallthrough]] 属性:\n";
    std::cout << "  switch中故意穿透case时标注\n";

    std::cout << "\n6. std::byte:\n";
    std::cout << "  独立的字节类型，不与char混淆\n";

    std::cout << "\n7. std::size / std::empty / std::data:\n";
    int arr[] = {1, 2, 3, 4, 5};
    std::cout << "  std::size(arr) = " << std::size(arr) << "\n";
}

int main() {
    demo_cpp17_in_practice();
    demo_guaranteed_copy_elision();
    demo_inline_variables();
    demo_parallel_algorithms();
    demo_other_cpp17_features();
    return 0;
}
