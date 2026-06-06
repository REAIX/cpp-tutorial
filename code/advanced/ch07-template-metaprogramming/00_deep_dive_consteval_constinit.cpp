/** @file 00_deep_dive_consteval_constinit.cpp
 *  @brief consteval函数、constinit变量、constexpr/consteval/constinit三者的区别与联系
 *  @description 对应文档: 07-模板元编程与编译期计算 / 编译期计算基础(深入)
 */

#include <iostream>
#include <string>
#include <string_view>
#include <array>
#include <type_traits>
#include <cstdint>

// ============================================================
// 1. consteval —— 立即函数（C++20）
// ============================================================

// consteval 修饰的函数必须在编译期执行，称为"立即函数"
// 与 constexpr 的关键区别：
//   - constexpr 函数"可以"在编译期执行（如果参数是常量）
//   - consteval 函数"必须"在编译期执行（否则编译错误）

// consteval 阶乘：保证一定在编译期计算
consteval int factorial_ce(int n) {
    int result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

// consteval 字符串长度
consteval std::size_t const_strlen(const char* str) {
    std::size_t len = 0;
    while (str[len] != '\0') ++len;
    return len;
}

// consteval 编译期哈希
consteval std::uint32_t const_hash(std::string_view str) {
    std::uint32_t hash = 5381;
    for (char c : str) {
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(c);
    }
    return hash;
}

void demo_consteval() {
    std::cout << "=== consteval 立即函数 ===\n";

    // consteval 函数的结果一定是编译期常量
    constexpr int f10 = factorial_ce(10);
    std::cout << "factorial_ce(10) = " << f10 << " (保证编译期计算)\n";

    constexpr auto len = const_strlen("Hello, consteval!");
    std::cout << "const_strlen(\"Hello, consteval!\") = " << len << "\n";

    // 编译期哈希可用于 switch-case 标签
    constexpr auto hash_hello = const_hash("hello");
    constexpr auto hash_world = const_hash("world");
    std::cout << "const_hash(\"hello\") = " << hash_hello << "\n";
    std::cout << "const_hash(\"world\") = " << hash_world << "\n";

    // consteval 的关键限制：不能用运行期值调用
    // int n;
    // std::cin >> n;
    // int bad = factorial_ce(n);  // 编译错误! n 不是常量表达式

    // 但可以将 consteval 结果赋给非 constexpr 变量
    int runtime_val = factorial_ce(5);  // OK: 编译期计算，运行期赋值
    std::cout << "int runtime_val = factorial_ce(5) = " << runtime_val << "\n";

    std::cout << "\n";
}

// ============================================================
// 2. consteval 的实际应用场景
// ============================================================

// 场景1：编译期字符串验证
consteval bool is_valid_identifier(std::string_view name) {
    if (name.empty()) return false;
    if (!(name[0] == '_' || (name[0] >= 'a' && name[0] <= 'z') ||
          (name[0] >= 'A' && name[0] <= 'Z'))) {
        return false;
    }
    for (char c : name) {
        if (!(c == '_' || (c >= 'a' && c <= 'z') ||
              (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))) {
            return false;
        }
    }
    return true;
}

// 场景2：编译期配置验证
struct Config {
    int buffer_size;
    int max_connections;
    int timeout_ms;
};

consteval bool validate_config(const Config& cfg) {
    return cfg.buffer_size > 0 &&
           cfg.buffer_size <= 1024 * 1024 &&
           cfg.max_connections > 0 &&
           cfg.max_connections <= 10000 &&
           cfg.timeout_ms >= 100 &&
           cfg.timeout_ms <= 60000;
}

// 场景3：编译期格式字符串检查（简化版）
consteval bool check_format_string(std::string_view fmt) {
    int brace_count = 0;
    for (std::size_t i = 0; i < fmt.size(); ++i) {
        if (fmt[i] == '{') {
            if (i + 1 < fmt.size() && fmt[i + 1] == '{') {
                ++i;  // 跳过转义的 {{
                continue;
            }
            ++brace_count;
        } else if (fmt[i] == '}') {
            if (i + 1 < fmt.size() && fmt[i + 1] == '}') {
                ++i;  // 跳过转义的 }}
                continue;
            }
            --brace_count;
            if (brace_count < 0) return false;  // 多余的 }
        }
    }
    return brace_count == 0;
}

void demo_consteval_applications() {
    std::cout << "=== consteval 实际应用场景 ===\n";

    // 编译期标识符验证
    static_assert(is_valid_identifier("my_var"), "my_var 应为有效标识符");
    static_assert(is_valid_identifier("_private"), "_private 应为有效标识符");
    static_assert(!is_valid_identifier("123abc"), "123abc 不应为有效标识符");
    static_assert(!is_valid_identifier(""), "空字符串不应为有效标识符");
    std::cout << "标识符验证 static_assert 全部通过\n";

    // 编译期配置验证
    constexpr Config valid_cfg{4096, 100, 5000};
    static_assert(validate_config(valid_cfg), "配置应为有效");
    // constexpr Config invalid_cfg{-1, 0, 10};
    // static_assert(validate_config(invalid_cfg));  // 编译错误! 配置无效
    std::cout << "配置验证 static_assert 通过\n";

    // 编译期格式字符串检查
    static_assert(check_format_string("Hello, {}!"), "格式字符串应有效");
    static_assert(check_format_string("{} + {} = {}"), "格式字符串应有效");
    static_assert(!check_format_string("}invalid{"), "格式字符串应无效");
    std::cout << "格式字符串检查 static_assert 全部通过\n";

    std::cout << "\n";
}

// ============================================================
// 3. constinit —— 常量初始化（C++20）
// ============================================================

// constinit 保证变量在编译期初始化，但变量本身不是 const
// 核心用途：避免全局/静态变量的"静态初始化顺序问题"

// 问题：全局变量的初始化顺序在不同翻译单元间是未定义的
// constinit 确保变量在编译期完成初始化（零开销），不依赖运行期初始化

// constinit 全局变量
constinit int global_counter = 0;             // 编译期初始化，但可修改
constinit const char* global_name = "TMP Demo"; // 编译期初始化

// constinit 与 constexpr 的区别：
// constexpr 变量 = 编译期初始化 + 不可修改
// constinit 变量 = 编译期初始化 + 可修改

// constinit 与 const 的组合
constinit const int global_version = 2;  // 编译期初始化 + 不可修改（等同于 constexpr）

// 使用 consteval 函数初始化 constinit 变量
constinit std::uint32_t global_hash = const_hash("init_tag");

void demo_constinit() {
    std::cout << "=== constinit 常量初始化 ===\n";

    std::cout << "global_counter = " << global_counter << "\n";
    std::cout << "global_name = " << global_name << "\n";
    std::cout << "global_version = " << global_version << "\n";
    std::cout << "global_hash = " << global_hash << "\n";

    // constinit 变量可以修改（不像 constexpr）
    global_counter = 42;
    std::cout << "修改后 global_counter = " << global_counter << "\n";

    // constinit 也可用于局部静态变量
    static constinit int call_count = 0;    ++call_count;
    std::cout << "call_count = " << call_count << "\n";

    // constinit 的限制：初始化器必须是常量表达式
    // constinit int bad = std::rand();  // 编译错误! rand() 不是常量表达式

    std::cout << "\n";
}

// ============================================================
// 4. 避免静态初始化顺序问题（SIOF）
// ============================================================

// 经典的静态初始化顺序问题（Static Initialization Order Fiasco）
// 当一个全局变量的初始化依赖另一个全局变量时，如果两者在不同翻译单元，
// 初始化顺序是未定义的

// 传统解决方案：函数内静态变量（Meyer's Singleton）
class Logger {
public:
    static Logger& instance() {
        static Logger inst;  // C++11 保证线程安全的延迟初始化
        return inst;
    }
    void log(const std::string& msg) {
        std::cout << "[LOG] " << msg << "\n";
    }
private:
    Logger() = default;
};

// constinit 解决方案：编译期初始化，完全避免顺序问题
constinit int init_order_safe_value = 100;

// 用 constinit + consteval 组合确保安全
constinit std::size_t safe_buffer_size = factorial_ce(5);  // 120

void demo_siof_solution() {
    std::cout << "=== 避免静态初始化顺序问题 ===\n";

    Logger::instance().log("通过 Meyer's Singleton 避免初始化顺序问题");
    Logger::instance().log("constinit 变量在编译期初始化，完全避免 SIOF");

    std::cout << "init_order_safe_value = " << init_order_safe_value << "\n";
    std::cout << "safe_buffer_size = " << safe_buffer_size << "\n";

    std::cout << "\n";
}

// ============================================================
// 5. constexpr vs consteval vs constinit 对比
// ============================================================

// 对比表：
// ┌─────────────┬──────────────┬──────────────┬──────────────┐
// │   特性       │  constexpr   │  consteval   │  constinit   │
// ├─────────────┼──────────────┼──────────────┼──────────────┤
// │ 适用对象     │ 变量/函数     │ 函数         │ 变量         │
// │ 编译期计算   │ 可以          │ 必须          │ 初始化必须   │
// │ 运行期执行   │ 可以(退化)    │ 不可以        │ 不适用       │
// │ 结果可修改   │ 不可以(const) │ 不适用        │ 可以         │
// │ 解决SIOF    │ 是(constexpr) │ 不适用        │ 是           │
// └─────────────┴──────────────┴──────────────┴──────────────┘

// constexpr 函数：可以在编译期或运行期执行
constexpr int constexpr_func(int n) {
    return n * n;
}

// consteval 函数：必须在编译期执行
consteval int consteval_func(int n) {
    return n * n;
}

void demo_comparison() {
    std::cout << "=== constexpr vs consteval vs constinit 对比 ===\n";

    // constexpr 函数的双重性
    constexpr int ce_result = constexpr_func(5);  // 编译期执行
    int runtime_n = 5;
    int rt_result = constexpr_func(runtime_n);    // 运行期执行
    std::cout << "constexpr_func(5) 编译期: " << ce_result << "\n";
    std::cout << "constexpr_func(runtime_n) 运行期: " << rt_result << "\n";

    // consteval 函数只能编译期执行
    constexpr int cve_result = consteval_func(5);  // OK: 编译期
    // int bad = consteval_func(runtime_n);  // 编译错误! 必须编译期
    std::cout << "consteval_func(5) = " << cve_result << " (只能编译期)\n";

    // constinit 变量（全局/静态）
    static constinit int ci_var = constexpr_func(3);  // OK: 编译期初始化
    ci_var = 999;  // OK: 可以修改
    std::cout << "constinit 变量修改后: " << ci_var << "\n";

    // constexpr 变量
    constexpr int cx_var = constexpr_func(3);  // OK: 编译期初始化
    // cx_var = 999;  // 编译错误! constexpr 变量不可修改
    std::cout << "constexpr 变量: " << cx_var << " (不可修改)\n";

    std::cout << "\n选择指南:\n";
    std::cout << "  - 需要编译期常量且不可变 → constexpr 变量\n";
    std::cout << "  - 需要函数强制编译期执行 → consteval 函数\n";
    std::cout << "  - 需要编译期初始化但可变 → constinit 变量\n";
    std::cout << "  - 函数可能编译期也可能运行期 → constexpr 函数\n";

    std::cout << "\n";
}

// ============================================================
// 6. consteval 函数的递归与模板交互
// ============================================================

// consteval 可以与模板结合使用
template<typename T>
consteval T max_value() {
    if constexpr (std::is_same_v<T, int>) {
        return INT_MAX;
    } else if constexpr (std::is_same_v<T, long>) {
        return LONG_MAX;
    } else if constexpr (std::is_same_v<T, unsigned int>) {
        return UINT_MAX;
    } else {
        return T{0};  // 默认值
    }
}

// 需要头文件
#include <climits>

// consteval 递归：编译期字符串比较
consteval bool const_strequal(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return false;
        ++a;
        ++b;
    }
    return *a == '\0' && *b == '\0';
}

void demo_consteval_advanced() {
    std::cout << "=== consteval 高级用法 ===\n";

    // consteval + 模板
    constexpr auto int_max = max_value<int>();
    constexpr auto uint_max = max_value<unsigned int>();
    std::cout << "max_value<int>() = " << int_max << "\n";
    std::cout << "max_value<unsigned int>() = " << uint_max << "\n";

    // consteval 字符串比较
    static_assert(const_strequal("hello", "hello"), "相同字符串应相等");
    static_assert(!const_strequal("hello", "world"), "不同字符串应不等");
    std::cout << "const_strequal 验证通过\n";

    // consteval 函数可以调用其他 consteval 或 constexpr 函数
    // 但不能调用非 constexpr 函数
    constexpr auto hash1 = const_hash("test");
    constexpr auto hash2 = const_hash("test");
    static_assert(hash1 == hash2, "相同字符串的哈希应相同");
    std::cout << "consteval 函数间调用验证通过\n";

    std::cout << "\n";
}

// ============================================================
// main
// ============================================================

int main() {
    std::cout << "============================================\n";
    std::cout << "  consteval / constinit 深入解析\n";
    std::cout << "============================================\n\n";

    demo_consteval();
    demo_consteval_applications();
    demo_constinit();
    demo_siof_solution();
    demo_comparison();
    demo_consteval_advanced();

    std::cout << "============================================\n";
    std::cout << "  核心要点总结:\n";
    std::cout << "  1. consteval = 必须编译期执行的函数\n";
    std::cout << "  2. constinit = 必须编译期初始化的变量\n";
    std::cout << "  3. constexpr = 可以编译期也可以运行期\n";
    std::cout << "  4. constinit 解决静态初始化顺序问题\n";
    std::cout << "  5. consteval 适合编译期验证和计算\n";
    std::cout << "============================================\n";

    return 0;
}
