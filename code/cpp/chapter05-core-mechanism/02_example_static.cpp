/** @file 02_example_static.cpp
 *  @brief static局部变量、static类成员、static成员函数、static vs namespace
 *  @description 对应文档: 02-CPP/05-core-mechanism
 */

#include <iostream>
#include <string>
#include <vector>

// ===== 1. static 局部变量 =====
void counter() {
    static int count = 0;  // 只初始化一次, 生命周期持续到程序结束
    ++count;
    std::cout << "  counter 调用次数: " << count << std::endl;
}

int generate_id() {
    static int next_id = 0;
    return ++next_id;
}

void demo_static_local() {
    std::cout << "===== static 局部变量 =====" << std::endl;

    counter();
    counter();
    counter();

    std::cout << "\n生成 ID:" << std::endl;
    std::cout << "  ID 1: " << generate_id() << std::endl;
    std::cout << "  ID 2: " << generate_id() << std::endl;
    std::cout << "  ID 3: " << generate_id() << std::endl;

    std::cout << "\nstatic 局部变量特性:" << std::endl;
    std::cout << "  - 只初始化一次 (首次执行到声明处)" << std::endl;
    std::cout << "  - 生命周期持续到程序结束" << std::endl;
    std::cout << "  - 作用域仍限于函数内部" << std::endl;
    std::cout << "  - 线程安全的初始化 (C++11 保证)" << std::endl;
}

// ===== 2. static 类成员 =====
class Config {
public:
    Config(const std::string& name) : name_(name), id_(next_id_++) {
        std::cout << "  创建配置: " << name_ << " (id=" << id_ << ")" << std::endl;
        ++instance_count_;
    }

    ~Config() {
        --instance_count_;
    }

    // static 成员函数: 访问 static 成员
    static int get_instance_count() {
        return instance_count_;
    }

    static int get_next_id() {
        return next_id_;
    }

    // static 成员函数没有 this 指针
    // 不能访问非 static 成员
    // 不能声明为 const / virtual

    const std::string& name() const { return name_; }
    int id() const { return id_; }

private:
    std::string name_;
    int id_;

    // static 成员变量: 所有对象共享
    static int next_id_;
    static int instance_count_;
};

// static 成员变量必须在类外定义 (C++17 inline static 除外)
int Config::next_id_ = 1;
int Config::instance_count_ = 0;

void demo_static_class_members() {
    std::cout << "\n===== static 类成员 =====" << std::endl;

    std::cout << "创建前: instance_count = " << Config::get_instance_count() << std::endl;

    {
        Config c1("数据库配置");
        Config c2("网络配置");
        std::cout << "  当前实例数: " << Config::get_instance_count() << std::endl;
    }

    std::cout << "离开作用域后: instance_count = " << Config::get_instance_count() << std::endl;

    std::cout << "\nstatic 成员要点:" << std::endl;
    std::cout << "  - static 变量: 所有对象共享一份" << std::endl;
    std::cout << "  - static 函数: 无 this 指针, 只能访问 static 成员" << std::endl;
    std::cout << "  - 通过 类名::成员 访问" << std::endl;
    std::cout << "  - C++17: inline static 可在类内初始化" << std::endl;
}

// ===== 3. C++17 inline static 成员 =====
class ModernConfig {
public:
    ModernConfig(const std::string& name) : name_(name) {}

    // C++17: inline static 成员, 无需类外定义
    inline static const std::string kVersion = "2.0";
    inline static int global_counter = 0;

    const std::string& name() const { return name_; }

private:
    std::string name_;
};

void demo_inline_static() {
    std::cout << "\n===== C++17 inline static =====" << std::endl;

    std::cout << "ModernConfig::kVersion = " << ModernConfig::kVersion << std::endl;
    ModernConfig::global_counter = 42;
    std::cout << "ModernConfig::global_counter = " << ModernConfig::global_counter << std::endl;

    std::cout << "\ninline static 优势:" << std::endl;
    std::cout << "  - 无需类外定义, 减少样板代码" << std::endl;
    std::cout << "  - 头文件中定义不会导致 ODR 违规" << std::endl;
    std::cout << "  - 替代原来的 #define 和全局常量" << std::endl;
}

// ===== 4. static vs namespace =====
namespace math_utils {
    const double PI = 3.14159265358979;

    double circle_area(double r) {
        return PI * r * r;
    }

    double circle_perimeter(double r) {
        return 2 * PI * r;
    }
}

class MathUtils {
public:
    static constexpr double PI = 3.14159265358979;

    static double circle_area(double r) {
        return PI * r * r;
    }

    static double circle_perimeter(double r) {
        return 2 * PI * r;
    }

private:
    MathUtils() = delete;  // 禁止实例化
};

void demo_static_vs_namespace() {
    std::cout << "\n===== static vs namespace =====" << std::endl;

    // 命名空间方式
    std::cout << "namespace: 面积 = " << math_utils::circle_area(5.0) << std::endl;

    // 类 static 方式
    std::cout << "class static: 面积 = " << MathUtils::circle_area(5.0) << std::endl;

    std::cout << "\n对比:" << std::endl;
    std::cout << "  命名空间:" << std::endl;
    std::cout << "    + 更灵活: 可以 using, 可以重新打开" << std::endl;
    std::cout << "    + 更自然: 纯工具函数不需要类" << std::endl;
    std::cout << "    + ADL 友好" << std::endl;

    std::cout << "  类 static:" << std::endl;
    std::cout << "    + 可以私有化: 控制访问" << std::endl;
    std::cout << "    + 可以 delete 构造函数禁止实例化" << std::endl;
    std::cout << "    + 模板参数可以是类类型" << std::endl;

    std::cout << "  建议: 纯工具函数用命名空间" << std::endl;
    std::cout << "        需要封装或模板化时用类 static" << std::endl;
}

int main() {
    std::cout << "========== static 详解 ==========\n" << std::endl;

    demo_static_local();
    demo_static_class_members();
    demo_inline_static();
    demo_static_vs_namespace();

    return 0;
}
