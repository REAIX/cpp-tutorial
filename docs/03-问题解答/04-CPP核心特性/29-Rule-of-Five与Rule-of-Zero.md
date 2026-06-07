# Rule of Five 与 Rule of Zero
> 📖 相关章节：[类与对象](../../02-CPP/03-类与对象.md)、[移动语义与完美转发](../../02-CPP/09-移动语义与完美转发.md)

> **📖 本文定位**：聚焦资源管理类的五大特殊成员函数设计，包括 copy-and-swap 惯用法、noexcept 重要性、移动语义
>
> **🔗 相关阅读**：
> - [= default 和 = delete 详解](06-default和delete.md) — 聚焦 =default/=delete 语法本身，包括语法规则、使用场景、与编译器默认行为的关系
> - [三法则与五法则](30-三法则与五法则.md) — 聚焦法则的系统性讲解，包括法则演进（三→五→零）、零法则失效场景、继承中的特殊成员函数、实战检查清单

### 1. 本质速解

**Rule of Five**：如果自定义了析构/拷贝构造/拷贝赋值/移动构造/移动赋值中任何一个，就五个全写。**Rule of Zero**：如果类不直接管理资源，五个全不写，用编译器默认的。

***

### 2. Rule of Three（C++98 时代）

在 C++11 之前，只有三个特殊成员函数需要关注：析构函数、拷贝构造函数、拷贝赋值运算符。

**核心原则**：如果你需要自定义其中任何一个，通常三个都需要自定义。

```cpp
class Buffer {
    int* data_;
    size_t size_;
public:
    Buffer(size_t n) : data_(new int[n]), size_(n) {}

    // 1. 析构函数：需要释放资源
    ~Buffer() { delete[] data_; }

    // 2. 拷贝构造函数：需要深拷贝
    Buffer(const Buffer& other) : data_(new int[other.size_]), size_(other.size_) {
        std::copy(other.data_, other.data_ + size_, data_);
    }

    // 3. 拷贝赋值运算符：需要深拷贝 + 自赋值检查
    Buffer& operator=(const Buffer& other) {
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            data_ = new int[size_];
            std::copy(other.data_, other.data_ + size_, data_);
        }
        return *this;
    }
};
```

**为什么三个都要写**：如果只写了析构函数，编译器生成的默认拷贝构造/拷贝赋值会做浅拷贝，导致双重释放。

```cpp
// 只写了析构，没写拷贝构造 → 灾难
Buffer a(10);
Buffer b = a;  // 浅拷贝：b.data_ = a.data_
// a 析构 → delete[] data_
// b 析构 → delete[] data_  → 双重释放！UB
```

### 3. Rule of Five（C++11 起）

C++11 引入移动语义后，特殊成员函数增加到五个。

| 特殊成员函数 | 作用 |
|------|------|
| 析构函数 | 释放资源 |
| 拷贝构造函数 | 深拷贝构造新对象 |
| 拷贝赋值运算符 | 深拷贝赋值给已有对象 |
| 移动构造函数 | 窃取资源构造新对象 |
| 移动赋值运算符 | 窃取资源赋值给已有对象 |

**完整的 Buffer 类实现（五个特殊成员函数）**：

```cpp
class Buffer {
    int* data_;
    size_t size_;

public:
    // 普通构造函数
    explicit Buffer(size_t n = 0) : data_(n ? new int[n] : nullptr), size_(n) {}

    // 1. 析构函数
    ~Buffer() { delete[] data_; }

    // 2. 拷贝构造函数
    Buffer(const Buffer& other) : data_(other.size_ ? new int[other.size_] : nullptr),
                                   size_(other.size_) {
        if (data_) {
            std::copy(other.data_, other.data_ + size_, data_);
        }
    }

    // 3. 拷贝赋值运算符（copy-and-swap 惯用法）
    Buffer& operator=(const Buffer& other) {
        Buffer tmp(other);           // 拷贝构造临时对象
        swap(*this, tmp);            // 交换
        return *this;                // tmp 析构时释放旧资源
    }

    // 4. 移动构造函数
    Buffer(Buffer&& other) noexcept : data_(other.data_), size_(other.size_) {
        other.data_ = nullptr;
        other.size_ = 0;
    }

    // 5. 移动赋值运算符
    Buffer& operator=(Buffer&& other) noexcept {
        if (this != &other) {
            delete[] data_;          // 释放旧资源
            data_ = other.data_;     // 窃取资源
            size_ = other.size_;
            other.data_ = nullptr;   // 置空源对象
            other.size_ = 0;
        }
        return *this;
    }

    // 辅助：swap 函数
    friend void swap(Buffer& a, Buffer& b) noexcept {
        using std::swap;
        swap(a.data_, b.data_);
        swap(a.size_, b.size_);
    }

    // 其他成员函数
    size_t size() const { return size_; }
    int& operator[](size_t i) { return data_[i]; }
    const int& operator[](size_t i) const { return data_[i]; }
};
```

**为什么移动操作也要写**：如果只写了析构和拷贝，编译器不会自动生成移动操作，对象只能被拷贝，无法利用移动语义优化性能。

```cpp
Buffer createBuffer() {
    Buffer b(1000);
    return b;  // 如果没有移动构造，只能拷贝！
}

vector<Buffer> buffers;
buffers.push_back(Buffer(1000));  // 没有移动构造 → 深拷贝，慢！
```

### 4. Rule of Zero

如果类不直接管理资源（没有裸指针、文件句柄等），就什么都不写，让编译器生成默认实现。

```cpp
#include <memory>  // 智能指针所需头文件
// Rule of Zero：类只使用 RAII 成员
class Student {
    std::string name;           // string 自己管理内存
    std::vector<int> scores;    // vector 自己管理内存
    std::unique_ptr<Record> record;  // unique_ptr 自己管理生命周期
public:
    // 不需要析构、拷贝、移动 — 编译器默认的就够
    // string、vector、unique_ptr 的特殊成员函数会自动被调用

    Student(const std::string& n) : name(n) {}
};

Student s1("Alice");
Student s2 = s1;               // 拷贝构造：string 和 vector 各自深拷贝
Student s3 = std::move(s1);    // 移动构造：string 和 vector 各自移动
// s1 的 name 变为空，scores 变为空，record 变为 nullptr
```

**Rule of Zero 的好处**：

| 好处 | 说明 |
|------|------|
| 代码更少 | 不写五个函数，减少出错可能 |
| 正确性 | 编译器生成的默认实现一定是正确的 |
| 可维护 | 添加新成员时不需要修改特殊成员函数 |
| 异常安全 | 编译器生成的代码天然异常安全 |

### 5. 何时用哪个规则

| 场景 | 规则 | 说明 |
|------|------|------|
| 类直接管理资源（裸指针、文件句柄、socket） | Rule of Five | 必须手动管理资源的获取和释放 |
| 类只使用 RAII 成员 | Rule of Zero | 编译器默认实现即可 |
| 需要禁止拷贝/移动 | Rule of Five（= delete） | 显式声明意图 |
| 类有虚析构函数 | 特殊情况 | 通常不需要其他四个 |
| 包装第三方 C 库句柄 | Rule of Five | 自定义删除器或 RAII 包装 |

**禁止拷贝/移动的写法**：

```cpp
// C++11：= delete
class NonCopyable {
public:
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
    NonCopyable(NonCopyable&&) = delete;
    NonCopyable& operator=(NonCopyable&&) = delete;
};

// C++20：更简洁（只需 delete 拷贝，移动自动被禁止）
class NonCopyable2 {
public:
    NonCopyable2() = default;
    NonCopyable2(const NonCopyable2&) = delete;
    NonCopyable2& operator=(const NonCopyable2&) = delete;
    // 移动操作自动被隐式删除（因为有 delete 的拷贝操作）
};
```

### 6. =default vs 手写

```cpp
class Widget {
    std::string name_;
    int value_;
public:
    // =default：让编译器生成，通常更高效
    Widget() = default;
    Widget(const Widget&) = default;
    Widget& operator=(const Widget&) = default;
    Widget(Widget&&) = default;
    Widget& operator=(Widget&&) = default;
    ~Widget() = default;

    // 手写：需要特殊逻辑时
    Widget(Widget&& other) noexcept
        : name_(std::move(other.name_))
        , value_(other.value_) {
        other.value_ = 0;  // 额外逻辑：重置源对象
    }
};
```

**=default vs 手写对比**：

| 特性 | =default | 手写 |
|------|:---:|:---:|
| 正确性 | 编译器保证 | 程序员保证 |
| 效率 | 通常更优（trivial 优化） | 取决于实现 |
| 可维护 | 添加成员自动处理 | 需要手动更新 |
| 灵活性 | 无额外逻辑 | 可添加自定义逻辑 |
| trivial 属性 | 可能保持 trivial | 一定不是 trivial |

### 7. 拷贝赋值的几种实现方式

**方式1：自赋值检查 + 释放 + 分配**

```cpp
Buffer& operator=(const Buffer& other) {
    if (this != &other) {           // 自赋值检查
        delete[] data_;             // 释放旧资源
        size_ = other.size_;
        data_ = new int[size_];     // 分配新资源
        std::copy(other.data_, other.data_ + size_, data_);
    }
    return *this;
}
// 问题：如果 new 抛异常，data_ 已经被释放，对象处于无效状态
```

**方式2：先分配再释放（异常安全）**

```cpp
Buffer& operator=(const Buffer& other) {
    if (this != &other) {
        int* new_data = new int[other.size_];  // 先分配
        std::copy(other.data_, other.data_ + other.size_, new_data);
        delete[] data_;             // 再释放旧资源
        data_ = new_data;
        size_ = other.size_;
    }
    return *this;
}
// 异常安全，但代码较冗长
```

**方式3：copy-and-swap 惯用法（推荐）**

```cpp
Buffer& operator=(Buffer other) {   // 注意：参数是值传递（拷贝）
    swap(*this, other);              // 交换 this 和 other
    return *this;                    // other 析构时释放旧资源
}
// 优点：异常安全、自赋值安全、代码简洁
// 同时处理拷贝赋值和移动赋值（如果也有移动构造）
```

### 8. 移动操作与 noexcept

移动操作应该标记为 `noexcept`，这对标准容器的性能至关重要。

```cpp
// 没有 noexcept：vector 扩容时不敢用移动
class BadBuffer {
public:
    BadBuffer(BadBuffer&& other);  // 没有 noexcept
};

vector<BadBuffer> vec;
vec.push_back(BadBuffer());  // 扩容时，vector 选择拷贝而不是移动
// 原因：如果移动抛异常，数据会丢失；拷贝虽然慢，但异常安全

// 有 noexcept：vector 扩容时使用移动
class GoodBuffer {
public:
    GoodBuffer(GoodBuffer&& other) noexcept;  // 有 noexcept
};

vector<GoodBuffer> vec2;
vec2.push_back(GoodBuffer());  // 扩容时使用移动，快！
```

**noexcept 对 vector 的影响**：

| 移动构造 | vector 扩容策略 |
|:---:|------|
| noexcept | 使用移动（快） |
| 非 noexcept | 使用拷贝（安全但慢） |
| =default | 如果成员都是 noexcept，自动 noexcept |

### 9. 完整示例：Rule of Five 与 Rule of Zero

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>
using namespace std;

// === Rule of Five：管理资源的类 ===
class FileHandle {
    FILE* file_;
    string path_;
public:
    // 构造
    explicit FileHandle(const string& path, const string& mode = "r")
        : file_(fopen(path.c_str(), mode.c_str())), path_(path) {
        if (!file_) throw runtime_error("Cannot open: " + path);
        cout << "Opened: " << path_ << "\n";
    }

    // 1. 析构
    ~FileHandle() {
        if (file_) {
            fclose(file_);
            cout << "Closed: " << path_ << "\n";
        }
    }

    // 2. 拷贝构造：禁止（文件句柄不应拷贝）
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;

    // 3. 移动构造
    FileHandle(FileHandle&& other) noexcept
        : file_(other.file_), path_(std::move(other.path_)) {
        other.file_ = nullptr;
        cout << "Moved from: " << path_ << "\n";
    }

    // 4. 移动赋值
    FileHandle& operator=(FileHandle&& other) noexcept {
        if (this != &other) {
            if (file_) fclose(file_);     // 释放当前资源
            file_ = other.file_;
            path_ = std::move(other.path_);
            other.file_ = nullptr;
        }
        return *this;
    }

    // 业务方法
    void write(const string& data) {
        if (file_) fprintf(file_, "%s", data.c_str());
    }

    const string& path() const { return path_; }
};

// === Rule of Zero：不管理资源的类 ===
class Document {
    string title_;
    vector<string> paragraphs_;
    shared_ptr<FileHandle> file_;  // 用 shared_ptr 管理，不需要自己写
public:
    Document(const string& title) : title_(title) {}

    void addParagraph(const string& text) {
        paragraphs_.push_back(text);
    }

    // 不需要析构、拷贝、移动 — 编译器默认的就行
    // string、vector、shared_ptr 各自处理自己的资源
};

int main() {
    // Rule of Five：移动语义
    {
        FileHandle f1("test1.txt", "w");
        f1.write("Hello, ");
        FileHandle f2 = std::move(f1);  // 移动构造
        f2.write("World!\n");
        // f1 不再可用
    }

    // Rule of Zero：自动处理
    {
        Document doc("My Doc");
        doc.addParagraph("First paragraph");
        doc.addParagraph("Second paragraph");

        Document doc2 = doc;                  // 深拷贝，自动正确
        Document doc3 = std::move(doc);       // 移动，自动正确
    }

    // vector + 移动语义
    {
        vector<FileHandle> files;
        files.push_back(FileHandle("test2.txt", "w"));  // 移动构造
        files.push_back(FileHandle("test3.txt", "w"));  // 移动构造
        // vector 扩容时使用移动（因为 noexcept）
    }

    return 0;
}
```

### 10. 常见陷阱

**陷阱1：只写析构，不写拷贝操作**

```cpp
class Bad {
    int* data_;
public:
    Bad() : data_(new int[10]) {}
    ~Bad() { delete[] data_; }
    // 没有拷贝构造和拷贝赋值！
};

Bad a;
Bad b = a;  // 浅拷贝 → 双重释放！
```

**陷阱2：移动后使用源对象**

```cpp
Buffer a(100);
Buffer b = std::move(a);  // a 的资源已被窃取
cout << a.size();          // 0（如果移动构造正确置空了源对象）
// a[0] = 42;             // UB！data_ 是 nullptr
```

**陷阱3：移动操作没有 noexcept**

```cpp
class SlowBuffer {
public:
    SlowBuffer(SlowBuffer&& other);  // 没有 noexcept
};

vector<SlowBuffer> vec;
vec.reserve(100);
// 扩容时 vector 选择拷贝而不是移动，因为移动不保证 noexcept
```

**陷阱4：自移动赋值**

```cpp
Buffer& operator=(Buffer&& other) noexcept {
    if (this != &other) {  // 必须检查自赋值！
        delete[] data_;
        data_ = other.data_;
        other.data_ = nullptr;
    }
    return *this;
}

Buffer b(10);
b = std::move(b);  // 如果没有自赋值检查，data_ 会被 delete[] 后再赋值
```

### 11. 极简总结

**Rule of Three = 析构+拷贝构造+拷贝赋值 | Rule of Five = +移动构造+移动赋值 | Rule of Zero = 不管资源就全不写 | 拷贝赋值用 copy-and-swap | 移动操作加 noexcept | 只写一个就五个全写**

***

### 相关阅读

- [智能指针选择](15-智能指针选择.md)
- [智能指针循环引用](16-智能指针循环引用.md)
- [三法则与五法则](30-三法则与五法则.md)