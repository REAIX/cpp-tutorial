/**
 * @file 01_deep_dive_cpp23_patterns.cpp
 * @brief C++23实践模式深入探讨
 * @description 对应文档: 02-CPP/28-cpp23
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <stdexcept>
#include <functional>
#include <cstdint>
#include <variant>

#if defined(__cpp_lib_expected) && __cpp_lib_expected >= 202211L
#include <expected>
using std::expected;
using std::unexpected;
#endif

struct Error {
    int code;
    std::string message;
};

#if !(defined(__cpp_lib_expected) && __cpp_lib_expected >= 202211L)
template<typename T, typename E>
class expected {
    std::variant<T, E> data_;
    bool has_value_;
public:
    expected(T val) : data_(std::move(val)), has_value_(true) {}
    expected(E err, bool) : data_(std::move(err)), has_value_(false) {}
    expected(const expected&) = default;
    expected(expected&&) = default;

    bool has_value() const { return has_value_; }
    T& value() & { if (!has_value_) throw std::runtime_error("bad expected access"); return std::get<0>(data_); }
    const T& value() const& { if (!has_value_) throw std::runtime_error("bad expected access"); return std::get<0>(data_); }
    E& error() & { if (has_value_) throw std::runtime_error("bad expected access"); return std::get<1>(data_); }
    const E& error() const& { if (has_value_) throw std::runtime_error("bad expected access"); return std::get<1>(data_); }
    T value_or(T default_val) const { return has_value_ ? value() : default_val; }
    T& operator*() { return value(); }
    T* operator->() { return &value(); }
    explicit operator bool() const { return has_value_; }
};

template<typename E>
class unexpected {
    E error_;
public:
    explicit unexpected(E err) : error_(std::move(err)) {}
    const E& error() const { return error_; }
};
#endif

expected<int, Error> parse_int(const std::string& s) {
    try {
        size_t pos;
        int val = std::stoi(s, &pos);
        if (pos != s.size()) {
            return expected<int, Error>(Error{2, "部分解析: " + s}, false);
        }
        return expected<int, Error>(val);
    } catch (const std::exception&) {
        return expected<int, Error>(Error{1, "无法解析: " + s}, false);
    }
}

expected<double, Error> safe_divide(double a, double b) {
    if (b == 0) return expected<double, Error>(Error{100, "除零错误"}, false);
    return expected<double, Error>(a / b);
}

expected<std::string, Error> format_result(double val) {
    if (val != val) return expected<std::string, Error>(Error{200, "NaN结果"}, false);
    return expected<std::string, Error>(std::to_string(val));
}

void demo_expected_error_handling() {
    std::cout << "\n=== expected错误处理模式 ===\n";

    auto r1 = parse_int("42");
    if (r1.has_value()) {
        std::cout << "解析'42': " << r1.value() << "\n";
    }

    auto r2 = parse_int("abc");
    if (!r2.has_value()) {
        std::cout << "解析'abc'错误: " << r2.error().message << "\n";
    }

    auto r3 = parse_int("42abc");
    if (!r3.has_value()) {
        std::cout << "解析'42abc'错误: " << r3.error().message << "\n";
    }

    std::cout << "\n举一反三 - 链式错误处理:\n";
    auto result = safe_divide(10.0, 3.0);
    if (result.has_value()) {
        auto formatted = format_result(result.value());
        if (formatted.has_value()) {
            std::cout << "10/3 = " << formatted.value() << "\n";
        } else {
            std::cout << "格式化错误: " << formatted.error().message << "\n";
        }
    } else {
        std::cout << "计算错误: " << result.error().message << "\n";
    }

    auto div_zero = safe_divide(10.0, 0.0);
    if (!div_zero.has_value()) {
        std::cout << "10/0 错误: " << div_zero.error().message << "\n";
    }
}

void demo_expected_vs_exceptions() {
    std::cout << "\n=== expected vs 异常 ===\n";

    std::cout << "错误处理方式对比:\n\n";

    std::cout << "1. 异常(Exceptions):\n";
    std::cout << "   优点: 错误传播自动, 代码简洁\n";
    std::cout << "   缺点: 控制流不透明, 有运行时开销\n";
    std::cout << "   适合: 罕见错误, 跨层传播\n";

    std::cout << "\n2. expected<T,E>:\n";
    std::cout << "   优点: 显式处理, 零开销, 类型安全\n";
    std::cout << "   缺点: 每步都需要检查, 代码较冗长\n";
    std::cout << "   适合: 常见错误, 局部处理, 性能敏感\n";

    std::cout << "\n3. optional<T>:\n";
    std::cout << "   优点: 最简单, 无错误信息\n";
    std::cout << "   缺点: 无法区分错误类型\n";
    std::cout << "   适合: 简单的值/空判断\n";

    std::cout << "\n4. 错误码(Return codes):\n";
    std::cout << "   优点: 最底层, 无依赖\n";
    std::cout << "   缺点: 容易忽略, 不类型安全\n";
    std::cout << "   适合: C接口, 极度性能敏感\n";

    std::cout << "\n推荐策略:\n";
    std::cout << "  - 正常路径: expected<T,E>\n";
    std::cout << "  - 不可恢复错误: 异常或terminate\n";
    std::cout << "  - 简单判断: optional<T>\n";
    std::cout << "  - 混合使用: expected + 异常\n";
}

void demo_std_print_vs_iostream() {
    std::cout << "\n=== std::print vs iostream ===\n";

    std::cout << "iostream的问题:\n";
    std::cout << "  1. 性能差(虚函数调用, locale, 同步)\n";
    std::cout << "  2. 格式化不灵活(需手动拼接)\n";
    std::cout << "  3. Unicode支持不完善\n";
    std::cout << "  4. 线程安全(但输出交错)\n";

    std::cout << "\nstd::print的优势:\n";
    std::cout << "  1. 高性能(直接写缓冲区)\n";
    std::cout << "  2. 格式化字符串(类似fmt库)\n";
    std::cout << "  3. Unicode原生支持\n";
    std::cout << "  4. 线程安全(原子输出)\n";

    std::cout << "\n性能对比(概念性):\n";
    std::cout << "  iostream: cout << \"Value: \" << 42 << \"\\n\";\n";
    std::cout << "  std::print: std::print(\"Value: {}\\n\", 42);\n";
    std::cout << "  std::print通常快2-5倍\n";

    std::cout << "\n格式化示例:\n";
    std::cout << R"(  std::print("{:>10}", 42);     // 右对齐, 宽10
  std::print("{:06d}", 42);     // 补零, 宽6
  std::print("{:.2f}", 3.14159); // 2位小数
  std::print("{:x}", 255);      // 十六进制
)" << "\n";
}

void demo_feature_test_macros() {
    std::cout << "\n=== C++23特性测试宏 ===\n";

    std::cout << "__cpp_lib_expected: "
#if defined(__cpp_lib_expected)
        << __cpp_lib_expected << "\n"
#else
        << "未定义\n"
#endif
    ;

    std::cout << "__cpp_lib_print: "
#if defined(__cpp_lib_print)
        << __cpp_lib_print << "\n"
#else
        << "未定义\n"
#endif
    ;

    std::cout << "__cpp_lib_flat_map: "
#if defined(__cpp_lib_flat_map)
        << __cpp_lib_flat_map << "\n"
#else
        << "未定义\n"
#endif
    ;

    std::cout << "__cpp_lib_generator: "
#if defined(__cpp_lib_generator)
        << __cpp_lib_generator << "\n"
#else
        << "未定义\n"
#endif
    ;

    std::cout << "__cpp_lib_byteswap: "
#if defined(__cpp_lib_byteswap)
        << __cpp_lib_byteswap << "\n"
#else
        << "未定义\n"
#endif
    ;

    std::cout << "__cpp_lib_unreachable: "
#if defined(__cpp_lib_unreachable)
        << __cpp_lib_unreachable << "\n"
#else
        << "未定义\n"
#endif
    ;

    std::cout << "\n用法: #if __cpp_lib_expected >= 202211L ... #endif\n";
}

void demo_cpp23_in_practice() {
    std::cout << "\n=== C++23实践建议 ===\n";

    std::cout << "1. 渐进采用:\n";
    std::cout << "   先使用向后兼容的特性(expected, print)\n";
    std::cout << "   再使用需要编译器升级的特性(deducing this)\n";

    std::cout << "\n2. 条件编译:\n";
    std::cout << "   使用__has_include和__cpp_lib_*宏\n";
    std::cout << "   提供回退实现确保代码可编译\n";

    std::cout << "\n3. 编译器支持:\n";
    std::cout << "   GCC 12+: 部分C++23\n";
    std::cout << "   GCC 13+: 大部分C++23\n";
    std::cout << "   Clang 15+: 部分C++23\n";
    std::cout << "   MSVC 19.34+: 部分C++23\n";

    std::cout << "\n4. 构建配置:\n";
    std::cout << "   g++ -std=c++23 file.cpp\n";
    std::cout << "   或 g++ -std=c++2b file.cpp (旧写法)\n";
}

void demo_expected_monadic() {
    std::cout << "\n=== expected单子操作 ===\n";

    std::cout << "expected支持单子操作(monadic operations):\n";
    std::cout << "  and_then: 链式操作(成功时执行)\n";
    std::cout << "  or_else:  错误恢复(失败时执行)\n";
    std::cout << "  transform: 值变换(成功时变换)\n";
    std::cout << "  transform_error: 错误变换\n";

    std::cout << "\n示例(概念性, 需要标准库支持):\n";
    std::cout << R"(  auto result = parse_int("42")
      .and_then([](int n) { return safe_divide(n, 2); })
      .transform([](double d) { return d * 10; })
      .or_else([](Error e) -> expected<double, Error> {
          std::cout << "恢复: " << e.message << "\n";
          return 0.0;
      });
)" << "\n";

    auto r = parse_int("42");
    if (r.has_value()) {
        auto d = safe_divide(r.value(), 2.0);
        if (d.has_value()) {
            std::cout << "链式结果: " << d.value() * 10 << "\n";
        }
    }
}

int main() {
    std::cout << "========== C++23 实践模式深入探讨 ==========\n";
    std::cout << "注意: 部分特性可能需要较新编译器支持\n";

    demo_expected_error_handling();
    demo_expected_vs_exceptions();
    demo_std_print_vs_iostream();
    demo_feature_test_macros();
    demo_cpp23_in_practice();
    demo_expected_monadic();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
