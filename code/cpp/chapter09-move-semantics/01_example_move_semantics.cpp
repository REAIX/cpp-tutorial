/** @file 01_example_move_semantics.cpp
 *  @brief 移动语义基础：左值右值、移动构造、移动赋值、std::move、移动触发条件
 *  @description 对应文档: 09-移动语义与完美转发
 */

#include <algorithm>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

class Buffer {
public:
    Buffer(size_t size) : size_(size), data_(new int[size]()) {
        std::cout << "Buffer(" << size << ") 分配内存\n";
    }

    Buffer(const Buffer& other) : size_(other.size_), data_(new int[other.size_]) {
        std::copy(other.data_, other.data_ + size_, data_);
        std::cout << "Buffer 拷贝构造, 大小=" << size_ << "\n";
    }

    Buffer(Buffer&& other) noexcept : size_(other.size_), data_(other.data_) {
        other.size_ = 0;
        other.data_ = nullptr;
        std::cout << "Buffer 移动构造, 大小=" << size_ << "\n";
    }

    Buffer& operator=(const Buffer& other) {
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            data_ = new int[size_];
            std::copy(other.data_, other.data_ + size_, data_);
            std::cout << "Buffer 拷贝赋值, 大小=" << size_ << "\n";
        }
        return *this;
    }

    Buffer& operator=(Buffer&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            data_ = other.data_;
            other.size_ = 0;
            other.data_ = nullptr;
            std::cout << "Buffer 移动赋值, 大小=" << size_ << "\n";
        }
        return *this;
    }

    ~Buffer() {
        delete[] data_;
        std::cout << "Buffer 析构, 大小=" << size_ << "\n";
    }

    size_t size() const { return size_; }
    int* data() const { return data_; }
private:
    size_t size_ = 0;
    int* data_ = nullptr;
};

void demo_lvalue_rvalue() {
    std::cout << "=== 左值与右值 ===\n";

    int a = 10;
    int b = 20;
    int c = a + b;

    std::cout << "a = " << a << "  (a 是左值, 有名字, 可取地址)\n";
    std::cout << "a + b = " << a + b << "  (a+b 是右值, 临时值, 不可取地址)\n";
    std::cout << "c = " << c << "  (c 是左值)\n";

    int& lr = a;
    std::cout << "\n左值引用 int& lr = a; lr = " << lr << "\n";

    int&& rr = 42;
    std::cout << "右值引用 int&& rr = 42; rr = " << rr << "\n";

    int&& rr2 = a + b;
    std::cout << "右值引用 int&& rr2 = a+b; rr2 = " << rr2 << "\n";

    std::cout << "\n左值 vs 右值:\n";
    std::cout << "  左值: 有名字, 可取地址, 生命周期超出表达式\n";
    std::cout << "  右值: 无名字, 不可取地址, 临时存在\n";
    std::cout << "  右值引用: 延长临时对象的生命周期\n";

    std::cout << "\n";
}

void demo_move_constructor() {
    std::cout << "=== 移动构造函数 ===\n";

    Buffer buf1(1000);
    std::cout << "buf1 大小: " << buf1.size() << "\n";

    std::cout << "\n拷贝构造:\n";
    Buffer buf2 = buf1;
    std::cout << "buf2 大小: " << buf2.size() << "\n";

    std::cout << "\n移动构造:\n";
    Buffer buf3 = std::move(buf1);
    std::cout << "buf3 大小: " << buf3.size() << "\n";
    std::cout << "buf1 移动后大小: " << buf1.size() << " (资源已转移)\n";

    std::cout << "\n移动构造 vs 拷贝构造:\n";
    std::cout << "  拷贝: 分配新内存 + 复制数据 (深拷贝)\n";
    std::cout << "  移动: 转移资源所有权 (浅拷贝 + 置空源对象)\n";

    std::cout << "\n";
}

void demo_move_assignment() {
    std::cout << "=== 移动赋值运算符 ===\n";

    Buffer buf1(500);
    Buffer buf2(200);

    std::cout << "\n移动赋值:\n";
    buf2 = std::move(buf1);
    std::cout << "buf2 大小: " << buf2.size() << "\n";
    std::cout << "buf1 移动后大小: " << buf1.size() << "\n";

    std::cout << "\n移动赋值的步骤:\n";
    std::cout << "  1. 释放自身资源\n";
    std::cout << "  2. 窃取对方资源\n";
    std::cout << "  3. 将对方置空\n";

    std::cout << "\n";
}

void demo_std_move() {
    std::cout << "=== std::move 详解 ===\n";

    std::string str = "Hello, Move Semantics!";
    std::cout << "原始字符串: " << str << "\n";

    std::string moved = std::move(str);
    std::cout << "移动后 moved: " << moved << "\n";
    std::cout << "移动后 str: \"" << str << "\" (有效但未指定状态)\n";

    std::cout << "\nstd::move 的本质:\n";
    std::cout << "  std::move 并不移动任何东西!\n";
    std::cout << "  它只是一个类型转换: 将左值转为右值引用\n";
    std::cout << "  真正的移动由移动构造/赋值完成\n";

    std::cout << "\nstd::move 后源对象的状态:\n";
    std::cout << "  对象处于'有效但未指定'(valid but unspecified)状态\n";
    std::cout << "  可以安全地: 析构、赋新值\n";
    std::cout << "  不应该: 假设其值, 直接使用\n";

    std::cout << "\n";
}

void demo_when_move_kicks_in() {
    std::cout << "=== 移动何时触发 ===\n";

    std::cout << "1. 显式使用 std::move:\n";
    {
        std::vector<int> v1 = {1, 2, 3, 4, 5};
        std::vector<int> v2 = std::move(v1);
        std::cout << "   v2 大小: " << v2.size() << ", v1 大小: " << v1.size() << "\n";
    }

    std::cout << "\n2. 返回局部对象 (编译器自动优化):\n";
    {
        auto make_buffer = []() -> Buffer {
            Buffer b(100);
            return b;
        };
        Buffer buf = make_buffer();
        std::cout << "   返回值可能被 RVO 优化, 不触发移动\n";
    }

    std::cout << "\n3. 临时对象绑定到右值引用:\n";
    {
        Buffer buf = Buffer(200);
        std::cout << "   临时对象触发移动构造 (或被 RVO 优化)\n";
    }

    std::cout << "\n4. 容器插入右值:\n";
    {
        std::vector<std::string> vec;
        std::string s = "hello";
        vec.push_back(s);
        std::cout << "   push_back(左值): 触发拷贝\n";
        vec.push_back(std::move(s));
        std::cout << "   push_back(std::move): 触发移动\n";
    }

    std::cout << "\n5. 容器重新分配:\n";
    {
        std::vector<Buffer> buffers;
        buffers.reserve(2);
        buffers.emplace_back(10);
        buffers.emplace_back(20);
        std::cout << "   容量已满, 再添加时:\n";
        buffers.emplace_back(30);
        std::cout << "   容器扩容时会移动已有元素\n";
    }

    std::cout << "\n";
}

int main() {
    demo_lvalue_rvalue();
    demo_move_constructor();
    demo_move_assignment();
    demo_std_move();
    demo_when_move_kicks_in();

    return 0;
}
