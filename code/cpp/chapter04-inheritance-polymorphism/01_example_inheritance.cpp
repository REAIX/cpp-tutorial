/** @file 01_example_inheritance.cpp
 *  @brief 单继承、protected成员、构造函数链、析构函数顺序
 *  @description 对应文档: 02-CPP/04-inheritance-polymorphism
 */

#include <iostream>
#include <string>

// ===== 1. 单继承基础 =====
class Animal {
public:
    Animal(const std::string& name, int age)
        : name_(name), age_(age) {
        std::cout << "  Animal(\"" << name_ << "\", " << age_ << ") 构造" << std::endl;
    }

    virtual ~Animal() {
        std::cout << "  ~Animal() \"" << name_ << "\" 析构" << std::endl;
    }

    void eat() const {
        std::cout << "  " << name_ << " 在吃东西" << std::endl;
    }

    void sleep() const {
        std::cout << "  " << name_ << " 在睡觉" << std::endl;
    }

    const std::string& name() const { return name_; }
    int age() const { return age_; }

protected:
    void set_age(int age) { age_ = age; }

protected:
    std::string name_;

private:
    int age_;
};

class Dog : public Animal {
public:
    Dog(const std::string& name, int age, const std::string& breed)
        : Animal(name, age), breed_(breed) {
        std::cout << "  Dog(\"" << name_ << "\", " << breed_ << ") 构造" << std::endl;
    }

    ~Dog() override {
        std::cout << "  ~Dog() \"" << name_ << "\" 析构" << std::endl;
    }

    void bark() const {
        std::cout << "  " << name_ << ": 汪汪!" << std::endl;
    }

    const std::string& breed() const { return breed_; }

private:
    std::string breed_;
};

class Cat : public Animal {
public:
    Cat(const std::string& name, int age, bool is_indoor)
        : Animal(name, age), is_indoor_(is_indoor) {
        std::cout << "  Cat(\"" << name_ << "\") 构造" << std::endl;
    }

    ~Cat() override {
        std::cout << "  ~Cat() \"" << name_ << "\" 析构" << std::endl;
    }

    void meow() const {
        std::cout << "  " << name_ << ": 喵~" << std::endl;
    }

private:
    bool is_indoor_;
};

void demo_single_inheritance() {
    std::cout << "===== 单继承基础 =====" << std::endl;

    Dog dog("旺财", 3, "金毛");
    dog.eat();
    dog.bark();

    std::cout << std::endl;
    Cat cat("咪咪", 2, true);
    cat.eat();
    cat.meow();

    std::cout << "\n继承要点:" << std::endl;
    std::cout << "  - 派生类继承基类的成员变量和成员函数" << std::endl;
    std::cout << "  - 派生类可以添加新的成员" << std::endl;
    std::cout << "  - public 继承: 基类的 public 成员在派生类中仍为 public" << std::endl;
}

// ===== 2. protected 成员 =====
class Vehicle {
public:
    Vehicle(int speed) : speed_(speed) {}

    void accelerate(int delta) {
        adjust_speed(delta);
    }

protected:
    int speed_;

    void adjust_speed(int delta) {
        speed_ += delta;
        if (speed_ < 0) speed_ = 0;
    }
};

class Car : public Vehicle {
public:
    Car(int speed, int gears) : Vehicle(speed), gears_(gears) {}

    void show() const {
        std::cout << "  速度: " << speed_  // OK: 派生类可以访问 protected 成员
                  << ", 档位: " << gears_ << std::endl;
    }

    void turbo() {
        adjust_speed(50);  // OK: 派生类可以访问 protected 成员函数
        std::cout << "  涡轮增压! 速度: " << speed_ << std::endl;
    }

private:
    int gears_;
};

void demo_protected_members() {
    std::cout << "\n===== protected 成员 =====" << std::endl;

    Car car(60, 6);
    car.show();
    car.accelerate(20);
    car.show();
    car.turbo();

    // car.speed_;       // 错误: 外部不能访问 protected 成员
    // car.adjust_speed(10);  // 错误: 外部不能访问 protected 成员函数

    std::cout << "\nprotected 访问规则:" << std::endl;
    std::cout << "  - 类内部: 可以访问" << std::endl;
    std::cout << "  - 派生类: 可以访问" << std::endl;
    std::cout << "  - 外部代码: 不能访问" << std::endl;
}

// ===== 3. 构造函数链与析构函数顺序 =====
class Base {
public:
    Base() {
        std::cout << "  Base() 构造" << std::endl;
    }
    virtual ~Base() {
        std::cout << "  ~Base() 析构" << std::endl;
    }
};

class Middle : public Base {
public:
    Middle() : Base() {
        std::cout << "  Middle() 构造" << std::endl;
    }
    ~Middle() override {
        std::cout << "  ~Middle() 析构" << std::endl;
    }
};

class Leaf : public Middle {
public:
    Leaf() : Middle() {
        std::cout << "  Leaf() 构造" << std::endl;
    }
    ~Leaf() override {
        std::cout << "  ~Leaf() 析构" << std::endl;
    }
};

void demo_construction_destruction_chain() {
    std::cout << "\n===== 构造函数链与析构函数顺序 =====" << std::endl;

    std::cout << "创建 Leaf 对象:" << std::endl;
    Leaf leaf;

    std::cout << "\n销毁 Leaf 对象:" << std::endl;
    // 析构顺序: Leaf -> Middle -> Base (与构造相反)

    std::cout << "\n构造顺序: 基类 -> 派生类 (从根到叶)" << std::endl;
    std::cout << "析构顺序: 派生类 -> 基类 (从叶到根)" << std::endl;
    std::cout << "多态基类的析构函数必须为 virtual!" << std::endl;
}

int main() {
    std::cout << "========== 继承基础 ==========\n" << std::endl;

    demo_single_inheritance();
    demo_protected_members();
    demo_construction_destruction_chain();

    return 0;
}
