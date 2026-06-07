# static 关键字完全指南
> 📖 相关章节：[函数](../../01-C语言/04-函数.md)、[多文件编程](../../01-C语言/16-多文件编程.md)

> **static: once and only, here and only.** — static：只一次，仅此处。

***

### 1. 核心要义

**static** 在 C/C++ 中有 **五种完全不同的含义**，唯一共同点是：**都和"唯一性"或"限制可见性"有关**。

***

### 2. 五种用法一览

| 用法 | 位置 | 作用 |
|------|------|------|
| 静态局部变量 | 函数内部 | 只初始化一次，生命周期到程序结束 |
| 静态全局变量 | .c/.cpp 文件内 | 仅当前文件可见（内部链接） |
| 静态函数 | .c/.cpp 文件内 | 仅当前文件可见（内部链接） |
| 静态成员变量 | class 内部 | 类级别的变量，所有对象共享 |
| 静态成员函数 | class 内部 | 类级别的函数，没有 this 指针 |

### 3. 用法1：静态局部变量

```cpp
#include <iostream>

void counter() {
    static int n = 0;
    ++n;
    std::cout << n << std::endl;
}

int main() {
    counter();  // 1
    counter();  // 2
    counter();  // 3
}
```

**特点**：生命周期 = 全局，作用域 = 局部。

静态局部变量存储在全局/静态区，不在栈上。函数第一次执行时初始化，之后不再初始化。

### 4. 线程安全的局部静态初始化（C++11）

C++11 保证局部静态变量的初始化是线程安全的：

```cpp
#include <iostream>
#include <thread>

class Singleton {
public:
    static Singleton& instance() {
        static Singleton inst;  // C++11保证线程安全
        return inst;
    }

    void hello() {
        std::cout << "Singleton hello" << std::endl;
    }

private:
    Singleton() = default;
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
};

void worker() {
    Singleton::instance().hello();
}

int main() {
    std::thread t1(worker);
    std::thread t2(worker);
    t1.join();
    t2.join();
}
```

C++11 之前，多线程同时首次调用 `instance()` 可能导致多次构造。C++11 之后，编译器会自动加锁保护初始化过程（即"魔法静态变量" Magic Statics）。

### 5. 用法2：静态全局变量

```cpp
// file1.c
static int secret = 42;

// file2.c
extern int secret;  // 链接错误！找不到
```

**特点**：内部链接性，其他翻译单元看不到。

### 6. 用法3：静态函数

```cpp
// file1.c
static void helper() {}

// file2.c
void helper();  // 不同的函数，互不影响
```

静态函数的好处：
- 避免命名冲突：不同文件可以有同名static函数
- 编译器优化：编译器知道函数只在本文件使用，可以更激进地内联

### 7. 用法4：静态成员变量

```cpp
class Config {
    static int port;
};
int Config::port = 8080;
```

**必须在类外定义**（C++17 可用 `inline static` 简化）：

```cpp
class Config {
    inline static int port = 8080;  // C++17，类内定义
};
```

### 8. C++17 inline static 成员详解

C++17 之前，静态成员变量必须在某个 .cpp 文件中定义，否则链接错误：

```cpp
// C++17之前
class Game {
    static int score;
};
int Game::score = 0;  // 必须在.cpp中定义

// C++17
class Game {
    inline static int score = 0;  // 头文件中直接定义，无需.cpp
};
```

| 方案 | 头文件 | 需要.cpp | C++版本 |
|------|--------|---------|---------|
| 类外定义 | `static int x;` 声明 | 需要 `int Cls::x = 0;` | C++98 |
| inline static | `inline static int x = 0;` | 不需要 | C++17 |
| constexpr static | `static constexpr int x = 0;` | 不需要（整型） | C++11 |

### 9. 用法5：静态成员函数

```cpp
class Math {
public:
    static int max(int a, int b) { return a > b ? a : b; }
};

int m = Math::max(3, 5);
```

**限制**：没有 `this` 指针，只能访问静态成员。

```cpp
class Example {
    int value_;
    static int count_;
public:
    static int get_count() {
        return count_;   // OK
    }
    static int bad() {
        return value_;   // 编译错误：没有this，无法访问非静态成员
    }
};
```

### 10. 静态初始化顺序问题（Static Initialization Order Fiasco）

这是C++中最臭名昭著的陷阱之一：不同翻译单元中的非局部静态对象的初始化顺序是**未定义的**。

```cpp
// file1.cpp
#include <iostream>

class Logger {
public:
    Logger() { std::cout << "Logger constructed" << std::endl; }
    void log(const char* msg) { std::cout << msg << std::endl; }
};

Logger g_logger;

// file2.cpp
class Database {
public:
    Database() {
        g_logger.log("Database constructed");  // 可能崩溃！
    }
};

Database g_database;
```

如果 `g_database` 在 `g_logger` 之前初始化，`g_logger.log()` 就会访问未初始化的对象。

**解决方案：用函数内的静态变量替代全局静态变量（Meyers' Singleton）**

```cpp
// file1.cpp
class Logger {
public:
    Logger() { std::cout << "Logger constructed" << std::endl; }
    void log(const char* msg) { std::cout << msg << std::endl; }
};

Logger& get_logger() {
    static Logger instance;  // 首次调用时初始化，C++11线程安全
    return instance;
}

// file2.cpp
class Database {
public:
    Database() {
        get_logger().log("Database constructed");  // 安全！
    }
};

Database g_database;
```

### 11. static vs extern 对比表

| 维度 | static（全局） | extern |
|------|---------------|--------|
| 链接性 | 内部链接 | 外部链接 |
| 可见性 | 仅当前翻译单元 | 所有翻译单元 |
| 典型用途 | 文件私有变量/函数 | 跨文件共享变量 |
| 与const组合 | `static const` = 内部链接 | `extern const` = 外部链接 |
| 内存 | 分配（定义） | 不分配（仅声明） |

### 12. 常见陷阱

#### 1. 陷阱1：类内静态成员未定义

```cpp
class Foo {
    static int count;
};

int main() {
    Foo::count = 1;  // 链接错误：undefined reference to Foo::count
}
```

解决：在 .cpp 中定义 `int Foo::count = 0;`，或用 C++17 `inline static`。

#### 2. 陷阱2：静态局部变量与多线程（C++11之前）

```cpp
// C++11之前，这不是线程安全的
Singleton& instance() {
    static Singleton inst;  // 可能被两个线程同时构造
    return inst;
}
```

C++11 之后安全，但旧代码需要注意。

#### 3. 陷阱3：静态成员函数访问非静态成员

```cpp
class Bad {
    int x_;
    static void func() {
        x_ = 1;  // 编译错误
    }
};
```

静态成员函数没有 `this` 指针，必须通过对象实例访问非静态成员：

```cpp
class Good {
    int x_;
public:
    static void func(Good& obj) {
        obj.x_ = 1;  // OK
    }
};
```

### 13. 最佳实践

| 实践 | 说明 |
|------|------|
| 优先用匿名命名空间（C++） | 替代static全局变量/函数，更符合C++风格 |
| 用inline static（C++17） | 避免类外定义静态成员 |
| 用Meyers' Singleton | 避免静态初始化顺序问题 |
| const + static 替代宏 | `static const int kMax = 100;` 优于 `#define MAX 100` |
| 静态成员函数用于工厂方法 | `static Shape* create(type);` |

匿名命名空间替代 static：

```cpp
// C风格
static void helper() {}
static int file_data = 42;

// C++推荐
namespace {
    void helper() {}
    int file_data = 42;
}
```

### 14. 极简总结

**static 五种用法 = 局部持久化 + 文件私有变量 + 文件私有函数 + 类共享变量 + 类共享函数 → C++11线程安全局部静态 → C++17 inline static → 警惕静态初始化顺序问题 → 优先用匿名命名空间替代static全局**

***

### 相关阅读

- [什么是RAII](12-什么是RAII.md)
- [什么是零开销抽象Zero-overhead](./32-什么是零开销抽象Zero-overhead.md)
- [extern关键字](14-extern关键字.md)