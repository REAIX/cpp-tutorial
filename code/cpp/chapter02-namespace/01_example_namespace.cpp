/** @file 01_example_namespace.cpp
 *  @brief 命名空间定义、using、嵌套、匿名、别名
 *  @description 对应文档: 02-CPP/02-namespace
 */

#include <iostream>
#include <string>
#include <algorithm>

namespace A { int value = 1; }
namespace B { int value = 2; }

// ===== 1. 命名空间定义 =====
namespace Math {
    const double PI = 3.14159265358979;

    double circleArea(double r) {
        return PI * r * r;
    }

    double circlePerimeter(double r) {
        return 2 * PI * r;
    }
}

namespace Physics {
    const double G = 9.80665;

    double freeFallDistance(double t) {
        return 0.5 * G * t * t;
    }

    double freeFallVelocity(double t) {
        return G * t;
    }
}

void demo_namespace_definition() {
    std::cout << "===== 命名空间定义 =====" << std::endl;

    // 使用完全限定名访问
    std::cout << "圆面积(r=3): " << Math::circleArea(3.0) << std::endl;
    std::cout << "自由落体距离(t=2): " << Physics::freeFallDistance(2.0) << std::endl;

    // 不同命名空间可以有同名成员
    std::cout << "A::value = " << A::value << ", B::value = " << B::value << std::endl;
}

// ===== 2. using 声明与 using 指令 =====
void demo_using() {
    std::cout << "\n===== using 声明与 using 指令 =====" << std::endl;

    // using 声明: 引入单个名称
    using Math::PI;
    std::cout << "using Math::PI; PI = " << PI << std::endl;

    // using 指令: 引入整个命名空间
    using namespace Physics;
    std::cout << "using namespace Physics; G = " << G << std::endl;
    std::cout << "freeFallVelocity(3) = " << freeFallVelocity(3.0) << std::endl;

    // using 指令的风险
    std::cout << "\nusing namespace 的风险:" << std::endl;
    std::cout << "  - 名称冲突: 不同命名空间可能有同名成员" << std::endl;
    std::cout << "  - 头文件中禁止使用 using namespace!" << std::endl;
    std::cout << "  - 源文件中可以谨慎使用" << std::endl;
}

// ===== 3. 嵌套命名空间 =====
namespace Company {
    namespace ProjectA {
        namespace Utils {
            void helper() {
                std::cout << "Company::ProjectA::Utils::helper()" << std::endl;
            }
        }
    }

    // C++17 嵌套命名空间简写
    namespace ProjectB::Utils {
        void helper() {
            std::cout << "Company::ProjectB::Utils::helper()" << std::endl;
        }
    }
}

void demo_nested_namespace() {
    std::cout << "\n===== 嵌套命名空间 =====" << std::endl;

    Company::ProjectA::Utils::helper();
    Company::ProjectB::Utils::helper();

    // C++17 简写
    std::cout << "C++17: namespace A::B::C { ... } 等价于三层嵌套" << std::endl;
}

// ===== 4. 匿名命名空间 =====
namespace {
    int internal_counter = 0;

    void internalFunction() {
        ++internal_counter;
        std::cout << "匿名命名空间: internalFunction 调用次数 " << internal_counter << std::endl;
    }
}

void demo_anonymous_namespace() {
    std::cout << "\n===== 匿名命名空间 =====" << std::endl;

    internalFunction();
    internalFunction();

    // 匿名命名空间等价于:
    // namespace unique_name { ... }
    // using namespace unique_name;
    // 每个翻译单元有唯一的匿名命名空间

    std::cout << "匿名命名空间的用途:" << std::endl;
    std::cout << "  - 替代 static: 限制符号在当前翻译单元内可见" << std::endl;
    std::cout << "  - 比 static 更通用: 对类型也有效" << std::endl;
    std::cout << "  - 避免链接时的名称冲突" << std::endl;
}

// ===== 5. 命名空间别名 =====
void demo_namespace_alias() {
    std::cout << "\n===== 命名空间别名 =====" << std::endl;

    // 长命名空间可以用别名简化
    namespace PAU = Company::ProjectA::Utils;
    PAU::helper();

    namespace PBU = Company::ProjectB::Utils;
    PBU::helper();

    std::cout << "命名空间别名: namespace 别名 = 原始命名空间;" << std::endl;
    std::cout << "  - 使长命名空间更易用" << std::endl;
    std::cout << "  - 不创建新命名空间, 只是别名" << std::endl;
}

int main() {
    std::cout << "========== 命名空间基础 ==========\n" << std::endl;

    demo_namespace_definition();
    demo_using();
    demo_nested_namespace();
    demo_anonymous_namespace();
    demo_namespace_alias();

    return 0;
}
