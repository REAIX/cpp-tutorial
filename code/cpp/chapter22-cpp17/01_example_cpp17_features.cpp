/** @file 01_example_cpp17_features.cpp
 *  @brief C++17新特性：结构化绑定、if/switch初始化器、optional、variant、any、string_view、filesystem、折叠表达式
 *  @description 对应文档: 02-CPP/22-cpp17 | 演示C++17最重要的新特性
 *  编译命令: g++ -std=c++20 01_example_cpp17_features.cpp -o 01_example_cpp17_features
 */

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <optional>
#include <variant>
#include <any>
#include <tuple>
#include <filesystem>
#include <algorithm>

void demo_structured_bindings() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  结构化绑定 (Structured Bindings)\n";
    std::cout << "═══════════════════════════════════════\n\n";

    auto [x, y, z] = std::make_tuple(1, 2.0, "three");
    std::cout << "tuple 解包: x=" << x << ", y=" << y << ", z=" << z << "\n\n";

    std::pair<std::string, int> person = {"张三", 25};
    auto [name, age] = person;
    std::cout << "pair 解包: name=" << name << ", age=" << age << "\n\n";

    std::map<std::string, int> scores = {{"语文", 95}, {"数学", 87}, {"英语", 92}};
    std::cout << "map 遍历:\n";
    for (const auto& [subject, score] : scores) {
        std::cout << "  " << subject << ": " << score << "\n";
    }

    struct Point { double x, y, z; };
    Point p = {1.0, 2.0, 3.0};
    auto [px, py, pz] = p;
    std::cout << "\n结构体解包: (" << px << ", " << py << ", " << pz << ")\n\n";

    int arr[] = {10, 20, 30};
    auto [a, b, c] = arr;
    std::cout << "数组解包: " << a << ", " << b << ", " << c << "\n";
}

void demo_if_switch_init() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  if/switch 带初始化器\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::map<std::string, int> cache = {{"key1", 100}, {"key2", 200}};

    std::cout << "if 带初始化器:\n";
    if (auto it = cache.find("key1"); it != cache.end()) {
        std::cout << "  找到 key1: " << it->second << "\n";
    } else {
        std::cout << "  未找到 key1\n";
    }

    if (auto it = cache.find("key3"); it != cache.end()) {
        std::cout << "  找到 key3: " << it->second << "\n";
    } else {
        std::cout << "  未找到 key3\n";
    }

    std::cout << "\nswitch 带初始化器:\n";
    auto get_status = []() { return 2; };
    switch (auto status = get_status(); status) {
        case 0: std::cout << "  状态: 未开始\n"; break;
        case 1: std::cout << "  状态: 进行中\n"; break;
        case 2: std::cout << "  状态: 已完成\n"; break;
        default: std::cout << "  状态: 未知\n"; break;
    }

    std::cout << "\n优势: 限制变量作用域，避免命名冲突和意外使用\n";
}

void demo_optional() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  std::optional —— 可选值\n";
    std::cout << "═══════════════════════════════════════\n\n";

    auto find_user = [](int id) -> std::optional<std::string> {
        if (id == 1) return "张三";
        if (id == 2) return "李四";
        return std::nullopt;
    };

    if (auto user = find_user(1)) {
        std::cout << "找到用户: " << *user << "\n";
    }

    if (auto user = find_user(3)) {
        std::cout << "找到用户: " << *user << "\n";
    } else {
        std::cout << "用户不存在\n";
    }

    std::cout << "\nvalue_or 默认值:\n";
    std::cout << "  find_user(1).value_or(\"未知\"): " << find_user(1).value_or("未知") << "\n";
    std::cout << "  find_user(3).value_or(\"未知\"): " << find_user(3).value_or("未知") << "\n";

    std::cout << "\noptional 操作:\n";
    std::optional<int> opt = 42;
    std::cout << "  has_value(): " << opt.has_value() << "\n";
    std::cout << "  operator*(): " << *opt << "\n";
    std::cout << "  value(): " << opt.value() << "\n";
    opt.reset();
    std::cout << "  reset()后 has_value(): " << opt.has_value() << "\n";
}

void demo_variant() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  std::variant —— 类型安全联合体\n";
    std::cout << "═══════════════════════════════════════\n\n";

    using Value = std::variant<int, double, std::string>;

    Value v1 = 42;
    Value v2 = 3.14;
    Value v3 = std::string("hello");

    auto visitor = [](const auto& val) {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, int>) {
            std::cout << "  整数: " << val << "\n";
        } else if constexpr (std::is_same_v<T, double>) {
            std::cout << "  浮点: " << val << "\n";
        } else if constexpr (std::is_same_v<T, std::string>) {
            std::cout << "  字符串: " << val << "\n";
        }
    };

    std::cout << "std::visit 访问:\n";
    std::visit(visitor, v1);
    std::visit(visitor, v2);
    std::visit(visitor, v3);

    std::cout << "\nholds_alternative / get:\n";
    std::cout << "  holds_alternative<int>(v1): " << std::holds_alternative<int>(v1) << "\n";
    std::cout << "  get<int>(v1): " << std::get<int>(v1) << "\n";
    std::cout << "  get_if<double>(&v2): " << (std::get_if<double>(&v2) ? "有值" : "无值") << "\n";

    std::cout << "\nvs union:\n";
    std::cout << "  union: 无类型安全，不知道当前存储什么类型\n";
    std::cout << "  variant: 类型安全，始终知道当前类型\n";
}

void demo_any() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  std::any —— 任意类型容器\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::any a = 42;
    std::cout << "存入int: " << std::any_cast<int>(a) << "\n";

    a = 3.14;
    std::cout << "存入double: " << std::any_cast<double>(a) << "\n";

    a = std::string("hello");
    std::cout << "存入string: " << std::any_cast<std::string>(a) << "\n";

    std::cout << "\ntype() 检查:\n";
    std::cout << "  type().name(): " << a.type().name() << "\n";
    std::cout << "  has_value(): " << a.has_value() << "\n";

    a.reset();
    std::cout << "  reset()后 has_value(): " << a.has_value() << "\n\n";

    try {
        a = 42;
        std::any_cast<std::string>(a);
    } catch (const std::bad_any_cast& e) {
        std::cout << "  bad_any_cast: 类型不匹配时抛异常\n";
    }

    std::cout << "\nany vs variant:\n";
    std::cout << "  any: 完全动态，可存任意类型，但访问需 any_cast\n";
    std::cout << "  variant: 限定类型集合，编译期类型安全\n";
    std::cout << "  推荐: 优先使用 variant，仅在需要完全动态时用 any\n";
}

void demo_string_view() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  std::string_view —— 字符串视图\n";
    std::cout << "═══════════════════════════════════════\n\n";

    auto print_sv = [](std::string_view sv) {
        std::cout << "  \"" << sv << "\" (长度=" << sv.size() << ")\n";
    };

    std::string s = "Hello, World!";
    print_sv(s);
    print_sv("字面量");
    print_sv(s.substr(0, 5));

    std::cout << "\nstring_view vs const string&:\n";
    std::cout << "  string_view: 不拷贝，零开销视图\n";
    std::cout << "  const string&: 传字面量时需构造临时string\n\n";

    std::cout << "子串操作(无拷贝):\n";
    std::string_view text = "2024-01-15";
    auto year = text.substr(0, 4);
    auto month = text.substr(5, 2);
    auto day = text.substr(8, 2);
    std::cout << "  年: " << year << ", 月: " << month << ", 日: " << day << "\n\n";

    std::cout << "注意:\n";
    std::cout << "  - string_view 不拥有数据，不保证以\\0结尾\n";
    std::cout << "  - 不要用 string_view 持有临时 string 的引用\n";
    std::cout << "  - 适合函数参数，不适合长期存储\n";
}

void demo_fold_expressions() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  折叠表达式 (Fold Expressions)\n";
    std::cout << "═══════════════════════════════════════\n\n";

    auto sum = [](auto... args) {
        return (args + ...);
    };

    auto product = [](auto... args) {
        return (args * ...);
    };

    auto print_all = [](auto... args) {
        ((std::cout << args << " "), ...);
        std::cout << "\n";
    };

    auto all_true = [](auto... args) {
        return (... && args);
    };

    auto any_true = [](auto... args) {
        return (... || args);
    };

    std::cout << "右折叠 (args op ...):\n";
    std::cout << "  sum(1,2,3,4,5) = " << sum(1, 2, 3, 4, 5) << "\n";
    std::cout << "  product(1,2,3,4,5) = " << product(1, 2, 3, 4, 5) << "\n";
    std::cout << "  print_all: ";
    print_all("Hello", "C++17", "World");
    std::cout << "  all_true(true, true, false) = " << all_true(true, true, false) << "\n";
    std::cout << "  any_true(true, true, false) = " << any_true(true, true, false) << "\n\n";

    std::cout << "折叠表达式形式:\n";
    std::cout << "  (args op ...)     —— 右折叠\n";
    std::cout << "  (... op args)     —— 左折叠\n";
    std::cout << "  (args op ... op init) —— 右折叠带初值\n";
    std::cout << "  (init op ... op args) —— 左折叠带初值\n";
}

int main() {
    demo_structured_bindings();
    demo_if_switch_init();
    demo_optional();
    demo_variant();
    demo_any();
    demo_string_view();
    demo_fold_expressions();
    return 0;
}
