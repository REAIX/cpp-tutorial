/** @file 03_example_compile_time_recursion.cpp
 *  @brief 编译期递归：编译期阶乘、斐波那契、幂运算、编译期计算优化
 *  @description 对应文档: 07-模板元编程与编译期计算 / 模板元编程模式
 */

#include <iostream>
#include <array>
#include <type_traits>
#include <cstdint>
#include <cstdio>

// ============================================================
// 1. 经典模板递归：阶乘
// ============================================================

// 方式1：传统模板特化递归（C++98 风格）
template<int N>
struct Factorial {
    static constexpr int value = N * Factorial<N - 1>::value;
};

template<>
struct Factorial<0> {
    static constexpr int value = 1;
};

// 方式2：constexpr 函数（C++14 风格，推荐）
constexpr int factorial(int n) {
    int result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

// 方式3：consteval 函数（C++20 风格，强制编译期）
consteval int factorial_ce(int n) {
    int result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

void demo_factorial() {
    std::cout << "=== 编译期阶乘 ===\n";

    // 模板特化方式
    std::cout << "模板特化:\n";
    std::cout << "  Factorial<5>::value = " << Factorial<5>::value << "\n";
    std::cout << "  Factorial<10>::value = " << Factorial<10>::value << "\n";

    // constexpr 函数方式
    std::cout << "\nconstexpr 函数:\n";
    constexpr int f5 = factorial(5);
    constexpr int f10 = factorial(10);
    std::cout << "  factorial(5) = " << f5 << "\n";
    std::cout << "  factorial(10) = " << f10 << "\n";

    // consteval 函数方式
    std::cout << "\nconsteval 函数:\n";
    constexpr int f5_ce = factorial_ce(5);
    std::cout << "  factorial_ce(5) = " << f5_ce << "\n";

    // 编译期验证
    static_assert(Factorial<5>::value == 120);
    static_assert(factorial(5) == 120);
    static_assert(factorial_ce(5) == 120);

    std::cout << "\n三种方式对比:\n";
    std::cout << "  模板特化: C++98, 语法复杂, 递归实例化\n";
    std::cout << "  constexpr: C++14, 语法简洁, 可编译期可运行期\n";
    std::cout << "  consteval: C++20, 语法简洁, 强制编译期\n";

    std::cout << "\n";
}

// ============================================================
// 2. 编译期斐波那契数列
// ============================================================

// 模板递归方式
template<int N>
struct Fibonacci {
    static constexpr int value = Fibonacci<N - 1>::value + Fibonacci<N - 2>::value;
};

template<>
struct Fibonacci<0> {
    static constexpr int value = 0;
};

template<>
struct Fibonacci<1> {
    static constexpr int value = 1;
};

// constexpr 函数方式（更高效，线性复杂度）
constexpr int fibonacci(int n) {
    if (n <= 1) return n;
    int a = 0, b = 1;
    for (int i = 2; i <= n; ++i) {
        int tmp = a + b;
        a = b;
        b = tmp;
    }
    return b;
}

// 编译期生成斐波那契数列
template<int N>
constexpr std::array<int, N> make_fibonacci_array() {
    std::array<int, N> arr{};
    if constexpr (N > 0) arr[0] = 0;
    if constexpr (N > 1) arr[1] = 1;
    for (int i = 2; i < N; ++i) {
        arr[i] = arr[i - 1] + arr[i - 2];
    }
    return arr;
}

void demo_fibonacci() {
    std::cout << "=== 编译期斐波那契 ===\n";

    // 模板递归方式
    std::cout << "模板递归:\n";
    std::cout << "  Fibonacci<10>::value = " << Fibonacci<10>::value << "\n";
    std::cout << "  Fibonacci<20>::value = " << Fibonacci<20>::value << "\n";

    // constexpr 函数方式
    std::cout << "\nconstexpr 函数:\n";
    std::cout << "  fibonacci(10) = " << fibonacci(10) << "\n";
    std::cout << "  fibonacci(20) = " << fibonacci(20) << "\n";

    // 编译期生成整个数列
    constexpr auto fib_arr = make_fibonacci_array<15>();
    std::cout << "\n编译期斐波那契数列(前15项): ";
    for (auto v : fib_arr) std::cout << v << " ";
    std::cout << "\n";

    // 编译期验证
    static_assert(Fibonacci<10>::value == 55);
    static_assert(fibonacci(10) == 55);
    static_assert(fib_arr[10] == 55);

    std::cout << "\n";
}

// ============================================================
// 3. 编译期幂运算
// ============================================================

// 模板递归方式：快速幂
template<int Base, int Exp>
struct Power {
    static constexpr int value =
        (Exp % 2 == 0)
            ? Power<Base * Base, Exp / 2>::value
            : Base * Power<Base, Exp - 1>::value;
};

template<int Base>
struct Power<Base, 0> {
    static constexpr int value = 1;
};

template<int Base>
struct Power<Base, 1> {
    static constexpr int value = Base;
};

// constexpr 快速幂函数
constexpr long long fast_power(long long base, int exp) {
    long long result = 1;
    bool negative = exp < 0;
    if (negative) exp = -exp;
    while (exp > 0) {
        if (exp % 2 == 1) {
            result *= base;
        }
        base *= base;
        exp /= 2;
    }
    return negative ? 0 : result;  // 简化：忽略负指数
}

void demo_power() {
    std::cout << "=== 编译期幂运算 ===\n";

    // 模板递归方式
    std::cout << "模板递归(快速幂):\n";
    std::cout << "  Power<2, 10>::value = " << Power<2, 10>::value << "\n";
    std::cout << "  Power<3, 5>::value = " << Power<3, 5>::value << "\n";
    std::cout << "  Power<2, 20>::value = " << Power<2, 20>::value << "\n";

    // constexpr 函数方式
    std::cout << "\nconstexpr 快速幂:\n";
    constexpr auto p1 = fast_power(2, 10);
    constexpr auto p2 = fast_power(3, 5);
    constexpr auto p3 = fast_power(2, 20);
    std::cout << "  fast_power(2, 10) = " << p1 << "\n";
    std::cout << "  fast_power(3, 5) = " << p2 << "\n";
    std::cout << "  fast_power(2, 20) = " << p3 << "\n";

    // 编译期验证
    static_assert(Power<2, 10>::value == 1024);
    static_assert(fast_power(2, 10) == 1024);

    std::cout << "\n快速幂原理:\n";
    std::cout << "  x^n = (x^2)^(n/2)     当 n 为偶数\n";
    std::cout << "  x^n = x * x^(n-1)     当 n 为奇数\n";
    std::cout << "  时间复杂度: O(log n) 而非 O(n)\n";

    std::cout << "\n";
}

// ============================================================
// 4. 编译期数学工具
// ============================================================

// 编译期绝对值
template<typename T>
constexpr T ct_abs(T x) {
    return x < 0 ? -x : x;
}

// 编译期最大值/最小值
template<typename T>
constexpr T ct_max(T a, T b) {
    return a > b ? a : b;
}

template<typename T>
constexpr T ct_min(T a, T b) {
    return a < b ? a : b;
}

// 编译期整数平方根（牛顿法）
constexpr int ct_isqrt(int n) {
    if (n < 0) return -1;
    if (n < 2) return n;
    int x = n / 2;
    while (true) {
        int next = (x + n / x) / 2;
        if (next >= x) return x;
        x = next;
    }
}

// 编译期判断素数
constexpr bool is_prime(int n) {
    if (n < 2) return false;
    if (n < 4) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

// 编译期最大公约数
constexpr int ct_gcd(int a, int b) {
    while (b != 0) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

// 编译期最小公倍数
constexpr int ct_lcm(int a, int b) {
    return a / ct_gcd(a, b) * b;
}

void demo_math_utils() {
    std::cout << "=== 编译期数学工具 ===\n";

    constexpr auto abs_val = ct_abs(-42);
    constexpr auto max_val = ct_max(10, 20);
    constexpr auto min_val = ct_min(10, 20);
    constexpr auto sqrt_val = ct_isqrt(144);
    constexpr auto prime_check = is_prime(97);
    constexpr auto gcd_val = ct_gcd(48, 36);
    constexpr auto lcm_val = ct_lcm(12, 18);

    std::cout << "ct_abs(-42) = " << abs_val << "\n";
    std::cout << "ct_max(10, 20) = " << max_val << "\n";
    std::cout << "ct_min(10, 20) = " << min_val << "\n";
    std::cout << "ct_isqrt(144) = " << sqrt_val << "\n";
    std::cout << "is_prime(97) = " << prime_check << "\n";
    std::cout << "ct_gcd(48, 36) = " << gcd_val << "\n";
    std::cout << "ct_lcm(12, 18) = " << lcm_val << "\n";

    // 编译期验证
    static_assert(abs_val == 42);
    static_assert(sqrt_val == 12);
    static_assert(prime_check == true);
    static_assert(gcd_val == 12);
    static_assert(lcm_val == 36);

    std::cout << "\n";
}

// ============================================================
// 5. 编译期查表：生成查找表
// ============================================================

// 编译期生成正弦查找表（定点数表示）
constexpr double PI = 3.141592653589793;

constexpr double ct_sin(double x) {
    // 泰勒级数: sin(x) = x - x^3/3! + x^5/5! - ...
    double result = 0.0;
    double term = x;
    for (int i = 1; i < 20; ++i) {
        result += term;
        term *= -x * x / ((2 * i) * (2 * i + 1));
    }
    return result;
}

template<int N>
constexpr std::array<double, N> make_sin_table() {
    std::array<double, N> table{};
    for (int i = 0; i < N; ++i) {
        table[i] = ct_sin(i * PI / N);
    }
    return table;
}

// 编译期生成 CRC32 查找表
constexpr std::uint32_t crc32_poly = 0xEDB88320;

// 运行期风格的 CRC 条目计算（可在 constexpr 函数中使用）
constexpr std::uint32_t compute_crc_entry(std::uint32_t n) {
    std::uint32_t crc = n;
    for (int i = 0; i < 8; ++i) {
        crc = (crc & 1) ? (crc >> 1) ^ crc32_poly : crc >> 1;
    }
    return crc;
}

constexpr std::array<std::uint32_t, 256> make_crc32_table() {
    std::array<std::uint32_t, 256> table{};
    for (int i = 0; i < 256; ++i) {
        table[i] = compute_crc_entry(static_cast<std::uint32_t>(i));
    }
    return table;
}

void demo_lookup_tables() {
    std::cout << "=== 编译期查表 ===\n";

    // 正弦查找表
    constexpr auto sin_table = make_sin_table<16>();
    std::cout << "正弦查找表(16点):\n";
    for (int i = 0; i < 16; ++i) {
        printf("  sin(%2d*PI/16) = %7.4f\n", i, sin_table[i]);
    }

    // CRC32 查找表（只打印前8项）
    constexpr auto crc_table = make_crc32_table();
    std::cout << "\nCRC32 查找表(前8项):\n";
    for (int i = 0; i < 8; ++i) {
        printf("  crc_table[%d] = 0x%08X\n", i, crc_table[i]);
    }

    std::cout << "\n编译期查表的优势:\n";
    std::cout << "  1. 零运行期开销: 表在编译期生成\n";
    std::cout << "  2. 无初始化顺序问题\n";
    std::cout << "  3. 编译器可以优化查表操作\n";

    std::cout << "\n";
}

// ============================================================
// 6. 编译期字符串处理
// ============================================================

// 编译期字符串长度
template<std::size_t N>
constexpr std::size_t ct_strlen(const char (&str)[N]) {
    std::size_t len = 0;
    while (len < N - 1 && str[len] != '\0') ++len;
    return len;
}

// 编译期字符串相等比较
template<std::size_t M, std::size_t N>
constexpr bool ct_strequal(const char (&a)[M], const char (&b)[N]) {
    if (M != N) return false;
    for (std::size_t i = 0; i < M; ++i) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

// 编译期字符串哈希
constexpr std::uint32_t ct_hash(const char* str, std::size_t len) {
    std::uint32_t hash = 5381;
    for (std::size_t i = 0; i < len; ++i) {
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(str[i]);
    }
    return hash;
}

void demo_compile_time_string() {
    std::cout << "=== 编译期字符串处理 ===\n";

    constexpr auto len1 = ct_strlen("hello");
    constexpr auto len2 = ct_strlen("template metaprogramming");
    constexpr bool eq1 = ct_strequal("abc", "abc");
    constexpr bool eq2 = ct_strequal("abc", "abd");
    constexpr auto h1 = ct_hash("hello", 5);
    constexpr auto h2 = ct_hash("world", 5);

    std::cout << "ct_strlen(\"hello\") = " << len1 << "\n";
    std::cout << "ct_strlen(\"template metaprogramming\") = " << len2 << "\n";
    std::cout << "ct_strequal(\"abc\", \"abc\") = " << eq1 << "\n";
    std::cout << "ct_strequal(\"abc\", \"abd\") = " << eq2 << "\n";
    std::cout << "ct_hash(\"hello\") = " << h1 << "\n";
    std::cout << "ct_hash(\"world\") = " << h2 << "\n";

    static_assert(len1 == 5);
    static_assert(eq1 == true);
    static_assert(eq2 == false);

    std::cout << "\n";
}

// ============================================================
// main
// ============================================================

int main() {
    std::cout << "============================================\n";
    std::cout << "  编译期递归与计算\n";
    std::cout << "============================================\n\n";

    demo_factorial();
    demo_fibonacci();
    demo_power();
    demo_math_utils();
    demo_lookup_tables();
    demo_compile_time_string();

    std::cout << "============================================\n";
    std::cout << "  核心要点总结:\n";
    std::cout << "  1. 模板特化递归: C++98经典方式\n";
    std::cout << "  2. constexpr函数: C++14推荐方式\n";
    std::cout << "  3. consteval函数: C++20强制编译期\n";
    std::cout << "  4. 编译期查表: 零开销优化\n";
    std::cout << "  5. 快速幂: O(log n)编译期计算\n";
    std::cout << "============================================\n";

    return 0;
}
