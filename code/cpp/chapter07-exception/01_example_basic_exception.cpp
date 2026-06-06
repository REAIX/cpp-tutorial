/** @file 01_example_basic_exception.cpp
 *  @brief try/catch/throw、异常类型、标准异常、catch顺序
 *  @description 对应文档: 02-CPP/07-exception
 */

#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <new>

// ===== 1. try/catch/throw 基础 =====
double safe_divide(double a, double b) {
    if (b == 0.0) {
        throw std::runtime_error("除数不能为零!");
    }
    return a / b;
}

void demo_try_catch_throw() {
    std::cout << "===== try/catch/throw 基础 =====" << std::endl;

    try {
        std::cout << "  10 / 3 = " << safe_divide(10.0, 3.0) << std::endl;
        std::cout << "  10 / 0 = " << safe_divide(10.0, 0.0) << std::endl;  // 抛出异常
        std::cout << "  这行不会执行" << std::endl;
    } catch (const std::runtime_error& e) {
        std::cout << "  捕获异常: " << e.what() << std::endl;
    }

    std::cout << "\n异常处理流程:" << std::endl;
    std::cout << "  1. throw 抛出异常对象" << std::endl;
    std::cout << "  2. 程序跳转到最近的匹配 catch 块" << std::endl;
    std::cout << "  3. 栈展开: 自动析构路径上的局部对象" << std::endl;
    std::cout << "  4. catch 块处理异常" << std::endl;
}

// ===== 2. 异常类型 =====
void demo_exception_types() {
    std::cout << "\n===== 异常类型 =====" << std::endl;

    // 可以抛出任何可拷贝的类型
    // 但推荐使用 std::exception 的派生类

    // 抛出整数 (不推荐)
    try {
        throw 42;
    } catch (int e) {
        std::cout << "  捕获 int: " << e << std::endl;
    }

    // 抛出字符串 (不推荐)
    try {
        throw "出错了!";
    } catch (const char* e) {
        std::cout << "  捕获 const char*: " << e << std::endl;
    }

    // 抛出标准异常 (推荐)
    try {
        throw std::invalid_argument("参数无效");
    } catch (const std::invalid_argument& e) {
        std::cout << "  捕获 invalid_argument: " << e.what() << std::endl;
    }

    std::cout << "\n推荐: 总是抛出 std::exception 派生类的对象" << std::endl;
}

// ===== 3. 标准异常层次 =====
void demo_standard_exceptions() {
    std::cout << "\n===== 标准异常层次 =====" << std::endl;

    // std::exception (所有标准异常的基类)
    //   ├── std::logic_error (逻辑错误, 可在编译期检测)
    //   │     ├── std::invalid_argument (无效参数)
    //   │     ├── std::domain_error (域错误)
    //   │     ├── std::length_error (长度超限)
    //   │     └── std::out_of_range (越界)
    //   ├── std::runtime_error (运行时错误)
    //   │     ├── std::range_error (范围错误)
    //   │     ├── std::overflow_error (溢出)
    //   │     └── std::underflow_error (下溢)
    //   └── std::bad_alloc (内存分配失败)
    //         std::bad_cast (dynamic_cast 失败)
    //         std::bad_typeid (typeid 失败)

    try {
        std::vector<int> v;
        v.at(10);  // 越界访问, 抛出 std::out_of_range
    } catch (const std::out_of_range& e) {
        std::cout << "  out_of_range: " << e.what() << std::endl;
    }

    try {
        throw std::length_error("容器长度超限");
    } catch (const std::logic_error& e) {
        std::cout << "  logic_error: " << e.what() << std::endl;
    }

    try {
        throw std::overflow_error("数值溢出");
    } catch (const std::runtime_error& e) {
        std::cout << "  runtime_error: " << e.what() << std::endl;
    }

    // 用基类捕获所有标准异常
    try {
        throw std::invalid_argument("测试");
    } catch (const std::exception& e) {
        std::cout << "  exception 基类捕获: " << e.what() << std::endl;
    }
}

// ===== 4. catch 顺序 =====
void demo_catch_order() {
    std::cout << "\n===== catch 顺序 =====" << std::endl;

    // catch 块按顺序匹配, 第一个匹配的执行
    // 派生类异常必须放在基类前面!

    auto test_catch = [](int type) {
        try {
            if (type == 1) throw std::invalid_argument("invalid_argument");
            if (type == 2) throw std::logic_error("logic_error");
            if (type == 3) throw std::runtime_error("runtime_error");
            if (type == 4) throw 42;
        }
        // 正确顺序: 派生类在前, 基类在后
        catch (const std::invalid_argument& e) {
            std::cout << "  捕获 invalid_argument: " << e.what() << std::endl;
        }
        catch (const std::logic_error& e) {
            std::cout << "  捕获 logic_error: " << e.what() << std::endl;
        }
        catch (const std::runtime_error& e) {
            std::cout << "  捕获 runtime_error: " << e.what() << std::endl;
        }
        catch (const std::exception& e) {
            std::cout << "  捕获 exception: " << e.what() << std::endl;
        }
        catch (...) {
            std::cout << "  捕获未知异常" << std::endl;
        }
    };

    test_catch(1);
    test_catch(2);
    test_catch(3);
    test_catch(4);

    std::cout << "\ncatch 顺序规则:" << std::endl;
    std::cout << "  - 按声明顺序匹配, 第一个匹配的执行" << std::endl;
    std::cout << "  - 派生类必须放在基类前面" << std::endl;
    std::cout << "  - catch(...) 捕获所有异常, 放在最后" << std::endl;
    std::cout << "  - 错误顺序: 基类在前会'吞掉'派生类异常" << std::endl;
}

int main() {
    std::cout << "========== 异常处理基础 ==========\n" << std::endl;

    demo_try_catch_throw();
    demo_exception_types();
    demo_standard_exceptions();
    demo_catch_order();

    return 0;
}
