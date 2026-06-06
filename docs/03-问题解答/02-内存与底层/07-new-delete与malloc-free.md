# new/delete 与 malloc/free 的区别
> 📖 相关章节：[指针](../../01-C语言/06-指针.md)、[结构体与联合体](../../01-C语言/08-结构体与联合体.md)、[内存管理](../../01-C语言/09-内存管理.md)、[智能指针](../../02-CPP/08-智能指针与内存管理.md)、[内存模型](../../02-CPP/32-内存模型.md)

### 1. 先抓核心

**new/delete** 是 C++ 运算符，会调用构造/析构函数；**malloc/free** 是 C 库函数，只分配/释放内存。**两者不可混用**！

***

### 2. 核心区别

| 特性 | new/delete | malloc/free |
|------|:---:|:---:|
| 语言 | C++ | C |
| 性质 | 运算符 | 库函数 |
| 构造/析构 | 自动调用 | 不调用 |
| 返回类型 | 类型安全（T*） | void* |
| 失败处理 | 抛出 bad_alloc | 返回 NULL |
| 大小计算 | 自动 | 手动 sizeof |
| 重载 | 可以重载 | 不可重载 |
| 数组 | new[]/delete[] | 手动计算 |
| 对齐方式 | 默认对齐（alignof(std::max_align_t)） | 默认 8/16 字节对齐 |
| 底层实现 | 通常基于 malloc | 系统调用（brk/mmap） |

### 3. 构造/析构的区别

```cpp
class Widget {
public:
    Widget() { cout << "构造" << endl; }
    ~Widget() { cout << "析构" << endl; }
};

// new/delete：调用构造和析构
Widget* w1 = new Widget;   // 输出 "构造"
delete w1;                  // 输出 "析构"

// malloc/free：不调用构造和析构
Widget* w2 = (Widget*)malloc(sizeof(Widget));  // 无输出
free(w2);                                       // 无输出
// w2 指向的内存是未初始化的！
// 使用 *w2 是未定义行为！
```

new 的内部实现分解：
```cpp
// new Widget() 等价于：
void* raw = operator new(sizeof(Widget));  // 分配原始内存
Widget* w = static_cast<Widget*>(raw);
w->Widget::Widget();                        // 调用构造函数

// delete w 等价于：
w->~Widget();                               // 调用析构函数
operator delete(w);                          // 释放原始内存
```

### 4. 为什么不能混用

```cpp
// 错误1：new 分配，free 释放
int* p = new int(42);
free(p);  // 未定义行为！new 可能用不同的内存分配器

// 错误2：malloc 分配，delete 释放
int* q = (int*)malloc(sizeof(int));
delete q;  // 未定义行为！

// 错误3：new[] 分配，delete 释放
int* arr = new int[10];
delete arr;  // 未定义行为！必须用 delete[]

// 错误4：scalar delete 释放数组
int* arr = new int[10];
delete[] arr;  // 正确
```

为什么混用是危险的：
```cpp
// new 分配的内存头部可能存储了额外信息
// 例如：数组元素个数（供 delete[] 使用）
int* arr = new int[10];
// 内存布局：
// [元素个数:10][arr[0]][arr[1]]...[arr[9]]
// free(arr) 可能释放了错误的位置
```

### 5. 失败处理

```cpp
// new 失败抛异常
try {
    int* p = new int[1000000000];
} catch (const std::bad_alloc& e) {
    // 处理
}

// new 失败不抛异常（nothrow版本）
int* p = new(std::nothrow) int[1000000000];
if (p == nullptr) {
    // 处理
}

// malloc 失败返回 NULL
int* p = (int*)malloc(1000000000 * sizeof(int));
if (p == NULL) {
    // 处理
}

// 避免在 noexcept 函数中使用可能抛异常的 new
void process() noexcept {
    int* p = new(std::nothrow) int[100];
    if (!p) return;  // 安全处理
    delete[] p;
}
```

### 6. operator new 的重载

```cpp
#include <new>
#include <cstdlib>

// 全局重载 operator new
void* operator new(size_t size) {
    std::cout << "Allocating " << size << " bytes" << std::endl;
    void* p = malloc(size);
    if (!p) throw std::bad_alloc();
    return p;
}

void operator delete(void* p) noexcept {
    std::cout << "Deallocating" << std::endl;
    free(p);
}

// 类级别的 operator new 重载
class Pool {
    static constexpr size_t BLOCK_SIZE = 1024;
    static char pool[BLOCK_SIZE];
    static bool used[BLOCK_SIZE / sizeof(double)];
public:
    void* operator new(size_t size) {
        // 从内存池中分配
        for (size_t i = 0; i < BLOCK_SIZE / sizeof(double); ++i) {
            if (!used[i]) {
                used[i] = true;
                return pool + i * sizeof(double);
            }
        }
        throw std::bad_alloc();
    }

    void operator delete(void* p) noexcept {
        // 归还到内存池
        ptrdiff_t index = (char*)p - pool;
        if (index >= 0 && index < BLOCK_SIZE) {
            used[index / sizeof(double)] = false;
        }
    }
};
```

### 7. Placement new

```cpp
#include <new>

// placement new：在已分配的内存上构造对象
void* buffer = malloc(sizeof(Widget));
Widget* w = new(buffer) Widget();  // 在 buffer 上构造

// 显式调用析构
w->~Widget();

// 注意：不能使用 delete 释放 placement new 的对象
// delete w;  // 错误！会尝试释放 buffer

// 应该只用 free 释放原始内存
free(buffer);

// placement new 的典型用途：内存池
class MemoryPool {
    char pool[4096];
    size_t offset = 0;
public:
    template<typename T, typename... Args>
    T* create(Args&&... args) {
        T* ptr = new(pool + offset) T(std::forward<Args>(args)...);
        offset += sizeof(T);
        return ptr;
    }

    void reset() { offset = 0; }
};

// 数组 placement new
char buf[sizeof(int) * 10];
int* arr = new(buf) int[10] {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

// aligned placement new (C++17)
std::align_val_t alignment(64);
void* aligned_buf = operator new(1024, alignment);
```

### 8. 内存池与自定义分配器

```cpp
template<typename T>
class StaticAllocator {
    static constexpr size_t POOL_SIZE = 1000;
    static T pool[POOL_SIZE];
    static bool in_use[POOL_SIZE];
public:
    T* allocate(size_t n) {
        for (size_t i = 0; i < POOL_SIZE; ++i) {
            if (!in_use[i]) {
                in_use[i] = true;
                return &pool[i];
            }
        }
        throw std::bad_alloc();
    }

    void deallocate(T* p, size_t) noexcept {
        ptrdiff_t index = p - pool;
        if (index >= 0 && index < POOL_SIZE) {
            in_use[index] = false;
        }
    }
};

// 使用自定义分配器的 STL 容器
// std::vector<int, StaticAllocator<int>> vec;
```

### 9. new/delete 与 malloc/free 底层实现差异

**malloc 实现常见策略**：
- **ptmalloc**（glibc）：多个分配区（arena），小对象用 bins，大对象用 mmap
- **jemalloc**：每个线程独立缓存，减少锁竞争
- **tcmalloc**（Google）：线程本地缓存 + 中央堆

**new 的典型调用链**：
```
new → operator new → malloc → brk()/mmap() 系统调用
```

内存管理对比：
| 特性 | malloc | new |
|------|:------:|:---:|
| 内存头部开销 | ~4-16 字节/块 | 同 malloc + 可能的 vptr |
| 分配策略 | free list / brk / mmap | 依赖底层 malloc |
| 碎片处理 | 合并相邻空闲块 | 依赖底层实现 |
| 线程安全 | 是（加锁） | 是 |
| 对齐保证 | 至少 alignof(max_align_t) | 同 malloc |

### 10. 选择指南

| 场景 | 推荐方式 |
|------|----------|
| C++ 对象（需构造/析构） | new/delete |
| C 代码 | malloc/free |
| realloc | malloc/free（C++ 用 vector） |
| 内存池 | operator new 重载 / placement new |
| 大型连续内存 | std::vector |
| 单个对象 | make_unique / make_shared |
| 对齐要求 | aligned_alloc (C11) / alignas |

### 11. 极简总结

**new/delete = C++关键字 + 调用构造/析构 + 可重载 + 抛异常；malloc/free = C函数 + 仅分配内存 + 不调用构造/析构 + 返回void* → 永远不要混用 → 配对使用 → 优先 new/delete + 智能指针 → placement new 在已有内存上构造**

***

### 相关阅读

- [栈与堆](./00-栈与堆.md)
- [什么是内存池Memory-Pool](./18-什么是内存池Memory-Pool.md)
