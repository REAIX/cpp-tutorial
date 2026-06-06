/** @file 01_example_arithmetic_overload.cpp
 *  @brief 重载 +, -, *, /, ==, <, <<, >> 为自定义类型
 *  @description 对应文档: 02-CPP/06-operator-overload
 */

#include <iostream>
#include <string>
#include <cmath>

// ===== 自定义类型: 二维向量 =====
class Vec2 {
public:
    Vec2(double x = 0.0, double y = 0.0) : x_(x), y_(y) {}

    // 算术运算符
    Vec2 operator+(const Vec2& rhs) const {
        return Vec2(x_ + rhs.x_, y_ + rhs.y_);
    }

    Vec2 operator-(const Vec2& rhs) const {
        return Vec2(x_ - rhs.x_, y_ - rhs.y_);
    }

    Vec2 operator*(double scalar) const {
        return Vec2(x_ * scalar, y_ * scalar);
    }

    Vec2 operator/(double scalar) const {
        return Vec2(x_ / scalar, y_ / scalar);
    }

    // 复合赋值运算符
    Vec2& operator+=(const Vec2& rhs) {
        x_ += rhs.x_;
        y_ += rhs.y_;
        return *this;
    }

    Vec2& operator-=(const Vec2& rhs) {
        x_ -= rhs.x_;
        y_ -= rhs.y_;
        return *this;
    }

    // 比较运算符
    bool operator==(const Vec2& rhs) const {
        return x_ == rhs.x_ && y_ == rhs.y_;
    }

    bool operator!=(const Vec2& rhs) const {
        return !(*this == rhs);
    }

    bool operator<(const Vec2& rhs) const {
        return length() < rhs.length();
    }

    bool operator<=(const Vec2& rhs) const {
        return length() <= rhs.length();
    }

    bool operator>(const Vec2& rhs) const {
        return rhs < *this;
    }

    bool operator>=(const Vec2& rhs) const {
        return rhs <= *this;
    }

    // 输出运算符 (友元函数)
    friend std::ostream& operator<<(std::ostream& os, const Vec2& v) {
        os << "(" << v.x_ << ", " << v.y_ << ")";
        return os;
    }

    // 输入运算符 (友元函数)
    friend std::istream& operator>>(std::istream& is, Vec2& v) {
        is >> v.x_ >> v.y_;
        return is;
    }

    double x() const { return x_; }
    double y() const { return y_; }
    double length() const { return std::sqrt(x_ * x_ + y_ * y_); }

private:
    double x_;
    double y_;
};

// 标量 * 向量 (成员函数只能向量*标量, 反向需要非成员函数)
Vec2 operator*(double scalar, const Vec2& v) {
    return v * scalar;
}

void demo_arithmetic_operators() {
    std::cout << "===== 算术运算符重载 =====" << std::endl;

    Vec2 a(3.0, 4.0);
    Vec2 b(1.0, 2.0);

    std::cout << "a = " << a << ", b = " << b << std::endl;
    std::cout << "a + b = " << (a + b) << std::endl;
    std::cout << "a - b = " << (a - b) << std::endl;
    std::cout << "a * 2 = " << (a * 2) << std::endl;
    std::cout << "2 * a = " << (2 * a) << std::endl;
    std::cout << "a / 2 = " << (a / 2) << std::endl;

    Vec2 c = a;
    c += b;
    std::cout << "a += b -> " << c << std::endl;
}

void demo_comparison_operators() {
    std::cout << "\n===== 比较运算符重载 =====" << std::endl;

    Vec2 a(3.0, 4.0);  // 长度 5
    Vec2 b(1.0, 2.0);  // 长度 sqrt(5) ≈ 2.24

    std::cout << "a = " << a << " (长度 " << a.length() << ")" << std::endl;
    std::cout << "b = " << b << " (长度 " << b.length() << ")" << std::endl;
    std::cout << "a == b: " << std::boolalpha << (a == b) << std::endl;
    std::cout << "a != b: " << (a != b) << std::endl;
    std::cout << "a < b: " << (a < b) << std::endl;
    std::cout << "a > b: " << (a > b) << std::endl;
}

void demo_io_operators() {
    std::cout << "\n===== 输入输出运算符重载 =====" << std::endl;

    Vec2 v(1.5, 2.5);
    std::cout << "输出: " << v << std::endl;

    // 输入运算符示例 (从字符串流模拟)
    std::cout << "输入运算符: is >> v.x_ >> v.y_" << std::endl;

    std::cout << "\n运算符重载要点:" << std::endl;
    std::cout << "  - 算术运算符: 返回新对象, 不修改操作数" << std::endl;
    std::cout << "  - 复合赋值: 返回 *this 的引用" << std::endl;
    std::cout << "  - 比较运算符: 返回 bool" << std::endl;
    std::cout << "  - << >> : 必须为友元 (左操作数是流)" << std::endl;
    std::cout << "  - 对称运算符优先用非成员函数" << std::endl;
}

int main() {
    std::cout << "========== 算术与比较运算符重载 ==========\n" << std::endl;

    demo_arithmetic_operators();
    demo_comparison_operators();
    demo_io_operators();

    return 0;
}
