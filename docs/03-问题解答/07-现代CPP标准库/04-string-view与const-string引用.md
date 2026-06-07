# string_view 与 const string& 引用的区别
> 📖 相关章节：[STL容器](../../02-CPP/14-STL容器.md)、[字符串处理](../../01-C语言/07-字符串处理.md)

### 1. 要点直击

**string_view 是不拥有字符串的轻量级视图（零拷贝），const string& 是拥有字符串的常量引用（绑定已有对象）。函数参数只读用 string_view，需要存储或绑定临时对象用 const string&。**

***

### 2. 核心定义

| | string_view | const string& |
|---|---|---|
| 是什么 | 指向字符序列的非拥有视图 | 绑定到 string 对象的常量引用 |
| 是否拥有数据 | 不拥有，只观察 | 不拥有，但依赖已有 string 的存在 |
| 内存开销 | 2 个指针大小（指针+长度） | 1 个指针大小 |
| 可接受参数 | string、const char*、字符数组、子串 | 只能绑定 string（含隐式构造的临时 string） |
| C++ 版本 | C++17 | C++98 |

**本质区别**：

```cpp
// string_view：只记录"在哪里"和"有多长"
std::string_view sv = "hello";  // sv = {ptr="hello", len=5}
// 不分配内存，不拷贝字符

// const string&：绑定到一个 string 对象
std::string s = "hello";
const std::string& ref = s;     // ref 是 s 的别名
// 如果传 const char*，会隐式构造临时 string，产生拷贝
```

***

### 3. 生活类比

| | string_view | const string& |
|---|---|---|
| 类比 | 看橱窗里的商品 | 借了别人的书 |
| 说明 | 你能看到商品，但不拥有它；橱窗撤了你就看不到了 | 书存在但你不拥有；书被主人收走你就没法用了 |
| 关键区别 | 不需要商品属于你，只要能看到就行 | 必须有一本真实的书存在 |

**具体场景**：

- **string_view**：你路过商店橱窗，看到里面摆着商品。你不需要买下商品就能看到它，但如果商店把橱窗撤了（原字符串销毁），你看到的就没了。
- **const string&**：你从图书馆借了一本书。书是真实存在的，你只是借来看，不能改。但书必须一直存在于图书馆里。

***

### 4. 零拷贝优势

**string_view 的核心价值：避免不必要的字符串拷贝**

```cpp
// 场景：函数需要读取字符串内容

// 方式1：const string& — 传 const char* 时会隐式构造临时 string
void print1(const std::string& s) {
    std::cout << s << "\n";
}
print1("hello");   // 隐式构造临时 string，分配堆内存，拷贝字符！

// 方式2：string_view — 任何字符串都能零拷贝传入
void print2(std::string_view sv) {
    std::cout << sv << "\n";
}
print2("hello");   // 零拷贝，sv 直接指向字符串字面量

std::string s = "world";
print2(s);         // 零拷贝，sv 指向 s 的内部数据

char buf[] = "buffer";
print2(buf);       // 零拷贝，sv 指向栈上的字符数组
```

**子串操作更是零拷贝**：

```cpp
std::string s = "hello world";

// string::substr — 返回新的 string，分配内存，拷贝字符
std::string sub1 = s.substr(0, 5);   // 拷贝 "hello"

// string_view::substr — 返回新的 string_view，零拷贝
std::string_view sv = s;
std::string_view sub2 = sv.substr(0, 5);  // 只调整指针和长度
```

**性能对比**：

```cpp
#include <string_view>
#include <string>
#include <iostream>

void processRef(const std::string& s) {
    // 传 const char* 时：1次堆分配 + 1次拷贝
}

void processView(std::string_view sv) {
    // 任何输入：0次堆分配 + 0次拷贝
}

int main() {
    // const string& 版本：每次都构造临时 string
    processRef("short");                  // 堆分配 + 拷贝
    processRef(std::string("temp"));      // 堆分配 + 拷贝

    // string_view 版本：零拷贝
    processView("short");                 // 零拷贝
    processView(std::string("temp"));     // 零拷贝（指向临时对象的内部数据，但临时对象在此表达式内有效）

    return 0;
}
```

***

### 5. 悬空风险（string_view 的最大陷阱）

**string_view 不延长生命周期，指向的对象销毁后，string_view 变成悬空引用**

```cpp
// 陷阱1：指向临时 string
std::string_view sv1 = std::string("hello");  // 临时 string 在表达式结束后销毁
// sv1 现在是悬空的！使用 sv1 是未定义行为

// 陷阱2：函数返回 string_view 指向局部变量
std::string_view getPrefix() {
    std::string s = "prefix_";
    return s;   // 返回指向局部变量 s 的 string_view
}   // s 销毁，返回的 string_view 悬空

// 陷阱3：容器操作使 string_view 失效
std::vector<std::string> vec = {"hello", "world"};
std::string_view sv2 = vec[0];
vec.push_back("new");   // vector 可能重新分配内存
// sv2 悬空！之前的内存已被释放

// 陷阱4：string_view 指向的 string 被修改
std::string s = "hello";
std::string_view sv3 = s;
s = "modified";   // string 可能重新分配内部缓冲区
// sv3 可能悬空或指向旧数据
```

**const string& 的安全性**：

```cpp
// const string& 绑定临时对象时，临时对象的生命周期延长到引用的生命周期
const std::string& ref = std::string("hello");  // 临时对象存活到 ref 销毁
// ref 安全，临时对象仍然存在

// 但注意：函数返回 const string& 指向局部变量同样危险
const std::string& getPrefix() {
    std::string s = "prefix_";
    return s;   // 同样悬空！
}
```

**安全使用规则**：

```cpp
// 规则1：string_view 只用于函数参数（生命周期在调用期间确定）
void process(std::string_view sv);  // 安全：调用者保证参数在调用期间有效

// 规则2：不要用 string_view 存储字符串
class Bad {
    std::string_view stored_;   // 危险！
public:
    void set(std::string_view sv) { stored_ = sv; }  // 可能悬空
};

// 规则3：不要返回 string_view 指向局部或临时对象
// 安全的返回：指向静态数据或调用者拥有的数据
std::string_view getFileExtension(std::string_view filename) {
    auto pos = filename.rfind('.');
    if (pos == std::string_view::npos) return "";
    return filename.substr(pos);  // 安全：指向调用者传入的 filename
}

// 规则4：string_view 不保证以 '\0' 结尾
void legacyCFunc(const char* s);  // 期望 C 风格字符串
std::string_view sv = "hello";
// legacyCFunc(sv.data());  // 危险！sv 可能不以 '\0' 结尾
legacyCFunc(std::string(sv).c_str());  // 安全但需要拷贝
```

***

### 6. 函数参数选择指南

**决策树**：

```
函数需要字符串参数？
├── 只读访问？
│   ├── 需要存储？→ const string& 或 string（拷贝存储）
│   ├── 需要以 '\0' 结尾？→ const char* 或 const string&
│   └── 只在函数内使用？→ string_view（首选）
└── 需要修改？
    ├── 修改已有对象？→ string&
    └── 获取新值？→ string（按值返回或参数）
```

**具体场景**：

```cpp
// 场景1：只读函数参数 → string_view
size_t countWords(std::string_view text);  // 最佳选择

// 场景2：需要存储字符串 → const string& 或按值
class Config {
    std::string name_;  // 需要存储
public:
    void setName(std::string name) {   // 按值传递 + move
        name_ = std::move(name);
    }
};

// 场景3：需要调用 C 接口 → const char* 或 const string&
void callCAPI(const std::string& s) {
    c_function(s.c_str());  // 保证以 '\0' 结尾
}

// 场景4：需要子串 → string_view
void parseHeader(std::string_view header) {
    auto colon = header.find(':');
    auto key = header.substr(0, colon);      // 零拷贝子串
    auto value = header.substr(colon + 2);   // 零拷贝子串
}

// 场景5：重载同时支持 → string_view 一统天下
void process(std::string_view sv);  // 一个版本覆盖所有输入类型
```

***

### 7. 对比表格

| 特性 | string_view | const string& |
|------|:---:|:---:|
| 所有权 | 不拥有，只观察 | 不拥有，绑定已有对象 |
| 生命周期依赖 | 依赖底层字符序列的存在 | 依赖绑定的 string 对象 |
| 可修改性 | 不可修改（const 视图） | 不可修改（const 引用） |
| 子串操作 | substr 零拷贝 | substr 返回新 string（拷贝） |
| 接受 const char* | 直接接受，零拷贝 | 隐式构造临时 string，有拷贝 |
| 接受 string | 直接接受，零拷贝 | 直接绑定，零拷贝 |
| 接受字符数组 | 直接接受，零拷贝 | 隐式构造临时 string，有拷贝 |
| 内存开销 | 2 个指针（16 字节/64 位） | 1 个指针（8 字节/64 位） |
| 保证 '\0' 结尾 | 不保证 | 保证（c_str()） |
| 临时对象安全 | 不安全（不延长生命周期） | 安全（延长临时对象生命周期） |
| 存储安全 | 不安全（可能悬空） | 相对安全 |
| C++ 版本 | C++17 | C++98 |
| 典型用途 | 函数只读参数、子串操作 | 需要存储、需要 C 接口 |

***

### 8. 完整示例

```cpp
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
using namespace std;

size_t countWords(string_view text) {
    size_t count = 0;
    bool inWord = false;
    for (char c : text) {
        if (c == ' ' || c == '\t' || c == '\n') {
            inWord = false;
        } else if (!inWord) {
            inWord = true;
            ++count;
        }
    }
    return count;
}

void printSubstrings(string_view sv) {
    cout << "Original: \"" << sv << "\" (len=" << sv.size() << ")\n";
    cout << "  substr(0,5): \"" << sv.substr(0, 5) << "\"\n";
    cout << "  substr(6):   \"" << sv.substr(6) << "\"\n";
}

void demonstrateDangling() {
    string_view sv;

    {
        string temp = "I'm temporary";
        sv = temp;
        cout << "Inside scope: \"" << sv << "\"\n";
    }

    // sv 现在悬空！不要使用
    // cout << sv << "\n";  // 未定义行为
    cout << "(string_view is now dangling after scope exit)\n";
}

int main() {
    // 零拷贝：各种输入类型
    cout << "=== Zero-copy with string_view ===\n";
    string s = "hello world from string";
    cout << "Words (string): " << countWords(s) << "\n";
    cout << "Words (literal): " << countWords("hello from literal") << "\n";
    cout << "Words (array): " << countWords("hello from array") << "\n";

    // 子串操作
    cout << "\n=== Substring (zero-copy) ===\n";
    printSubstrings("Hello, string_view world!");

    // 悬空风险
    cout << "\n=== Dangling risk ===\n";
    demonstrateDangling();

    // const string& 的隐式构造开销
    cout << "\n=== const string& implicit construction ===\n";
    auto countRef = [](const string& s) -> size_t {
        cout << "  [const string&] received string of length " << s.size() << "\n";
        size_t count = 0;
        bool inWord = false;
        for (char c : s) {
            if (c == ' ') inWord = false;
            else if (!inWord) { inWord = true; ++count; }
        }
        return count;
    };
    cout << "Words: " << countRef("implicit construction happens here") << "\n";

    return 0;
}
```

***

### 9. 极简总结

**string_view = 不拥有的轻量视图（零拷贝）| const string& = 绑定已有 string 的常量引用 | 函数只读参数首选 string_view | 不要用 string_view 存储或返回指向临时对象 | string_view 不保证 '\0' 结尾 | 需要存储或调 C 接口用 const string&**

***

### 相关阅读

- [什么是SSO小字符串优化](../02-内存与底层/16-什么是SSO小字符串优化.md)
- [std-span](./05-std-span.md)
- [STL容器底层实现](./01-STL容器底层实现.md)

***