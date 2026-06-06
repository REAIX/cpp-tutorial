/** @file 02_example_member_functions.cpp
 *  @brief 成员函数、const成员函数、static成员、friend、this指针
 *  @description 对应文档: 02-CPP/03-class-object
 */

#include <iostream>
#include <string>
#include <vector>

// ===== 1. 成员函数与 const 成员函数 =====
class BankAccount {
public:
    BankAccount(const std::string& owner, double balance)
        : owner_(owner), balance_(balance) {}

    // 普通成员函数: 可以修改成员变量
    void deposit(double amount) {
        balance_ += amount;
        std::cout << "  存入 " << amount << ", 余额 " << balance_ << std::endl;
    }

    void withdraw(double amount) {
        if (amount <= balance_) {
            balance_ -= amount;
            std::cout << "  取出 " << amount << ", 余额 " << balance_ << std::endl;
        } else {
            std::cout << "  余额不足!" << std::endl;
        }
    }

    // const 成员函数: 承诺不修改成员变量
    double get_balance() const { return balance_; }
    const std::string& get_owner() const { return owner_; }

    // const 对象只能调用 const 成员函数
    void display() const {
        std::cout << "  账户: " << owner_ << ", 余额: " << balance_ << std::endl;
    }

private:
    std::string owner_;
    double balance_;
};

void demo_const_member_functions() {
    std::cout << "===== const 成员函数 =====" << std::endl;

    BankAccount acc("张三", 1000.0);
    acc.deposit(500.0);
    acc.withdraw(200.0);
    acc.display();

    // const 对象
    const BankAccount const_acc("李四", 2000.0);
    const_acc.display();          // OK: const 成员函数
    std::cout << "  余额: " << const_acc.get_balance() << std::endl;
    // const_acc.deposit(100);    // 错误: const 对象不能调用非 const 成员函数

    std::cout << "\nconst 成员函数规则:" << std::endl;
    std::cout << "  - const 对象只能调用 const 成员函数" << std::endl;
    std::cout << "  - 非const 对象可以调用两种成员函数" << std::endl;
    std::cout << "  - 不修改成员的函数都应标记 const" << std::endl;
}

// ===== 2. static 成员 =====
class IdGenerator {
public:
    static int next_id() {
        return ++counter_;
    }

    static int get_counter() {
        return counter_;
    }

    // static 成员函数没有 this 指针
    // 不能访问非 static 成员
    // 不能声明为 const/virtual

private:
    static int counter_;  // static 成员变量: 类级别共享
};

// static 成员变量必须在类外定义 (C++17 inline static 除外)
int IdGenerator::counter_ = 0;

class User {
public:
    User(const std::string& name)
        : name_(name), id_(IdGenerator::next_id()) {
        std::cout << "  创建用户: " << name_ << " (id=" << id_ << ")" << std::endl;
        ++active_count_;
    }

    ~User() {
        --active_count_;
    }

    static int get_active_count() {
        return active_count_;
    }

private:
    std::string name_;
    int id_;
    static int active_count_;
};

int User::active_count_ = 0;

void demo_static_members() {
    std::cout << "\n===== static 成员 =====" << std::endl;

    std::cout << "ID 生成器:" << std::endl;
    std::cout << "  next_id() = " << IdGenerator::next_id() << std::endl;
    std::cout << "  next_id() = " << IdGenerator::next_id() << std::endl;
    std::cout << "  counter = " << IdGenerator::get_counter() << std::endl;

    std::cout << "\n活跃用户计数:" << std::endl;
    {
        User u1("Alice");
        User u2("Bob");
        std::cout << "  活跃用户: " << User::get_active_count() << std::endl;
    }
    std::cout << "  活跃用户: " << User::get_active_count() << std::endl;

    std::cout << "\nstatic 成员要点:" << std::endl;
    std::cout << "  - static 变量: 所有对象共享一份" << std::endl;
    std::cout << "  - static 函数: 无 this 指针, 只能访问 static 成员" << std::endl;
    std::cout << "  - 通过 类名::成员 访问, 不需要对象" << std::endl;
    std::cout << "  - C++17: inline static 可在类内初始化" << std::endl;
}

// ===== 3. friend =====
class Matrix {
public:
    Matrix(int rows, int cols) : rows_(rows), cols_(cols), data_(rows * cols, 0) {}

    int& at(int r, int c) { return data_[r * cols_ + c]; }
    int at(int r, int c) const { return data_[r * cols_ + c]; }

    // 友元函数: 可以访问私有成员
    friend Matrix operator+(const Matrix& a, const Matrix& b);
    friend void print_matrix(const Matrix& m);

    // 友元类: 整个类可以访问私有成员
    friend class MatrixAccessor;

private:
    int rows_;
    int cols_;
    std::vector<int> data_;
};

Matrix operator+(const Matrix& a, const Matrix& b) {
    Matrix result(a.rows_, a.cols_);
    for (size_t i = 0; i < a.data_.size(); ++i) {
        result.data_[i] = a.data_[i] + b.data_[i];
    }
    return result;
}

void print_matrix(const Matrix& m) {
    for (int r = 0; r < m.rows_; ++r) {
        std::cout << "  ";
        for (int c = 0; c < m.cols_; ++c) {
            std::cout << m.data_[r * m.cols_ + c] << " ";
        }
        std::cout << std::endl;
    }
}

class MatrixAccessor {
public:
    static void set(Matrix& m, int r, int c, int value) {
        m.data_[r * m.cols_ + c] = value;  // 友元类可以访问私有成员
    }
};

void demo_friend() {
    std::cout << "\n===== friend =====" << std::endl;

    Matrix m1(2, 3);
    Matrix m2(2, 3);

    for (int r = 0; r < 2; ++r) {
        for (int c = 0; c < 3; ++c) {
            m1.at(r, c) = r * 3 + c + 1;
            m2.at(r, c) = (r * 3 + c + 1) * 10;
        }
    }

    std::cout << "矩阵 m1:" << std::endl;
    print_matrix(m1);
    std::cout << "矩阵 m2:" << std::endl;
    print_matrix(m2);

    Matrix m3 = m1 + m2;
    std::cout << "m1 + m2:" << std::endl;
    print_matrix(m3);

    MatrixAccessor::set(m1, 0, 0, 99);
    std::cout << "友元类修改后 m1:" << std::endl;
    print_matrix(m1);

    std::cout << "\nfriend 要点:" << std::endl;
    std::cout << "  - friend 声明在类内部, 但不是成员函数" << std::endl;
    std::cout << "  - friend 可以访问类的私有和保护成员" << std::endl;
    std::cout << "  - friend 不受访问控制符限制" << std::endl;
    std::cout << "  - 友元关系是单向的, 不传递, 不继承" << std::endl;
}

// ===== 4. this 指针 =====
class Chain {
public:
    Chain() : value_(0) {}

    // 链式调用: 返回 *this 的引用
    Chain& set_value(int v) {
        value_ = v;
        return *this;
    }

    Chain& add(int v) {
        value_ += v;
        return *this;
    }

    Chain& multiply(int v) {
        value_ *= v;
        return *this;
    }

    int get_value() const { return value_; }

    // this 用于区分成员和参数
    void setValue(int value) {
        this->value_ = value;  // this-> 区分成员和参数
    }

private:
    int value_;
};

void demo_this_pointer() {
    std::cout << "\n===== this 指针 =====" << std::endl;

    // 链式调用
    Chain ch;
    ch.set_value(5).add(3).multiply(2);
    std::cout << "链式调用结果: " << ch.get_value() << std::endl;

    std::cout << "\nthis 指针要点:" << std::endl;
    std::cout << "  - this 是隐式参数, 指向调用对象" << std::endl;
    std::cout << "  - const 成员函数中 this 是 const 指针" << std::endl;
    std::cout << "  - 返回 *this 实现链式调用" << std::endl;
    std::cout << "  - 用于区分成员和同名参数" << std::endl;
    std::cout << "  - static 成员函数没有 this 指针" << std::endl;
}

int main() {
    std::cout << "========== 成员函数详解 ==========\n" << std::endl;

    demo_const_member_functions();
    demo_static_members();
    demo_friend();
    demo_this_pointer();

    return 0;
}
