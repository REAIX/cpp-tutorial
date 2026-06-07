/**
 * @file 01_example_cpp23_features.cpp
 * @brief C++23新特性示例
 * @description 对应文档: 02-CPP/28-cpp23
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <variant>
#include <stdexcept>
#include <functional>
#include <cstdint>
#include <array>
#include <algorithm>

#if defined(__cpp_lib_expected) && __cpp_lib_expected >= 202211L
#include <expected>
using std::expected;
using std::unexpected;
#endif

struct ErrorInfo {
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
    expected& operator=(const expected&) = default;
    expected& operator=(expected&&) = default;

    bool has_value() const { return has_value_; }
    T& value() & { if (!has_value_) throw std::runtime_error("bad expected access"); return std::get<0>(data_); }
    const T& value() const& { if (!has_value_) throw std::runtime_error("bad expected access"); return std::get<0>(data_); }
    E& error() & { if (has_value_) throw std::runtime_error("bad expected access"); return std::get<1>(data_); }
    const E& error() const& { if (has_value_) throw std::runtime_error("bad expected access"); return std::get<1>(data_); }
    T value_or(T default_val) const { return has_value_ ? value() : default_val; }
    T& operator*() { return value(); }
    const T& operator*() const { return value(); }
    T* operator->() { return &value(); }
    const T* operator->() const { return &value(); }
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

expected<int, ErrorInfo> make_expected_value(int val) {
    return expected<int, ErrorInfo>(val);
}

expected<int, ErrorInfo> make_expected_error(int code, const std::string& msg) {
    return expected<int, ErrorInfo>(ErrorInfo{code, msg}, false);
}

expected<int, ErrorInfo> safe_divide(int a, int b) {
    if (b == 0) {
        return make_expected_error(1, "除零错误");
    }
    if (a == 0 && b == 0) {
        return make_expected_error(2, "未定义");
    }
    return make_expected_value(a / b);
}

expected<std::string, ErrorInfo> read_config(const std::string& key) {
    static std::map<std::string, std::string> config = {
        {"host", "localhost"}, {"port", "8080"}, {"debug", "true"}
    };
    auto it = config.find(key);
    if (it == config.end()) {
        return expected<std::string, ErrorInfo>(ErrorInfo{404, "配置项不存在: " + key}, false);
    }
    return expected<std::string, ErrorInfo>(it->second);
}

void demo_expected() {
    std::cout << "\n=== std::expected ===\n";

    auto r1 = safe_divide(10, 3);
    if (r1.has_value()) {
        std::cout << "10/3 = " << r1.value() << "\n";
    }

    auto r2 = safe_divide(10, 0);
    if (!r2.has_value()) {
        std::cout << "10/0 错误: " << r2.error().message << " (code=" << r2.error().code << ")\n";
    }

    auto r3 = read_config("host");
    std::cout << "host = " << r3.value_or("default_host") << "\n";

    auto r4 = read_config("missing");
    std::cout << "missing = " << r4.value_or("default_value") << "\n";
    if (!r4.has_value()) {
        std::cout << "错误: " << r4.error().message << "\n";
    }

    std::cout << "\n举一反三:\n";
    std::cout << "  expected<T,E> vs optional<T>:\n";
    std::cout << "    optional: 只有值或空, 无错误信息\n";
    std::cout << "    expected: 值或详细错误信息\n";
    std::cout << "  expected<T,E> vs 异常:\n";
    std::cout << "    异常: 控制流跳转, 有开销\n";
    std::cout << "    expected: 显式处理, 零开销\n";
}

void demo_std_print() {
    std::cout << "\n=== std::print ===\n";

#if HAS_PRINT && 0
    std::print("std::print: 你好, {}! 数字={}\n", "C++23", 42);
    std::println("std::println: 自动换行");
    std::print(stderr, "错误输出: {}\n", "test");
#else
    std::cout << "std::print 不可用(需要C++23编译器支持), 概念演示:\n";
    std::cout << "  std::print(\"你好, {}! 数字={}\", \"C++23\", 42)\n";
    std::cout << "  std::println(\"自动换行\")\n";
    std::cout << "  std::print(stderr, \"错误输出: {}\", \"test\")\n";
#endif

    std::cout << "\nstd::print vs iostream:\n";
    std::cout << "  iostream: 类型安全, 但冗长, 性能较差\n";
    std::cout << "  std::print: 类型安全, 简洁, 性能更好\n";
    std::cout << "  std::print: 支持Unicode, 格式化字符串\n";
    std::cout << "  std::print: 类似Python的print和Rust的println!\n";
}

void demo_deducing_this() {
    std::cout << "\n=== Deducing this (推导this) ===\n";

    std::cout << "C++23允许显式声明this参数:\n\n";

    std::cout << R"(  // C++20: 需要写const和非const两个版本
  class Widget {
      int value_;
  public:
      int& get() { return value_; }
      const int& get() const { return value_; }
  };

  // C++23: 只需一个版本
  class Widget {
      int value_;
  public:
      auto&& get(this auto&& self) {
          return std::forward<decltype(self)>(self).value_;
      }
  };
)" << "\n";

    std::cout << "举一反三:\n";
    std::cout << "  1. 减少const/non-const重复代码\n";
    std::cout << "  2. 实现递归lambda\n";
    std::cout << "  3. 完美转发成员函数\n";
    std::cout << "  4. CRTP模式简化\n";

    std::cout << "\n递归lambda示例:\n";
    std::cout << R"(  auto fibonacci = [](this auto self, int n) -> int {
      if (n <= 1) return n;
      return self(n - 1) + self(n - 2);
  };
)" << "\n";
}

void demo_std_unreachable() {
    std::cout << "\n=== std::unreachable() ===\n";

    std::cout << "标记不可达代码, 如果执行到则未定义行为:\n\n";

    auto color_to_string = [](int c) -> std::string {
        switch (c) {
            case 0: return "Red";
            case 1: return "Green";
            case 2: return "Blue";
            default:
                std::cout << "  (如果执行到这里, 是未定义行为)\n";
                std::terminate();
        }
    };

    std::cout << "color 0: " << color_to_string(0) << "\n";
    std::cout << "color 1: " << color_to_string(1) << "\n";

    std::cout << "\n用途:\n";
    std::cout << "  1. switch的default分支(已处理所有情况)\n";
    std::cout << "  2. 编译器优化提示(不需要处理不可达路径)\n";
    std::cout << "  3. 比__builtin_unreachable()更标准\n";
}

void demo_multidimensional_subscript() {
    std::cout << "\n=== 多维下标运算符 ===\n";

    std::cout << "C++23允许operator[]接受多个参数:\n\n";

    std::cout << R"(  // C++20: 需要逗号hack或函数调用
  matrix(i, j)   // 函数调用语法
  matrix[(i, j)] // 逗号运算符hack(危险!)

  // C++23: 直接多维下标
  matrix[i, j]   // 多维下标运算符
)" << "\n";

    class Matrix2D {
        std::array<std::array<int, 3>, 3> data_{};
    public:
        int& operator()(size_t i, size_t j) { return data_[i][j]; }
        const int& operator()(size_t i, size_t j) const { return data_[i][j]; }
    };

    Matrix2D m;
    m(0, 0) = 1;
    m(1, 1) = 5;
    m(2, 2) = 9;
    std::cout << "m(0,0)=" << m(0, 0) << " m(1,1)=" << m(1, 1) << " m(2,2)=" << m(2, 2) << "\n";
    std::cout << "C++23中可以写 m[0,0] 代替 m(0,0)\n";
}

void demo_other_cpp23_features() {
    std::cout << "\n=== 其他C++23特性 ===\n";

    std::cout << "1. std::byteswap:\n";
    std::cout << "   翻转字节序: std::byteswap(uint32_t(0x01020304))\n";
    std::cout << "   结果: 0x04030201\n";

    std::cout << "\n2. std::flat_map / std::flat_set:\n";
    std::cout << "   基于排序数组的关联容器\n";
    std::cout << "   更好的缓存局部性\n";
    std::cout << "   适合小数据集和只读场景\n";

    std::cout << "\n3. std::generator:\n";
    std::cout << "   标准库的协程生成器\n";
    std::cout << "   类似我们手写的Generator, 但更完善\n";

    std::cout << "\n4. contains() for string:\n";
    std::string s = "hello world";
    bool has_hello = s.find("hello") != std::string::npos;
    std::cout << "   C++23: s.contains(\"hello\") = " << std::boolalpha << has_hello << "\n";

    std::cout << "\n5. ranges改进:\n";
    std::cout << "   std::views::zip, std::views::chunk, std::views::slide\n";
    std::cout << "   std::views::join_with, std::views::enumerate\n";

    std::cout << "\n6. if consteval:\n";
    std::cout << "   if consteval { /* 编译期 */ } else { /* 运行时 */ }\n";
    std::cout << "   比if constexpr(std::is_constant_evaluated())更清晰\n";
}

int main() {
    std::cout << "========== C++23 新特性示例 ==========\n";
    std::cout << "注意: 部分特性可能需要较新编译器支持\n";

    demo_expected();
    demo_std_print();
    demo_deducing_this();
    demo_std_unreachable();
    demo_multidimensional_subscript();
    demo_other_cpp23_features();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
