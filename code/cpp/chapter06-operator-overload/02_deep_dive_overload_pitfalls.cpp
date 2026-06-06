/** @file 02_deep_dive_overload_pitfalls.cpp
 *  @brief 重载解析规则、歧义重载、SFINAE基础、ADL与运算符
 *  @description 对应文档: 02-CPP/06-operator-overload
 */

#include <iostream>
#include <string>
#include <vector>
#include <type_traits>

// ===== 1. 重载解析规则 =====
void func(int x) {
    std::cout << "  func(int): " << x << std::endl;
}

void func(double x) {
    std::cout << "  func(double): " << x << std::endl;
}

void func(const std::string& s) {
    std::cout << "  func(string): " << s << std::endl;
}

void func(int a, int b) {
    std::cout << "  func(int, int): " << a << ", " << b << std::endl;
}

void demo_overload_resolution() {
    std::cout << "===== 重载解析规则 =====" << std::endl;

    func(42);         // 精确匹配: func(int)
    func(3.14);       // 精确匹配: func(double)
    func(3.14f);      // 浮点提升: func(double)
    func('a');         // 整数提升: func(int)
    func("hello");     // 精确匹配: func(string) (const char* -> string)
    func(1, 2);       // 精确匹配: func(int, int)

    std::cout << "\n重载解析步骤:" << std::endl;
    std::cout << "  1. 名称查找: 找到所有候选函数" << std::endl;
    std::cout << "  2. 模板推导: 对模板函数推导参数" << std::endl;
    std::cout << "  3. 重载决议: 选择最佳匹配" << std::endl;

    std::cout << "\n匹配优先级 (从优到劣):" << std::endl;
    std::cout << "  1. 精确匹配 (无转换或平凡转换)" << std::endl;
    std::cout << "  2. 提升 (char->int, float->double)" << std::endl;
    std::cout << "  3. 标准转换 (int->double, double->int)" << std::endl;
    std::cout << "  4. 用户定义转换 (构造函数/operator)" << std::endl;
    std::cout << "  5. 省略号 (...)" << std::endl;
}

// ===== 2. 歧义重载 =====
class Number {
public:
    Number(int v) : value_(v) {
        std::cout << "  Number(int " << v << ")" << std::endl;
    }

    Number(double v) : value_(static_cast<int>(v)) {
        std::cout << "  Number(double " << v << ")" << std::endl;
    }

    int value() const { return value_; }

private:
    int value_;
};

void process(Number n) {
    std::cout << "  process(Number): " << n.value() << std::endl;
}

// 歧义示例
void ambiguous_func(long x) {
    std::cout << "  ambiguous_func(long): " << x << std::endl;
}

void ambiguous_func(float x) {
    std::cout << "  ambiguous_func(float): " << x << std::endl;
}

void demo_ambiguous_overloads() {
    std::cout << "\n===== 歧义重载 =====" << std::endl;

    // 歧义1: 两个用户定义转换
    process(42);     // OK: 精确匹配 Number(int)
    process(3.14);   // OK: 精确匹配 Number(double)
    // process(42L);  // 可能歧义: long -> int? long -> double?

    // 歧义2: 标准转换等级相同
    // ambiguous_func(42);  // 歧义! int -> long 和 int -> float 都是标准转换
    ambiguous_func(42L);   // OK: long 精确匹配
    ambiguous_func(42.0f); // OK: float 精确匹配

    std::cout << "\n避免歧义的方法:" << std::endl;
    std::cout << "  1. 避免多个相同等级的隐式转换路径" << std::endl;
    std::cout << "  2. 使用 explicit 构造函数" << std::endl;
    std::cout << "  3. 提供精确匹配的重载" << std::endl;
    std::cout << "  4. 使用函数模板 + 概念约束" << std::endl;
}

// ===== 3. SFINAE 基础 =====
// SFINAE: Substitution Failure Is Not An Error
// 模板参数替换失败不是错误, 只是该候选被排除

// C++11/14 风格: std::enable_if
template<typename T>
typename std::enable_if<std::is_integral<T>::value, void>::type
process_type(T value) {
    std::cout << "  整数类型: " << value << std::endl;
}

template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, void>::type
process_type(T value) {
    std::cout << "  浮点类型: " << value << std::endl;
}

// C++17 风格: if constexpr
template<typename T>
void process_type_modern(T value) {
    if constexpr (std::is_integral_v<T>) {
        std::cout << "  [modern] 整数类型: " << value << std::endl;
    } else if constexpr (std::is_floating_point_v<T>) {
        std::cout << "  [modern] 浮点类型: " << value << std::endl;
    } else {
        std::cout << "  [modern] 其他类型" << std::endl;
    }
}

// C++20 风格: Concepts (概念)
// template<std::integral T>  // 需要 C++20
// void process_type_concept(T value) { ... }

void demo_sfinae() {
    std::cout << "\n===== SFINAE 基础 =====" << std::endl;

    process_type(42);       // 整数版本
    process_type(3.14);     // 浮点版本
    // process_type("hello");  // 编译错误: 无匹配函数

    process_type_modern(42);
    process_type_modern(3.14);
    process_type_modern("hello");

    std::cout << "\nSFINAE 原理:" << std::endl;
    std::cout << "  - 模板参数替换失败不是错误" << std::endl;
    std::cout << "  - 失败的候选被排除, 不导致编译错误" << std::endl;
    std::cout << "  - 用于条件性地启用/禁用模板" << std::endl;

    std::cout << "\nSFINAE 的演进:" << std::endl;
    std::cout << "  C++11: std::enable_if" << std::endl;
    std::cout << "  C++14: std::enable_if_t 简写" << std::endl;
    std::cout << "  C++17: if constexpr (编译期分支)" << std::endl;
    std::cout << "  C++20: Concepts (最清晰的方式)" << std::endl;
}

// ===== 4. ADL 与运算符 =====
namespace math {
    struct Complex {
        double real;
        double imag;
    };

    // ADL: 在 math 命名空间中定义运算符
    Complex operator+(const Complex& a, const Complex& b) {
        return {a.real + b.real, a.imag + b.imag};
    }

    std::ostream& operator<<(std::ostream& os, const Complex& c) {
        os << c.real << "+" << c.imag << "i";
        return os;
    }
}

void demo_adl_and_operators() {
    std::cout << "\n===== ADL 与运算符 =====" << std::endl;

    math::Complex a{1.0, 2.0};
    math::Complex b{3.0, 4.0};

    // ADL 自动在 math 命名空间中查找 operator+
    auto c = a + b;  // 不需要 math::operator+
    std::cout << "  a + b = " << c << std::endl;

    // ADL 自动在 math 命名空间中查找 operator<<
    std::cout << "  直接输出: " << a << std::endl;

    std::cout << "\nADL 与运算符重载:" << std::endl;
    std::cout << "  - 运算符重载应放在参数的命名空间中" << std::endl;
    std::cout << "  - ADL 使得运算符可以自然使用, 无需前缀" << std::endl;
    std::cout << "  - 这是 std::ostream 的 operator<< 的工作原理" << std::endl;

    std::cout << "\nADL 的注意事项:" << std::endl;
    std::cout << "  1. using namespace 可能导致意外的 ADL 查找" << std::endl;
    std::cout << "  2. 不同命名空间的同名函数可能冲突" << std::endl;
    std::cout << "  3. 模板中的 ADL 可能找到非预期的函数" << std::endl;
    std::cout << "  4. 在命名空间中提供 find, swap 等常见名称时要小心" << std::endl;
}

// ===== 5. 举一反三: 运算符重载最佳实践 =====
void demo_overload_best_practices() {
    std::cout << "\n===== 举一反三: 运算符重载最佳实践 =====" << std::endl;

    std::cout << "应该重载的运算符:" << std::endl;
    std::cout << "  - == != < > <= >= : 自定义类型的比较" << std::endl;
    std::cout << "  - << >> : 自定义类型的 I/O" << std::endl;
    std::cout << "  - [] : 容器类型的下标访问" << std::endl;
    std::cout << "  - () : 仿函数/可调用对象" << std::endl;
    std::cout << "  - -> * : 智能指针" << std::endl;
    std::cout << "  - <=> : C++20 三路比较" << std::endl;

    std::cout << "\n不应重载的运算符:" << std::endl;
    std::cout << "  - , (逗号): 语义不直观" << std::endl;
    std::cout << "  - & (取地址): 破坏语言基本操作" << std::endl;
    std::cout << "  - && || : 破坏短路求值" << std::endl;
    std::cout << "  - ->* : 语义不直观" << std::endl;

    std::cout << "\n重载原则:" << std::endl;
    std::cout << "  1. 保持语义一致性: + 应该是加法, 不是减法" << std::endl;
    std::cout << "  2. 保持交换律: a+b == b+a (用非成员函数)" << std::endl;
    std::cout << "  3. 保持关联律: (a+b)+c == a+(b+c)" << std::endl;
    std::cout << "  4. 对称运算符用非成员函数" << std::endl;
    std::cout << "  5. 赋值类运算符用成员函数" << std::endl;
    std::cout << "  6. 不要过度重载: 只在语义自然时使用" << std::endl;
}

int main() {
    std::cout << "========== 运算符重载陷阱与规则 ==========\n" << std::endl;

    demo_overload_resolution();
    demo_ambiguous_overloads();
    demo_sfinae();
    demo_adl_and_operators();
    demo_overload_best_practices();

    return 0;
}
