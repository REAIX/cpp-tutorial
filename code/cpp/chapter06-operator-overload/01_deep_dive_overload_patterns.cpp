/** @file 01_deep_dive_overload_patterns.cpp
 *  @brief 安全bool惯用法、太空船运算符(<=> C++20)、表达式模板概念、代理对象
 *  @description 对应文档: 02-CPP/06-operator-overload
 *  编译命令: g++ -std=c++20 01_deep_dive_overload_patterns.cpp -o 01_deep_dive_overload_patterns
 */

#include <iostream>
#include <string>
#include <vector>
#include <compare>

// ===== 1. 安全 bool 惯用法 (Safe Bool Idiom) =====

// C++98 的问题: operator bool() 导致意外行为
class UnsafeBool {
public:
    UnsafeBool(bool v) : value_(v) {}

    // 危险: 隐式转为 bool 后可以参与算术运算
    operator bool() const { return value_; }

private:
    bool value_;
};

void demo_unsafe_bool() {
    std::cout << "===== 安全 bool 惯用法 =====" << std::endl;

    UnsafeBool ub(true);
    // int n = ub + 1;  // 危险! bool -> int -> 2
    // std::cout << n << std::endl;  // 编译通过, 但不是预期行为

    std::cout << "C++98 的安全 bool 惯用法: 返回成员指针" << std::endl;
    std::cout << "  operator void*() const; // 旧方式" << std::endl;
    std::cout << "  问题: 可以 delete ptr;" << std::endl;

    std::cout << "\nC++11 的解决方案: explicit operator bool()" << std::endl;
    std::cout << "  - 可以在 if/while 条件中使用" << std::endl;
    std::cout << "  - 不能参与算术运算" << std::endl;
    std::cout << "  - 标准库: unique_ptr, shared_ptr, basic_ios 都用此方式" << std::endl;
}

// ===== 2. 太空船运算符 <=> (C++20) =====
class Point {
public:
    Point(double x, double y) : x_(x), y_(y) {}

    // C++20: 太空船运算符 (three-way comparison)
    // 自动生成 <, <=, >, >=, ==, !=
    auto operator<=>(const Point&) const = default;

    double x() const { return x_; }
    double y() const { return y_; }

private:
    double x_;
    double y_;
};

class Version {
public:
    Version(int major, int minor, int patch)
        : major_(major), minor_(minor), patch_(patch) {}

    // 自定义 <=> 实现
    std::strong_ordering operator<=>(const Version& other) const {
        if (auto cmp = major_ <=> other.major_; cmp != 0) return cmp;
        if (auto cmp = minor_ <=> other.minor_; cmp != 0) return cmp;
        return patch_ <=> other.patch_;
    }

    // 手写 <=> 时, == 需要单独定义
    // (注意: =default 的 <=> 会自动生成 ==, 无需手写)
    bool operator==(const Version& other) const {
        return major_ == other.major_ && minor_ == other.minor_ && patch_ == other.patch_;
    }

    std::string str() const {
        return std::to_string(major_) + "." + std::to_string(minor_) + "." + std::to_string(patch_);
    }

private:
    int major_;
    int minor_;
    int patch_;
};

void demo_spaceship_operator() {
    std::cout << "\n===== 太空船运算符 <=> (C++20) =====" << std::endl;

    Point p1(1.0, 2.0);
    Point p2(1.0, 2.0);
    Point p3(3.0, 4.0);

    std::cout << std::boolalpha;
    std::cout << "  p1 == p2: " << (p1 == p2) << std::endl;
    std::cout << "  p1 < p3: " << (p1 < p3) << std::endl;
    std::cout << "  p3 > p1: " << (p3 > p1) << std::endl;

    Version v1(2, 0, 1);
    Version v2(2, 1, 0);
    Version v3(2, 0, 1);

    std::cout << "\n  " << v1.str() << " < " << v2.str() << ": " << (v1 < v2) << std::endl;
    std::cout << "  " << v1.str() << " == " << v3.str() << ": " << (v1 == v3) << std::endl;

    std::cout << "\n<=> 的三种比较类别:" << std::endl;
    std::cout << "  strong_ordering: 强排序 (可替换性, 如 int)" << std::endl;
    std::cout << "  weak_ordering:   弱排序 (等价但不相同, 如大小写不敏感字符串)" << std::endl;
    std::cout << "  partial_ordering: 偏序 (可能有不可比较, 如 float 的 NaN)" << std::endl;

    std::cout << "\n<=> 的优势:" << std::endl;
    std::cout << "  - 一个运算符生成 6 个比较运算符" << std::endl;
    std::cout << "  - = default 让编译器自动生成" << std::endl;
    std::cout << "  - 性能: 一次比较确定全部关系" << std::endl;
}

// ===== 3. 表达式模板概念 =====
// 表达式模板: 延迟计算, 避免临时对象
// 这里展示简化版概念

class Vec {
public:
    Vec(std::initializer_list<double> vals) : data_(vals) {}

    double operator[](size_t i) const { return data_[i]; }
    double& operator[](size_t i) { return data_[i]; }
    size_t size() const { return data_.size(); }

    // 传统方式: 返回新 Vec (产生临时对象)
    Vec operator+(const Vec& rhs) const {
        Vec result(data_.size(), 0);
        for (size_t i = 0; i < data_.size(); ++i) {
            result[i] = data_[i] + rhs[i];
        }
        return result;
    }

private:
    Vec(size_t n, double) : data_(n, 0) {}
    std::vector<double> data_;
};

void demo_expression_templates_concept() {
    std::cout << "\n===== 表达式模板概念 =====" << std::endl;

    Vec a = {1.0, 2.0, 3.0};
    Vec b = {4.0, 5.0, 6.0};
    Vec c = {7.0, 8.0, 9.0};

    // 传统方式: a + b + c
    // 1. 计算 a + b -> 临时对象 tmp1
    // 2. 计算 tmp1 + c -> 临时对象 tmp2
    // 3. 两次循环, 两次临时对象

    Vec result = a + b + c;
    std::cout << "  result[0] = " << result[0] << std::endl;

    std::cout << "\n表达式模板的优化思路:" << std::endl;
    std::cout << "  - 不立即计算, 而是构建表达式树" << std::endl;
    std::cout << "  - 赋值时一次性遍历计算" << std::endl;
    std::cout << "  - 避免中间临时对象" << std::endl;
    std::cout << "  - 实际库: Eigen, Blaze, Boost.Proto" << std::endl;

    std::cout << "\n简化原理:" << std::endl;
    std::cout << "  a + b 返回 AddExpr<Vec, Vec>" << std::endl;
    std::cout << "  AddExpr<Vec, Vec> + c 返回 AddExpr<AddExpr<Vec,Vec>, Vec>" << std::endl;
    std::cout << "  赋值时: result[i] = expr[i] 一次计算" << std::endl;
}

// ===== 4. 代理对象 (Proxy Objects) =====

// 代理对象: 代表另一个对象的中间对象
// 典型应用: std::vector<bool> 的 bit reference

class BitArray {
public:
    BitArray(size_t size) : size_(size), data_((size + 7) / 8, 0) {}

    // 代理类: 代表一个 bit
    class BitRef {
    public:
        BitRef(unsigned char& byte, int bit_pos)
            : byte_(byte), mask_(1 << bit_pos) {}

        // 读取
        operator bool() const {
            return (byte_ & mask_) != 0;
        }

        // 写入
        BitRef& operator=(bool value) {
            if (value) {
                byte_ |= mask_;
            } else {
                byte_ &= ~mask_;
            }
            return *this;
        }

        // 赋值来自另一个 BitRef
        BitRef& operator=(const BitRef& other) {
            *this = static_cast<bool>(other);
            return *this;
        }

    private:
        unsigned char& byte_;
        unsigned char mask_;
    };

    // 返回代理对象
    BitRef operator[](size_t index) {
        return BitRef(data_[index / 8], index % 8);
    }

    bool operator[](size_t index) const {
        return (data_[index / 8] & (1 << (index % 8))) != 0;
    }

    size_t size() const { return size_; }

private:
    size_t size_;
    std::vector<unsigned char> data_;
};

void demo_proxy_objects() {
    std::cout << "\n===== 代理对象 =====" << std::endl;

    BitArray bits(16);

    // 通过代理对象读写
    bits[0] = true;
    bits[3] = true;
    bits[7] = true;

    std::cout << "  bits[0] = " << bits[0] << std::endl;
    std::cout << "  bits[1] = " << bits[1] << std::endl;
    std::cout << "  bits[3] = " << bits[3] << std::endl;
    std::cout << "  bits[7] = " << bits[7] << std::endl;

    // 代理对象的陷阱
    auto ref = bits[0];  // ref 是 BitRef 代理对象, 不是 bool!
    // ref 在 bits 销毁后变成悬垂引用

    std::cout << "\n代理对象的陷阱:" << std::endl;
    std::cout << "  1. auto 可能推导为代理类型而非值类型" << std::endl;
    std::cout << "     解决: auto x = static_cast<bool>(bits[0]);" << std::endl;
    std::cout << "  2. 代理对象可能比预期活得更久" << std::endl;
    std::cout << "  3. 取地址 & 可能得到代理的地址, 而非真实对象" << std::endl;
    std::cout << "  4. vector<bool> 是最常见的代理对象陷阱" << std::endl;

    std::cout << "\n代理对象的应用:" << std::endl;
    std::cout << "  - vector<bool>: 位引用" << std::endl;
    std::cout << "  - 智能指针: 代理原始指针" << std::endl;
    std::cout << "  - 延迟计算: 代理表达式" << std::endl;
    std::cout << "  - 属性: 代理属性的读写" << std::endl;
}

int main() {
    std::cout << "========== 运算符重载高级模式 ==========\n" << std::endl;

    demo_unsafe_bool();
    demo_spaceship_operator();
    demo_expression_templates_concept();
    demo_proxy_objects();

    return 0;
}
