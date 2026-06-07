/** @file 02_example_cpp17_advanced.cpp
 *  @brief C++17高级用法：if constexpr、折叠表达式进阶、结构化绑定自定义类型、optional/variant/any实战、string_view优化、inline变量、嵌套命名空间
 *  @description 对应文档: 02-CPP/22-cpp17 | 深入C++17高级特性与实战模式
 *  编译命令: g++ -std=c++20 02_example_cpp17_advanced.cpp -o 02_example_cpp17_advanced
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
#include <type_traits>
#include <complex>
#include <algorithm>

// ── 嵌套命名空间 (C++17) ──
namespace math::geometry {
    inline constexpr double PI = 3.141592653589793;

    inline double circle_area(double radius) {
        return PI * radius * radius;
    }
}

// ── inline 变量 (C++17) ──
namespace config {
    inline const std::string APP_NAME = "C++17高级示例";
    inline constexpr int VERSION_MAJOR = 1;
    inline constexpr int VERSION_MINOR = 0;
}

// ── 自定义类型支持结构化绑定 ──
class Student {
    std::string name_;
    int age_;
    double score_;
public:
    Student(std::string name, int age, double score)
        : name_(std::move(name)), age_(age), score_(score) {}

    // 提供tuple-like接口以支持结构化绑定
    template <std::size_t N>
    auto get() const {
        if constexpr (N == 0) return name_;
        else if constexpr (N == 1) return age_;
        else if constexpr (N == 2) return score_;
    }

    std::string_view name() const { return name_; }
    int age() const { return age_; }
    double score() const { return score_; }
};

// 为自定义类型特化 std::tuple_size 和 std::tuple_element
namespace std {
    template <>
    struct tuple_size<Student> : integral_constant<size_t, 3> {};

    template <>
    struct tuple_element<0, Student> { using type = std::string; };
    template <>
    struct tuple_element<1, Student> { using type = int; };
    template <>
    struct tuple_element<2, Student> { using type = double; };
}

// ====================================================================
// 1. if constexpr - 编译期条件编译
// ====================================================================
void demo_if_constexpr() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  if constexpr - 编译期条件编译\n";
    std::cout << "═══════════════════════════════════════\n\n";

    // 通用序列化函数：根据类型选择不同输出方式
    auto serialize = [](const auto& value) -> std::string {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_integral_v<T>) {
            return "整数:" + std::to_string(value);
        } else if constexpr (std::is_floating_point_v<T>) {
            return "浮点:" + std::to_string(value);
        } else if constexpr (std::is_same_v<T, std::string>) {
            return "字符串:" + value;
        } else if constexpr (std::is_same_v<T, bool>) {
            return value ? "布尔:真" : "布尔:假";
        } else {
            return "未知类型";
        }
    };

    std::cout << "通用序列化:\n";
    std::cout << "  " << serialize(42) << "\n";
    std::cout << "  " << serialize(3.14) << "\n";
    std::cout << "  " << serialize(std::string("Hello")) << "\n";
    std::cout << "  " << serialize(true) << "\n\n";

    // if constexpr vs 普通if
    std::cout << "if constexpr vs 普通if:\n";
    std::cout << "  普通if: 两个分支都会编译，类型不匹配则报错\n";
    std::cout << "  if constexpr: 未选中的分支被丢弃，不会编译\n\n";

    // 编译期类型安全的容器打印
    auto safe_print = [](const auto& container) {
        using T = std::decay_t<decltype(container)>;
        if constexpr (std::is_same_v<T, std::string>) {
            std::cout << "  字符串内容: \"" << container << "\"\n";
        } else {
            std::cout << "  容器元素: ";
            for (const auto& elem : container) {
                std::cout << elem << " ";
            }
            std::cout << "\n";
        }
    };

    std::vector<int> vec = {1, 2, 3, 4, 5};
    safe_print(vec);
    safe_print(std::string("你好C++17"));
}

// ====================================================================
// 2. 折叠表达式 - 变参模板展开
// ====================================================================
void demo_fold_expressions() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  折叠表达式 - 变参模板展开\n";
    std::cout << "═══════════════════════════════════════\n\n";

    // 左折叠 vs 右折叠
    auto left_fold_sum = [](auto... args) {
        return (... + args);  // 左折叠: ((a1 + a2) + a3) + a4
    };

    auto right_fold_sum = [](auto... args) {
        return (args + ...);  // 右折叠: a1 + (a2 + (a3 + a4))
    };

    std::cout << "左折叠 (... + args): " << left_fold_sum(1, 2, 3, 4) << "\n";
    std::cout << "右折叠 (args + ...): " << right_fold_sum(1, 2, 3, 4) << "\n\n";

    // 带初值的折叠表达式
    auto sum_with_init = [](auto... args) {
        return (0 + ... + args);  // 左折叠带初值
    };

    auto product_with_init = [](auto... args) {
        return (1 * ... * args);  // 左折叠带初值
    };

    std::cout << "带初值折叠:\n";
    std::cout << "  sum(空包) = " << sum_with_init() << " (初值0)\n";
    std::cout << "  product(空包) = " << product_with_init() << " (初值1)\n";
    std::cout << "  sum(1,2,3) = " << sum_with_init(1, 2, 3) << "\n";
    std::cout << "  product(1,2,3,4) = " << product_with_init(1, 2, 3, 4) << "\n\n";

    // 实用：逗号折叠打印
    auto print_args = [](auto first, auto... rest) {
        std::cout << "  参数列表: " << first;
        ((std::cout << ", " << rest), ...);
        std::cout << "\n";
    };

    // 实用：范围检查
    auto in_range = [](auto min, auto max, auto... vals) {
        return ((min <= vals && vals <= max) && ...);
    };

    std::cout << "范围检查:\n";
    std::cout << "  1~10内? (3,5,7): " << (in_range(1, 10, 3, 5, 7) ? "是" : "否") << "\n";
    std::cout << "  1~10内? (3,15,7): " << (in_range(1, 10, 3, 15, 7) ? "是" : "否") << "\n\n";

    // 实用：调用多个函数
    std::cout << "逗号折叠调用:\n";
    auto call_all = [](auto&&... funcs) {
        (funcs(), ...);  // 右折叠依次调用
    };

    int counter = 0;
    call_all(
        [&counter]() { std::cout << "  任务A完成 (计数=" << ++counter << ")\n"; },
        [&counter]() { std::cout << "  任务B完成 (计数=" << ++counter << ")\n"; },
        [&counter]() { std::cout << "  任务C完成 (计数=" << ++counter << ")\n"; }
    );
}

// ====================================================================
// 3. 结构化绑定 - 自定义类型
// ====================================================================
void demo_structured_bindings_custom() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  结构化绑定 - 自定义类型支持\n";
    std::cout << "═══════════════════════════════════════\n\n";

    // 绑定自定义类型 Student
    Student stu("王五", 20, 88.5);
    auto [name, age, score] = stu;
    std::cout << "Student 解包:\n";
    std::cout << "  姓名: " << name << "\n";
    std::cout << "  年龄: " << age << "\n";
    std::cout << "  成绩: " << score << "\n\n";

    // pair 的结构化绑定
    std::cout << "pair 解包:\n";
    auto [key, value] = std::make_pair("端口", 8080);
    std::cout << "  " << key << " = " << value << "\n\n";

    // tuple 的结构化绑定
    std::cout << "tuple 解包:\n";
    auto [x, y, z] = std::make_tuple(1.0, 2.0, 3.0);
    std::cout << "  坐标: (" << x << ", " << y << ", " << z << ")\n\n";

    // map 遍历 + 结构化绑定
    std::cout << "map 遍历:\n";
    std::map<std::string, double> grades = {
        {"物理", 92.5}, {"化学", 88.0}, {"生物", 95.5}
    };
    for (const auto& [subject, grade] : grades) {
        std::cout << "  " << subject << ": " << grade << "\n";
    }
    std::cout << "\n自定义类型支持结构化绑定的方法:\n";
    std::cout << "  1. 特化 std::tuple_size 获取元素个数\n";
    std::cout << "  2. 特化 std::tuple_element 获取元素类型\n";
    std::cout << "  3. 提供 get<N>() 成员或ADL函数\n";
}

// ====================================================================
// 4. std::optional - 表示可选值
// ====================================================================
void demo_optional_advanced() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  std::optional - 表示可选值\n";
    std::cout << "═══════════════════════════════════════\n\n";

    // 模拟配置查找
    std::map<std::string, std::string> config = {
        {"host", "127.0.0.1"},
        {"port", "8080"},
        {"timeout", "30"}
    };

    auto get_config = [&](const std::string& key) -> std::optional<std::string> {
        auto it = config.find(key);
        if (it != config.end()) return it->second;
        return std::nullopt;
    };

    std::cout << "配置查找:\n";
    std::cout << "  host: " << get_config("host").value_or("localhost") << "\n";
    std::cout << "  port: " << get_config("port").value_or("3000") << "\n";
    std::cout << "  debug: " << get_config("debug").value_or("false") << "\n\n";

    // optional 链式处理
    auto parse_int = [](const std::string& s) -> std::optional<int> {
        try { return std::stoi(s); }
        catch (...) { return std::nullopt; }
    };

    auto check_port = [](int port) -> std::optional<int> {
        if (port > 0 && port <= 65535) return port;
        return std::nullopt;
    };

    std::cout << "optional 链式处理:\n";
    if (auto port_str = get_config("port")) {
        if (auto port_num = parse_int(*port_str)) {
            if (auto valid_port = check_port(*port_num)) {
                std::cout << "  有效端口: " << *valid_port << "\n";
            } else {
                std::cout << "  端口超出范围\n";
            }
        }
    }

    // optional 作为返回值 vs 异常
    std::cout << "\noptional vs 异常:\n";
    std::cout << "  异常: 表示真正的错误，有性能开销\n";
    std::cout << "  optional: 表示可能缺失的值，零开销\n";
    std::cout << "  建议: 缺值是正常情况用optional，异常情况用异常\n\n";

    // optional 的比较操作
    std::optional<int> a = 10;
    std::optional<int> b = 20;
    std::optional<int> c = std::nullopt;
    std::cout << "optional 比较:\n";
    std::cout << "  a(10) < b(20): " << (a < b) << "\n";
    std::cout << "  c(nullopt) < a(10): " << (c < a) << "\n";
    std::cout << "  nullopt 总是小于任何有值状态\n";
}

// ====================================================================
// 5. std::variant - 类型安全联合体
// ====================================================================
void demo_variant_advanced() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  std::variant - 类型安全联合体\n";
    std::cout << "═══════════════════════════════════════\n\n";

    // 模拟JSON值类型
    using JsonValue = std::variant<
        std::nullptr_t,
        bool,
        int,
        double,
        std::string
    >;

    auto json_to_string = [](const JsonValue& val) -> std::string {
        return std::visit([](const auto& v) -> std::string {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::nullptr_t>) {
                return "null";
            } else if constexpr (std::is_same_v<T, bool>) {
                return v ? "true" : "false";
            } else if constexpr (std::is_same_v<T, int>) {
                return std::to_string(v);
            } else if constexpr (std::is_same_v<T, double>) {
                return std::to_string(v);
            } else if constexpr (std::is_same_v<T, std::string>) {
                return "\"" + v + "\"";
            } else {
                return "unknown";
            }
        }, val);
    };

    std::vector<JsonValue> json_values = {
        nullptr,
        true,
        42,
        3.14,
        std::string("你好")
    };

    std::cout << "JSON值类型模拟:\n";
    for (const auto& val : json_values) {
        std::cout << "  " << json_to_string(val) << "\n";
    }

    // variant 的 index() 和类型检查
    std::cout << "\n类型索引:\n";
    JsonValue v = std::string("test");
    std::cout << "  当前索引: " << v.index() << " (4 = std::string)\n";
    std::cout << "  holds_alternative<string>: " << std::holds_alternative<std::string>(v) << "\n\n";

    // overloaded 惯用法: 在全局作用域定义 (见文件上方)
    // template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };

    std::cout << "variant vs union:\n";
    std::cout << "  union: 无类型安全，需手动跟踪类型\n";
    std::cout << "  variant: 编译期类型安全，自动跟踪当前类型\n";
    std::cout << "  variant: 支持visit模式匹配\n";
    std::cout << "  variant: 非平凡类型(如string)也可存储\n";
}

// 注意: overloaded 需放在全局/命名空间作用域
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };

// ====================================================================
// 6. std::any - 类型擦除值
// ====================================================================
void demo_any_advanced() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  std::any - 类型擦除值\n";
    std::cout << "═══════════════════════════════════════\n\n";

    // 动态属性表
    std::map<std::string, std::any> properties = {
        {"名称", std::string("C++17应用")},
        {"版本", 17},
        {"评分", 4.8},
        {"开源", true}
    };

    std::cout << "动态属性表:\n";
    auto print_property = [](const std::string& key, const std::any& val) {
        if (auto p = std::any_cast<std::string>(&val)) {
            std::cout << "  " << key << " (string): " << *p << "\n";
        } else if (auto p = std::any_cast<int>(&val)) {
            std::cout << "  " << key << " (int): " << *p << "\n";
        } else if (auto p = std::any_cast<double>(&val)) {
            std::cout << "  " << key << " (double): " << *p << "\n";
        } else if (auto p = std::any_cast<bool>(&val)) {
            std::cout << "  " << key << " (bool): " << (*p ? "真" : "假") << "\n";
        }
    };

    for (const auto& [key, val] : properties) {
        print_property(key, val);
    }

    // any_cast 指针版本 vs 引用版本
    std::cout << "\nany_cast 两种用法:\n";
    std::any a = 42;
    std::cout << "  指针版 any_cast<int>(&a): " << (std::any_cast<int>(&a) ? "有值" : "无值") << "\n";
    std::cout << "  引用版 any_cast<int>(a): " << std::any_cast<int>(a) << "\n";

    a.reset();
    std::cout << "  reset后指针版: " << (std::any_cast<int>(&a) ? "有值" : "无值") << "\n\n";

    try {
        std::any bad = std::string("不是数字");
        std::any_cast<int>(bad);  // 抛出 bad_any_cast
    } catch (const std::bad_any_cast& e) {
        std::cout << "  类型不匹配时抛出 bad_any_cast 异常\n";
    }

    std::cout << "\nany vs variant 选择建议:\n";
    std::cout << "  类型集合已知 → 用 variant (编译期安全)\n";
    std::cout << "  类型集合开放 → 用 any (运行时灵活)\n";
    std::cout << "  插件系统/脚本绑定 → any 更合适\n";
}

// ====================================================================
// 7. std::string_view - 非拥有字符串引用
// ====================================================================
void demo_string_view_advanced() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  std::string_view - 非拥有字符串引用\n";
    std::cout << "═══════════════════════════════════════\n\n";

    // string_view 作为函数参数避免拷贝
    auto count_words = [](std::string_view text) -> size_t {
        size_t count = 0;
        bool in_word = false;
        for (char c : text) {
            if (c == ' ' || c == '\t' || c == '\n') {
                in_word = false;
            } else if (!in_word) {
                in_word = true;
                ++count;
            }
        }
        return count;
    };

    std::string text = "C++17 带来了 string_view 非常好用";
    std::cout << "词数统计:\n";
    std::cout << "  \"" << text << "\" → " << count_words(text) << " 个词\n";
    std::cout << "  \"Hello World C++17\" → " << count_words("Hello World C++17") << " 个词\n\n";

    // string_view 子串操作（零拷贝）
    std::cout << "零拷贝子串:\n";
    std::string_view url = "https://example.com/api/v1/users";
    auto protocol = url.substr(0, url.find("://"));
    auto host = url.substr(url.find("://") + 3, url.find("/", 8) - url.find("://") - 3);
    auto path = url.substr(url.find("/", 8));
    std::cout << "  URL: " << url << "\n";
    std::cout << "  协议: " << protocol << "\n";
    std::cout << "  主机: " << host << "\n";
    std::cout << "  路径: " << path << "\n\n";

    // string_view 性能对比
    std::cout << "string_view 性能优势:\n";
    std::cout << "  传 const string&: 字面量需构造临时string\n";
    std::cout << "  传 string_view: 零拷贝，直接传递\n";
    std::cout << "  子串操作: string::substr() 返回新string\n";
    std::cout << "           string_view::substr() 返回新视图\n\n";

    // string_view 注意事项
    std::cout << "⚠ 注意事项:\n";
    std::cout << "  1. 不保证以 \\0 结尾，不要传给需要C字符串的函数\n";
    std::cout << "  2. 不要用 string_view 持有临时 string\n";
    std::cout << "  3. 原始字符串销毁后，view 变为悬空引用\n";
    std::cout << "  4. 适合函数参数和短生命周期场景\n";
}

// ====================================================================
// 8. inline 变量
// ====================================================================
void demo_inline_variables() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  inline 变量\n";
    std::cout << "═══════════════════════════════════════\n\n";

    // 使用全局 inline 变量
    std::cout << "全局 inline 变量:\n";
    std::cout << "  APP_NAME: " << config::APP_NAME << "\n";
    std::cout << "  VERSION: " << config::VERSION_MAJOR << "." << config::VERSION_MINOR << "\n\n";

    // 使用命名空间中的 inline constexpr
    std::cout << "命名空间中的 inline constexpr:\n";
    std::cout << "  math::geometry::PI = " << math::geometry::PI << "\n";
    std::cout << "  圆面积(r=5) = " << math::geometry::circle_area(5.0) << "\n\n";

    std::cout << "C++17前的问题:\n";
    std::cout << "  头文件中: extern const int N; // 声明\n";
    std::cout << "  源文件中: const int N = 42;    // 定义\n";
    std::cout << "  多个源文件包含头文件 → ODR违规\n\n";

    std::cout << "C++17 inline 变量:\n";
    std::cout << "  头文件中: inline const int N = 42; // 定义，允许重复\n";
    std::cout << "  链接器保证只保留一个定义\n\n";

    std::cout << "inline 变量规则:\n";
    std::cout << "  - constexpr 变量隐式 inline\n";
    std::cout << "  - inline 变量可在头文件中定义\n";
    std::cout << "  - 多次包含不会违反ODR规则\n";
    std::cout << "  - 适合定义跨编译单元的常量\n";
}

// ====================================================================
// 9. 嵌套命名空间
// ====================================================================
void demo_nested_namespaces() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  嵌套命名空间声明\n";
    std::cout << "═══════════════════════════════════════\n\n";

    // 使用前面定义的嵌套命名空间
    std::cout << "使用嵌套命名空间:\n";
    std::cout << "  math::geometry::PI = " << math::geometry::PI << "\n";
    std::cout << "  math::geometry::circle_area(3) = " << math::geometry::circle_area(3.0) << "\n\n";

    std::cout << "语法对比:\n";
    std::cout << "  C++14 写法:\n";
    std::cout << "    namespace math {\n";
    std::cout << "      namespace geometry {\n";
    std::cout << "        constexpr double PI = 3.14;\n";
    std::cout << "      }\n";
    std::cout << "    }\n\n";

    std::cout << "  C++17 写法:\n";
    std::cout << "    namespace math::geometry {\n";
    std::cout << "      constexpr double PI = 3.14;\n";
    std::cout << "    }\n\n";

    std::cout << "嵌套命名空间的优势:\n";
    std::cout << "  - 减少缩进层级，代码更扁平\n";
    std::cout << "  - 模块组织更清晰\n";
    std::cout << "  - 与其他语言(Java/C#)的包语法类似\n";
}

// ====================================================================
// main
// ====================================================================
int main() {
    demo_if_constexpr();
    demo_fold_expressions();
    demo_structured_bindings_custom();
    demo_optional_advanced();
    demo_variant_advanced();
    demo_any_advanced();
    demo_string_view_advanced();
    demo_inline_variables();
    demo_nested_namespaces();
    return 0;
}
