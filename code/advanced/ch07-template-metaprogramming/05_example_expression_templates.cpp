/** @file 05_example_expression_templates.cpp
 *  @brief 表达式模板：向量数学运算的惰性求值，避免临时对象
 *  @description 对应文档: 07-模板元编程与编译期计算 / 表达式模板
 */

#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <algorithm>

// ============================================================
// 1. 表达式模板的问题背景
// ============================================================

// 问题：向量运算产生大量临时对象
// 例如: a + b + c 会产生两个临时向量
//   temp1 = a + b    (临时对象1)
//   result = temp1 + c (临时对象2)
// 每个临时对象都需要分配内存和拷贝数据

// 朴素向量实现（会产生临时对象）
class NaiveVector {
    std::vector<double> data_;

public:
    explicit NaiveVector(std::size_t n) : data_(n, 0.0) {}
    NaiveVector(std::initializer_list<double> init) : data_(init) {}

    std::size_t size() const { return data_.size(); }
    double operator[](std::size_t i) const { return data_[i]; }
    double& operator[](std::size_t i) { return data_[i]; }

    // 朴素实现：返回新向量（产生临时对象）
    NaiveVector operator+(const NaiveVector& other) const {
        NaiveVector result(size());
        for (std::size_t i = 0; i < size(); ++i) {
            result[i] = data_[i] + other[i];
        }
        return result;  // 临时对象!
    }

    NaiveVector operator*(double scalar) const {
        NaiveVector result(size());
        for (std::size_t i = 0; i < size(); ++i) {
            result[i] = data_[i] * scalar;
        }
        return result;  // 临时对象!
    }
};

void demo_naive_vector_problem() {
    std::cout << "=== 朴素向量的问题 ===\n";

    NaiveVector a{1.0, 2.0, 3.0};
    NaiveVector b{4.0, 5.0, 6.0};
    NaiveVector c{7.0, 8.0, 9.0};

    // a + b + c 产生两个临时向量
    NaiveVector result = a + b + c;
    std::cout << "a + b + c = [" << result[0] << ", " << result[1] << ", " << result[2] << "]\n";

    std::cout << "\n问题:\n";
    std::cout << "  a + b 产生临时向量 temp1\n";
    std::cout << "  temp1 + c 产生临时向量 result\n";
    std::cout << "  每次运算都遍历整个向量\n";
    std::cout << "  多次内存分配和数据拷贝\n";

    std::cout << "\n";
}

// ============================================================
// 2. 表达式模板核心设计
// ============================================================

// 表达式模板的核心思想：
// 不立即计算结果，而是构建一个"表达式树"
// 在最终赋值时才遍历表达式树进行计算
// 这样 a + b + c 只需一次遍历，无需临时对象

// 前向声明
template<typename T>
class Vec;

// 表达式基类：所有表达式都继承此类
template<typename Derived>
class VecExpr {
public:
    // 通过 CRTP 访问派生类的元素
    double operator[](std::size_t i) const {
        return static_cast<const Derived&>(*this)[i];
    }

    std::size_t size() const {
        return static_cast<const Derived&>(*this).size();
    }

    // 隐式转换为 Vec（触发实际计算）
    operator Vec<double>() const;
};

// 向量类：存储实际数据
template<typename T>
class Vec : public VecExpr<Vec<T>> {
    std::vector<T> data_;

public:
    Vec() = default;
    explicit Vec(std::size_t n) : data_(n, 0.0) {}
    Vec(std::initializer_list<T> init) : data_(init) {}

    // 从表达式构造（触发惰性求值）
    template<typename Expr>
    Vec(const VecExpr<Expr>& expr) : data_(expr.size()) {
        for (std::size_t i = 0; i < expr.size(); ++i) {
            data_[i] = expr[i];
        }
    }

    // 从表达式赋值（触发惰性求值）
    template<typename Expr>
    Vec& operator=(const VecExpr<Expr>& expr) {
        data_.resize(expr.size());
        for (std::size_t i = 0; i < expr.size(); ++i) {
            data_[i] = expr[i];
        }
        return *this;
    }

    std::size_t size() const { return data_.size(); }
    T operator[](std::size_t i) const { return data_[i]; }
    T& operator[](std::size_t i) { return data_[i]; }

    // 输出
    void print(const std::string& name = "") const {
        if (!name.empty()) std::cout << name << " = ";
        std::cout << "[";
        for (std::size_t i = 0; i < data_.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << data_[i];
        }
        std::cout << "]\n";
    }
};

// 实现隐式转换
template<typename Derived>
VecExpr<Derived>::operator Vec<double>() const {
    return Vec<double>(*this);
}

// ============================================================
// 3. 二元运算表达式
// ============================================================

// 加法表达式
template<typename L, typename R>
class VecAdd : public VecExpr<VecAdd<L, R>> {
    const L& left_;
    const R& right_;

public:
    VecAdd(const L& left, const R& right) : left_(left), right_(right) {}

    double operator[](std::size_t i) const {
        return left_[i] + right_[i];
    }

    std::size_t size() const {
        return left_.size();
    }
};

// 减法表达式
template<typename L, typename R>
class VecSub : public VecExpr<VecSub<L, R>> {
    const L& left_;
    const R& right_;

public:
    VecSub(const L& left, const R& right) : left_(left), right_(right) {}

    double operator[](std::size_t i) const {
        return left_[i] - right_[i];
    }

    std::size_t size() const {
        return left_.size();
    }
};

// 乘法表达式（向量 * 标量）
template<typename L>
class VecScale : public VecExpr<VecScale<L>> {
    const L& vec_;
    double scalar_;

public:
    VecScale(const L& vec, double scalar) : vec_(vec), scalar_(scalar) {}

    double operator[](std::size_t i) const {
        return vec_[i] * scalar_;
    }

    std::size_t size() const {
        return vec_.size();
    }
};

// 逐元素乘法表达式
template<typename L, typename R>
class VecMul : public VecExpr<VecMul<L, R>> {
    const L& left_;
    const R& right_;

public:
    VecMul(const L& left, const R& right) : left_(left), right_(right) {}

    double operator[](std::size_t i) const {
        return left_[i] * right_[i];
    }

    std::size_t size() const {
        return left_.size();
    }
};

// ============================================================
// 4. 运算符重载
// ============================================================

// 向量 + 向量
template<typename L, typename R>
VecAdd<L, R> operator+(const VecExpr<L>& left, const VecExpr<R>& right) {
    return VecAdd<L, R>(static_cast<const L&>(left), static_cast<const R&>(right));
}

// 向量 - 向量
template<typename L, typename R>
VecSub<L, R> operator-(const VecExpr<L>& left, const VecExpr<R>& right) {
    return VecSub<L, R>(static_cast<const L&>(left), static_cast<const R&>(right));
}

// 向量 * 标量
template<typename L>
VecScale<L> operator*(const VecExpr<L>& vec, double scalar) {
    return VecScale<L>(static_cast<const L&>(vec), scalar);
}

// 标量 * 向量
template<typename L>
VecScale<L> operator*(double scalar, const VecExpr<L>& vec) {
    return VecScale<L>(static_cast<const L&>(vec), scalar);
}

// 逐元素乘法
template<typename L, typename R>
VecMul<L, R> operator*(const VecExpr<L>& left, const VecExpr<R>& right) {
    return VecMul<L, R>(static_cast<const L&>(left), static_cast<const R&>(right));
}

// ============================================================
// 5. 演示表达式模板
// ============================================================

void demo_expression_templates() {
    std::cout << "=== 表达式模板演示 ===\n";

    Vec<double> a{1.0, 2.0, 3.0, 4.0, 5.0};
    Vec<double> b{10.0, 20.0, 30.0, 40.0, 50.0};
    Vec<double> c{100.0, 200.0, 300.0, 400.0, 500.0};

    a.print("a");
    b.print("b");
    c.print("c");

    // 简单加法：不产生临时向量
    std::cout << "\na + b:\n";
    Vec<double> r1 = a + b;
    r1.print("r1");

    // 链式加法：仍然不产生临时向量！
    std::cout << "\na + b + c:\n";
    Vec<double> r2 = a + b + c;
    r2.print("r2");

    // 复杂表达式：一次遍历完成
    std::cout << "\na * 2.0 + b - c * 0.5:\n";
    Vec<double> r3 = a * 2.0 + b - c * 0.5;
    r3.print("r3");

    // 逐元素乘法
    std::cout << "\na * b (逐元素):\n";
    Vec<double> r4 = a * b;
    r4.print("r4");

    std::cout << "\n";
}

// ============================================================
// 6. 表达式模板的原理详解
// ============================================================

void demo_how_it_works() {
    std::cout << "=== 表达式模板原理详解 ===\n";

    Vec<double> a{1.0, 2.0, 3.0};
    Vec<double> b{4.0, 5.0, 6.0};
    Vec<double> c{7.0, 8.0, 9.0};

    // a + b + c 的类型是什么？
    // a + b → VecAdd<Vec<double>, Vec<double>>
    // (a + b) + c → VecAdd<VecAdd<Vec<double>, Vec<double>>, Vec<double>>

    auto expr = a + b + c;  // 不计算，只构建表达式树

    std::cout << "表达式 a + b + c 的类型:\n";
    std::cout << "  VecAdd<VecAdd<Vec<double>, Vec<double>>, Vec<double>>\n\n";

    std::cout << "当访问 expr[i] 时:\n";
    std::cout << "  expr[i] = (a + b)[i] + c[i]\n";
    std::cout << "          = (a[i] + b[i]) + c[i]\n";
    std::cout << "          = a[i] + b[i] + c[i]\n\n";

    std::cout << "验证: expr[0] = " << expr[0] << " (应为 1+4+7=12)\n";
    std::cout << "验证: expr[1] = " << expr[1] << " (应为 2+5+8=15)\n";
    std::cout << "验证: expr[2] = " << expr[2] << " (应为 3+6+9=18)\n\n";

    std::cout << "关键优势:\n";
    std::cout << "  1. 无临时向量: 不分配中间结果的内存\n";
    std::cout << "  2. 单次遍历: 所有操作在一次循环中完成\n";
    std::cout << "  3. 编译期展开: 表达式树在编译期构建\n";
    std::cout << "  4. 编译器可优化: 内联后接近手写循环\n";

    std::cout << "\n";
}

// ============================================================
// 7. 性能对比
// ============================================================

void demo_performance() {
    std::cout << "=== 性能对比 ===\n";

    const std::size_t N = 1000000;
    const int ITERATIONS = 100;

    // 朴素向量
    {
        NaiveVector a(N), b(N), c(N), d(N);
        for (std::size_t i = 0; i < N; ++i) {
            a[i] = i * 0.1;
            b[i] = i * 0.2;
            c[i] = i * 0.3;
            d[i] = i * 0.4;
        }

        auto start = std::chrono::high_resolution_clock::now();
        volatile double sink = 0.0;
        for (int iter = 0; iter < ITERATIONS; ++iter) {
            NaiveVector result = a + b + c + d;
            sink = result[0];  // 防止优化掉
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::cout << "朴素向量: " << ms << " us\n";
    }

    // 表达式模板
    {
        Vec<double> a(N), b(N), c(N), d(N);
        for (std::size_t i = 0; i < N; ++i) {
            a[i] = i * 0.1;
            b[i] = i * 0.2;
            c[i] = i * 0.3;
            d[i] = i * 0.4;
        }

        auto start = std::chrono::high_resolution_clock::now();
        volatile double sink = 0.0;
        for (int iter = 0; iter < ITERATIONS; ++iter) {
            Vec<double> result = a + b + c + d;
            sink = result[0];  // 防止优化掉
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::cout << "表达式模板: " << ms << " us\n";
    }

    std::cout << "\n表达式模板通常更快，因为:\n";
    std::cout << "  - 无临时对象分配/释放\n";
    std::cout << "  - 单次遍历，缓存友好\n";
    std::cout << "  - 编译器可以充分内联优化\n";

    std::cout << "\n";
}

// ============================================================
// 8. 扩展：数学函数表达式
// ============================================================

// 一元函数表达式
template<typename Arg>
class VecUnaryOp : public VecExpr<VecUnaryOp<Arg>> {
    const Arg& arg_;
    double (*func_)(double);

public:
    VecUnaryOp(const Arg& arg, double (*func)(double))
        : arg_(arg), func_(func) {}

    double operator[](std::size_t i) const {
        return func_(arg_[i]);
    }

    std::size_t size() const { return arg_.size(); }
};

// 辅助函数：对向量应用数学函数
template<typename Arg>
VecUnaryOp<Arg> apply_func(const VecExpr<Arg>& arg, double (*func)(double)) {
    return VecUnaryOp<Arg>(static_cast<const Arg&>(arg), func);
}

// 便捷函数
template<typename Arg>
VecUnaryOp<Arg> vec_sqrt(const VecExpr<Arg>& arg) {
    return apply_func(arg, std::sqrt);
}

template<typename Arg>
VecUnaryOp<Arg> vec_abs(const VecExpr<Arg>& arg) {
    return apply_func(arg, std::fabs);
}

template<typename Arg>
VecUnaryOp<Arg> vec_sin(const VecExpr<Arg>& arg) {
    return apply_func(arg, std::sin);
}

template<typename Arg>
VecUnaryOp<Arg> vec_cos(const VecExpr<Arg>& arg) {
    return apply_func(arg, std::cos);
}

void demo_math_functions() {
    std::cout << "=== 数学函数表达式 ===\n";

    Vec<double> a{1.0, 4.0, 9.0, 16.0, 25.0};
    a.print("a");

    // sqrt(a): 不产生临时向量
    Vec<double> r1 = vec_sqrt(a);
    r1.print("sqrt(a)");

    // 复合表达式：sqrt(a) * 2.0 + 1.0
    // 注意：这里需要用 VecScale 包裹
    Vec<double> r2 = vec_sqrt(a) * 2.0;
    r2.print("sqrt(a) * 2.0");

    // sin + cos 组合
    Vec<double> angles{0.0, 0.5, 1.0, 1.5, 2.0};
    Vec<double> r3 = vec_sin(angles) * vec_sin(angles) + vec_cos(angles) * vec_cos(angles);
    r3.print("sin^2 + cos^2 (应全为1.0)");

    std::cout << "\n";
}

// ============================================================
// 9. 表达式模板的局限性
// ============================================================

void demo_limitations() {
    std::cout << "=== 表达式模板的局限性 ===\n";

    std::cout << "1. 调试困难:\n";
    std::cout << "   编译器错误信息极其冗长\n";
    std::cout << "   表达式类型嵌套很深\n\n";

    std::cout << "2. 代码膨胀:\n";
    std::cout << "   每种表达式组合生成不同的模板实例\n";
    std::cout << "   可能增加编译时间和二进制大小\n\n";

    std::cout << "3. 生命周期问题:\n";
    std::cout << "   表达式对象持有引用，不能超出原向量生命周期\n";
    std::cout << "   例如: auto expr = Vec{1,2,3} + Vec{4,5,6}; // 危险!\n\n";

    std::cout << "4. 不适合所有场景:\n";
    std::cout << "   简单运算不值得使用表达式模板\n";
    std::cout << "   需要性能分析确认收益\n\n";

    std::cout << "5. 实际应用:\n";
    std::cout << "   Eigen 线性代数库大量使用表达式模板\n";
    std::cout << "   Blaze 高性能数学库\n";
    std::cout << "   Boost.Proto 表达式模板框架\n";

    std::cout << "\n";
}

// ============================================================
// main
// ============================================================

int main() {
    std::cout << "============================================\n";
    std::cout << "  表达式模板 (Expression Templates)\n";
    std::cout << "============================================\n\n";

    demo_naive_vector_problem();
    demo_expression_templates();
    demo_how_it_works();
    demo_performance();
    demo_math_functions();
    demo_limitations();

    std::cout << "============================================\n";
    std::cout << "  核心要点总结:\n";
    std::cout << "  1. 表达式模板: 延迟计算，避免临时对象\n";
    std::cout << "  2. 核心机制: CRTP + 运算符重载\n";
    std::cout << "  3. 性能优势: 单次遍历，无内存分配\n";
    std::cout << "  4. 适用场景: 向量/矩阵运算\n";
    std::cout << "  5. 注意生命周期和调试问题\n";
    std::cout << "============================================\n";

    return 0;
}
