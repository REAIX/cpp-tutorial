/** @file 03_example_friend.cpp
 *  @brief 友元函数、友元类、友元与封装
 *  @description 对应文档: 02-CPP/06-operator-overload
 */

#include <iostream>
#include <string>
#include <vector>

// ===== 1. 友元函数 =====
class Matrix {
public:
    Matrix(int rows, int cols) : rows_(rows), cols_(cols), data_(rows * cols, 0) {}

    int& at(int r, int c) { return data_[r * cols_ + c]; }
    int at(int r, int c) const { return data_[r * cols_ + c]; }

    int rows() const { return rows_; }
    int cols() const { return cols_; }

    // 友元函数: 可以访问私有成员
    friend Matrix operator*(const Matrix& a, const Matrix& b);
    friend std::ostream& operator<<(std::ostream& os, const Matrix& m);
    friend void scale_matrix(Matrix& m, int factor);

private:
    int rows_;
    int cols_;
    std::vector<int> data_;
};

Matrix operator*(const Matrix& a, const Matrix& b) {
    if (a.cols_ != b.rows_) {
        throw std::invalid_argument("矩阵维度不匹配");
    }
    Matrix result(a.rows_, b.cols_);
    for (int i = 0; i < a.rows_; ++i) {
        for (int j = 0; j < b.cols_; ++j) {
            int sum = 0;
            for (int k = 0; k < a.cols_; ++k) {
                sum += a.data_[i * a.cols_ + k] * b.data_[k * b.cols_ + j];
            }
            result.data_[i * b.cols_ + j] = sum;
        }
    }
    return result;
}

std::ostream& operator<<(std::ostream& os, const Matrix& m) {
    for (int r = 0; r < m.rows_; ++r) {
        os << "  [";
        for (int c = 0; c < m.cols_; ++c) {
            if (c > 0) os << ", ";
            os << m.data_[r * m.cols_ + c];
        }
        os << "]" << std::endl;
    }
    return os;
}

void scale_matrix(Matrix& m, int factor) {
    for (auto& elem : m.data_) {
        elem *= factor;
    }
}

void demo_friend_function() {
    std::cout << "===== 友元函数 =====" << std::endl;

    Matrix a(2, 3);
    Matrix b(3, 2);

    int val = 1;
    for (int r = 0; r < 2; ++r)
        for (int c = 0; c < 3; ++c)
            a.at(r, c) = val++;
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 2; ++c)
            b.at(r, c) = val++;

    std::cout << "矩阵 A:" << std::endl << a;
    std::cout << "矩阵 B:" << std::endl << b;

    Matrix c = a * b;
    std::cout << "A * B:" << std::endl << c;

    scale_matrix(a, 2);
    std::cout << "A 缩放 2x:" << std::endl << a;

    std::cout << "\n友元函数的典型用途:" << std::endl;
    std::cout << "  1. 运算符重载 (<<, >>, 二元运算符)" << std::endl;
    std::cout << "  2. 需要访问两个类的私有成员的函数" << std::endl;
    std::cout << "  3. 工厂函数" << std::endl;
}

// ===== 2. 友元类 =====
class Engine {
public:
    Engine(int horsepower) : horsepower_(horsepower), running_(false) {}

private:
    int horsepower_;
    bool running_;

    // Car 是 Engine 的友元类, 可以访问 Engine 的私有成员
    friend class Car;
};

class Car {
public:
    Car(const std::string& model, Engine engine)
        : model_(model), engine_(engine) {}

    void start() {
        engine_.running_ = true;  // 友元类可以访问私有成员
        std::cout << "  " << model_ << " (" << engine_.horsepower_
                  << "HP) 启动" << std::endl;
    }

    void stop() {
        engine_.running_ = false;
        std::cout << "  " << model_ << " 停止" << std::endl;
    }

private:
    std::string model_;
    Engine engine_;
};

void demo_friend_class() {
    std::cout << "\n===== 友元类 =====" << std::endl;

    Car car("特斯拉 Model 3", 283);
    car.start();
    car.stop();

    std::cout << "\n友元类要点:" << std::endl;
    std::cout << "  - 友元类可以访问授予友元的类的所有成员" << std::endl;
    std::cout << "  - 友元关系是单向的: A 是 B 的友元, B 不是 A 的友元" << std::endl;
    std::cout << "  - 友元关系不传递: A 是 B 的友元, B 是 C 的友元, A 不是 C 的友元" << std::endl;
    std::cout << "  - 友元关系不继承: A 是 B 的友元, A 不是 B 的派生类的友元" << std::endl;
}

// ===== 3. 友元与封装 =====
class BankAccount {
public:
    BankAccount(const std::string& owner, double balance)
        : owner_(owner), balance_(balance) {}

    // 只暴露必要的公共接口
    double get_balance() const { return balance_; }
    const std::string& get_owner() const { return owner_; }

    void deposit(double amount) {
        if (amount > 0) balance_ += amount;
    }

    void withdraw(double amount) {
        if (amount > 0 && amount <= balance_) balance_ -= amount;
    }

    // 友元: 审计员需要完全访问权限
    // 这是友元的合理使用: 特定角色需要特殊权限
    friend class Auditor;

    // 友元函数: 转账需要访问两个账户
    friend void transfer(BankAccount& from, BankAccount& to, double amount);

private:
    std::string owner_;
    double balance_;
};

class Auditor {
public:
    static void audit(const BankAccount& account) {
        std::cout << "  审计: " << account.owner_
                  << " 余额 " << account.balance_ << std::endl;
    }

    static void force_set_balance(BankAccount& account, double balance) {
        account.balance_ = balance;  // 友元类可以访问私有成员
    }
};

void transfer(BankAccount& from, BankAccount& to, double amount) {
    if (from.balance_ >= amount) {
        from.balance_ -= amount;
        to.balance_ += amount;
        std::cout << "  转账 " << amount << ": "
                  << from.owner_ << " -> " << to.owner_ << std::endl;
    }
}

void demo_friend_and_encapsulation() {
    std::cout << "\n===== 友元与封装 =====" << std::endl;

    BankAccount a1("张三", 1000.0);
    BankAccount a2("李四", 500.0);

    transfer(a1, a2, 200.0);
    std::cout << "  张三余额: " << a1.get_balance() << std::endl;
    std::cout << "  李四余额: " << a2.get_balance() << std::endl;

    Auditor::audit(a1);
    Auditor::force_set_balance(a1, 9999.0);
    Auditor::audit(a1);

    std::cout << "\n友元与封装的平衡:" << std::endl;
    std::cout << "  友元不破坏封装, 友元是类的接口的一部分" << std::endl;
    std::cout << "  友元声明是类对'谁可以访问私有成员'的明确控制" << std::endl;
    std::cout << "  比让成员 public 更好: 限制了访问范围" << std::endl;

    std::cout << "\n友元使用原则:" << std::endl;
    std::cout << "  1. 优先使用公共接口, 而非友元" << std::endl;
    std::cout << "  2. 友元用于: 运算符重载, 紧密协作的类" << std::endl;
    std::cout << "  3. 最小化友元数量" << std::endl;
    std::cout << "  4. 友元声明应与类定义一起维护" << std::endl;
}

int main() {
    std::cout << "========== 友元详解 ==========\n" << std::endl;

    demo_friend_function();
    demo_friend_class();
    demo_friend_and_encapsulation();

    return 0;
}
