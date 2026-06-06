/** @file 03_example_rvalue_reference.cpp
 *  @brief 右值引用基础：右值引用语法、延长对象生命周期、返回引用的注意事项
 *  @description 对应文档: 09-移动语义与完美转发
 */

#include <iostream>
#include <string>
#include <utility>

class Tracker {
public:
    Tracker(std::string name) : name_(std::move(name)) {
        std::cout << "Tracker(\"" << name_ << "\") 构造\n";
    }
    Tracker(const Tracker& other) : name_(other.name_ + "_拷贝") {
        std::cout << "Tracker 拷贝构造: \"" << name_ << "\"\n";
    }
    Tracker(Tracker&& other) noexcept : name_(std::move(other.name_)) {
        other.name_ = "(已移动)";
        std::cout << "Tracker 移动构造: \"" << name_ << "\"\n";
    }
    ~Tracker() {
        std::cout << "Tracker(\"" << name_ << "\") 析构\n";
    }
    const std::string& name() const { return name_; }
private:
    std::string name_;
};

Tracker make_tracker(const std::string& name) {
    return Tracker(name);
}

void demo_rvalue_reference_basics() {
    std::cout << "=== 右值引用基础 ===\n";

    int&& r1 = 42;
    std::cout << "int&& r1 = 42; r1 = " << r1 << "\n";
    r1 = 100;
    std::cout << "r1 = 100; r1 = " << r1 << " (右值引用是左值, 可修改)\n";

    double&& r2 = 3.14;
    std::cout << "double&& r2 = 3.14; r2 = " << r2 << "\n";

    std::string&& r3 = std::string("临时字符串");
    std::cout << "string&& r3 = 临时字符串; r3 = " << r3 << "\n";

    std::cout << "\n右值引用的特点:\n";
    std::cout << "  1. 只能绑定到右值 (临时对象、std::move的结果)\n";
    std::cout << "  2. 右值引用变量本身是左值 (有名字)\n";
    std::cout << "  3. 用于实现移动语义\n";

    std::cout << "\n";
}

void demo_lifetime_extension() {
    std::cout << "=== 右值引用延长对象生命周期 ===\n";

    std::cout << "--- const 左值引用也能延长生命周期 ---\n";
    {
        const Tracker& cr = make_tracker("const引用");
        std::cout << "cr.name() = " << cr.name() << "\n";
        std::cout << "const& 延长了临时对象的生命周期\n";
    }
    std::cout << "离开作用域, 临时对象析构\n\n";

    std::cout << "--- 右值引用延长生命周期 ---\n";
    {
        Tracker&& rr = make_tracker("右值引用");
        std::cout << "rr.name() = " << rr.name() << "\n";
        std::cout << "&& 延长了临时对象的生命周期\n";
    }
    std::cout << "离开作用域, 临时对象析构\n\n";

    std::cout << "--- 不绑定引用, 临时对象立即销毁 ---\n";
    {
        make_tracker("无引用");
        std::cout << "临时对象在完整表达式结束后立即析构\n";
    }

    std::cout << "\nconst& vs && 延长生命周期的区别:\n";
    std::cout << "  const&: 可以延长, 但不能修改\n";
    std::cout << "  &&: 可以延长, 且可以修改 (可移动)\n";

    std::cout << "\n";
}

void demo_returning_references() {
    std::cout << "=== 返回引用的注意事项 ===\n";

    std::cout << "--- 返回右值引用 (通常不推荐) ---\n";
    {
        auto bad_func = []() -> std::string&& {
            std::string local = "局部变量";
            // return std::move(local);  // 危险! 返回局部变量的右值引用
            std::cout << "  返回局部变量的引用是未定义行为!\n";
            return std::move(local);  // 仅演示, 实际中不要这样做
        };
        std::cout << "  返回局部变量的右值引用 = 悬垂引用\n";
    }

    std::cout << "\n--- 正确做法: 返回值 ---\n";
    {
        auto good_func = []() -> std::string {
            std::string local = "局部变量";
            return local;  // RVO/NRVO 优化, 或移动语义
        };
        std::string result = good_func();
        std::cout << "  返回值: " << result << " (安全)\n";
    }

    std::cout << "\n--- 返回成员的右值引用 (有时合理) ---\n";
    {
        struct Container {
            std::string data = "容器数据";
            std::string&& extract() { return std::move(data); }
        };
        Container c;
        std::string taken = c.extract();
        std::cout << "  提取的数据: " << taken << "\n";
        std::cout << "  容器中的数据: \"" << c.data << "\" (已移动)\n";
    }

    std::cout << "\n返回引用的原则:\n";
    std::cout << "  1. 不要返回局部变量的引用 (左值或右值)\n";
    std::cout << "  2. 返回值优于返回引用 (编译器会优化)\n";
    std::cout << "  3. 返回成员的右值引用用于'提取'语义\n";

    std::cout << "\n";
}

void demo_rvalue_reference_in_overloading() {
    std::cout << "=== 右值引用与函数重载 ===\n";

    struct Processor {
        void process(const std::string& s) {
            std::cout << "处理左值: " << s << " (拷贝语义)\n";
        }
        void process(std::string&& s) {
            std::cout << "处理右值: " << s << " (移动语义)\n";
        }
    };

    Processor p;
    std::string data = "重要数据";

    p.process(data);
    p.process(std::move(data));
    p.process(std::string("临时数据"));

    std::cout << "\n重载解析规则:\n";
    std::cout << "  左值 => 匹配 const T& 重载\n";
    std::cout << "  右值 => 匹配 T&& 重载\n";
    std::cout << "  没有右值重载时, 右值也能匹配 const T&\n";

    std::cout << "\n";
}

void demo_reference_collapsing() {
    std::cout << "=== 引用折叠规则 ===\n";

    std::cout << "引用折叠发生在模板实例化和 typedef 中:\n\n";
    std::cout << "  T&  &   => T&    (左值引用的左值引用 = 左值引用)\n";
    std::cout << "  T&  &&  => T&    (左值引用的右值引用 = 左值引用)\n";
    std::cout << "  T&& &   => T&    (右值引用的左值引用 = 左值引用)\n";
    std::cout << "  T&& &&  => T&&   (右值引用的右值引用 = 右值引用)\n\n";

    std::cout << "记忆: 只要有一个左值引用, 结果就是左值引用\n";
    std::cout << "只有两个都是右值引用, 结果才是右值引用\n\n";

    std::cout << "这是万能引用工作原理的基础:\n";
    std::cout << "  template<typename T> void f(T&& arg);\n";
    std::cout << "  f(左值) => T = int&,  T&& = int& && => int&\n";
    std::cout << "  f(右值) => T = int,   T&& = int&&\n";

    std::cout << "\n";
}

int main() {
    demo_rvalue_reference_basics();
    demo_lifetime_extension();
    demo_returning_references();
    demo_rvalue_reference_in_overloading();
    demo_reference_collapsing();

    return 0;
}
