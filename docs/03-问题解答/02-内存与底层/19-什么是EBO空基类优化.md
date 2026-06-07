# 什么是EBO空基类优化
> 📖 相关章节：[类与对象](../../02-CPP/03-类与对象.md)、[继承与多态](../../02-CPP/04-继承与多态.md)

> 空类不空，EBO让它归零。

***

### 1. 本质速解

EBO（Empty Base Optimization）是C++编译器的一种优化：当空类（size=1）作为另一个类的基类时，编译器可以将其占用的1字节压缩为0字节，从而减少派生类的总大小。

***

### 2. 空类为什么大小是1

C++标准要求每个完整对象必须有唯一的地址，因此即使是空类也必须占至少1字节：

```cpp
#include <iostream>

class Empty {};

class AlsoEmpty {
    void foo() {}
    static int value;
};

int main() {
    std::cout << "Empty:      " << sizeof(Empty) << " 字节\n";
    std::cout << "AlsoEmpty:  " << sizeof(AlsoEmpty) << " 字节\n";

    Empty a, b;
    std::cout << "a 地址: " << &a << "\n";
    std::cout << "b 地址: " << &b << "\n";
    std::cout << "地址不同: " << (&a != &b) << "\n";
}
```

输出（典型64位平台）：

```
Empty:      1 字节
AlsoEmpty:  1 字节
a 地址: 0x7ffd...
b 地址: 0x7ffd...
地址不同: 1
```

关键规则：

| 成员类型 | 是否占空间 | 说明 |
|---------|-----------|------|
| 非静态数据成员 | 是 | 按类型大小计算 |
| 静态数据成员 | 否 | 存储在全局/静态区 |
| 成员函数 | 否 | 存储在代码段 |
| 虚函数 | 间接占 | vptr指针（通常8字节） |
| 空类无以上任何 | 1字节 | 保证地址唯一 |

***

### 3. EBO的基本原理

当空类作为基类时，编译器不需要为其分配独立地址——派生类对象本身的地址就可以充当基类子对象的地址。这就是EBO：

```cpp
#include <iostream>

class Empty {};

class WithoutEBO {
    Empty e;
    int x;
};

class WithEBO : public Empty {
    int x;
};

int main() {
    std::cout << "Empty:      " << sizeof(Empty) << " 字节\n";
    std::cout << "WithoutEBO: " << sizeof(WithoutEBO) << " 字节\n";
    std::cout << "WithEBO:    " << sizeof(WithEBO) << " 字节\n";
}
```

输出（64位平台）：

```
Empty:      1 字节
WithoutEBO: 8 字节   (1字节Empty + 3字节填充 + 4字节int)
WithEBO:    4 字节   (EBO生效，Empty占0字节，只有int)
```

对比图示：

```
WithoutEBO 内存布局:        WithEBO 内存布局:
+---+---+---+---+           +---+---+---+---+
| E | pad  |   int x   |   |       int x       |
+---+---+---+---+           +---+---+---+---+
 1B   3B       4B                   4B
总计: 8字节                    总计: 4字节
```

***

### 4. EBO的适用条件与失效场景

EBO并非总是生效，以下情况会导致EBO失效：

```cpp
#include <iostream>

class Empty {};

class Derived1 : public Empty {
    int x;
};

class Derived2 : public Empty {
    int x;
};

class Multiple : public Derived1, public Derived2 {
};

class SameBase1 : public Empty {
    int x;
};

class SameBase2 : public Empty {
    int y;
};

class Diamond : public SameBase1, public SameBase2 {
};

class NonEmptyBase {
    int data;
};

class NoEBO : public NonEmptyBase {
    int x;
};

int main() {
    std::cout << "Derived1:   " << sizeof(Derived1) << "\n";
    std::cout << "Multiple:   " << sizeof(Multiple) << "\n";
    std::cout << "Diamond:    " << sizeof(Diamond) << "\n";
    std::cout << "NoEBO:      " << sizeof(NoEBO) << "\n";
}
```

EBO失效条件汇总：

| 条件 | EBO是否生效 | 原因 |
|------|------------|------|
| 单继承空基类 | ✅ 生效 | 基类子对象可与派生类共享地址 |
| 多继承同一空基类 | ❌ 失效 | 两个基类子对象必须地址不同 |
| 菱形继承空基类 | ❌ 失效 | 同一基类出现两次 |
| 基类非空 | ❌ 不适用 | 非空基类本身有数据 |
| 成员变量而非基类 | ❌ 不适用 | 成员必须有独立地址 |
| 虚继承 | ❌ 失效 | 虚基类指针引入额外开销 |

***

### 5. 标准库中的EBO实例

标准库大量使用EBO来消除无状态组件的空间开销：

**std::allocator 示例**

```cpp
#include <iostream>
#include <vector>
#include <memory>

template<typename T>
struct DebugAllocator {
    int tag = 42;
    T* allocate(std::size_t n) {
        std::cout << "分配 " << n << " 个 " << sizeof(T) << " 字节对象\n";
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }
    void deallocate(T* p, std::size_t n) {
        std::cout << "释放 " << n << " 个对象\n";
        ::operator delete(p);
    }
};

int main() {
    std::cout << "std::allocator<int> 大小: "
              << sizeof(std::allocator<int>) << " 字节\n";
    std::cout << "DebugAllocator<int> 大小: "
              << sizeof(DebugAllocator<int>) << " 字节\n";

    std::vector<int, std::allocator<int>> v1;
    std::vector<int, DebugAllocator<int>> v2;

    std::cout << "vector(默认分配器) 大小: " << sizeof(v1) << " 字节\n";
    std::cout << "vector(Debug分配器) 大小: " << sizeof(v2) << " 字节\n";
}
```

典型输出：

```
std::allocator<int> 大小: 1 字节
DebugAllocator<int> 大小: 4 字节
vector(默认分配器) 大小: 24 字节
vector(Debug分配器) 大小: 28 字节
```

**std::less 示例**

```cpp
#include <iostream>
#include <functional>
#include <map>

int main() {
    std::cout << "std::less<int> 大小: "
              << sizeof(std::less<int>) << " 字节\n";

    std::map<int, int> m;
    std::cout << "std::map<int,int> 大小: "
              << sizeof(m) << " 字节\n";
}
```

`std::less<int>` 是空类，通过EBO在 `std::map` 内部不占额外空间。

***

### 6. 自定义删除器与EBO

`std::unique_ptr` 是EBO的经典应用场景——自定义删除器如果无状态，则不增加指针大小：

```cpp
#include <iostream>
#include <memory>

struct DefaultDelete {
    template<typename T>
    void operator()(T* p) const {
        delete p;
    }
};

struct FunctorDelete {
    int log_level = 0;
    template<typename T>
    void operator()(T* p) const {
        std::cout << "删除日志级别: " << log_level << "\n";
        delete p;
    }
};

struct StatelessDelete {
    void operator()(int* p) const {
        std::cout << "无状态删除器释放\n";
        delete p;
    }
};

int main() {
    using P1 = std::unique_ptr<int>;
    using P2 = std::unique_ptr<int, FunctorDelete>;
    using P3 = std::unique_ptr<int, StatelessDelete>;

    std::cout << "unique_ptr(默认删除器):   " << sizeof(P1) << " 字节\n";
    std::cout << "unique_ptr(FunctorDelete): " << sizeof(P2) << " 字节\n";
    std::cout << "unique_ptr(StatelessDelete): " << sizeof(P3) << " 字节\n";
}
```

典型输出：

```
unique_ptr(默认删除器):    8 字节
unique_ptr(FunctorDelete): 12 字节
unique_ptr(StatelessDelete): 8 字节
```

原理剖析：

```
unique_ptr 内部结构（简化）:

template<typename T, typename Deleter>
class unique_ptr {
    T* ptr;          // 8字节
    Deleter del;     // EBO生效时0字节，否则按实际大小
};

StatelessDelete 是空类 → EBO → del 占0字节 → 总共8字节
FunctorDelete 有 log_level → EBO不适用 → del 占4字节 → 总共12字节
```

***

### 7. 手动实现EBO友好组件

在实际工程中，可以利用EBO设计零开销的策略类：

```cpp
#include <iostream>
#include <cstring>

template<typename Allocator>
class Container : private Allocator {
    int* data_;
    size_t size_;

public:
    Container(size_t n) : size_(n) {
        data_ = this->allocate(n);
    }

    ~Container() {
        this->deallocate(data_, size_);
    }

    int& operator[](size_t i) { return data_[i]; }

    Allocator& get_allocator() { return *this; }
};

class SimpleAlloc {
public:
    int* allocate(size_t n) {
        std::cout << "SimpleAlloc: 分配 " << n << " 个int\n";
        return static_cast<int*>(::operator new(n * sizeof(int)));
    }

    void deallocate(int* p, size_t n) {
        std::cout << "SimpleAlloc: 释放\n";
        ::operator delete(p);
    }
};

class TrackingAlloc {
    int alloc_count_ = 0;
public:
    int* allocate(size_t n) {
        ++alloc_count_;
        std::cout << "TrackingAlloc: 第 " << alloc_count_ << " 次分配\n";
        return static_cast<int*>(::operator new(n * sizeof(int)));
    }

    void deallocate(int* p, size_t) {
        std::cout << "TrackingAlloc: 释放 (共分配过 " << alloc_count_ << " 次)\n";
        ::operator delete(p);
    }

    int get_alloc_count() const { return alloc_count_; }
};

int main() {
    Container<SimpleAlloc> c1(10);
    Container<TrackingAlloc> c2(10);

    std::cout << "Container<SimpleAlloc>:   " << sizeof(c1) << " 字节\n";
    std::cout << "Container<TrackingAlloc>:  " << sizeof(c2) << " 字节\n";
}
```

输出：

```
SimpleAlloc: 分配 10 个int
TrackingAlloc: 第 1 次分配
Container<SimpleAlloc>:   16 字节   (指针8 + size_t8 + EBO 0)
Container<TrackingAlloc>: 20 字节   (指针8 + size_t8 + int4)
```

***

### 8. C++20 [[no_unique_address]] 与EBO对比

C++20引入 `[[no_unique_address]]` 属性，可以在不使用继承的情况下实现类似EBO的效果：

```cpp
#include <iostream>

class Empty {};

class WithEBO : private Empty {
    int x;
};

class WithAttr {
    [[no_unique_address]] Empty e;
    int x;
};

class WithoutOpt {
    Empty e;
    int x;
};

struct TwoEmptiesEBO1 : Empty {
    [[no_unique_address]] Empty e2;
    int x;
};

struct TwoEmptiesAttr {
    [[no_unique_address]] Empty e1;
    [[no_unique_address]] Empty e2;
    int x;
};

int main() {
    std::cout << "WithEBO:     " << sizeof(WithEBO) << " 字节\n";
    std::cout << "WithAttr:    " << sizeof(WithAttr) << " 字节\n";
    std::cout << "WithoutOpt:  " << sizeof(WithoutOpt) << " 字节\n";
    std::cout << "TwoEmptiesEBO1: " << sizeof(TwoEmptiesEBO1) << " 字节\n";
    std::cout << "TwoEmptiesAttr: " << sizeof(TwoEmptiesAttr) << " 字节\n";
}
```

输出（GCC/Clang，MSVC行为不同）：

```
WithEBO:        4 字节
WithAttr:       4 字节
WithoutOpt:     8 字节
TwoEmptiesEBO1: 4 字节
TwoEmptiesAttr: 4 字节    (GCC/Clang: 同类型可重叠; MSVC: 8字节)
```

EBO vs `[[no_unique_address]]` 对比：

| 特性 | EBO | [[no_unique_address]] |
|------|-----|----------------------|
| 要求C++版本 | C++98 | C++20 |
| 实现方式 | 继承 | 成员属性 |
| 是否改变类层次 | 是 | 否 |
| 多个同类型空成员 | 受限 | 编译器决定（GCC可重叠） |
| 代码侵入性 | 高（需改继承） | 低（加属性即可） |
| MSVC支持 | 完全 | 有限（同类型不重叠） |
| 语义清晰度 | 间接 | 直观 |

> ⚠️ **平台注意**：MSVC对 `[[no_unique_address]]` 的实现与GCC/Clang不同——当同一类型出现多次时，MSVC不会让它们重叠，导致大小比GCC/Clang大。

***

### 9. EBO在标准库中的更多案例

```cpp
#include <iostream>
#include <functional>
#include <string>

int main() {
    auto lambda_no_capture = []() {};
    auto lambda_with_capture = [x = 42]() { return x; };

    std::cout << "无捕获lambda大小: " << sizeof(lambda_no_capture) << "\n";
    std::cout << "有捕获lambda大小: " << sizeof(lambda_with_capture) << "\n";

    std::cout << "std::less<int>:       " << sizeof(std::less<int>) << "\n";
    std::cout << "std::greater<int>:    " << sizeof(std::greater<int>) << "\n";
    std::cout << "std::equal_to<int>:   " << sizeof(std::equal_to<int>) << "\n";
    std::cout << "std::hash<int>:       " << sizeof(std::hash<int>) << "\n";
    std::cout << "std::hash<std::string>: " << sizeof(std::hash<std::string>) << "\n";
}
```

标准库中利用EBO的组件一览：

| 组件 | 是否空类 | EBO效果 |
|------|---------|---------|
| `std::allocator<T>` | 是（默认） | 容器不因分配器增大 |
| `std::less<T>` | 是 | map/set不因比较器增大 |
| `std::greater<T>` | 是 | 同上 |
| `std::hash<T>`（基础类型） | 是 | unordered容器零开销 |
| `std::default_delete<T>` | 是 | unique_ptr与裸指针同大小 |
| `std::pointer_traits` | 是 | 零开销类型萃取 |

***

### 10. EBO实战陷阱与最佳实践

```cpp
#include <iostream>
#include <memory>

class Empty {};

class Bad : public Empty {
    Empty e;
    int x;
};

class Good : public Empty {
    int x;
};

class AlsoBad {
    Empty e1;
    Empty e2;
    int x;
};

template<typename D>
class GoodDeleter : private D {
public:
    void operator()(void* p) const {
        D::operator()(p);
    }
};

template<typename D>
class BadDeleter {
    D deleter_;
public:
    void operator()(void* p) const {
        deleter_(p);
    }
};

int main() {
    std::cout << "Bad:      " << sizeof(Bad) << "\n";
    std::cout << "Good:     " << sizeof(Good) << "\n";
    std::cout << "AlsoBad:  " << sizeof(AlsoBad) << "\n";

    std::cout << "GoodDeleter: " << sizeof(GoodDeleter<Empty>) << "\n";
    std::cout << "BadDeleter:  " << sizeof(BadDeleter<Empty>) << "\n";
}
```

最佳实践清单：

| 做法 | 推荐 | 原因 |
|------|------|------|
| 无状态策略类用继承 | ✅ | 触发EBO |
| 无状态策略类用成员 | ❌ | 浪费1字节+填充 |
| 同时继承和成员同一空类 | ❌ | 重复占用 |
| 用EBO实现零开销策略模式 | ✅ | 标准库做法 |
| C++20优先用[[no_unique_address]] | ✅ | 更直观 |
| 跨平台代码注意MSVC差异 | ✅ | MSVC行为不同 |

***

### 11. 极简总结

| 概念 | 要点 |
|------|------|
| 空类大小 | 必须为1字节（保证地址唯一） |
| EBO原理 | 空基类子对象可与派生类共享地址 |
| EBO条件 | 单继承空基类，基类无数据成员 |
| EBO失效 | 多继承同一空基类、虚继承、成员变量 |
| 标准库应用 | allocator、less、default_delete、hash |
| C++20替代 | [[no_unique_address]] 更直观 |
| 平台差异 | MSVC对[[no_unique_address]]支持有限 |
| 核心收益 | 无状态组件零空间开销 |

***

### 相关阅读

- [结构体内存对齐](04-结构体内存对齐.md)
- [C++对象内存布局](12-C++对象内存布局.md)