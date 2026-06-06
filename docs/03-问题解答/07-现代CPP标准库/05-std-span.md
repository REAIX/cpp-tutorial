# std::span 详解
> 📖 相关章节：[STL容器](../../02-CPP/14-STL容器.md)、[STL算法](../../02-CPP/15-STL算法与迭代器.md)、[正则表达式](../../02-CPP/16-正则表达式.md)、[文件系统](../../02-CPP/19-文件系统库.md)

### 1. 本质速解

**std::span 是 C++20 的非拥有式视图，引用一段连续内存，类似 string_view 但用于任意类型。函数参数中替代 (const T* data, size_t len) 模式，零拷贝、类型安全、范围安全。**

***

### 2. 核心定义

| | std::span<T> | std::string_view |
|---|---|---|
| 是什么 | 指向任意类型连续内存的非拥有视图 | 指向字符序列的非拥有视图 |
| 元素类型 | 任意类型 T | 仅 char / wchar_t 等 |
| 是否拥有数据 | 不拥有，只观察 | 不拥有，只观察 |
| 内存开销 | 指针 + 长度（动态）或仅指针（固定） | 指针 + 长度 |
| C++ 版本 | C++20 | C++17 |

**本质**：

```cpp
// span：只记录"在哪里"和"有多长"，不分配、不拷贝
int arr[] = {1, 2, 3, 4, 5};
std::span<int> s = arr;  // s = {ptr=&arr[0], len=5}

// span 可以指向任何连续内存
std::vector<int> v = {10, 20, 30};
std::span<int> sv = v;   // sv = {ptr=v.data(), len=v.size()}

std::array<int, 3> a = {1, 2, 3};
std::span<int> sa = a;   // sa = {ptr=a.data(), len=3}
```

***

### 3. 生活类比

| | std::span | array / vector |
|---|---|---|
| 类比 | 书签（标记书中的某几页） | 整本书 |
| 说明 | 书签标记了从第几页到第几页，但不拥有书本身 | 书是真实存在的，有所有页 |
| 关键区别 | 不拥有数据，只是观察一段范围 | 拥有数据，管理生命周期 |

**具体场景**：

- **span**：你在图书馆看到一本厚书，用书签标记了第 10 页到第 20 页。书签不拥有这些页面，如果图书馆把书收走了（原数据销毁），书签就失效了。但书在的时候，你可以自由翻阅标记的页面。
- **array/vector**：你自己买了一本书，整本书都是你的。你可以随时翻阅、修改、甚至撕掉几页。

***

### 4. 固定大小 span vs 动态大小 span

```cpp
#include <span>

// 动态大小 span：运行时确定长度
std::span<int> dynamic_span;  // 可以指向任意长度的 int 序列

// 固定大小 span：编译时确定长度（模板参数）
std::span<int, 5> fixed_span;  // 只能指向恰好 5 个 int 的序列

int arr[5] = {1, 2, 3, 4, 5};

// 动态大小：从数组推断
std::span<int> s1 = arr;        // s1.size() == 5（运行时）
// 固定大小：从数组推断
std::span<int, 5> s2 = arr;    // s2.size() == 5（编译时）

// 固定大小 span 的优势：编译器可以优化边界检查
void process_fixed(std::span<int, 4> s);  // 编译时知道大小
void process_dynamic(std::span<int> s);    // 运行时才知道大小

// 固定大小 span 可以隐式转为动态大小 span
std::span<int> s3 = s2;  // OK，固定 → 动态
// std::span<int, 5> s4 = s1;  // 错误！动态 → 固定不行

// first / last / subspan
std::span<int> full = arr;
std::span<int> first3 = full.first<3>();   // 前 3 个（固定大小）
std::span<int> last2 = full.last(2);       // 后 2 个（动态大小）
std::span<int> mid = full.subspan(1, 3);   // 从第 1 个开始取 3 个
```

***

### 5. 替代 (const T* data, size_t len) 参数模式

**传统 C 风格参数的问题**：

```cpp
// 传统方式：指针 + 长度
void process_old(const int* data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        // data[i] ...
    }
}

// 调用时各种不便
std::vector<int> v = {1, 2, 3};
process_old(v.data(), v.size());        // 需要手动拆分
process_old(nullptr, 0);                // 空序列怎么传？

int arr[] = {4, 5, 6};
process_old(arr, 3);                    // 需要手动写长度
```

**span 方式：统一、安全、简洁**：

```cpp
#include <span>
#include <vector>
#include <array>
#include <iostream>

void process(std::span<const int> data) {
    for (int x : data) {
        std::cout << x << " ";
    }
    std::cout << "\n";
    std::cout << "size: " << data.size() << "\n";
}

int main() {
    std::vector<int> v = {1, 2, 3, 4, 5};
    process(v);                          // vector → span，零拷贝

    std::array<int, 3> a = {10, 20, 30};
    process(a);                          // array → span，零拷贝

    int arr[] = {100, 200, 300, 400};
    process(arr);                        // C 数组 → span，零拷贝

    process({});                         // 空序列，安全

    // 子范围
    process(std::span{v}.first(3));      // 前 3 个
    process(std::span{v}.subspan(2));    // 从第 2 个开始

    return 0;
}
```

**span 作为函数参数的优势**：

| 对比项 | (const T*, size_t) | std::span<const T> |
|------|:---:|:---:|
| 类型安全 | 不安全，指针类型可混用 | 安全，类型明确 |
| 范围安全 | 不安全，长度可能不匹配 | 安全，长度自动管理 |
| 空序列 | 需要传 nullptr, 0 | 直接传 {} |
| 支持 range-for | 不支持 | 支持 |
| 支持 size() | 不支持 | 支持 |
| 支持容器类型 | 需要手动 .data() + .size() | 自动转换 |
| 子范围 | 需要手动计算指针偏移 | first/last/subspan |

***

### 6. span 与 array / vector / string_view 对比

```cpp
#include <span>
#include <vector>
#include <array>
#include <string>
#include <string_view>

// span vs vector：span 不拥有，vector 拥有
std::vector<int> vec = {1, 2, 3};
std::span<int> sp = vec;       // sp 观察 vec 的数据
vec.push_back(4);              // 可能重新分配！sp 可能悬空

// span vs array：span 不拥有，array 拥有（栈上固定大小）
std::array<int, 3> arr = {1, 2, 3};
std::span<int> spa = arr;      // spa 观察 arr 的数据

// span vs string_view：span 通用，string_view 专用于字符
std::string str = "hello";
std::string_view sv = str;     // 字符串视图
std::span<const char> spc = std::span{str.data(), str.size()};  // 等效但更底层

// span 可以修改元素（如果 T 不是 const）
std::span<int> mutable_sp = vec;
mutable_sp[0] = 999;           // 修改 vec[0]

// string_view 不能修改
// std::string_view mutable_sv = str;
// mutable_sv[0] = 'H';       // 编译错误，string_view 是 const 视图
```

***

### 7. span 的注意事项

**注意1：悬空风险（和 string_view 一样）**

```cpp
// 陷阱：span 指向的容器重新分配
std::vector<int> v = {1, 2, 3};
std::span<int> s = v;
v.push_back(4);        // vector 可能重新分配内存
// s 现在可能悬空！

// 陷阱：返回 span 指向局部变量
std::span<int> getData() {
    int arr[] = {1, 2, 3};
    return arr;         // 返回指向局部数组的 span
}   // arr 销毁，返回的 span 悬空
```

**注意2：span 要求连续内存**

```cpp
// span 只能指向连续内存
std::vector<int> vec;          // 连续，OK
std::array<int, 3> arr;        // 连续，OK
int c_arr[5];                  // 连续，OK

// 不能指向非连续容器
std::list<int> lst;            // 不连续，不能创建 span
std::deque<int> dq;            // 分段连续，不能创建 span
std::set<int> st;              // 节点式，不能创建 span
```

**注意3：span 的迭代器失效**

```cpp
std::vector<int> v = {1, 2, 3};
std::span<int> s = v;
auto it = s.begin();
v.push_back(4);        // vector 重新分配
// it 失效！使用 it 是 UB
```

***

### 8. 对比表格

| 特性 | std::span | std::array | std::vector | std::string_view |
|------|:---:|:---:|:---:|:---:|
| 所有权 | 不拥有 | 拥有（栈上） | 拥有（堆上） | 不拥有 |
| 大小 | 动态或固定 | 固定（编译时） | 动态（运行时） | 动态 |
| 元素类型 | 任意 T | 任意 T | 任意 T | 字符类型 |
| 内存连续 | 要求连续 | 连续 | 连续 | 连续 |
| 可修改元素 | 是（span<T>） | 是 | 是 | 否（const 视图） |
| 堆分配 | 无 | 无 | 有 | 无 |
| C++ 版本 | C++20 | C++11 | C++98 | C++17 |
| 典型用途 | 函数参数、视图 | 固定大小容器 | 动态大小容器 | 字符串只读视图 |

***

### 9. 完整示例

```cpp
#include <iostream>
#include <span>
#include <vector>
#include <array>
#include <algorithm>
using namespace std;

int sum(span<const int> data) {
    int total = 0;
    for (int x : data) {
        total += x;
    }
    return total;
}

void sortSpan(span<int> data) {
    sort(data.begin(), data.end());
}

void printSpan(span<const int> data, const char* label) {
    cout << label << ": [";
    for (size_t i = 0; i < data.size(); ++i) {
        if (i > 0) cout << ", ";
        cout << data[i];
    }
    cout << "]\n";
}

int main() {
    vector<int> v = {5, 3, 1, 4, 2};
    array<int, 4> a = {40, 10, 30, 20};
    int c_arr[] = {100, 300, 200};

    cout << "=== Sum with span ===\n";
    cout << "sum(vector): " << sum(v) << "\n";
    cout << "sum(array):  " << sum(a) << "\n";
    cout << "sum(c_arr):  " << sum(c_arr) << "\n";

    cout << "\n=== Sort with mutable span ===\n";
    printSpan(v, "Before sort");
    sortSpan(v);
    printSpan(v, "After sort ");

    cout << "\n=== Subspan ===\n";
    span<const int> sv = v;
    printSpan(sv.first(3), "First 3 ");
    printSpan(sv.last(2), "Last 2  ");
    printSpan(sv.subspan(1, 3), "Sub(1,3)");

    cout << "\n=== Fixed-size span ===\n";
    span<int, 5> fixed = v;
    cout << "Fixed span size: " << fixed.size() << "\n";
    span<int> dynamic = fixed;
    cout << "Dynamic span from fixed: " << dynamic.size() << "\n";

    return 0;
}
```

***

### 10. 极简总结

**span = 非拥有的连续内存视图（零拷贝）| 类似 string_view 但适用于任意类型 | 函数参数替代 (const T*, size_t) | 支持固定大小和动态大小 | 不拥有数据，注意悬空风险 | 只适用于连续内存 | C++20**

***

### 相关阅读

- [string-view与const-string引用](./02-string-view与const-string引用.md)
- [STL容器底层实现](./00-STL容器底层实现.md)
- [std-array与C数组](./18-std-array与C数组.md)

***