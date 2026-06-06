/** @file 02_example_coding_style.cpp
 *  @brief C++ 命名规范、头文件组织、include 顺序
 *  @description 对应文档: 02-CPP/02-namespace
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>

// ===== 1. 命名规范 =====

// 命名风格约定 (以 Google C++ Style Guide 为参考)

// (1) snake_case: 变量, 函数, 命名空间
int student_count = 0;
double average_score = 0.0;

void calculate_average() {}
void print_report() {}

namespace data_processing {
    const int max_retries = 3;
}

// (2) camelCase: 部分风格用于变量和函数 (Java/C# 风格)
// Qt, LLVM 等项目使用此风格
int studentCount = 0;

// (3) PascalCase (CamelCase): 类, 结构体, 枚举, 类型别名
class StudentRecord {
public:
    StudentRecord(const std::string& name, int score)
        : name_(name), score_(score) {}

    std::string GetName() const { return name_; }
    int GetScore() const { return score_; }

private:
    std::string name_;
    int score_;
};

struct PointData {
    double x;
    double y;
};

enum class ColorType {
    Red,
    Green,
    Blue
};

using RecordList = std::vector<StudentRecord>;

// (4) kConstantName: 常量 (Google 风格)
const int kMaxSize = 100;
const double kPi = 3.14159265;

// (5) 成员变量后缀 _ (Google 风格) 或 前缀 m_ (Qt 风格)
class Widget {
public:
    Widget() : width_(0), height_(0) {}

private:
    int width_;    // 后缀 _
    int height_;
};

// (6) 宏: 全大写 + 下划线 (但尽量用 const/constexpr/enum 替代)
#define MAX_BUFFER_SIZE 1024

// (7) 模板参数: PascalCase 或 全大写
template<typename ElementType>
class Container {
public:
    void Add(const ElementType& element) {
        elements_.push_back(element);
    }
private:
    std::vector<ElementType> elements_;
};

void demo_naming_conventions() {
    std::cout << "===== 命名规范 =====" << std::endl;

    std::cout << "常见命名风格:" << std::endl;
    std::cout << "  snake_case:  变量, 函数 (Google/STL 风格)" << std::endl;
    std::cout << "  camelCase:   变量, 函数 (Qt/LLVM 风格)" << std::endl;
    std::cout << "  PascalCase:  类, 结构体, 枚举, 类型" << std::endl;
    std::cout << "  kCamelCase:  常量 (Google 风格)" << std::endl;
    std::cout << "  ALL_CAPS:    宏 (尽量避免用宏)" << std::endl;
    std::cout << "  name_:       成员变量后缀下划线" << std::endl;

    std::cout << "\n关键原则: 项目内保持一致!" << std::endl;
    std::cout << "  - 选定一种风格, 全项目统一" << std::endl;
    std::cout << "  - 用 clang-format 自动格式化" << std::endl;
    std::cout << "  - 用 .clang-format 配置文件" << std::endl;
}

// ===== 2. 头文件组织 =====
void demo_header_organization() {
    std::cout << "\n===== 头文件组织 =====" << std::endl;

    // 头文件应该包含:
    std::cout << "头文件内容:" << std::endl;
    std::cout << "  1. 头文件保护 (#ifndef / #define / #endif)" << std::endl;
    std::cout << "     或 #pragma once (非标准但广泛支持)" << std::endl;
    std::cout << "  2. 命名空间包裹" << std::endl;
    std::cout << "  3. 前向声明 (减少头文件依赖)" << std::endl;
    std::cout << "  4. 类声明, 函数声明" << std::endl;
    std::cout << "  5. 内联函数和模板定义" << std::endl;

    std::cout << "\n头文件不应该包含:" << std::endl;
    std::cout << "  - using namespace (污染包含者的命名空间)" << std::endl;
    std::cout << "  - 非内联函数的定义" << std::endl;
    std::cout << "  - 静态变量定义" << std::endl;
    std::cout << "  - 不必要的 #include" << std::endl;

    std::cout << "\n头文件保护示例:" << std::endl;
    std::cout << "  #ifndef MY_PROJECT_UTILS_H_" << std::endl;
    std::cout << "  #define MY_PROJECT_UTILS_H_" << std::endl;
    std::cout << "  // ... 内容 ..." << std::endl;
    std::cout << "  #endif  // MY_PROJECT_UTILS_H_" << std::endl;
}

// ===== 3. include 顺序 =====
void demo_include_order() {
    std::cout << "\n===== #include 顺序 =====" << std::endl;

    // Google 风格推荐的 include 顺序:
    std::cout << "推荐顺序 (从近到远):" << std::endl;
    std::cout << "  1. 对应的头文件 (如 foo.cpp 包含 foo.h)" << std::endl;
    std::cout << "  2. C++ 标准库 (<vector>, <string>)" << std::endl;
    std::cout << "  3. 其他库的头文件 (<boost/...>)" << std::endl;
    std::cout << "  4. 本项目的头文件 (\"myproject/...\")" << std::endl;

    std::cout << "\n每类之间空一行, 每类内按字母序排列" << std::endl;

    std::cout << "\n示例:" << std::endl;
    std::cout << "  #include \"foo.h\"           // 1. 对应头文件" << std::endl;
    std::cout << "                              // 空行" << std::endl;
    std::cout << "  #include <algorithm>        // 2. C++ 标准库" << std::endl;
    std::cout << "  #include <iostream>" << std::endl;
    std::cout << "  #include <vector>" << std::endl;
    std::cout << "                              // 空行" << std::endl;
    std::cout << "  #include \"myproject/bar.h\"  // 4. 本项目头文件" << std::endl;
    std::cout << "  #include \"myproject/baz.h\"" << std::endl;

    std::cout << "\n为什么要先包含对应头文件?" << std::endl;
    std::cout << "  - 确保头文件自包含 (不依赖其他头文件的间接包含)" << std::endl;
    std::cout << "  - 如果缺少依赖, 编译错误出现在头文件而非使用者" << std::endl;
}

int main() {
    std::cout << "========== C++ 编码规范 ==========\n" << std::endl;

    demo_naming_conventions();
    demo_header_organization();
    demo_include_order();

    return 0;
}
