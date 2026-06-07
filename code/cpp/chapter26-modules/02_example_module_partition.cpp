/**
 * @file 02_example_module_partition.cpp
 * @brief 模块分区与接口单元示例
 * @description 对应文档: 02-CPP/27-modules
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cmath>

/*
 * ============================================================
 * 模块接口单元与实现单元语法参考
 * ============================================================
 *
 * 模块接口单元 (.cppm):
 *   export module my_module;        // 必须有export
 *   export void func();             // 导出声明
 *
 * 模块实现单元 (.cpp):
 *   module my_module;               // 无export
 *   void func() { /* 实现 *\/ }    // 定义
 *
 * 模块分区接口单元:
 *   export module my_module:part;   // 有export
 *   export void part_func();
 *
 * 模块分区实现单元:
 *   module my_module:part_impl;     // 无export
 *   void part_func() { /* ... *\/ }
 *
 * 头文件单元:
 *   import <vector>;                // 将头文件作为模块导入
 *   import "my_header.h";           // 将自定义头文件作为模块导入
 *
 * ============================================================
 */

namespace geometry {

struct Point {
    double x, y;
};

double distance(const Point& a, const Point& b) {
    return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

struct Circle {
    Point center;
    double radius;
};

double area(const Circle& c) {
    return 3.14159265358979323846 * c.radius * c.radius;
}

bool contains(const Circle& c, const Point& p) {
    return distance(c.center, p) <= c.radius;
}

}

namespace algebra {

struct Matrix2x2 {
    double a[2][2];

    static Matrix2x2 identity() {
        return {{{1, 0}, {0, 1}}};
    }

    Matrix2x2 multiply(const Matrix2x2& other) const {
        Matrix2x2 result{{{0, 0}, {0, 0}}};
        for (int i = 0; i < 2; ++i)
            for (int j = 0; j < 2; ++j)
                for (int k = 0; k < 2; ++k)
                    result.a[i][j] += a[i][k] * other.a[k][j];
        return result;
    }

    double determinant() const {
        return a[0][0] * a[1][1] - a[0][1] * a[1][0];
    }

    void print() const {
        for (int i = 0; i < 2; ++i) {
            std::cout << "  [" << a[i][0] << ", " << a[i][1] << "]\n";
        }
    }
};

}

namespace statistics {

double mean(const std::vector<double>& data) {
    if (data.empty()) return 0.0;
    double sum = 0;
    for (double d : data) sum += d;
    return sum / data.size();
}

double variance(const std::vector<double>& data) {
    if (data.empty()) return 0.0;
    double m = mean(data);
    double sum_sq = 0;
    for (double d : data) sum_sq += (d - m) * (d - m);
    return sum_sq / data.size();
}

double stddev(const std::vector<double>& data) {
    return std::sqrt(variance(data));
}

}

void demo_interface_unit() {
    std::cout << "\n=== 模块接口单元 ===\n";

    /*
     * 接口单元示例:
     *
     * // geometry.cppm
     * export module geometry;
     *
     * export struct Point { double x, y; };
     *
     * export double distance(const Point& a, const Point& b);
     *
     * // 不导出内部辅助函数
     * double squared_distance(const Point& a, const Point& b);
     */

    geometry::Point a{0, 0}, b{3, 4};
    std::cout << "距离: " << geometry::distance(a, b) << "\n";

    geometry::Circle c{{0, 0}, 5};
    std::cout << "面积: " << geometry::area(c) << "\n";
    std::cout << "(3,4)在圆内: " << std::boolalpha << geometry::contains(c, {3, 4}) << "\n";
    std::cout << "(4,4)在圆内: " << std::boolalpha << geometry::contains(c, {4, 4}) << "\n";
}

void demo_implementation_unit() {
    std::cout << "\n=== 模块实现单元 ===\n";

    /*
     * 实现单元示例:
     *
     * // geometry.cpp
     * module geometry;
     *
     * #include <cmath>
     *
     * double distance(const Point& a, const Point& b) {
     *     return std::sqrt(squared_distance(a, b));
     * }
     *
     * double squared_distance(const Point& a, const Point& b) {
     *     auto dx = a.x - b.x;
     *     auto dy = a.y - b.y;
     *     return dx * dx + dy * dy;
     * }
     *
     * 注意: #include在模块实现单元中是安全的
     *       宏不会泄漏到模块使用者
     */

    algebra::Matrix2x2 m1 = algebra::Matrix2x2::identity();
    algebra::Matrix2x2 m2{{{2, 3}, {4, 5}}};
    auto m3 = m1.multiply(m2);
    std::cout << "矩阵乘法:\n";
    m3.print();
    std::cout << "行列式: " << m2.determinant() << "\n";
}

void demo_header_unit() {
    std::cout << "\n=== 头文件单元 ===\n";

    /*
     * 头文件单元:
     *
     * // 传统方式
     * #include <vector>
     *
     * // 头文件单元方式
     * import <vector>;
     *
     * 优势:
     *   1. 预编译, 不需要每次重新解析
     *   2. 隔离宏污染
     *   3. 与模块可以混合使用
     *
     * 自定义头文件单元:
     *   import "my_header.h";
     *   需要编译器支持头文件单元的构建
     */

    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0};
    std::cout << "均值: " << statistics::mean(data) << "\n";
    std::cout << "方差: " << statistics::variance(data) << "\n";
    std::cout << "标准差: " << statistics::stddev(data) << "\n";
}

void demo_global_module_fragment() {
    std::cout << "\n=== 全局模块片段 ===\n";

    /*
     * 全局模块片段:
     *
     * module;                        // 全局模块片段开始
     *
     * #include <cstring>             // 只能包含预处理指令
     * #define DEBUG_MODE 1           // 宏定义
     *
     * module my_module;              // 模块声明
     *
     * // 这里的代码可以使用上面的include和define
     * // 但这些不会泄漏到import此模块的代码中
     *
     * 用途:
     *   1. 在模块中使用传统的#include
     *   2. 定义模块内部使用的宏
     *   3. 迁移旧代码时的过渡方案
     */

    std::cout << "全局模块片段(module;)的用途:\n";
    std::cout << "  1. 包含传统头文件(#include)\n";
    std::cout << "  2. 定义模块内部宏\n";
    std::cout << "  3. 迁移旧代码的过渡方案\n";
    std::cout << "\n重要: 全局模块片段中的宏不会泄漏到模块使用者\n";
}

int main() {
    std::cout << "========== 模块分区与接口单元示例 ==========\n";
    std::cout << "注意: 完整模块支持需要较新编译器, 本文件使用传统方式\n";

    demo_interface_unit();
    demo_implementation_unit();
    demo_header_unit();
    demo_global_module_fragment();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
