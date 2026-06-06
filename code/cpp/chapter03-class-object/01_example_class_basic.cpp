/** @file 01_example_class_basic.cpp
 *  @brief 类定义、访问控制、构造函数(默认/参数/拷贝)、析构函数
 *  @description 对应文档: 02-CPP/03-class-object
 */

#include <iostream>
#include <string>

// ===== 1. 类定义与访问控制 =====
class Student {
public:
    // 默认构造函数
    Student() : name_("未知"), age_(0), score_(0.0) {
        std::cout << "  Student() 默认构造: " << name_ << std::endl;
    }

    // 参数化构造函数
    Student(const std::string& name, int age, double score)
        : name_(name), age_(age), score_(score) {
        std::cout << "  Student(name,age,score) 参数构造: " << name_ << std::endl;
    }

    // 拷贝构造函数
    Student(const Student& other)
        : name_(other.name_), age_(other.age_), score_(other.score_) {
        std::cout << "  Student(const Student&) 拷贝构造: " << name_ << std::endl;
    }

    // 析构函数
    ~Student() {
        std::cout << "  ~Student() 析构: " << name_ << std::endl;
    }

    // 公共接口
    void display() const {
        std::cout << "  姓名: " << name_
                  << ", 年龄: " << age_
                  << ", 成绩: " << score_ << std::endl;
    }

    void set_score(double score) {
        if (score >= 0 && score <= 100) {
            score_ = score;
        } else {
            std::cout << "  无效分数!" << std::endl;
        }
    }

    double get_score() const { return score_; }
    const std::string& get_name() const { return name_; }

private:
    std::string name_;
    int age_;
    double score_;

protected:
    // 派生类可以访问
    int get_age() const { return age_; }
};

void demo_class_definition() {
    std::cout << "===== 类定义与访问控制 =====" << std::endl;

    Student s1;  // 默认构造
    s1.display();

    Student s2("张三", 20, 95.5);  // 参数构造
    s2.display();

    Student s3 = s2;  // 拷贝构造
    s3.display();

    // 访问控制
    // s2.name_;  // 错误: private 不可外部访问
    std::cout << "通过接口访问: " << s2.get_name()
              << " 的成绩是 " << s2.get_score() << std::endl;

    std::cout << "\n访问控制:" << std::endl;
    std::cout << "  public:    任何地方可访问" << std::endl;
    std::cout << "  private:   仅类内部可访问" << std::endl;
    std::cout << "  protected: 类内部和派生类可访问" << std::endl;
}

// ===== 2. 构造函数详解 =====
class Rectangle {
public:
    // 委托构造函数 (C++11): 一个构造函数调用另一个
    Rectangle() : Rectangle(0.0, 0.0) {
        std::cout << "  委托构造 -> Rectangle()" << std::endl;
    }

    Rectangle(double width, double height)
        : width_(width), height_(height) {
        std::cout << "  Rectangle(" << width_ << ", " << height_ << ")" << std::endl;
    }

    // 单参数构造函数用 explicit 防止隐式转换
    explicit Rectangle(double side)
        : width_(side), height_(side) {
        std::cout << "  explicit Rectangle(" << side << ") 正方形" << std::endl;
    }

    double area() const { return width_ * height_; }

private:
    double width_;
    double height_;
};

void demo_constructors() {
    std::cout << "\n===== 构造函数详解 =====" << std::endl;

    Rectangle r1;             // 默认构造 (委托到双参数)
    Rectangle r2(3.0, 4.0);  // 双参数构造
    Rectangle r3(5.0);        // explicit 单参数构造

    std::cout << "r1 面积: " << r1.area() << std::endl;
    std::cout << "r2 面积: " << r2.area() << std::endl;
    std::cout << "r3 面积: " << r3.area() << std::endl;

    // explicit 防止隐式转换
    // Rectangle r4 = 5.0;  // 编译错误: explicit 阻止隐式转换
    Rectangle r4(5.0);       // OK: 显式构造

    std::cout << "\n构造函数要点:" << std::endl;
    std::cout << "  - 默认构造: 无参或所有参数有默认值" << std::endl;
    std::cout << "  - 参数化构造: 接受参数初始化成员" << std::endl;
    std::cout << "  - 拷贝构造: 用同类型对象初始化" << std::endl;
    std::cout << "  - 委托构造: C++11, 减少重复代码" << std::endl;
    std::cout << "  - explicit: 防止单参数隐式转换" << std::endl;
}

// ===== 3. 析构函数 =====
class Resource {
public:
    Resource(const std::string& name) : name_(name) {
        std::cout << "  获取资源: " << name_ << std::endl;
    }

    ~Resource() {
        std::cout << "  释放资源: " << name_ << std::endl;
    }

    const std::string& name() const { return name_; }

private:
    std::string name_;
};

void demo_destructor() {
    std::cout << "\n===== 析构函数 =====" << std::endl;

    {
        Resource r1("文件句柄");
        Resource r2("网络连接");
        std::cout << "  使用资源: " << r1.name() << ", " << r2.name() << std::endl;
    }  // r2 先析构, r1 后析构 (与构造相反)

    std::cout << "\n析构函数要点:" << std::endl;
    std::cout << "  - 对象销毁时自动调用" << std::endl;
    std::cout << "  - 析构顺序与构造顺序相反 (栈展开)" << std::endl;
    std::cout << "  - 用于释放资源 (RAII 的核心)" << std::endl;
    std::cout << "  - 多态基类的析构函数应为 virtual" << std::endl;
}

int main() {
    std::cout << "========== 类与对象基础 ==========\n" << std::endl;

    demo_class_definition();
    demo_constructors();
    demo_destructor();

    return 0;
}
