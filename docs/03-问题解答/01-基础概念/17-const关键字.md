# const 关键字完全指南
> 📖 相关章节：[数据类型与变量](../../01-C语言/01-数据类型与变量.md)、[指针](../../01-C语言/06-指针.md)、[核心机制](../../02-CPP/05-核心机制.md)

### 1. 本质洞察

**const** = 只读、不可修改。在 C/C++ 中有 **六种用法**，核心原则：**const 修饰它左边的东西，如果左边没有就修饰右边**。

***

### 2. 六种用法一览

| 用法 | 示例 | 含义 |
|------|------|------|
| const 变量 | `const int x = 10;` | x 不可修改 |
| const 指针（指向常量） | `const int* p;` | *p 不可修改，p 可改 |
| const 指针（指针常量） | `int* const p;` | p 不可修改，*p 可改 |
| 双 const 指针 | `const int* const p;` | p 和 *p 都不可改 |
| const 引用 | `const int& r;` | r 不可修改指向的值 |
| const 成员函数 | `void foo() const;` | 不修改成员变量 |

### 3. const 指针的阅读技巧

**从右往左读**：

```cpp
const int* p;        // p is a pointer to int const → 指向常量的指针
int* const p;        // p is a const pointer to int → 常量指针
const int* const p;  // p is a const pointer to int const → 都不可改
```

#### 1. 更多阅读示例

```cpp
const char* const* const p;
// 从右往左：p is a const pointer to (const pointer to (const char))
// p 不可改，*p 不可改，**p 不可改
```

### 4. const 成员函数

```cpp
class Widget {
    int value_;
public:
    int getValue() const { return value_; }  // 承诺不修改成员
    void setValue(int v) { value_ = v; }     // 非 const，可能修改
};
```

**规则**：
- const 对象只能调用 const 成员函数
- const 成员函数不能调用非 const 成员函数
- const 成员函数不能修改成员变量（mutable 除外）

#### 1. const 和 non-const 重载

```cpp
class Array {
    int data_[100];
public:
    // 非 const 版本：返回可修改的引用
    int& operator[](size_t i) { return data_[i]; }

    // const 版本：返回只读引用
    const int& operator[](size_t i) const { return data_[i]; }
};

void use() {
    Array arr;
    arr[0] = 42;            // 调非 const 版本

    const Array& carr = arr;
    int x = carr[0];        // 调 const 版本
    // carr[0] = 42;        // 编译错误：不能通过 const 引用修改
}
```

### 5. const 与 mutable

```cpp
class Cache {
    mutable int access_count_;  // mutable 允许在 const 函数中修改
    int data_;
public:
    int getData() const {
        ++access_count_;  // OK，mutable
        return data_;
    }
};
```

**mutable 的合理使用场景**：

| 场景 | 说明 |
|:---|:---|
| 统计/计数 | 记录 const 函数调用次数 |
| 缓存 | 惰性求值，缓存计算结果 |
| 互斥锁 | 在 const 函数中锁定（mutex 通常声明为 mutable） |
| 调试标记 | 不影响逻辑状态的调试信息 |

### 6. const 与函数参数/返回值

```cpp
// 参数：避免拷贝 + 防止修改
void print(const std::string& s);

// 返回值：防止调用者修改
const std::string& getName() const;

// const 值返回（基本类型没什么意义）
const int getValue();  // 基本类型按值返回，加 const 无实际效果
```

#### 1. 常见 const 参数模式

| 写法 | 含义 | 推荐场景 |
|:---|:---|:---|
| `void f(const T&)` | 引用 + 只读 | 大对象的参数传递 |
| `void f(T)` | 值拷贝 | 小对象或需要修改副本 |
| `void f(T*)` | 指针，可修改 | 输出参数 |
| `void f(const T*)` | 指针，只读 | 可选输入参数 |

### 7. const 与 move 语义

```cpp
class BigObject {
    std::string name_;
public:
    // const 引用可以绑定到右值，但不能移动（因为是 const）
    BigObject(const std::string& name) : name_(name) {}
    // 字符串会拷贝而非移动

    // 正确的写法：同时提供 const& 和 &&
    BigObject(std::string name) : name_(std::move(name)) {}
    // 传值 + move：const 实参拷贝，非 const 右值移动
};
```

**注意**：`const` 对象不能移动，因为移动操作需要修改源对象。

### 8. const_cast 的合理使用场景

```cpp
// 场景1：调用 C 风格 API（函数参数需要非 const 指针）
void old_c_api(char* buffer);
void modern_caller(const char* buffer) {
    old_c_api(const_cast<char*>(buffer));  // 安全的：old_c_api 实际不修改
}

// 场景2：const 成员函数中调用第三方非 const 函数
class Logger {
    mutable FILE* file_;
public:
    void log(const char* msg) const {
        // const 函数需要写入文件
        fprintf(const_cast<FILE*>(file_), "%s\n", msg);  // printf 不改内容
    }
};
```

**const_cast 的规则**：

- 对象本身是 const → 通过 const_cast 写入 → **未定义行为**
- 对象本身不是 const → 通过 const_cast 去除 const → **安全**
- **尽量不用**，只在 C 兼容层等少数场景使用

### 9. constexpr 与 const 的关系

| 特性 | const | constexpr |
|:---|:---|:---|
| 编译期求值 | 不一定 | 必须 |
| 运行时常量 | ✅ | ✅ |
| 函数 | ❌ | ✅（C++11 起） |
| if 语句分支 | ❌ | ✅（C++17 if constexpr） |
| 对象 | ✅ | ✅（C++11 起） |

```cpp
const int x = 10;                     // 运行时常量
constexpr int y = 20;                 // 编译期常量
constexpr int square(int n) {         // 编译期函数
    return n * n;
}
int arr[square(3)];                   // OK：编译期求值
```

### 10. C 与 C++ 的 const 区别

| 特性 | C | C++ |
|------|:---:|:---:|
| 默认链接性 | 外部链接 | 内部链接（全局const） |
| 数组大小 | C99 块作用域可作VLA但非常量表达式；文件作用域不行 | 可以 `const int N = 10; int a[N];` |
| 编译器检查 | 弱（可通过指针绕过） | 强（真正的类型系统约束） |
| constexpr | ❌ | ✅ |

### 11. 极简总结

**const 六种用法 = 常量 + 指向常量的指针 + 常量指针 + 双const指针 + 常量引用 + 常量成员函数 → const_cast 谨慎用 → constexpr 是编译期 const**

***

### 相关阅读

- [inline关键字的真实含义](19-inline关键字的真实含义.md)
- [头文件守卫与pragmaonce](21-头文件守卫与pragmaonce.md)
- [volatile关键字](18-volatile关键字.md)