/**
 * @file 02_example_cpp20_advanced.cpp
 * @brief C++20高级用法模式
 * @description 对应文档: 02-CPP/25-cpp20, 深入展示C++20高级特性
 * 编译命令: g++ -std=c++20 -o cpp20_advanced 02_example_cpp20_advanced.cpp
 */

#include <iostream>
#include <vector>
#include <string>
#include <array>
#include <span>
#include <compare>
#include <concepts>
#include <ranges>
#include <algorithm>
#include <type_traits>
#include <source_location>
#include <cassert>
#include <limits>
#include <tuple>
#include <utility>

#if __cpp_lib_format
#include <format>
#endif

// ============================================================
// 1. 三路比较运算符高级用法
// ============================================================

void demo_three_way_comparison_advanced() {
    std::cout << "\n═══════════════════════════════════════════\n";
    std::cout << "=== 三路比较运算符(<=>)高级用法 ===\n";
    std::cout << "═══════════════════════════════════════════\n";

    // 自定义比较类别: strong_ordering vs weak_ordering vs partial_ordering
    struct Employee {
        int id;
        std::string name;
        double salary;

        // 强序: id唯一确定等价性, 可替换
        std::strong_ordering operator<=>(const Employee& other) const {
            return id <=> other.id;
        }
        // C++20: 定义<=>后, 编译器自动生成 <, >, <=, >=
        // 但==需要单独处理(当<=>不是default时)
        bool operator==(const Employee& other) const {
            return id == other.id;
        }
    };

    Employee e1{1, "Alice", 8000.0};
    Employee e2{2, "Bob", 9000.0};
    Employee e3{1, "Alice2", 7500.0}; // 同id, 强序认为等价

    std::cout << "e1 < e2 (按id): " << std::boolalpha << (e1 < e2) << "\n";
    std::cout << "e1 == e3 (同id): " << std::boolalpha << (e1 == e3) << "\n";
    std::cout << "强序含义: id相同即等价, 即使name/salary不同\n";

    // partial_ordering: 浮点数比较(存在NaN)
    std::cout << "\npartial_ordering示例(浮点数):\n";
    double normal = 1.0;
    double nan_val = std::numeric_limits<double>::quiet_NaN();
    auto cmp = normal <=> nan_val;
    std::cout << "1.0 <=> NaN: ";
    if (std::is_lt(cmp)) std::cout << "小于\n";
    else if (std::is_gt(cmp)) std::cout << "大于\n";
    else if (std::is_eq(cmp)) std::cout << "等于\n";
    else std::cout << "无序(unordered)\n";

    // default比较: 自动按成员逐一比较
    struct Rect {
        int w, h;
        auto operator<=>(const Rect&) const = default;
    };
    Rect r1{3, 4}, r2{3, 5};
    std::cout << "\ndefault比较 Rect{3,4} < Rect{3,5}: "
              << std::boolalpha << (r1 < r2) << "\n";
    std::cout << "default会自动生成 ==, !=, <, >, <=, => 六个运算符\n";
}

// ============================================================
// 2. std::span - 非拥有式连续内存视图
// ============================================================

void demo_span() {
    std::cout << "\n═══════════════════════════════════════════\n";
    std::cout << "=== std::span - 非拥有式连续内存视图 ===\n";
    std::cout << "═══════════════════════════════════════════\n";

    // span统一了数组、vector、array的访问接口
    auto print_span = [](std::span<const int> s, const std::string& label) {
        std::cout << label << " [size=" << s.size() << "]: ";
        for (auto v : s) std::cout << v << " ";
        std::cout << "\n";
    };

    // 从vector创建
    std::vector<int> vec = {10, 20, 30, 40, 50};
    print_span(vec, "vector");

    // 从array创建
    std::array<int, 4> arr = {1, 2, 3, 4};
    print_span(arr, "array");

    // 从C数组创建
    int c_arr[] = {100, 200, 300};
    print_span(c_arr, "C数组");

    // 子视图: first, last, subspan
    std::span<int> sp(vec);
    std::cout << "\n子视图操作:\n";
    std::cout << "first(3): ";
    for (auto v : sp.first(3)) std::cout << v << " ";
    std::cout << "\n";

    std::cout << "last(2): ";
    for (auto v : sp.last(2)) std::cout << v << " ";
    std::cout << "\n";

    std::cout << "subspan(1, 3): ";
    for (auto v : sp.subspan(1, 3)) std::cout << v << " ";
    std::cout << "\n";

    // 固定大小的span
    std::span<int, 5> fixed_sp(vec);
    std::cout << "\n固定大小span<std::span<int,5>>: size=" << fixed_sp.size()
              << " (编译期已知)\n";

    // span作为函数参数: 避免指针+长度的C风格接口
    auto sum = [](std::span<const int> s) -> int {
        int total = 0;
        for (auto v : s) total += v;
        return total;
    };
    std::cout << "sum(vec) = " << sum(vec) << "\n";
    std::cout << "sum(arr) = " << sum(arr) << "\n";
    std::cout << "sum(c_arr) = " << sum(c_arr) << "\n";
}

// ============================================================
// 3. std::format (C++20, 需编译器支持)
// ============================================================

void demo_format() {
    std::cout << "\n═══════════════════════════════════════════\n";
    std::cout << "=== std::format (C++20格式化库) ===\n";
    std::cout << "═══════════════════════════════════════════\n";

#if __cpp_lib_format
    std::string s1 = std::format("姓名: {}, 年龄: {}", "张三", 25);
    std::cout << s1 << "\n";

    std::string s2 = std::format("PI = {:.4f}", 3.14159265);
    std::cout << s2 << "\n";

    std::string s3 = std::format("{0} + {1} = {2}", 3, 4, 7);
    std::cout << s3 << "\n";

    std::cout << "std::format: 类型安全的格式化, 替代printf/snprintf\n";
#else
    std::cout << "当前编译器尚未支持std::format\n";
    std::cout << "std::format是C++20引入的类型安全格式化库\n";
    std::cout << "用法: std::format(\"{} + {} = {}\", 3, 4, 7)\n";
    std::cout << "支持: 位置参数{0}{1}, 格式说明{:.2f}, 宽度{:>10}\n";
    std::cout << "GCC 13+, MSVC 19.29+ 支持 <format>\n";
    std::cout << "替代方案: fmt库 (https://github.com/fmtlib/fmt)\n";

    // 用现有方式演示等价输出
    std::cout << "等价输出 - 姓名: 张三, 年龄: 25\n";
    std::cout << "等价输出 - PI = 3.1416\n";
    std::cout << "等价输出 - 3 + 4 = 7\n";
#endif
}

// ============================================================
// 4. constinit - 编译期初始化
// ============================================================

constexpr int compute_magic(int base) {
    return base * base + base + 42;
}

constinit int magic_value = compute_magic(10);
constinit const int const_magic = compute_magic(5);

void demo_constinit() {
    std::cout << "\n═══════════════════════════════════════════\n";
    std::cout << "=== constinit - 编译期初始化 ===\n";
    std::cout << "═══════════════════════════════════════════\n";

    std::cout << "constinit magic_value = " << magic_value << "\n";
    std::cout << "constinit const_magic = " << const_magic << "\n";

    // constinit vs constexpr vs const 的区别
    std::cout << "\nconstinit vs constexpr vs const:\n";
    std::cout << "  constexpr: 编译期常量, 值不可修改\n";
    std::cout << "  constinit: 编译期初始化, 但值可修改(非const时)\n";
    std::cout << "  const: 运行期或编译期初始化, 值不可修改\n";

    // 修改constinit(非const)变量
    magic_value = 999;
    std::cout << "\n修改后 magic_value = " << magic_value << "\n";
    std::cout << "constinit变量可以在运行时修改(除非同时声明const)\n";

    // 避免静态初始化顺序问题
    std::cout << "\nconstinit核心价值: 避免静态初始化顺序问题(SIOF)\n";
    std::cout << "  普通全局变量: 初始化顺序跨翻译单元未定义\n";
    std::cout << "  constinit全局变量: 编译期初始化, 无顺序依赖\n";
}

// ============================================================
// 5. consteval - 立即函数
// ============================================================

consteval int factorial(int n) {
    return (n <= 1) ? 1 : n * factorial(n - 1);
}

consteval int fibonacci(int n) {
    if (n <= 0) return 0;
    if (n == 1) return 1;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

consteval bool is_power_of_two(int n) {
    return n > 0 && (n & (n - 1)) == 0;
}

void demo_consteval() {
    std::cout << "\n═══════════════════════════════════════════\n";
    std::cout << "=== consteval - 立即函数 ===\n";
    std::cout << "═══════════════════════════════════════════\n";

    // consteval函数保证在编译期执行
    std::cout << "factorial(5) = " << factorial(5) << "\n";
    std::cout << "factorial(10) = " << factorial(10) << "\n";
    std::cout << "fibonacci(10) = " << fibonacci(10) << "\n";
    std::cout << "is_power_of_two(64) = " << std::boolalpha << is_power_of_two(64) << "\n";
    std::cout << "is_power_of_two(100) = " << std::boolalpha << is_power_of_two(100) << "\n";

    // consteval结果可用于编译期上下文
    constexpr int fact_8 = factorial(8);
    std::cout << "\nfactorial(8)作为constexpr: " << fact_8 << "\n";

    // consteval vs constexpr 对比
    std::cout << "\nconsteval vs constexpr:\n";
    std::cout << "  constexpr: 可能编译期执行, 也可能运行时执行\n";
    std::cout << "  consteval: 必须编译期执行, 否则编译错误\n";
    std::cout << "  consteval参数必须是编译期常量或常量表达式\n";

    // 运行时变量不能传给consteval
    // int x = 5; factorial(x); // 编译错误!
    std::cout << "\n注意: 运行时变量不能作为consteval函数参数\n";
    std::cout << "  int x = 5; factorial(x); // 编译错误!\n";
}

// ============================================================
// 6. std::source_location - 源码位置信息
// ============================================================

void log_with_location(const std::string& message,
                       const std::source_location& loc = std::source_location::current()) {
    std::cout << "[" << loc.file_name() << ":" << loc.line()
              << " 函数:" << loc.function_name() << "] " << message << "\n";
}

void demo_source_location() {
    std::cout << "\n═══════════════════════════════════════════\n";
    std::cout << "=== std::source_location - 源码位置信息 ===\n";
    std::cout << "═══════════════════════════════════════════\n";

    // 基本用法: 获取调用位置
    log_with_location("这是一条日志消息");
    log_with_location("这是另一条日志消息");

    // 直接获取source_location
    auto loc = std::source_location::current();
    std::cout << "\n当前文件: " << loc.file_name() << "\n";
    std::cout << "当前行号: " << loc.line() << "\n";
    std::cout << "当前列号: " << loc.column() << "\n";
    std::cout << "当前函数: " << loc.function_name() << "\n";

    // 实际应用: 实现轻量级断言
    auto assert_check = [](bool condition, const std::string& msg,
                           const std::source_location& sl = std::source_location::current()) {
        if (!condition) {
            std::cout << "断言失败 [" << sl.file_name() << ":" << sl.line()
                      << "]: " << msg << "\n";
        }
    };

    assert_check(true, "这不会触发");
    assert_check(1 > 2, "1不应该大于2");

    std::cout << "\nsource_location优势: 替代 __FILE__, __LINE__, __func__ 预处理宏\n";
    std::cout << "  默认参数在调用点求值, 自动获取正确的调用位置\n";
}

// ============================================================
// 7. 指定初始化器高级用法
// ============================================================

void demo_designated_initializers_advanced() {
    std::cout << "\n═══════════════════════════════════════════\n";
    std::cout << "=== 指定初始化器高级用法 ===\n";
    std::cout << "═══════════════════════════════════════════\n";

    // 嵌套指定初始化
    struct Display {
        int width = 1920;
        int height = 1080;
        int refresh_rate = 60;
    };

    struct WindowConfig {
        std::string title = "Untitled";
        Display display;
        bool vsync = true;
    };

    WindowConfig cfg{
        .title = "My Game",
        .display = {.width = 2560, .height = 1440, .refresh_rate = 144},
        .vsync = false
    };
    std::cout << "窗口: " << cfg.title
              << " " << cfg.display.width << "x" << cfg.display.height
              << "@" << cfg.display.refresh_rate << "Hz"
              << " vsync=" << std::boolalpha << cfg.vsync << "\n";

    // 指定初始化器与数组
    struct SensorData {
        int id = 0;
        double values[3] = {0.0, 0.0, 0.0};
        bool active = true;
    };

    SensorData sensor{.id = 42, .values = {1.1, 2.2, 3.3}, .active = true};
    std::cout << "传感器#" << sensor.id << " 值: ";
    for (auto v : sensor.values) std::cout << v << " ";
    std::cout << " active=" << std::boolalpha << sensor.active << "\n";

    // 指定初始化器与位域
    struct Flags {
        unsigned int visible : 1 = 1;
        unsigned int enabled : 1 = 1;
        unsigned int reserved : 6 = 0;
    };

    Flags f{.visible = 0, .enabled = 1};
    std::cout << "Flags: visible=" << f.visible << " enabled=" << f.enabled << "\n";

    // 顺序规则: 必须按声明顺序
    std::cout << "\n重要规则:\n";
    std::cout << "  1. 初始化器必须按成员声明顺序出现\n";
    std::cout << "  2. 未指定的成员使用默认值\n";
    std::cout << "  3. 混合使用指定和位置初始化: {1, .y=2} 不允许\n";
    std::cout << "  4. 指定初始化器适用于聚合体(无自定义构造函数)\n";
}

// ============================================================
// 8. 范围for语句带初始化器
// ============================================================

void demo_range_for_with_init() {
    std::cout << "\n═══════════════════════════════════════════\n";
    std::cout << "=== 范围for语句带初始化器 ===\n";
    std::cout << "═══════════════════════════════════════════\n";

    // 基本用法: 在for循环中声明临时变量
    std::cout << "遍历并记录索引:\n";
    for (int i = 0; auto val : std::vector{10, 20, 30, 40, 50}) {
        std::cout << "  [" << i << "] = " << val << "\n";
        ++i;
    }

    // 保存容器, 避免悬垂引用
    std::cout << "\n保存临时容器(避免悬垂):\n";
    // C++17危险写法: for (auto x : get_vector()) 可能悬垂
    // C++20安全写法:
    for (auto&& vec = std::vector{1, 2, 3}; auto val : vec) {
        std::cout << val << " ";
    }
    std::cout << "\n";

    // 带守卫条件的遍历
    std::cout << "\n带初始化器和条件:\n";
    for (auto nums = std::vector{5, 3, 8, 1, 9, 2, 7}; auto val : nums) {
        if (val > 4) std::cout << val << " ";
    }
    std::cout << "(大于4的元素)\n";

    // 与锁配合使用
    std::cout << "\n典型应用: 遍历前获取锁\n";
    std::cout << "  for (auto lock = std::unique_lock(mtx); auto& item : container)\n";
    std::cout << "  锁的生命周期与for循环一致, 自动释放\n";

    std::cout << "\n语法: for (初始化语句; 声明 : 范围) { 循环体 }\n";
    std::cout << "初始化语句中的变量在循环结束后销毁\n";
}

// ============================================================
// 9. 类模板参数推导(CTAD)高级用法
// ============================================================

// 自定义类模板, 演示CTAD
template<typename T>
struct Wrapper {
    T value;
    Wrapper(T v) : value(v) {}
};
// 自定义推导指引示例: 从int推导为Wrapper<double>
// Wrapper(int) -> Wrapper<double>;

void demo_ctad_advanced() {
    std::cout << "\n═══════════════════════════════════════════\n";
    std::cout << "=== 类模板参数推导(CTAD)高级用法 ===\n";
    std::cout << "═══════════════════════════════════════════\n";

    // 基本CTAD: 标准库容器
    std::vector v1 = {1, 2, 3};           // 推导为 vector<int>
    std::vector v2 = {1.0, 2.0, 3.0};     // 推导为 vector<double>
    std::array arr1 = {10, 20, 30};        // 推导为 array<int, 3>
    std::pair p1 = {1, 2.5};              // 推导为 pair<int, double>
    std::tuple t1 = {1, "hello", 3.14};   // 推导为 tuple<int, const char*, double>

    std::cout << "vector推导: {1,2,3} -> vector<int>\n";
    std::cout << "array推导: {10,20,30} -> array<int,3>\n";
    std::cout << "pair推导: {1, 2.5} -> pair<int, double>\n";
    std::cout << "tuple推导: {1, \"hello\", 3.14} -> tuple<int, const char*, double>\n";

    // span的CTAD
    int data[] = {1, 2, 3, 4, 5};
    std::span sp1(data);                   // 推导为 span<int, dynamic_extent>
    std::span sp2(data, 3);                // 推导为 span<int, dynamic_extent>
    std::cout << "\nspan CTAD: span<int> (动态大小)\n";

    // 自定义类模板的CTAD
    std::cout << "\n自定义类模板CTAD示例:\n";
    Wrapper w1 = 42;    // CTAD: Wrapper<int>
    Wrapper w2 = 3.14;  // CTAD: Wrapper<double>
    std::cout << "Wrapper(42) -> Wrapper<int>, value=" << w1.value << "\n";
    std::cout << "Wrapper(3.14) -> Wrapper<double>, value=" << w2.value << "\n";

    // CTAD与范围工厂
    std::cout << "\nCTAD与Ranges:\n";
    auto nums = std::vector{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto even_view = nums | std::views::filter([](int n) { return n % 2 == 0; });
    std::cout << "偶数视图: ";
    for (auto v : even_view) std::cout << v << " ";
    std::cout << "\n";

    std::cout << "\nCTAD要点:\n";
    std::cout << "  1. 简化代码: vector<int> v{1,2,3} -> vector v{1,2,3}\n";
    std::cout << "  2. 自定义推导指引: MyType(参数) -> MyType<模板参数>\n";
    std::cout << "  3. 聚合体的CTAD (C++20新增)\n";
    std::cout << "  4. 别名模板的CTAD (C++20新增)\n";
}

// ============================================================
// main
// ============================================================

int main() {
    std::cout << "========== C++20 高级用法模式 ==========\n";

    demo_three_way_comparison_advanced();
    demo_span();
    demo_format();
    demo_constinit();
    demo_consteval();
    demo_source_location();
    demo_designated_initializers_advanced();
    demo_range_for_with_init();
    demo_ctad_advanced();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
