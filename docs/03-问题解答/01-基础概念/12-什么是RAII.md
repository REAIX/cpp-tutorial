# 什么是 RAII
> 📖 相关章节：[类与对象](../../02-CPP/03-类与对象.md)、[智能指针与内存管理](../../02-CPP/08-智能指针与内存管理.md)

> "Acquire resources when needed, release them when done" — 按需获取，用完即释

### 1. 核心定义

**RAII**（Resource Acquisition Is Initialization）= **资源获取即初始化**

**大白话**：变量创建时自动获取资源，变量销毁/离开作用域时自动释放资源。

### 2. 生活类比

把**资源**比作**租房子**：

- 初始化（定义变量）→ **自动租房拿钥匙**
- 作用域结束（变量失效）→ **自动退房、归还钥匙**

资源包括：内存、文件句柄、网络socket、互斥锁、数据库连接……

### 3. 原生支持

| 语言 | 是否原生支持 | 实现方式 |
| --- | ------ | ----------------------------------- |
| C++ | ✅ 是 | 构造函数 + 析构函数 |
| C | ❌ 否 | 靠 GCC `__attribute__((cleanup))` 模拟 |

### 4. C++ 原生 RAII 示例

#### 1. 文件管理

```cpp
#include <fstream>
void writeFile() {
    std::ofstream file("test.txt");  // 构造 → 自动打开文件
    file << "Hello RAII";
    // 函数结束 → 析构 → 自动关闭文件
    // 即使中间抛出异常，析构函数也会被调用
}
```

#### 2. 互斥锁管理

```cpp
#include <mutex>
std::mutex mtx;
int shared_data = 0;

void safe_increment() {
    std::lock_guard<std::mutex> lock(mtx);  // 构造 → 自动上锁
    ++shared_data;
    // 函数结束 → 析构 → 自动解锁
    // 不用担心忘记 unlock，异常安全
}
```

#### 3. 动态内存管理

```cpp
#include <memory>
void process() {
    auto ptr = std::make_unique<int>(42);   // 构造 → 分配内存
    auto sptr = std::make_shared<double>(3.14); // 共享所有权
    // 离开作用域 → 自动 delete
    // 不用手动写 delete，杜绝内存泄漏
}
```

#### 4. Socket 管理

```cpp
class SocketGuard {
    int sockfd_;
public:
    SocketGuard(const char* ip, int port) {
        sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
        // connect...
    }
    ~SocketGuard() {
        close(sockfd_);  // 自动关闭socket
    }
    int get() const { return sockfd_; }
};

void send_data() {
    SocketGuard sock("192.168.1.1", 8080);
    // send(sock.get(), ...);
    // 自动 close，即使提前 return 或抛异常
}
```

### 5. RAII 与移动语义

C++11 移动语义让 RAII 对象可以高效转移所有权：

```cpp
std::unique_ptr<int> create() {
    auto p = std::make_unique<int>(42);
    return p;  // 移动所有权给调用者
}

void use() {
    auto p = create();  // p 接管资源所有权
    auto q = std::move(p);  // 转移给 q，p 变为空
    // 当 q 离开作用域 → 自动释放
}
```

### 6. C 语言模拟 RAII

```c
// 使用 GCC 扩展模拟 RAII
int *data __attribute__((cleanup(free_auto))) = malloc(...);
FILE *fp __attribute__((cleanup(fclose_auto))) = fopen(...);

// 变量离开作用域 → 自动调用清理函数
```

### 7. 核心规则

1. **资源申请** 写在 **变量初始化** 那一刻
2. **资源释放** 绑定 **变量生命周期**，自动触发

### 8. 为什么要用 RAII

**传统写法缺点**：

- 容易忘释放 → 内存泄漏/句柄泄漏
- 中途 `return` 就跳过释放
- 代码嵌套、维护麻烦
- 异常安全无法保证

**RAII 优点**：

- 只管创建，不用管释放
- 编译器自动兜底，杜绝泄漏
- 异常安全：栈展开时自动调用析构

### 9. 常见 RAII 封装对照表

| 原始资源 | C 风格 | C++ RAII |
|:---|:---|:---|
| 动态内存 | malloc/free | unique_ptr / shared_ptr |
| 文件 | fopen/fclose | ofstream / ifstream |
| 互斥锁 | pthread_mutex_lock/unlock | lock_guard / unique_lock |
| Socket | socket/close | 自定义 SocketGuard |
| 数据库连接 | sqlite3_open/close | 自定义 DBConnection |

### 10. 极简总结

**RAII = 绑定变量生命周期，自动申请、自动释放资源 → 异常安全、杜绝泄漏**

***

### 相关阅读

- [什么是零开销抽象Zero-overhead](./32-什么是零开销抽象Zero-overhead.md)
- [static关键字](16-static关键字.md)
- [goto是什么](./25-goto是什么.md)