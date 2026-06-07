/**
 * @file 01_example_module_basics.cpp
 * @brief 模块基础示例
 * @description 对应文档: 02-CPP/27-modules
 */

#include <iostream>
#include <string>
#include <vector>

/*
 * ============================================================
 * C++20 模块语法说明 (编译器支持有限, 以下为语法参考)
 * ============================================================
 *
 * 模块声明:
 *   module;                          // 全局模块片段(只能包含预处理指令)
 *   module my_module;                // 模块声明
 *
 * 导出:
 *   export module my_module;         // 导出模块声明
 *   export int add(int a, int b);    // 导出函数
 *   export struct Point { int x, y; }; // 导出类型
 *
 * 导入:
 *   import my_module;                // 导入模块
 *   import <iostream>;               // 导入头文件单元
 *
 * 模块分区:
 *   export module my_module:part_a;  // 接口分区
 *   module my_module:impl;           // 实现分区
 *   import :part_a;                  // 导入同模块分区
 *
 * ============================================================
 * 由于编译器支持差异, 以下使用传统头文件方式实现可编译代码
 * 模块语法以注释形式展示
 */

namespace math_utils {

int add(int a, int b) {
    return a + b;
}

int multiply(int a, int b) {
    return a * b;
}

double divide(int a, int b) {
    if (b == 0) return 0.0;
    return static_cast<double>(a) / b;
}

}

namespace string_utils {

std::string to_upper(const std::string& s) {
    std::string result = s;
    for (auto& c : result) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return result;
}

std::string to_lower(const std::string& s) {
    std::string result = s;
    for (auto& c : result) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return result;
}

std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}

}

namespace container_utils {

template<typename T>
std::vector<T> filter(const std::vector<T>& v, bool (*pred)(const T&)) {
    std::vector<T> result;
    for (const auto& item : v) {
        if (pred(item)) result.push_back(item);
    }
    return result;
}

template<typename T, typename Func>
auto map(const std::vector<T>& v, Func f) -> std::vector<decltype(f(v[0]))> {
    std::vector<decltype(f(v[0]))> result;
    for (const auto& item : v) {
        result.push_back(f(item));
    }
    return result;
}

template<typename T>
T sum(const std::vector<T>& v) {
    T total{};
    for (const auto& item : v) total += item;
    return total;
}

}

void demo_module_concept() {
    std::cout << "\n=== 模块概念 ===\n";

    std::cout << "模块 vs 头文件:\n";
    std::cout << "  头文件: 每个翻译单元重复包含, 重复编译\n";
    std::cout << "  模块:   编译一次, 多次使用(预编译)\n";
    std::cout << "\n";
    std::cout << "  头文件: 宏泄漏到包含者的作用域\n";
    std::cout << "  模块:   宏不会泄漏, 隔离性更好\n";
    std::cout << "\n";
    std::cout << "  头文件: 依赖顺序敏感\n";
    std::cout << "  模块:   导入顺序无关\n";
    std::cout << "\n";
    std::cout << "  头文件: #include是文本替换\n";
    std::cout << "  模块:   import是语义导入\n";
}

void demo_export_import() {
    std::cout << "\n=== export与import ===\n";

    /*
     * 模块语法示例:
     *
     * // math_module.cppm (模块接口文件)
     * export module math_module;
     *
     * export int add(int a, int b) {
     *     return a + b;
     * }
     *
     * int internal_helper(int x) {  // 不导出, 模块私有
     *     return x * 2;
     * }
     *
     * // main.cpp
     * import math_module;
     * #include <iostream>
     *
     * int main() {
     *     std::cout << add(3, 4) << "\n";  // OK
     *     // internal_helper(5);  // 错误: 不可见
     * }
     */

    std::cout << "export: 标记需要对外可见的声明\n";
    std::cout << "import: 导入一个模块的所有导出声明\n";
    std::cout << "未export的声明只在模块内部可见\n";

    std::cout << "\n使用传统方式模拟:\n";
    std::cout << "add(3, 4) = " << math_utils::add(3, 4) << "\n";
    std::cout << "multiply(5, 6) = " << math_utils::multiply(5, 6) << "\n";
    std::cout << "divide(10, 3) = " << math_utils::divide(10, 3) << "\n";
}

void demo_module_partition() {
    std::cout << "\n=== 模块分区 ===\n";

    /*
     * 模块分区语法:
     *
     * // math_utils:arithmetic.cppm
     * export module math_utils:arithmetic;
     *
     * export int add(int a, int b) { return a + b; }
     * export int sub(int a, int b) { return a - b; }
     *
     * // math_utils:geometry.cppm
     * export module math_utils:geometry;
     *
     * export double circle_area(double r) { return 3.14159 * r * r; }
     *
     * // math_utils.cppm (主接口文件)
     * export module math_utils;
     *
     * export import :arithmetic;   // 重新导出分区
     * export import :geometry;
     *
     * // 使用者只需: import math_utils;
     */

    std::cout << "模块分区允许将大模块拆分为多个文件:\n";
    std::cout << "  :arithmetic - 算术分区\n";
    std::cout << "  :geometry   - 几何分区\n";
    std::cout << "  主接口文件重新导出所有分区\n";
    std::cout << "  使用者只需 import math_utils;\n";
}

void demo_private_module_fragment() {
    std::cout << "\n=== 私有模块片段 ===\n";

    /*
     * 私有模块片段:
     *
     * export module my_module;
     *
     * export int public_func();  // 声明
     *
     * module :private;           // 私有片段开始
     *
     * int public_func() {        // 实现
     *     return 42;
     * }
     *
     * int internal_state = 0;    // 模块内部状态
     */

    std::cout << "module :private; 之后的代码:\n";
    std::cout << "  只在该翻译单元可见\n";
    std::cout << "  不会影响模块的ABI\n";
    std::cout << "  适合将实现放在接口文件中\n";
}

int main() {
    std::cout << "========== C++20 模块基础示例 ==========\n";
    std::cout << "注意: 完整模块支持需要较新编译器, 本文件使用传统方式\n";

    demo_module_concept();
    demo_export_import();
    demo_module_partition();
    demo_private_module_fragment();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
