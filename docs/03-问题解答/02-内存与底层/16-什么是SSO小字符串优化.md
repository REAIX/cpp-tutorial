# 什么是 SSO 小字符串优化
> 📖 相关章节：[指针](../../01-C语言/06-指针.md)、[结构体与联合体](../../01-C语言/08-结构体与联合体.md)、[内存管理](../../01-C语言/09-内存管理.md)、[智能指针](../../02-CPP/08-智能指针与内存管理.md)、[内存模型](../../02-CPP/32-内存模型.md)

> "小件放口袋随身带，大件放仓库额外搬"——SSO 让短字符串免于堆分配的开销。

***

### 1. 核心定义

**SSO（Small String Optimization）** = 当字符串足够短时，数据直接存储在 `std::string` 对象内部，不分配堆内存；只有字符串超过阈值时才在堆上分配。

关键点：**短字符串零堆分配，长字符串才走堆**。这是 `std::string` 最重要的性能优化之一。

***

### 2. 生活类比

**口袋 vs 仓库**：

| 场景 | 类比 | 对应 SSO |
|------|------|---------|
| 小件物品（钥匙、手机） | 放口袋，随身携带 | 短字符串存在对象内部，零堆分配 |
| 大件物品（行李箱、家具） | 放仓库，需要时去取 | 长字符串在堆上分配，通过指针访问 |

口袋（对象内部缓冲区）大小有限，但日常用的小东西（短字符串）都能装下。只有装不下的大件才需要额外租仓库（堆分配）。

**好处**：大多数实际场景中的字符串都很短（错误信息、配置项、人名等），SSO 让这些常见操作免去堆分配/释放的开销。

***

### 3. SSO 原理详解

#### 1. std::string 的两种存储模式

```
短字符串模式（SSO）：
┌──────────────────────────────────────┐
│ 内部缓冲区（直接存数据） │ 大小 │ 模式标志 │
│ "Hello"                              │  5   │   SSO   │
└──────────────────────────────────────┘
sizeof(std::string) 整体大小不变，数据就在对象体内

长字符串模式（堆分配）：
┌──────────────────────────────────────┐
│ 堆指针 ──────────→ ┌──────────────┐ │
│                    │ 堆上的数据    │ │
│ 容量  │ 大小 │ 模式标志           │
└──────────────────────────────────────┘
数据在堆上，对象内部存的是指针
```

#### 2. 内部缓冲区从哪来？

`std::string` 对象在 64 位系统上通常占 24 或 32 字节。这些空间不用白不用：

- 指针占 8 字节 → 短字符串模式下，这 8 字节变成数据缓冲区
- 再加上大小/容量字段中"借"来的空间，总共能存 15~22 字节的字符串

#### 3. 切换时机

```
字符串长度 ≤ SSO阈值 → 内部缓冲区存储（无堆分配）
字符串长度 > SSO阈值 → 堆分配存储
```

SSO 阈值因标准库实现而异，通常是 15 或 22 字节（见 FAQ 85.5）。

***

### 4. 代码验证：sizeof 和 capacity

```cpp
#include <iostream>
#include <string>

void inspect_string(const std::string& s) {
    std::cout << "内容: \"" << s << "\"\n";
    std::cout << "  长度:     " << s.length() << "\n";
    std::cout << "  容量:     " << s.capacity() << "\n";
    std::cout << "  sizeof:   " << sizeof(s) << "\n";
    std::cout << "  数据地址: " << static_cast<const void*>(s.data()) << "\n";
    std::cout << "  对象地址: " << static_cast<const void*>(&s) << "\n";

    bool sso = (reinterpret_cast<uintptr_t>(s.data()) >= reinterpret_cast<uintptr_t>(&s) &&
                reinterpret_cast<uintptr_t>(s.data()) < reinterpret_cast<uintptr_t>(&s) + sizeof(s));
    std::cout << "  SSO模式:  " << (sso ? "是(内部存储)" : "否(堆分配)") << "\n\n";
}

int main() {
    std::cout << "sizeof(std::string) = " << sizeof(std::string) << " 字节\n\n";

    std::string a = "Hi";
    std::string b = "Hello, World!!!";
    std::string c = "This is a long string that exceeds SSO threshold";

    inspect_string(a);
    inspect_string(b);
    inspect_string(c);
}
```

**典型输出（GCC libstdc++，64 位）**：

```
sizeof(std::string) = 32 字节

内容: "Hi"
  长度:     2
  容量:     15
  SSO模式:  是(内部存储)

内容: "Hello, World!!!"
  长度:     15
  容量:     15
  SSO模式:  是(内部存储)

内容: "This is a long string that exceeds SSO threshold"
  长度:     46
  容量:     46
  SSO模式:  否(堆分配)
```

**观察**：
- 短字符串的 `capacity()` 就是 SSO 阈值（GCC 为 15）
- 短字符串的数据地址在对象地址范围内（SSO 模式）
- 长字符串的数据地址在对象地址范围外（堆分配）

***

### 5. 三大标准库的 SSO 策略差异

| 特性 | GCC libstdc++ | Clang libc++ | MSVC |
|------|:---:|:---:|:---:|
| sizeof(std::string) | 32 字节 | 24 字节 | 32 字节 |
| SSO 阈值 | 15 字节 | 22 字节 | 15 字节 |
| 内部布局 | union { char[16]; ptr; } + size + capacity | 压缩存储（短串标志位在 size 最高位） | union { char[16]; ptr; } + size + capacity |
| 32 位系统 SSO 阈值 | 15 字节 | 10 字节 | 15 字节 |

**为什么 libc++ 的 SSO 阈值更大？**

libc++ 使用了更紧凑的内部布局：
- 短字符串模式下，`size` 和 `capacity` 字段的空间也借给数据缓冲区
- 用 `size` 的最高位作为模式标志（0 = SSO，1 = 堆分配）
- 24 字节的对象中，22 字节可用于存数据 + 1 字节存长度 + 1 位模式标志

**为什么 libstdc++ 和 MSVC 的 SSO 阈值更小？**

它们使用传统的 union 布局：
- union 中指针和缓冲区共享空间（8 字节）
- 另外有独立的 size 和 capacity 字段
- 32 字节的对象中，只有 15~16 字节可用于数据

***

### 6. 性能影响

#### 1. 堆分配的开销

```
堆分配一次 ≈ 几百个时钟周期（需要找空闲块、加锁等）
SSO 存储   ≈ 几个时钟周期（直接在栈/对象上写数据）
```

#### 2. 缓存友好性

```
SSO 模式：数据和对象在同一个缓存行 → 一次加载全部拿到
堆分配：  数据在堆上，对象在栈上 → 可能两次缓存未命中
```

#### 3. 实测对比

```cpp
#include <iostream>
#include <string>
#include <chrono>

int main() {
    const int N = 10'000'000;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        std::string s = "Hello";  // SSO，无堆分配
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto sso_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        std::string s = "This is a long string exceeding SSO buffer";  // 堆分配
    }
    end = std::chrono::high_resolution_clock::now();
    auto heap_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "SSO 短字符串: " << sso_ms << "ms\n";
    std::cout << "堆分配长字符串: " << heap_ms << "ms\n";
    std::cout << "差距: " << (heap_ms > 0 ? double(sso_ms) / heap_ms : 0) << "x\n";
}
```

**典型结果**：短字符串（SSO）比长字符串（堆分配）快 2~5 倍。

#### 4. SSO 对 vector<string> 的影响

```cpp
std::vector<std::string> v;
v.push_back("Hi");   // SSO：只拷贝 32 字节（对象本身），无堆分配
v.push_back("long string...");  // 堆分配：拷贝 32 字节 + 堆上的数据
```

SSO 让 `vector<string>` 的拷贝和移动更高效——短字符串的拷贝就是 memcpy 32 字节，不需要深拷贝。

***

### 7. SSO 的注意事项

#### 1. SSO 阈值不可移植

```cpp
std::string s(15, 'x');   // GCC: SSO；libc++: SSO
std::string s(16, 'x');   // GCC: 堆分配；libc++: SSO
std::string s(23, 'x');   // GCC: 堆分配；libc++: 堆分配
```

**不要假设 SSO 阈值是固定的**，跨平台代码尤其要注意。

#### 2. 短字符串的 data() 指针可能失效

```cpp
std::string s = "Hi";
const char* p = s.data();  // SSO 模式，p 指向对象内部
s = "This is a very long string";  // 切换到堆分配！
// p 现在是悬空指针！对象内部缓冲区被覆盖了
```

**规则**：任何修改字符串的操作都可能使 `data()`/`c_str()` 返回的指针失效，无论是否 SSO。

#### 3. 移动语义与 SSO

```cpp
std::string a = "Hello";  // SSO
std::string b = std::move(a);  // 短字符串：仍然是拷贝（数据在对象内部，无法"偷"指针）
// a 移动后仍持有有效数据（空字符串或未指定状态）

std::string c = "This is a very long string...";  // 堆分配
std::string d = std::move(c);  // 长字符串：只偷指针，O(1)
// c 移动后为空
```

**SSO 字符串的移动不是 O(1)**，因为数据在对象内部，必须拷贝。只有堆分配的字符串才能 O(1) 移动。

| 操作 | SSO 短字符串 | 堆分配长字符串 |
|------|:---:|:---:|
| 构造 | O(n) 拷贝数据 | O(n) 堆分配+拷贝 |
| 拷贝 | O(n) 拷贝数据 | O(n) 堆分配+拷贝 |
| 移动 | O(n) 拷贝数据 | O(1) 偷指针 |
| 析构 | O(1) 无堆释放 | O(1) 堆释放 |

***

### 8. 极简总结

**SSO = 短字符串存对象内部免堆分配，长字符串才走堆**

| 要点 | 说明 |
|------|------|
| 核心思想 | 利用 string 对象自身的空间存短字符串，省去堆分配开销 |
| 典型阈值 | GCC/MSVC: 15 字节，libc++: 22 字节 |
| 性能收益 | 短字符串构造/析构快 2~5 倍，缓存更友好 |
| 注意事项 | 阈值不可移植；SSO 字符串移动不是 O(1)；data() 指针可能因模式切换失效 |
| 一句话 | 绝大多数字符串很短，SSO 让它们零堆分配 |

***

### 相关阅读

- [什么是SBO小缓冲区优化](../07-现代CPP标准库/13-什么是SBO小缓冲区优化.md)
- [什么是写时复制](./14-什么是写时复制.md)