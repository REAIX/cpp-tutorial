# = default 和 = delete 详解
> 📖 相关章节：[类与对象](../../02-CPP/03-类与对象.md)、[核心机制](../../02-CPP/05-核心机制.md)

> **📖 本文定位**：聚焦 =default/=delete 语法本身，包括语法规则、使用场景、与编译器默认行为的关系
>
> **🔗 相关阅读**：
> - [Rule of Five 与 Rule of Zero](29-Rule-of-Five与Rule-of-Zero.md) — 聚焦资源管理类的五大特殊成员函数设计，包括 copy-and-swap 惯用法、noexcept 重要性、移动语义
> - [三法则与五法则](30-三法则与五法则.md) — 聚焦法则的系统性讲解，包括法则演进（三→五→零）、零法则失效场景、继承中的特殊成员函数、实战检查清单

### 1. 一句话结论

**`= default`** **告诉编译器：帮我生成默认实现**
**`= delete`** **告诉编译器：这个函数彻底禁用，不能调用**

两者都是 **C++11** 引入的特殊成员函数修饰符，用于精确控制类的特殊成员函数行为。

***

### 2. = default — 显式要求编译器生成默认实现

#### 1. 核心作用

告诉编译器：**"请为我生成默认的实现"**

#### 2. 场景1：在 .cpp 文件中生成默认构造函数（提高编译速度）

```cpp
// MyClass.h
class MyClass {
public:
    MyClass();  // 声明
    ~MyClass();
private:
    int data;
};

// MyClass.cpp
// 方式1：手动写（旧方式）
MyClass::MyClass() : data(0) {}

// 方式2：使用 = default（推荐）
MyClass::MyClass() = default;  // 更简洁！

// 析构函数也可以
MyClass::~MyClass() = default;
```

**好处**：

- 代码更简洁
- 头文件改动不会导致所有包含该头文件的文件重新编译

#### 3. 场景2：控制默认成员函数的访问权限

```cpp
class ControlAccess {
public:
    // 允许外部调用默认构造
    ControlAccess() = default;

private:
    // 禁止拷贝，但让编译器生成实现（友元可以调用）
    ControlAccess(const ControlAccess&) = default;
    ControlAccess& operator=(const ControlAccess&) = default;

    int value;
};
```

#### 4. 场景3：有自定义构造函数时仍需要默认构造

```cpp
class Student {
public:
    // 自定义构造函数
    Student(std::string name, int age)
        : name_(name), age_(age) {}

    // 编译器不再自动生成默认构造！
    // 但我们可以显式要求它生成：
    Student() = default;  // 关键！

private:
    std::string name_;
    int age_ = 0;         // 类内初始化
};

int main() {
    Student s1("Alice", 20);  // 用自定义构造
    Student s2;               // 用默认构造 OK
}
```

#### 5. 场景4：移动操作（C++11 重要应用）

```cpp
class Resource {
public:
    Resource() : data(new int[1000]) {}

    // 拷贝操作 - 需要深拷贝
    Resource(const Resource& other);
    Resource& operator=(const Resource& other);

    // 移动操作 - 使用默认实现
    Resource(Resource&& other) = default;
    Resource& operator=(Resource&& other) = default;

    ~Resource() { delete[] data; }

private:
    int* data;
};
```

***

### 3. = delete — 显式禁止某个函数

#### 1. 核心作用

告诉编译器和用户：**"这个函数不存在，不能调用"**

#### 2. 场景1：禁止拷贝语义（单例模式、资源类）

```cpp
// ===== 旧方式（C++11之前）=====
class OldSingleton {
public:
    static OldSingleton& getInstance() {
        static OldSingleton instance;
        return instance;
    }

private:
    OldSingleton() {}

    // 声明但不实现（运行时链接错误）
    OldSingleton(const OldSingleton&);
    OldSingleton& operator=(const OldSingleton&);
};

// ===== 新方式（C++11及以后，推荐）=====
class Singleton {
public:
    static Singleton& getInstance() {
        static Singleton instance;
        return instance;
    }

    // 删除拷贝操作
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

    // 删除移动操作（可选）
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;

private:
    Singleton() = default;  // 默认构造保留但私有化
};
```

**`delete` 的优势**：

- **编译期检查**（而不是链接期报错）
- **意图更明确**
- 可以是 public 的，错误信息更清晰

#### 3. 场景2：禁止特定参数类型（防止隐式转换）

```cpp
class PrecisionNumber {
public:
    explicit PrecisionNumber(double value) : value_(value) {}

    void process(int x) {
        printf("Processing integer: %d\n", x);
    }

    // 禁止用 double 调用（防止精度丢失）
    void process(double) = delete;

private:
    double value_;
};

int main() {
    PrecisionNumber num(3.14);

    num.process(42);       // OK
    num.process(3.14);     // 编译错误！已删除

    return 0;
}
```

#### 4. 场景3：禁止动态分配（new/delete）

```cpp
class StackOnly {
public:
    StackOnly() = default;

    // 禁止在堆上创建
    void* operator new(size_t) = delete;
    void* operator new[](size_t) = delete;
    void operator delete(void*) = delete;
    void operator delete[](void*) = delete;

private:
    int buffer[1024];
};

int main() {
    StackOnly s1;           // 栈上分配 OK
    StackOnly* s2 = new StackOnly();  // 编译错误！

    return 0;
}
```

#### 5. 场景4：特定模板实例化的禁用

```cpp
template<typename T>
class Container {
public:
    void add(const T& value) {
        // 添加逻辑
    }

    // 禁止添加指针类型（防止内存管理混乱）
    void add(T*) = delete;
};

int main() {
    Container<int> c;
    c.add(42);          // OK
    int* ptr = nullptr;
    c.add(ptr);         // 错误！已删除

    return 0;
}
```

***

### 4. 可修饰的函数列表

| 特殊成员函数 | `= default` | `= delete` |
|:-----------:|:-----------:|:----------:|
| 默认构造函数 | ✅ | ✅ |
| 析构函数 | ✅ | ✅（极少用） |
| 拷贝构造函数 | ✅ | ✅ |
| 拷贝赋值运算符 | ✅ | ✅ |
| 移动构造函数 (C++11) | ✅ | ✅ |
| 移动赋值运算符 (C++11) | ✅ | ✅ |
| 普通成员函数 | ❌ | ✅ |
| 运算符重载 | ❌ | ✅ |
| operator new/delete | ❌ | ✅ |

***

### 5. Rule of Zero / Five

#### 1. Rule of Zero（推荐）

如果类不需要自己管理资源，让编译器处理一切：

```cpp
class Person {
public:
    // 所有特殊成员函数都用默认
    Person() = default;
    Person(const std::string& name, int age)
        : name_(name), age_(age) {}

    // 特殊成员全部 = default
    ~Person() = default;
    Person(const Person&) = default;
    Person& operator=(const Person&) = default;
    Person(Person&&) = default;
    Person& operator=(Person&&) = default;

private:
    std::string name_;
    int age_ = 0;
};
```

#### 2. Rule of Five

如果需要自定义析构/拷贝，通常五个都要定义：

```cpp
class ManagedResource {
public:
    ManagedResource(size_t size)
        : size_(size), data_(new int[size]) {}

    // 1. 析构
    ~ManagedResource() {
        delete[] data_;
    }

    // 2. 拷贝构造
    ManagedResource(const ManagedResource& other)
        : size_(other.size_), data_(new int[other.size_]) {
        std::copy(other.data_, other.data_ + size_, data_);
    }

    // 3. 拷贝赋值
    ManagedResource& operator=(const ManagedResource& other) {
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            data_ = new int[size_];
            std::copy(other.data_, other.data_ + size_, data_);
        }
        return *this;
    }

    // 4. 移动构造
    ManagedResource(ManagedResource&& other) noexcept
        : size_(other.size_), data_(other.data_) {
        other.data_ = nullptr;
        other.size_ = 0;
    }

    // 5. 移动赋值
    ManagedResource& operator=(ManagedResource&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            data_ = other.data_;
            other.data_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    // 或者禁止拷贝，只允许移动
    // ManagedResource(const ManagedResource&) = delete;
    // ManagedResource& operator=(const ManagedResource&) = delete;

private:
    int* data_;
    size_t size_;
};
```

***

### 6. 重要注意事项

#### 1. `= default` 的条件限制

```cpp
class BadExample {
    // const 成员变量 → 不能 = default 拷贝赋值
    const int id;

    // 引用成员 → 不能 = default 拷贝赋值
    int& ref;

    BadExample& operator=(const BadExample&) = default;  // 错误！
};
```

#### 2. 访问说明符的影响

```cpp
class AccessControl {
public:
    AccessControl() = default;           // 公开可用

private:
    AccessControl(const AccessControl&) = default;  // 只有友元/内部可用

public:
    AccessControl& operator=(const AccessAccess&) = delete;  // 完全禁用
};
```

***

### 7. 最佳实践总结

#### 1. 何时使用 `= default`？

| 场景 | 示例 |
|------|------|
| 在 cpp 文件中实现简单构造函数 | `MyClass::MyClass() = default;` |
| 有自定义构造但仍需默认构造 | `Student() = default;` |
| 需要移动操作但无需自定义 | `MyClass(MyClass&&) = default;` |
| 控制访问权限 | private 区域放 `= default` |

#### 2. 何时使用 `= delete`？

| 场景 | 示例 |
|------|------|
| 单例模式 / 禁止拷贝 | `Singleton(const Singleton&) = delete;` |
| 只允许栈分配 | `void* operator new(size_t) = delete;` |
| 防止隐式类型转换 | `void foo(double) = delete;` |
| 禁止某些参数类型 | `void process(T*) = delete;` |

***

### 8. 一句话记忆口诀

- **`= default`**：编译器你帮我写（我要默认版本）
- **`= delete`**：这个函数别想用（我彻底不要它）

***

### 相关阅读

- [Rule-of-Five与Rule-of-Zero](29-Rule-of-Five与Rule-of-Zero.md)
- [三法则与五法则](30-三法则与五法则.md)
- [构造函数成员初始化列表](03-构造函数成员初始化列表.md)