# 什么是SBO小缓冲区优化
> 📖 相关章节：[STL容器](../../02-CPP/14-STL容器.md)、[STL算法](../../02-CPP/15-STL算法与迭代器.md)、[正则表达式](../../02-CPP/16-正则表达式.md)、[文件操作与文件系统](../../02-CPP/18-文件操作与文件系统.md)

> "小对象就地存，大对象才堆分配——用空间换时间。"

***

### 1. 精髓速览

SBO（Small Buffer Optimization）是在对象内部预留一块固定大小的缓冲区，当存储的数据足够小时直接放在缓冲区中，避免堆分配；只有数据超出缓冲区容量时才回退到堆分配。

***

### 2. SBO 的工作原理

SBO 的核心思路是将"内联存储"和"动态分配"统一在同一个对象中：

```cpp
#include <iostream>
#include <cstring>
#include <algorithm>

class SmallString {
    static constexpr std::size_t SBO_SIZE = 15;

    union Storage {
        char small[SBO_SIZE + 1];
        char* large;
    } storage_;

    std::size_t size_;
    bool is_small_;

public:
    SmallString() : size_(0), is_small_(true) {
        storage_.small[0] = '\0';
    }

    SmallString(const char* str) {
        size_ = std::strlen(str);
        if (size_ <= SBO_SIZE) {
            is_small_ = true;
            std::memcpy(storage_.small, str, size_ + 1);
        } else {
            is_small_ = false;
            storage_.large = new char[size_ + 1];
            std::memcpy(storage_.large, str, size_ + 1);
        }
    }

    ~SmallString() {
        if (!is_small_) delete[] storage_.large;
    }

    SmallString(const SmallString& other) : size_(other.size_), is_small_(other.is_small_) {
        if (is_small_) {
            std::memcpy(storage_.small, other.storage_.small, size_ + 1);
        } else {
            storage_.large = new char[size_ + 1];
            std::memcpy(storage_.large, other.storage_.large, size_ + 1);
        }
    }

    SmallString& operator=(const SmallString& other) {
        if (this != &other) {
            if (!is_small_) delete[] storage_.large;
            size_ = other.size_;
            is_small_ = other.is_small_;
            if (is_small_) {
                std::memcpy(storage_.small, other.storage_.small, size_ + 1);
            } else {
                storage_.large = new char[size_ + 1];
                std::memcpy(storage_.large, other.storage_.large, size_ + 1);
            }
        }
        return *this;
    }

    const char* data() const {
        return is_small_ ? storage_.small : storage_.large;
    }

    std::size_t size() const { return size_; }
    bool is_small() const { return is_small_; }
};

int main() {
    SmallString s1("hello");
    SmallString s2("this is a very long string that exceeds SBO");

    std::cout << "s1: \"" << s1.data() << "\" size=" << s1.size()
              << " small=" << s1.is_small() << std::endl;
    std::cout << "s2: \"" << s2.data() << "\" size=" << s2.size()
              << " small=" << s2.is_small() << std::endl;
    std::cout << "sizeof(SmallString): " << sizeof(SmallString) << std::endl;
}
```

输出：

```
s1: "hello" size=5 small=1
s2: "this is a very long string that exceeds SBO" size=42 small=0
sizeof(SmallString): 24
```

SBO 内存布局：

```
小字符串模式 (is_small_ = true):
+-------------------+
| small[16] 缓冲区   |  <- 字符串数据直接存在这里
+-------------------+
| size_             |
+-------------------+
| is_small_ = true  |
+-------------------+

大字符串模式 (is_small_ = false):
+-------------------+
| large 指针        |  <- 指向堆上的字符串
+-------------------+
| size_             |
+-------------------+
| is_small_ = false |
+-------------------+
```

### 3. std::function 中的 SBO

`std::function` 是 SBO 最典型的应用场景。它需要存储任意可调用对象，但大部分 lambda 和函数指针都很小。

```cpp
#include <iostream>
#include <functional>
#include <cstring>

void demoFunctionSBO() {
    auto smallLambda = []() { return 42; };

    int captured[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    auto largeLambda = [captured]() {
        int sum = 0;
        for (int v : captured) sum += v;
        return sum;
    };

    std::function<int()> f1 = smallLambda;
    std::function<int()> f2 = largeLambda;

    std::cout << "smallLambda 结果: " << f1() << std::endl;
    std::cout << "largeLambda 结果: " << f2() << std::endl;
    std::cout << "sizeof(std::function<int()>): " << sizeof(std::function<int()>) << std::endl;
}

int main() {
    demoFunctionSBO();
}
```

不同平台 `std::function` 的 SBO 容量：

| 平台/编译器 | sizeof(std::function) | SBO 容量（约） |
|------------|----------------------|---------------|
| GCC (Linux) | 32 字节 | ~16 字节 |
| Clang (macOS) | 48 字节 | ~24 字节 |
| MSVC (Windows) | 64 字节 | ~32 字节 |

```cpp
#include <iostream>
#include <functional>

void checkSBOCapacity() {
    auto small = []() { return 1; };
    int data4[4] = {};
    auto medium = [data4]() { return 2; };
    int data16[16] = {};
    auto large = [data16]() { return 3; };

    std::function<int()> f1 = small;
    std::function<int()> f2 = medium;
    std::function<int()> f3 = large;

    std::cout << "小 lambda: " << f1() << std::endl;
    std::cout << "中 lambda: " << f2() << std::endl;
    std::cout << "大 lambda: " << f3() << std::endl;

    std::cout << "sizeof(small):  " << sizeof(small) << " 字节" << std::endl;
    std::cout << "sizeof(medium): " << sizeof(medium) << " 字节" << std::endl;
    std::cout << "sizeof(large):  " << sizeof(large) << " 字节" << std::endl;
}

int main() {
    checkSBOCapacity();
}
```

### 4. std::any 中的 SBO

`std::any` 也使用 SBO 来避免小对象的堆分配：

```cpp
#include <iostream>
#include <any>
#include <string>
#include <vector>

void demoAnySBO() {
    std::any a1 = 42;
    std::any a2 = 3.14;
    std::any a3 = std::string("hello");
    std::any a4 = std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    std::cout << "sizeof(std::any): " << sizeof(std::any) << std::endl;
    std::cout << "a1 (int):    " << std::any_cast<int>(a1) << std::endl;
    std::cout << "a2 (double): " << std::any_cast<double>(a2) << std::endl;
    std::cout << "a3 (string): " << std::any_cast<std::string>(a3) << std::endl;

    auto& vec = std::any_cast<std::vector<int>&>(a4);
    std::cout << "a4 (vector): size=" << vec.size() << std::endl;
}

int main() {
    demoAnySBO();
}
```

| 平台/编译器 | sizeof(std::any) | SBO 容量 |
|------------|-----------------|---------|
| GCC (Linux) | 16 字节 | ~8 字节 |
| Clang (macOS) | 32 字节 | ~24 字节 |
| MSVC (Windows) | 64 字节 | ~40 字节 |

| 存储类型 | 是否使用 SBO | 说明 |
|---------|-------------|------|
| `int` | ✅ | 小于 SBO 容量 |
| `double` | ✅ | 小于 SBO 容量 |
| `std::string`（短） | ✅ | string 本身也有 SSO |
| `std::string`（长） | ⚠️ | string 在堆上，any 存 string 对象本身 |
| `std::vector<int>` | ✅ | vector 对象本身很小（3 个指针） |
| 大结构体 | ❌ | 超出 SBO 容量，堆分配 |

### 5. SBO vs SSO

SBO 和 SSO 是相关但不同的概念：

| 特性 | SBO (Small Buffer Optimization) | SSO (Small String Optimization) |
|-----|-------------------------------|-------------------------------|
| 全称 | Small Buffer Optimization | Small String Optimization |
| 应用对象 | 通用（function, any 等） | 专用于字符串 |
| 缓冲区用途 | 存储小可调用对象/小值 | 存储短字符串 |
| 标准库实例 | `std::function`, `std::any` | `std::string` |
| 触发条件 | 对象大小 ≤ 缓冲区 | 字符串长度 ≤ 阈值 |
| 典型阈值 | 16~32 字节 | 15~22 字符 |

```cpp
#include <iostream>
#include <string>
#include <functional>

void demoSsoVsSbo() {
    std::string s1 = "hello";
    std::string s2 = "this is a very long string that definitely exceeds SSO buffer";

    std::cout << "=== SSO ===" << std::endl;
    std::cout << "短字符串 \"" << s1 << "\" 长度=" << s1.size()
              << " 可能使用 SSO" << std::endl;
    std::cout << "长字符串 \"" << s2.substr(0, 20) << "...\" 长度=" << s2.size()
              << " 使用堆分配" << std::endl;

    std::cout << "\n=== SBO ===" << std::endl;
    auto small = []() { return 100; };
    std::function<int()> f = small;
    std::cout << "小 lambda 存储在 std::function 内部缓冲区" << std::endl;
    std::cout << "f() = " << f() << std::endl;
}

int main() {
    demoSsoVsSbo();
}
```

`std::string` 的 SSO 内存布局（以 GCC libstdc++ 为例）：

```
短字符串 (length <= 15):
+------------------+
| 字符数据 [16字节] |  <- 数据直接存储
+------------------+
| size (8字节)     |
+------------------+

长字符串:
+------------------+
| pointer (8字节)  |  <- 指向堆上的字符数据
+------------------+
| size (8字节)     |
+------------------+
| capacity (8字节) |
+------------------+
```

### 6. 实现自定义 SBO 容器

下面实现一个简化版的 SBO function 包装器：

```cpp
#include <iostream>
#include <cstring>
#include <utility>
#include <new>

template <typename Signature>
class SboFunction;

template <typename R, typename... Args>
class SboFunction<R(Args...)> {
    static constexpr std::size_t BUF_SIZE = 32;
    static constexpr std::size_t BUF_ALIGN = alignof(std::max_align_t);

    struct ICallable {
        virtual R invoke(Args... args) = 0;
        virtual void move_to(void* dest) = 0;
        virtual void destroy() = 0;
        virtual ~ICallable() = default;
    };

    template <typename F>
    struct CallableImpl : ICallable {
        F func_;
        CallableImpl(F&& f) : func_(std::move(f)) {}
        R invoke(Args... args) override { return func_(std::forward<Args>(args)...); }
        void move_to(void* dest) override {
            new (dest) CallableImpl(std::move(func_));
        }
        void destroy() override { func_.~F(); }
    };

    alignas(BUF_ALIGN) unsigned char buffer_[BUF_SIZE];
    ICallable* callable_ = nullptr;
    bool uses_heap_ = false;

    ICallable* get_callable() { return callable_; }
    const ICallable* get_callable() const { return callable_; }

public:
    SboFunction() = default;

    template <typename F, typename = std::enable_if_t<!std::is_same_v<std::decay_t<F>, SboFunction>>>
    SboFunction(F&& f) {
        using Impl = CallableImpl<std::decay_t<F>>;
        if (sizeof(Impl) <= BUF_SIZE && alignof(Impl) <= BUF_ALIGN) {
            callable_ = new (buffer_) Impl(std::forward<F>(f));
            uses_heap_ = false;
        } else {
            callable_ = new Impl(std::forward<F>(f));
            uses_heap_ = true;
        }
    }

    SboFunction(SboFunction&& other) noexcept {
        if (other.callable_) {
            if (other.uses_heap_) {
                callable_ = other.callable_;
                uses_heap_ = true;
            } else {
                other.callable_->move_to(buffer_);
                callable_ = reinterpret_cast<ICallable*>(buffer_);
                uses_heap_ = false;
            }
            other.callable_ = nullptr;
            other.uses_heap_ = false;
        }
    }

    SboFunction& operator=(SboFunction&& other) noexcept {
        if (this != &other) {
            reset();
            if (other.callable_) {
                if (other.uses_heap_) {
                    callable_ = other.callable_;
                    uses_heap_ = true;
                } else {
                    other.callable_->move_to(buffer_);
                    callable_ = reinterpret_cast<ICallable*>(buffer_);
                    uses_heap_ = false;
                }
                other.callable_ = nullptr;
                other.uses_heap_ = false;
            }
        }
        return *this;
    }

    ~SboFunction() { reset(); }

    R operator()(Args... args) {
        return get_callable()->invoke(std::forward<Args>(args)...);
    }

    explicit operator bool() const { return callable_ != nullptr; }

    bool uses_heap() const { return uses_heap_; }

private:
    void reset() {
        if (callable_) {
            callable_->destroy();
            if (uses_heap_) delete callable_;
            callable_ = nullptr;
            uses_heap_ = false;
        }
    }
};

struct SmallFunctor {
    int x_;
    SmallFunctor(int x) : x_(x) {}
    int operator()(int y) const { return x_ + y; }
};

struct LargeFunctor {
    int data_[20];
    LargeFunctor(int v) { for (auto& d : data_) d = v; }
    int operator()(int y) const { return data_[0] + y; }
};

int main() {
    SboFunction<int(int)> f1 = SmallFunctor(10);
    SboFunction<int(int)> f2 = LargeFunctor(20);

    std::cout << "f1(5) = " << f1(5) << " uses_heap=" << f1.uses_heap() << std::endl;
    std::cout << "f2(5) = " << f2(5) << " uses_heap=" << f2.uses_heap() << std::endl;
    std::cout << "sizeof(SboFunction): " << sizeof(SboFunction<int(int)>) << std::endl;
}
```

输出：

```
f1(5) = 15 uses_heap=0
f2(5) = 25 uses_heap=1
sizeof(SboFunction): 40
```

### 7. SBO 的性能影响

SBO 的核心权衡是用空间换时间：

```cpp
#include <iostream>
#include <functional>
#include <chrono>
#include <vector>

volatile int sink = 0;

void benchmarkFunctionCreation() {
    auto lambda = []() { return 42; };

    constexpr int N = 10000000;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        std::function<int()> f = lambda;
        sink = f();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "std::function (SBO): " << ns / N << " ns/次" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        auto f = lambda;
        sink = f();
    }
    end = std::chrono::high_resolution_clock::now();
    ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "原始 lambda:         " << ns / N << " ns/次" << std::endl;
}

int main() {
    benchmarkFunctionCreation();
}
```

| 操作 | 无 SBO（总是堆分配） | 有 SBO（小对象内联） | 提升 |
|-----|-------------------|-------------------|------|
| 构造（小对象） | ~50 ns | ~5 ns | 10x |
| 构造（大对象） | ~50 ns | ~50 ns | 1x |
| 调用 | ~10 ns | ~10 ns | 1x |
| 析构（小对象） | ~30 ns | ~2 ns | 15x |
| 析构（大对象） | ~30 ns | ~30 ns | 1x |
| 对象大小 | 8~16 字节 | 32~64 字节 | 更大 |

### 8. SBO 失效的场景

SBO 并非万能，以下场景 SBO 会失效：

```cpp
#include <iostream>
#include <functional>
#include <any>
#include <string>
#include <array>

struct HugeState {
    std::array<double, 100> matrix;
    HugeState() { matrix.fill(0.0); }
    double operator()(double x) const { return matrix[0] + x; }
};

void demoSBOFailure() {
    std::cout << "sizeof(HugeState): " << sizeof(HugeState) << std::endl;

    std::function<double(double)> f = HugeState{};
    std::cout << "std::function 可以工作，但 HugeState 超出 SBO 容量，触发堆分配" << std::endl;

    std::any a = HugeState{};
    std::cout << "std::any 同理，大对象堆分配" << std::endl;
}

struct NonMovable {
    int value;
    NonMovable(int v) : value(v) {}
    NonMovable(NonMovable&&) = delete;
    NonMovable& operator=(NonMovable&&) = delete;
    int operator()() const { return value; }
};

void demoNonMovableSBO() {
    std::cout << "\n不可移动的对象：" << std::endl;
}

int main() {
    demoSBOFailure();
    demoNonMovableSBO();
}
```

| SBO 失效场景 | 原因 | 影响 |
|-------------|------|------|
| 对象过大 | 超出内联缓冲区 | 回退堆分配，SBO 无收益 |
| 对象对齐过高 | `alignof` 超出缓冲区对齐 | 无法 placement new |
| 不可移动对象 | SBO 移动语义需要移动构造 | 编译错误或堆分配 |
| 抛出异常的移动 | 移动构造可能抛异常 | 部分实现拒绝 SBO |
| 虚继承 | 对象布局复杂 | 可能超出 SBO 容量 |

### 9. SBO 的空间代价

SBO 的固定缓冲区增加了对象大小，即使不使用也会占用空间：

```cpp
#include <iostream>
#include <functional>
#include <string>
#include <any>

void printSizes() {
    std::cout << "=== SBO 对象大小 ===" << std::endl;
    std::cout << "sizeof(std::function<void()>):  " << sizeof(std::function<void()>) << " 字节" << std::endl;
    std::cout << "sizeof(std::function<int(int)>): " << sizeof(std::function<int(int)>) << " 字节" << std::endl;
    std::cout << "sizeof(std::any):                " << sizeof(std::any) << " 字节" << std::endl;
    std::cout << "sizeof(std::string):             " << sizeof(std::string) << " 字节" << std::endl;

    std::cout << "\n=== 对比：无 SBO 的等价结构 ===" << std::endl;
    std::cout << "sizeof(void*):                   " << sizeof(void*) << " 字节 (仅指针)" << std::endl;
    std::cout << "sizeof(函数指针):                 " << sizeof(void(*)()) << " 字节" << std::endl;
}

int main() {
    printSizes();
}
```

典型输出（64 位平台）：

```
=== SBO 对象大小 ===
sizeof(std::function<void()>):  32 字节
sizeof(std::function<int(int)>): 32 字节
sizeof(std::any):                16 字节
sizeof(std::string):             32 字节

=== 对比：无 SBO 的等价结构 ===
sizeof(void*):                   8 字节 (仅指针)
sizeof(函数指针):                 8 字节
```

| 对象 | 大小 | SBO 缓冲区 | 空间开销 |
|-----|------|-----------|---------|
| `std::function` | 32~64 字节 | 16~32 字节 | 4~8x vs 裸指针 |
| `std::any` | 16~64 字节 | 8~40 字节 | 2~8x vs `void*` |
| `std::string` | 24~32 字节 | 15~22 字节 | 3~4x vs `const char*` |

**空间换时间权衡**：

- ✅ 频繁创建/销毁小对象 → SBO 收益巨大
- ✅ 对缓存友好（数据局部性） → SBO 有帮助
- ❌ 大量存储空对象 → SBO 浪费空间
- ❌ 大对象为主 → SBO 无收益，反而增加对象大小

### 10. 极简总结

| 要点 | 说明 |
|-----|------|
| **定义** | 在对象内部预留缓冲区，小数据内联存储，大数据堆分配 |
| **核心机制** | union/aligned_storage + placement new + 大小判断 |
| **标准库应用** | `std::function`、`std::any`、`std::string`(SSO) |
| **SBO vs SSO** | SBO 通用，SSO 是 SBO 在字符串上的特化 |
| **性能收益** | 小对象避免堆分配，构造/析构快 5~15 倍 |
| **空间代价** | 对象体积增大 2~8 倍 |
| **失效条件** | 对象过大、对齐过高、不可移动 |
| **实现关键** | 缓冲区大小选择、对齐处理、移动语义 |
| **适用场景** | 频繁创建销毁的小对象、类型擦除容器 |

**口诀**：小对象内联存，大对象才堆分；空间换时间，SBO 是良方。

***

### 相关阅读

- [什么是SSO小字符串优化](../02-内存与底层/16-什么是SSO小字符串优化.md)
- [什么是std-any](./10-什么是std-any.md)
- [STL容器底层实现](./01-STL容器底层实现.md)

***