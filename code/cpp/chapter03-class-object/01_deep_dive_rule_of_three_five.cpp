/** @file 01_deep_dive_rule_of_three_five.cpp
 *  @brief 三之法则、五之法则、零之法则、拷贝/移动语义
 *  @description 对应文档: 02-CPP/03-class-object
 */

#include <iostream>
#include <string>
#include <utility>
#include <vector>

// ===== 1. 三之法则 (Rule of Three) =====
// 如果类需要自定义析构函数、拷贝构造函数或拷贝赋值运算符之一,
// 那么很可能三个都需要自定义

class Buffer {
public:
    explicit Buffer(size_t size)
        : size_(size), data_(new int[size]()) {
        std::cout << "  Buffer(" << size_ << ") 分配" << std::endl;
    }

    // 1. 析构函数
    ~Buffer() {
        delete[] data_;
        std::cout << "  ~Buffer(" << size_ << ") 释放" << std::endl;
    }

    // 2. 拷贝构造函数 (深拷贝)
    Buffer(const Buffer& other)
        : size_(other.size_), data_(new int[other.size_]) {
        std::copy(other.data_, other.data_ + size_, data_);
        std::cout << "  Buffer 拷贝构造, 大小=" << size_ << std::endl;
    }

    // 3. 拷贝赋值运算符 (深拷贝 + 自赋值检查)
    Buffer& operator=(const Buffer& other) {
        if (this != &other) {  // 自赋值检查
            delete[] data_;     // 释放旧资源
            size_ = other.size_;
            data_ = new int[size_];
            std::copy(other.data_, other.data_ + size_, data_);
            std::cout << "  Buffer 拷贝赋值, 大小=" << size_ << std::endl;
        }
        return *this;
    }

    int& operator[](size_t i) { return data_[i]; }
    int operator[](size_t i) const { return data_[i]; }
    size_t size() const { return size_; }

private:
    size_t size_;
    int* data_;
};

void demo_rule_of_three() {
    std::cout << "===== 三之法则 =====" << std::endl;

    Buffer b1(5);
    for (size_t i = 0; i < b1.size(); ++i) b1[i] = static_cast<int>(i * 10);

    Buffer b2 = b1;  // 拷贝构造
    std::cout << "  b2[2] = " << b2[2] << std::endl;

    Buffer b3(3);
    b3 = b1;  // 拷贝赋值
    std::cout << "  b3[2] = " << b3[2] << std::endl;

    // 如果不自定义拷贝操作, 默认的浅拷贝会导致:
    // - 两个对象指向同一块内存
    // - 析构时 double free
    // - 修改一个影响另一个

    std::cout << "\n三之法则: 需要自定义一个, 就需要自定义全部三个" << std::endl;
    std::cout << "  - 析构函数" << std::endl;
    std::cout << "  - 拷贝构造函数" << std::endl;
    std::cout << "  - 拷贝赋值运算符" << std::endl;
}

// ===== 2. 五之法则 (Rule of Five) =====
// C++11 扩展: 加上移动构造函数和移动赋值运算符

class FastBuffer {
public:
    explicit FastBuffer(size_t size)
        : size_(size), data_(new int[size]()) {
        std::cout << "  FastBuffer(" << size_ << ") 分配" << std::endl;
    }

    ~FastBuffer() {
        delete[] data_;
        std::cout << "  ~FastBuffer(" << size_ << ") 释放" << std::endl;
    }

    // 拷贝构造 (深拷贝)
    FastBuffer(const FastBuffer& other)
        : size_(other.size_), data_(new int[other.size_]) {
        std::copy(other.data_, other.data_ + size_, data_);
        std::cout << "  FastBuffer 拷贝构造" << std::endl;
    }

    // 拷贝赋值
    FastBuffer& operator=(const FastBuffer& other) {
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            data_ = new int[size_];
            std::copy(other.data_, other.data_ + size_, data_);
            std::cout << "  FastBuffer 拷贝赋值" << std::endl;
        }
        return *this;
    }

    // 移动构造 (窃取资源)
    FastBuffer(FastBuffer&& other) noexcept
        : size_(other.size_), data_(other.data_) {
        other.size_ = 0;
        other.data_ = nullptr;
        std::cout << "  FastBuffer 移动构造" << std::endl;
    }

    // 移动赋值 (窃取资源)
    FastBuffer& operator=(FastBuffer&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            data_ = other.data_;
            other.size_ = 0;
            other.data_ = nullptr;
            std::cout << "  FastBuffer 移动赋值" << std::endl;
        }
        return *this;
    }

    int& operator[](size_t i) { return data_[i]; }
    size_t size() const { return size_; }

private:
    size_t size_;
    int* data_;
};

FastBuffer create_buffer() {
    FastBuffer buf(1000);
    for (size_t i = 0; i < buf.size(); ++i) buf[i] = static_cast<int>(i);
    return buf;  // NRVO 或移动语义
}

void demo_rule_of_five() {
    std::cout << "\n===== 五之法则 =====" << std::endl;

    FastBuffer fb1(5);
    for (size_t i = 0; i < fb1.size(); ++i) fb1[i] = static_cast<int>(i * 100);

    FastBuffer fb2 = std::move(fb1);  // 移动构造
    std::cout << "  fb2[2] = " << fb2[2] << " (从 fb1 移动)" << std::endl;

    FastBuffer fb3(3);
    fb3 = std::move(fb2);  // 移动赋值
    std::cout << "  fb3[2] = " << fb3[2] << " (从 fb2 移动)" << std::endl;

    FastBuffer fb4 = create_buffer();  // 移动构造或 NRVO
    std::cout << "  fb4[0] = " << fb4[0] << std::endl;

    std::cout << "\n五之法则: 三之法则 + 移动构造 + 移动赋值" << std::endl;
    std::cout << "  - 移动操作应标记 noexcept" << std::endl;
    std::cout << "  - 移动后源对象应处于有效但未指定状态" << std::endl;
    std::cout << "  - 移动比拷贝高效: 窃取资源而非复制" << std::endl;
}

// ===== 3. 零之法则 (Rule of Zero) =====
// 最佳实践: 如果类不需要自定义任何特殊成员函数,
// 就不要自定义, 使用编译器生成的默认版本

class SimpleStudent {
public:
    SimpleStudent(const std::string& name, int age)
        : name_(name), age_(age) {}

    // 不需要自定义析构函数
    // 不需要自定义拷贝/移动操作
    // 编译器生成的版本完全正确!

    void display() const {
        std::cout << "  " << name_ << ", " << age_ << " 岁" << std::endl;
    }

private:
    std::string name_;  // string 自身管理资源
    int age_;
};

class SimpleClass {
public:
    SimpleClass(const std::string& name) : name_(name), scores_() {}

    void add_score(int s) { scores_.push_back(s); }

    // 零之法则: 不需要任何特殊成员函数
    // string 和 vector 的拷贝/移动/析构都是正确的

private:
    std::string name_;
    std::vector<int> scores_;
};

void demo_rule_of_zero() {
    std::cout << "\n===== 零之法则 =====" << std::endl;

    SimpleStudent s1("张三", 20);
    SimpleStudent s2 = s1;  // 编译器生成的拷贝构造, 正确
    SimpleStudent s3 = std::move(s1);  // 编译器生成的移动构造, 正确
    s2.display();
    s3.display();

    std::cout << "\n零之法则: 能不写就不写" << std::endl;
    std::cout << "  - 使用标准库类型管理资源" << std::endl;
    std::cout << "  - 让编译器生成默认的特殊成员函数" << std::endl;
    std::cout << "  - 代码更简洁, 更不容易出错" << std::endl;
    std::cout << "  - 如果需要自定义, 考虑用组合/智能指针代替" << std::endl;
}

// ===== 4. 举一反三: 拷贝/移动语义陷阱 =====
void demo_copy_move_pitfalls() {
    std::cout << "\n===== 举一反三: 拷贝/移动语义陷阱 =====" << std::endl;

    // 陷阱1: 移动后的对象仍在使用
    FastBuffer buf(5);
    for (size_t i = 0; i < buf.size(); ++i) buf[i] = static_cast<int>(i);
    FastBuffer moved_to = std::move(buf);
    // buf 此时处于"有效但未指定"状态
    // std::cout << buf[0];  // 危险! buf 可能已被清空
    std::cout << "陷阱1: 移动后的对象不应再使用" << std::endl;

    // 陷阱2: 拷贝赋值中的自赋值
    std::cout << "陷阱2: 拷贝赋值必须处理自赋值 (a = a)" << std::endl;
    std::cout << "  解决: 先检查 this != &other, 或 copy-and-swap 惯用法" << std::endl;

    // 陷阱3: 移动操作没有 noexcept
    std::cout << "陷阱3: 移动操作应标记 noexcept" << std::endl;
    std::cout << "  - vector 扩容时, 如果移动构造不是 noexcept, 会使用拷贝构造" << std::endl;
    std::cout << "  - noexcept 移动比拷贝更高效" << std::endl;

    // 陷阱4: 只声明移动操作会抑制拷贝
    std::cout << "陷阱4: 声明移动操作会抑制默认拷贝操作" << std::endl;
    std::cout << "  - 声明移动构造: 拷贝构造被隐式删除" << std::endl;
    std::cout << "  - 声明移动赋值: 拷贝赋值被隐式删除" << std::endl;
    std::cout << "  - 反之亦然" << std::endl;

    // 陷阱5: 返回值优化 (RVO/NRVO)
    std::cout << "陷阱5: 不要对返回值使用 std::move" << std::endl;
    std::cout << "  - return std::move(local); 阻止 NRVO!" << std::endl;
    std::cout << "  - return local; 允许 NRVO 或自动移动" << std::endl;
}

int main() {
    std::cout << "========== 三五零法则 ==========\n" << std::endl;

    demo_rule_of_three();
    demo_rule_of_five();
    demo_rule_of_zero();
    demo_copy_move_pitfalls();

    return 0;
}
