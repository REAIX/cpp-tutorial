/** @file 00_example_constexpr_basics.cpp
 *  @brief 编译期计算基础：constexpr函数、constexpr变量、if constexpr、static_assert
 *  @description 对应文档: 07-模板元编程与编译期计算 / 编译期计算基础
 */

#include <iostream>
#include <array>
#include <cmath>
#include <type_traits>

// ============================================================
// 1. constexpr 变量 —— 编译期常量
// ============================================================

// constexpr 变量必须在编译期就能确定值
constexpr int ARRAY_SIZE = 10;            // 编译期常量，可用于数组大小
constexpr double PI = 3.141592653589793;  // 编译期浮点常量
constexpr const char* APP_NAME = "TMP Demo";  // 编译期字符串常量

// constexpr 变量与 const 变量的区别：
// - constexpr 一定是编译期常量
// - const 可能是运行期常量（如 const int x = std::rand();）

void demo_constexpr_variable() {
    std::cout << "=== constexpr 变量 ===\n";

    // constexpr 变量可用于需要编译期常量的场景
    std::array<int, ARRAY_SIZE> arr{};  // 数组大小需要编译期常量
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        arr[i] = i * i;
    }

    std::cout << "ARRAY_SIZE = " << ARRAY_SIZE << "\n";
    std::cout << "PI = " << PI << "\n";
    std::cout << "APP_NAME = " << APP_NAME << "\n";
    std::cout << "arr[5] = " << arr[5] << "\n";

    // 运行期变量也可以用 const，但不能用 constexpr
    int runtime_val = 42;
    const int const_from_runtime = runtime_val;  // OK: const 可以接受运行期值
    // constexpr int ce_from_runtime = runtime_val;  // 错误! constexpr 要求编译期值

    std::cout << "const_from_runtime = " << const_from_runtime << "\n";
    std::cout << "\n";
}

// ============================================================
// 2. constexpr 函数 —— 可在编译期或运行期执行
// ============================================================

// constexpr 函数的核心特性：
// - 如果参数都是编译期常量，则结果也是编译期常量
// - 如果参数包含运行期值，则退化为运行期执行
// - C++14 起允许局部变量、循环、条件语句等

// 阶乘：经典的编译期计算示例
constexpr int factorial(int n) {
    int result = 1;
    for (int i = 2; i <= n; ++i) {  // C++14 允许循环
        result *= i;
    }
    return result;
}

// 斐波那契数列
constexpr int fibonacci(int n) {
    if (n <= 1) return n;            // C++14 允许 if 语句
    int a = 0, b = 1;
    for (int i = 2; i <= n; ++i) {
        int tmp = a + b;
        a = b;
        b = tmp;
    }
    return b;
}

// 编译期最大公约数（欧几里得算法）
constexpr int gcd(int a, int b) {
    while (b != 0) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

// 编译期幂运算
constexpr double power(double base, int exp) {
    double result = 1.0;
    bool negative = exp < 0;
    if (negative) exp = -exp;
    for (int i = 0; i < exp; ++i) {
        result *= base;
    }
    return negative ? 1.0 / result : result;
}

void demo_constexpr_function() {
    std::cout << "=== constexpr 函数 ===\n";

    // 编译期调用：结果在编译期确定，可用于需要常量表达式的场景
    constexpr int fact10 = factorial(10);   // 编译期计算
    constexpr int fib10 = fibonacci(10);    // 编译期计算
    constexpr int gcd_val = gcd(48, 36);    // 编译期计算
    constexpr double pow_val = power(2.0, 10);  // 编译期计算

    // 用编译期结果初始化数组大小
    std::array<int, factorial(5)> fact_arr{};  // 大小为 120

    std::cout << "factorial(10) = " << fact10 << " (编译期计算)\n";
    std::cout << "fibonacci(10) = " << fib10 << " (编译期计算)\n";
    std::cout << "gcd(48, 36) = " << gcd_val << " (编译期计算)\n";
    std::cout << "power(2.0, 10) = " << pow_val << " (编译期计算)\n";
    std::cout << "factorial(5) 数组大小 = " << fact_arr.size() << "\n";

    // 运行期调用：参数为运行期值时，函数在运行期执行
    int n;
    std::cout << "\n输入一个整数(运行期值): ";
    std::cin >> n;
    int runtime_fact = factorial(n);  // 运行期执行，结果不是编译期常量
    std::cout << "factorial(" << n << ") = " << runtime_fact << " (运行期计算)\n";

    std::cout << "\n";
}

// ============================================================
// 3. if constexpr —— 编译期条件分支（C++17）
// ============================================================

// if constexpr 根据编译期条件选择性地编译代码
// 不满足条件的分支会被完全丢弃（不会编译），而非仅仅不执行

// 编译期打印容器内容：根据容器类型选择不同策略
template<typename T>
void print_container(const T& container) {
    // 编译期判断是否有 size() 方法
    if constexpr (requires { container.size(); }) {
        std::cout << "容器大小: " << container.size() << ", 元素: ";
    } else {
        std::cout << "元素: ";
    }

    // 编译期判断是否有 begin()/end()
    if constexpr (requires { container.begin(); container.end(); }) {
        for (const auto& elem : container) {
            std::cout << elem << " ";
        }
    } else {
        std::cout << container;
    }
    std::cout << "\n";
}

// 编译期类型转换：根据类型选择不同处理方式
template<typename T>
auto process_value(T value) {
    if constexpr (std::is_integral_v<T>) {
        std::cout << "整型处理: " << value << " -> " << value * 2 << "\n";
        return value * 2;
    } else if constexpr (std::is_floating_point_v<T>) {
        std::cout << "浮点处理: " << value << " -> " << value * 1.5 << "\n";
        return value * 1.5;
    } else {
        std::cout << "其他类型处理: " << value << "\n";
        return value;
    }
}

// 编译期递归解包：处理不同数量的参数
template<typename T>
auto sum_all(T value) {
    return value;
}

template<typename T, typename... Rest>
auto sum_all(T first, Rest... rest) {
    if constexpr (sizeof...(rest) > 0) {
        return first + sum_all(rest...);
    } else {
        return first;
    }
}

void demo_if_constexpr() {
    std::cout << "=== if constexpr 编译期条件分支 ===\n";

    // 根据类型选择不同分支
    process_value(42);        // 整型分支
    process_value(3.14);      // 浮点分支
    process_value("hello");   // 其他类型分支

    std::cout << "\n";

    // 容器打印
    std::array<int, 5> arr = {1, 2, 3, 4, 5};
    print_container(arr);

    // if constexpr vs 普通 if 的区别
    std::cout << "\nif constexpr vs 普通 if:\n";
    std::cout << "  普通 if: 两个分支都会编译，运行期选择\n";
    std::cout << "  if constexpr: 不满足的分支完全不存在于编译结果中\n";
    std::cout << "  这意味着不满足分支中的语法错误不会触发编译错误\n";

    // 编译期递归求和
    auto result = sum_all(1, 2, 3, 4, 5);
    std::cout << "\nsum_all(1,2,3,4,5) = " << result << "\n";

    std::cout << "\n";
}

// ============================================================
// 4. static_assert —— 编译期断言
// ============================================================

// static_assert 在编译期检查条件，失败则编译错误
// 语法: static_assert(常量表达式, 错误信息)

// 编译期检查常量
static_assert(ARRAY_SIZE == 10, "ARRAY_SIZE 必须为 10");
static_assert(factorial(5) == 120, "factorial(5) 应该等于 120");
static_assert(fibonacci(10) == 55, "fibonacci(10) 应该等于 55");
static_assert(gcd(12, 8) == 4, "gcd(12, 8) 应该等于 4");

// 编译期类型检查
template<typename T>
struct NumericWrapper {
    T value;

    // 构造时检查类型约束
    NumericWrapper(T v) : value(v) {
        // 注意：这是运行期断言，但结合 if constexpr 可以做编译期检查
    }
};

// 使用 static_assert 约束模板参数
template<typename T>
T safe_divide(T a, T b) {
    static_assert(std::is_arithmetic_v<T>, "safe_divide 只支持算术类型");
    // static_assert 会在模板实例化时检查
    if constexpr (std::is_integral_v<T>) {
        // 整数除法额外检查
        if (b == 0) {
            std::cout << "警告: 整数除以零!\n";
            return T{0};
        }
    }
    return a / b;
}

void demo_static_assert() {
    std::cout << "=== static_assert 编译期断言 ===\n";

    // 以下 static_assert 在编译期通过
    std::cout << "static_assert(ARRAY_SIZE == 10) —— 通过\n";
    std::cout << "static_assert(factorial(5) == 120) —— 通过\n";
    std::cout << "static_assert(fibonacci(10) == 55) —— 通过\n";

    // 使用带约束的函数
    auto r1 = safe_divide(10, 3);
    auto r2 = safe_divide(10.0, 3.0);
    std::cout << "safe_divide(10, 3) = " << r1 << "\n";
    std::cout << "safe_divide(10.0, 3.0) = " << r2 << "\n";

    // safe_divide("a", "b");  // 编译错误! static_assert 失败

    // static_assert 的两种形式
    std::cout << "\nstatic_assert 两种形式:\n";
    std::cout << "  1. static_assert(常量表达式) —— C++17 起可省略消息\n";
    std::cout << "  2. static_assert(常量表达式, \"错误消息\") —— 带自定义错误信息\n";

    // 编译期验证数组大小
    constexpr std::array<int, 5> test_arr = {1, 2, 3, 4, 5};
    static_assert(test_arr.size() == 5, "数组大小应为 5");
    static_assert(test_arr[0] == 1, "第一个元素应为 1");
    std::cout << "  static_assert(test_arr.size() == 5) —— 通过\n";
    std::cout << "  static_assert(test_arr[0] == 1) —— 通过\n";

    std::cout << "\n";
}

// ============================================================
// 5. 综合示例：编译期查找素数表
// ============================================================

// 编译期判断是否为素数
constexpr bool is_prime(int n) {
    if (n < 2) return false;
    for (int i = 2; i * i <= n; ++i) {
        if (n % i == 0) return false;
    }
    return true;
}

// 编译期生成前 N 个素数的数组
template<int N>
constexpr std::array<int, N> generate_primes() {
    std::array<int, N> primes{};
    int count = 0;
    int candidate = 2;
    while (count < N) {
        if (is_prime(candidate)) {
            primes[count++] = candidate;
        }
        ++candidate;
    }
    return primes;
}

void demo_compile_time_primes() {
    std::cout << "=== 综合示例：编译期素数表 ===\n";

    // 整个素数表在编译期生成
    constexpr auto primes = generate_primes<20>();

    std::cout << "前 20 个素数: ";
    for (auto p : primes) {
        std::cout << p << " ";
    }
    std::cout << "\n";

    // 编译期验证
    static_assert(primes[0] == 2, "第一个素数应为 2");
    static_assert(primes[1] == 3, "第二个素数应为 3");
    static_assert(primes[4] == 11, "第五个素数应为 11");
    std::cout << "static_assert 验证全部通过\n";

    std::cout << "\n";
}

// ============================================================
// main
// ============================================================

int main() {
    std::cout << "============================================\n";
    std::cout << "  编译期计算基础 (constexpr Basics)\n";
    std::cout << "============================================\n\n";

    demo_constexpr_variable();
    demo_constexpr_function();
    demo_if_constexpr();
    demo_static_assert();
    demo_compile_time_primes();

    std::cout << "============================================\n";
    std::cout << "  核心要点总结:\n";
    std::cout << "  1. constexpr 变量 = 编译期常量\n";
    std::cout << "  2. constexpr 函数 = 可编译期可运行期\n";
    std::cout << "  3. if constexpr = 编译期分支选择\n";
    std::cout << "  4. static_assert = 编译期断言检查\n";
    std::cout << "============================================\n";

    return 0;
}
