/** @file 01_deep_dive_cpp11_migration.cpp
 *  @brief C++03到C++11迁移指南、弃用特性、向后兼容、编译器支持
 *  @description 对应文档: 02-CPP/20-cpp11 | 举一反三：理解C++11带来的破坏性变更和迁移要点
 *  编译命令: g++ -std=c++20 01_deep_dive_cpp11_migration.cpp -o 01_deep_dive_cpp11_migration
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <algorithm>

void demo_deprecated_features() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  C++11 弃用特性\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. auto_ptr → unique_ptr:\n";
    std::cout << "   auto_ptr 有危险的拷贝语义(转移所有权)\n";
    std::cout << "   C++11 引入 unique_ptr 使用移动语义\n\n";

    std::cout << "   auto_ptr<string> p1(new string(\"hello\"));\n";
    std::cout << "   auto_ptr<string> p2 = p1;  // p1 变为空!\n\n";

    auto up1 = std::make_unique<std::string>("hello");
    auto up2 = std::move(up1);
    std::cout << "   unique_ptr 移动后: up1=" << (up1 ? "非空" : "空")
              << ", up2=" << (up2 ? "非空" : "空") << "\n\n";

    std::cout << "2. bind1st/bind2nd → std::bind/lambda:\n";
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    int count_gt5 = std::count_if(v.begin(), v.end(),
        [](int x) { return x > 5; });
    std::cout << "   lambda: 大于5的元素数 = " << count_gt5 << "\n\n";

    std::cout << "3. 函数对象基类 → std::function:\n";
    std::function<int(int, int)> add = [](int a, int b) { return a + b; };
    std::cout << "   std::function: add(3,4) = " << add(3, 4) << "\n\n";

    std::cout << "4. 其他弃用:\n";
    std::cout << "   - auto_ptr      → unique_ptr\n";
    std::cout << "   - bind1st/2nd   → bind / lambda\n";
    std::cout << "   - ptr_fun       → lambda\n";
    std::cout << "   - mem_fun       → mem_fn\n";
    std::cout << "   - unary_function/binary_function → 直接定义\n";
    std::cout << "   - register 关键字 → 无替代(已无意义)\n";
    std::cout << "   - bool 类型的 ++ 操作 → 已移除\n";
}

void demo_breaking_changes() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  C++11 破坏性变更\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. 右尖括号解析:\n";
    std::cout << "   C++03: vector<vector<int> > 需要空格\n";
    std::cout << "   C++11: vector<vector<int>> 不需要空格\n\n";

    std::vector<std::vector<int>> vv = {{1, 2}, {3, 4}};
    std::cout << "   vector<vector<int>> 编译正常\n\n";

    std::cout << "2. 字符串字面量:\n";
    std::cout << "   C++11 新增 u8\"\"/u\"\"/U\"\"/R\"\"() 前缀\n";
    std::string normal = "普通字符串";
    std::string raw = R"(C:\Users\文档\文件.txt)";
    std::cout << "   普通字符串: " << normal << "\n";
    std::cout << "   原始字符串: " << raw << "\n\n";

    std::cout << "3. explicit 改进:\n";
    std::cout << "   C++11 扩展 explicit 到转换运算符\n";
    std::cout << "   防止意外的隐式转换\n\n";

    std::cout << "4. POD 定义变更:\n";
    std::cout << "   C++11 使用标准布局和平凡类型替代POD\n";
    std::cout << "   is_trivial / is_standard_layout\n\n";

    std::cout << "5. for循环变量作用域:\n";
    std::cout << "   C++11 中 for(int x : vec) x 是每次迭代的副本\n";
    std::cout << "   修改 x 不影响容器中的元素\n";
}

void demo_migration_checklist() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  C++03 → C++11 迁移检查清单\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "编译器支持:\n";
    std::cout << "  GCC 4.8+  —— 较完整的C++11支持\n";
    std::cout << "  Clang 3.3+ —— 完整C++11支持\n";
    std::cout << "  MSVC 2013+ —— 较完整的C++11支持\n";
    std::cout << "  推荐: GCC 5+, Clang 3.5+, MSVC 2015+\n\n";

    std::cout << "迁移步骤:\n";
    std::cout << "  1. 更新编译器，启用 -std=c++11\n";
    std::cout << "  2. 替换 auto_ptr 为 unique_ptr/shared_ptr\n";
    std::cout << "  3. 替换 NULL 为 nullptr\n";
    std::cout << "  4. 用 override/final 标注虚函数\n";
    std::cout << "  5. 用 enum class 替换 匿名enum\n";
    std::cout << "  6. 用 range-for 替换 手动迭代器循环\n";
    std::cout << "  7. 用 auto 简化冗长类型声明\n";
    std::cout << "  8. 用 lambda 替换 函数对象和bind\n";
    std::cout << "  9. 用 {} 初始化 替换 () 初始化\n";
    std::cout << "  10. 添加移动构造函数和移动赋值运算符\n\n";

    std::cout << "渐进式迁移:\n";
    std::cout << "  - C++11 几乎完全向后兼容C++03\n";
    std::cout << "  - 可以逐步引入新特性，无需一次重写\n";
    std::cout << "  - 建议先替换弃用特性，再逐步采用新特性\n";
    std::cout << "  - 使用 -Wall -Wextra 检测潜在问题\n";
}

void demo_compiler_flags() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  编译器标志与诊断\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "GCC/Clang C++11 相关标志:\n";
    std::cout << "  -std=c++11       启用C++11\n";
    std::cout << "  -std=gnu++11     启用C++11 + GNU扩展\n";
    std::cout << "  -Wall            常见警告\n";
    std::cout << "  -Wextra          额外警告\n";
    std::cout << "  -Wold-style-cast 检测C风格转换\n";
    std::cout << "  -Wzero-as-null-pointer-source 检测用0作空指针\n";
    std::cout << "  -Wuseless-cast   检测无用转换\n";
    std::cout << "  -Wdeprecated     检测弃用特性\n\n";

    std::cout << "MSVC C++11 相关标志:\n";
    std::cout << "  /std:c++11       启用C++11\n";
    std::cout << "  /W4              高级别警告\n";
    std::cout << "  /permissive-     严格标准一致性\n";
    std::cout << "  /Zc:__cplusplus  正确报告 __cplusplus 值\n\n";

    std::cout << "特性检测宏(C++20起标准化):\n";
    std::cout << "  __has_include(<header>)  检测头文件\n";
    std::cout << "  __cpp_lib_xxxxx         检测库特性\n";
    std::cout << "  __cpp_xxxxx             检测语言特性\n";
}

int main() {
    demo_deprecated_features();
    demo_breaking_changes();
    demo_migration_checklist();
    demo_compiler_flags();
    return 0;
}
