# 什么是std::array与C数组的区别
> 📖 相关章节：[STL容器](../../02-CPP/14-STL容器.md)、[STL算法](../../02-CPP/15-STL算法与迭代器.md)、[正则表达式](../../02-CPP/16-正则表达式.md)、[文件操作与文件系统](../../02-CPP/18-文件操作与文件系统.md)

> "std::array is a C array that grew up — same performance, but it knows its own size and plays well with the STL." — Scott Meyers

***

### 1. 核心速览

`std::array` 是 C++11 引入的固定大小容器，封装了 C 数组的零开销抽象，增加了大小感知、边界检查、迭代器支持和比较运算，同时保持与 C 数组完全相同的内存布局和性能。

***

### 2. 基本概念与创建初始化

`std::array<T, N>` 是一个固定大小为 N、元素类型为 T 的容器，内存连续分配，栈上存储。

```cpp
#include <array>
#include <string>
#include <iostream>

int main() {
    std::array<int, 5> a1 = {1, 2, 3, 4, 5};

    std::array<int, 5> a2 = {10, 20};
    // a2 = {10, 20, 0, 0, 0}

    std::array<int, 5> a3{};
    // a3 = {0, 0, 0, 0, 0}

    std::array<std::string, 3> names = {"Alice", "Bob", "Charlie"};

    auto a4 = std::array<double, 3>{1.1, 2.2, 3.3};

    std::array<int, 5> a5;
    a5.fill(7);
    // a5 = {7, 7, 7, 7, 7}
}
```

初始化方式汇总：

| 方式 | 代码 | 说明 |
|------|------|------|
| 聚合初始化 | `array<int,3> a = {1,2,3}` | 完整初始化 |
| 部分初始化 | `array<int,3> a = {1}` | 剩余元素值初始化为零 |
| 值初始化 | `array<int,3> a{}` | 全部零初始化 |
| 默认构造 | `array<int,3> a` | ⚠️ 元素未初始化（非类类型） |
| `fill` | `a.fill(42)` | 所有元素设为同一值 |
| CTAD | `std::array a{1,2,3}` | C++17 推导（→ `array<int,3>`) |

```cpp
// C++17 CTAD —— 最简洁的写法
std::array nums = {1, 2, 3, 4, 5};

std::array names = {"Alice", "Bob"};

std::array<double, 3> coords = {1.0, 2.0, 3.0};
```

> ⚠️ 注意：`std::array<int, 3> a;` 不初始化元素（值未定义），务必用 `a{}` 或 `a = {}` 进行零初始化。

***

### 3. operator[] 与 at() 访问

`std::array` 提供两种元素访问方式：`operator[]` 无边界检查，`at()` 有边界检查。

```cpp
#include <array>
#include <iostream>

int main() {
    std::array<int, 5> a = {10, 20, 30, 40, 50};

    int v1 = a[2];
    std::cout << v1 << "\n";

    int v2 = a.at(2);
    std::cout << v2 << "\n";

    a[3] = 99;
    a.at(4) = 100;

    try {
        int bad = a.at(10);
    } catch (const std::out_of_range& e) {
        std::cout << "越界: " << e.what() << "\n";
    }

    int front = a.front();
    int back = a.back();
    std::cout << "首: " << front << ", 尾: " << back << "\n";
}
```

`operator[]` vs `at()` 对比：

| 特性 | `operator[]` | `at()` |
|------|-------------|--------|
| 边界检查 | ❌ 无 | ✅ 有 |
| 越界行为 | 未定义行为 | 抛出 `std::out_of_range` |
| 性能 | 最快 | 略慢（检查开销） |
| 推荐场景 | 性能关键、索引已验证 | 安全优先、外部输入 |

```cpp
// 安全实践：Debug 模式用 at()，Release 模式用 []
#ifdef NDEBUG
    #define SAFE_ACCESS(arr, i) (arr)[i]
#else
    #define SAFE_ACCESS(arr, i) (arr).at(i)
#endif

int main() {
    std::array<int, 10> a{};
    int v = SAFE_ACCESS(a, 5);
}
```

***

### 4. size()、迭代器与遍历

`std::array` 的最大优势之一是知道自身大小，并提供完整的迭代器支持。

```cpp
#include <array>
#include <iostream>
#include <algorithm>

int main() {
    std::array<int, 5> a = {5, 3, 1, 4, 2};

    std::cout << "大小: " << a.size() << "\n";
    std::cout << "最大大小: " << a.max_size() << "\n";
    std::cout << "是否为空: " << a.empty() << "\n";

    for (auto it = a.begin(); it != a.end(); ++it) {
        std::cout << *it << " ";
    }

    for (const auto& elem : a) {
        std::cout << elem << " ";
    }

    for (auto& elem : a) {
        elem *= 2;
    }

    std::sort(a.begin(), a.end());

    for (auto it = a.rbegin(); it != a.rend(); ++it) {
        std::cout << *it << " ";
    }
}
```

迭代器与 STL 算法配合：

```cpp
#include <array>
#include <algorithm>
#include <numeric>
#include <iostream>

int main() {
    std::array<int, 6> a = {1, 2, 3, 4, 5, 6};

    int sum = std::accumulate(a.begin(), a.end(), 0);

    auto it = std::find(a.begin(), a.end(), 4);
    if (it != a.end()) {
        std::cout << "找到: " << *it << "\n";
    }

    int count = std::count_if(a.begin(), a.end(), [](int x) {
        return x % 2 == 0;
    });

    std::sort(a.begin(), a.end(), std::greater<int>{});
}
```

| 接口 | 说明 |
|------|------|
| `size()` | 返回元素个数（编译期常量） |
| `max_size()` | 等于 `size()` |
| `empty()` | `size() == 0` |
| `begin()/end()` | 正向迭代器 |
| `rbegin()/rend()` | 反向迭代器 |
| `cbegin()/cend()` | const 正向迭代器 |

***

### 5. fill()、swap() 与 std::get

`fill()` 批量赋值，`swap()` 交换两个同类型 array，`std::get<I>` 按索引编译期访问。

```cpp
#include <array>
#include <iostream>
#include <utility>

int main() {
    std::array<int, 4> a = {1, 2, 3, 4};
    std::array<int, 4> b = {5, 6, 7, 8};

    a.swap(b);
    // a = {5,6,7,8}, b = {1,2,3,4}

    std::swap(a, b);
    // a = {1,2,3,4}, b = {5,6,7,8}

    a.fill(0);
    // a = {0,0,0,0}

    auto c = std::array<int, 3>{10, 20, 30};
    int first = std::get<0>(c);
    int second = std::get<1>(c);
    int third = std::get<2>(c);

    // std::get<3>(c); // 编译错误：索引越界
}
```

`std::get<I>` 的特点：

| 特性 | 说明 |
|------|------|
| 编译期检查 | 索引越界直接编译错误 |
| 与 `tuple` 统一 | `array` 可视为同类型元素的 `tuple` |
| 结构化绑定 | C++17 可用 `auto [a,b,c] = arr` |
| 移动语义 | `get<I>(std::move(arr))` 返回右值引用 |

```cpp
std::array<int, 3> arr = {100, 200, 300};
auto [x, y, z] = arr;
std::cout << x << ", " << y << ", " << z << "\n";
```

***

### 6. 比较运算符

`std::array` 支持完整的比较运算符，按字典序逐元素比较。

```cpp
#include <array>
#include <iostream>

int main() {
    std::array<int, 3> a = {1, 2, 3};
    std::array<int, 3> b = {1, 2, 4};
    std::array<int, 3> c = {1, 2, 3};

    std::cout << (a < b) << "\n";
    std::cout << (a == c) << "\n";
    std::cout << (a != b) << "\n";
    std::cout << (a <= c) << "\n";
    std::cout << (b > a) << "\n";
    std::cout << (b >= a) << "\n";

    // C++20 太空船运算符
    auto cmp = (a <=> b);
    std::cout << (cmp < 0) << "\n";
}
```

C 数组 vs `std::array` 比较行为对比：

| 操作 | C 数组 | `std::array` |
|------|--------|-------------|
| `==` | ❌ 比较指针地址 | ✅ 逐元素比较 |
| `<` | ❌ 比较指针地址 | ✅ 字典序比较 |
| `!=` | ❌ 比较指针地址 | ✅ 逐元素比较 |
| `<=>` | ❌ 不支持 | ✅ C++20 三路比较 |

```cpp
// C 数组的陷阱
int ca1[] = {1, 2, 3};
int ca2[] = {1, 2, 3};
// ca1 == ca2  比较的是指针，不是内容！
// 需要用 std::equal 或 memcmp

// std::array 直接比较内容
std::array<int, 3> sa1 = {1, 2, 3};
std::array<int, 3> sa2 = {1, 2, 3};
bool same = (sa1 == sa2); // true
```

***

### 7. std::array vs C 数组：核心区别

这是本 FAQ 最核心的对比。`std::array` 解决了 C 数组的所有痛点。

```cpp
// C 数组的所有问题
void c_array_problems() {
    int arr[5] = {1, 2, 3, 4, 5};

    // 问题1：传参时退化为指针，丢失大小
    // sizeof(arr) 在函数外是 20，传参后变成指针大小

    // 问题2：无法直接比较
    int other[5] = {1, 2, 3, 4, 5};
    // arr == other  比较指针

    // 问题3：无法直接赋值
    // int copy[5] = arr;  编译错误

    // 问题4：无法返回
    // return arr;  返回局部数组地址
}

// std::array 解决所有问题
void std_array_solutions() {
    std::array<int, 5> arr = {1, 2, 3, 4, 5};

    std::cout << arr.size() << "\n";

    std::array<int, 5> other = {1, 2, 3, 4, 5};
    bool same = (arr == other);

    std::array<int, 5> copy = arr;

    auto make_array() -> std::array<int, 5> {
        return {1, 2, 3, 4, 5};
    }
}
```

完整对比表：

| 特性 | C 数组 | `std::array` |
|------|--------|-------------|
| 大小感知 | ❌ 传参时退化丢失 | ✅ `size()` 始终可用 |
| 赋值 | ❌ 不可直接赋值 | ✅ 支持赋值 |
| 比较 | ❌ 比较指针 | ✅ 逐元素比较 |
| 返回值 | ❌ 不可返回 | ✅ 值语义可返回 |
| 迭代器 | ❌ 无 | ✅ 完整迭代器支持 |
| 边界检查 | ❌ 无 | ✅ `at()` |
| STL 兼容 | ❌ 需手动传指针+大小 | ✅ 直接用 begin/end |
| 结构化绑定 | ❌ 不支持 | ✅ C++17 支持 |
| 内存布局 | 连续 | 连续（完全相同） |
| 性能开销 | 无 | 无（零开销抽象） |
| 退化为指针 | ✅ 会退化 | ❌ 不退化 |

***

### 8. std::array vs std::vector

`std::array` 是固定大小，`std::vector` 是动态大小，两者适用场景不同。

```cpp
#include <array>
#include <vector>
#include <iostream>

int main() {
    std::array<int, 5> a = {1, 2, 3, 4, 5};
    std::vector<int> v = {1, 2, 3, 4, 5};

    // array 大小编译期确定
    constexpr auto arr_size = a.size();

    // vector 大小运行时确定
    auto vec_size = v.size();
    v.push_back(6);
    v.resize(100);
}
```

| 对比维度 | `std::array` | `std::vector` |
|----------|-------------|--------------|
| 大小 | 编译期固定 | 运行时可变 |
| 内存位置 | 栈上 | 堆上（通常） |
| 动态扩容 | ❌ | ✅ |
| 内存开销 | 仅元素本身 | 元素 + 容量管理 |
| 赋值/拷贝 | 直接拷贝 | 深拷贝（可能分配） |
| 移动语义 | 无意义（栈上） | O(1) 移动 |
| 小数据性能 | 更优（无堆分配） | 较差（堆分配开销） |
| 大数据场景 | 栈溢出风险 | 安全 |
| 适用场景 | 维度固定的数学向量、查找表 | 动态集合、不确定大小 |

```cpp
// 适合 array 的场景
std::array<double, 3> position3d = {1.0, 2.0, 3.0};
std::array<std::array<double, 3>, 3> rotation_matrix = {
    std::array<double, 3>{1, 0, 0},
    std::array<double, 3>{0, 1, 0},
    std::array<double, 3>{0, 0, 1}
};
std::array<int, 256> lookup_table;
lookup_table.fill(-1);

// 适合 vector 的场景
std::vector<int> user_inputs;
int x;
while (std::cin >> x) {
    user_inputs.push_back(x);
}
```

***

### 9. data() 与 C 接口互操作

`std::array::data()` 返回指向底层连续内存的指针，可直接传给 C 接口。

```cpp
#include <array>
#include <cstring>
#include <iostream>

extern "C" void c_process_ints(int* data, int size);

void use_c_api() {
    std::array<int, 5> a = {10, 20, 30, 40, 50};

    c_process_ints(a.data(), static_cast<int>(a.size()));

    int* ptr = a.data();
    ptr[0] = 99;

    const int* cptr = a.data();
}

int main() {
    std::array<unsigned char, 256> buffer{};
    buffer.fill(0xAA);

    std::memcpy(buffer.data(), "hello", 5);

    std::array<char, 128> msg{};
    std::snprintf(msg.data(), msg.size(), "Value: %d", 42);
}
```

`data()` 的关键特性：

| 特性 | 说明 |
|------|------|
| 返回值 | `T*`（非 const 对象），`const T*`（const 对象） |
| 内存保证 | 指向连续内存，与 C 数组布局完全相同 |
| 有效性 | 只要 array 存在，指针就有效 |
| 与 `&arr[0]` | 等价，但 `data()` 对空 array 也安全 |

```cpp
// 二维 array 与 C 接口
std::array<std::array<float, 4>, 4> matrix{};

float* flat = &matrix[0][0];
// 或
float* flat2 = matrix[0].data();

// 传给 OpenGL 等图形 API
// glUniformMatrix4fv(loc, 1, GL_FALSE, matrix[0].data());
```

***

### 10. 零开销保证与性能

`std::array` 的核心承诺是零开销抽象——与 C 数组在内存布局和运行时性能上完全等价。

```cpp
#include <array>
#include <iostream>
#include <chrono>

constexpr int N = 1000000;
constexpr int ITER = 100;

void benchmark() {
    int c_arr[N];
    std::array<int, N> std_arr;

    for (int i = 0; i < N; ++i) {
        c_arr[i] = i;
        std_arr[i] = i;
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    volatile long long sum1 = 0;
    for (int iter = 0; iter < ITER; ++iter) {
        long long s = 0;
        for (int i = 0; i < N; ++i) s += c_arr[i];
        sum1 = s;
    }
    auto t2 = std::chrono::high_resolution_clock::now();

    volatile long long sum2 = 0;
    for (int iter = 0; iter < ITER; ++iter) {
        long long s = 0;
        for (int i = 0; i < N; ++i) s += std_arr[i];
        sum2 = s;
    }
    auto t3 = std::chrono::high_resolution_clock::now();

    auto c_time = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    auto arr_time = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();

    std::cout << "C 数组: " << c_time << " us\n";
    std::cout << "std::array: " << arr_time << " us\n";
}
```

零开销的保证：

| 方面 | 保证 |
|------|------|
| 内存布局 | 与 C 数组完全相同，连续存储 |
| 大小 | `sizeof(std::array<T,N>) == sizeof(T[N])`（无额外开销） |
| 访问速度 | `operator[]` 编译后与 C 数组下标访问相同 |
| 函数调用 | `size()` 等内联为常量 |
| 无虚函数 | 无虚表、无动态分发 |
| 无堆分配 | 完全栈上分配 |

```cpp
static_assert(sizeof(std::array<int, 5>) == sizeof(int[5]));
static_assert(sizeof(std::array<double, 10>) == sizeof(double[10]));
```

> ⚠️ 平台注意：部分实现可能有细微差异，如 Debug 模式下 MSVC 会添加迭代器调试信息。Release 模式下保证零开销。

***

### 11. 极简总结

| 概念 | 关键点 |
|------|--------|
| 本质 | C 数组的零开销封装，固定大小、栈上分配 |
| 创建 | `std::array<T,N> a = {...}` / C++17 CTAD `std::array a{...}` |
| 访问 | `a[i]`（无检查）/ `a.at(i)`（有检查） |
| 大小 | `a.size()` 编译期常量，永不退化 |
| 迭代器 | 完整支持，兼容所有 STL 算法 |
| `fill` | `a.fill(val)` 批量赋值 |
| `swap` | `a.swap(b)` 交换两个同类型 array |
| `std::get<I>` | 编译期索引访问，支持结构化绑定 |
| 比较 | 逐元素字典序比较（C 数组只比指针） |
| `data()` | 获取底层指针，与 C 接口互操作 |
| vs C 数组 | 不退化、可赋值、可比较、可返回、有迭代器 |
| vs vector | 固定大小、栈分配、零开销；vector 动态大小、堆分配 |
| 零开销 | 内存布局和性能与 C 数组完全等价 |

核心记忆：**std::array 就是 C 数组 + 大小感知 + STL 兼容 + 安全访问，零额外开销**。

***

### 相关阅读

- [STL容器底层实现](./01-STL容器底层实现.md)
- [std-span](./05-std-span.md)
- [emplace-back与push-back](./02-emplace-back与push-back.md)

***