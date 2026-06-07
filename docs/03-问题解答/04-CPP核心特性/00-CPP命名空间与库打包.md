# C++命名空间与库打包
> 📖 相关章节：[命名空间与编码规范](../../02-CPP/02-命名空间与编码规范.md)

### 1. 一句话结论

**命名空间解决名字冲突，库打包解决代码复用。两者组合 = 用 `命名空间::名字` 调用库中的功能。**

***

### 2. 命名空间的作用

#### 1. 核心问题：名字冲突

```cpp
// 团队A写的库
void process(int x);

// 团队B写的库
void process(int x);  // 冲突！同名函数

// 自己的代码
void process(int x);  // 又冲突！
```

#### 2. 解决方案：命名空间

```cpp
namespace TeamA {
    void process(int x);
}

namespace TeamB {
    void process(int x);
}

// 调用时明确指定
TeamA::process(42);
TeamB::process(42);
```

**命名空间 = 给名字加前缀，防止冲突。**

***

### 3. 命名空间的定义与使用

#### 1. 基本定义

```cpp
namespace MyMath {
    int add(int a, int b) { return a + b; }

    class Calculator {
    public:
        static int sub(int a, int b) { return a - b; }
    };

    const double PI = 3.14159265;
}
```

#### 2. 三种使用方式

```cpp
// 方式1：完全限定名（最安全）
int result = MyMath::add(1, 2);

// 方式2：using 声明（引入单个名字）
using MyMath::add;
int result = add(1, 2);  // 不用写 MyMath::

// 方式3：using 指令（引入整个命名空间）
using namespace MyMath;
int result = add(1, 2);
double pi = PI;
```

#### 3. 三种方式对比

| 方式 | 语法 | 安全性 | 推荐度 |
|:----:|:----:|:------:|:------:|
| 完全限定名 | `NS::func()` | 最高 | 推荐 |
| using 声明 | `using NS::func;` | 较高 | 推荐 |
| using 指令 | `using namespace NS;` | 最低 | 不推荐 |

***

### 4. 嵌套命名空间

```cpp
namespace Company {
    namespace Project {
        namespace Module {
            void func();
        }
    }
}

// 调用
Company::Project::Module::func();
```

#### 1. C++17 嵌套命名空间简写

```cpp
// C++17 之前
namespace Company { namespace Project { namespace Module {
    void func();
}}}

// C++17 简写
namespace Company::Project::Module {
    void func();
}
```

两种写法完全等价，C++17 简写更清晰。

***

### 5. 匿名命名空间

#### 1. 语法

```cpp
namespace {
    int internalVar = 42;       // 只在本文件可见
    void internalFunc() {}      // 只在本文件可见
}
```

#### 2. 作用：替代 static

```cpp
// C 方式：static 限制文件作用域
static int internalVar = 42;

// C++ 方式：匿名命名空间（推荐）
namespace {
    int internalVar = 42;
}
```

**匿名命名空间 vs static 对比**：

| 特性 | `static` | 匿名命名空间 |
|:----:|:--------:|:----------:|
| 限制可见性 | 只限当前翻译单元 | 只限当前翻译单元 |
| 适用于函数 | 可以 | 可以 |
| 适用于类 | 不可以 | 可以 |
| 适用于模板 | 不可以 | 可以 |
| 推荐度 | C 风格 | C++ 推荐 |

#### 3. 匿名命名空间的典型用法

```cpp
// mymodule.cpp
#include "mymodule.h"

namespace {
    // 辅助函数，只在本文档内部使用
    bool validateInput(int x) {
        return x >= 0 && x <= 100;
    }

    // 内部常量
    const int MAX_RETRY = 3;
}

// 公开接口
void MyModule::process(int value) {
    if (validateInput(value)) {
        // ...
    }
}
```

***

### 6. using 声明与 using 指令

#### 1. using 声明：引入单个名字

```cpp
using std::cout;
using std::endl;
using std::string;

cout << "hello" << endl;  // 不用写 std::
string name = "world";
```

#### 2. using 指令：引入整个命名空间

```cpp
using namespace std;

cout << "hello" << endl;
string name = "world";
vector<int> vec;
```

#### 3. using 指令的危险

```cpp
using namespace std;

// 如果自己定义了 distance 函数
int distance(int a, int b) { return abs(a - b); }

// std 中也有 distance（在 <iterator> 中）
// 一旦包含了 <iterator>，调用 distance 时产生歧义！
```

#### 4. 最佳实践

```cpp
// 头文件中：绝对不要用 using 指令
// myheader.h
#ifndef MYHEADER_H
#define MYHEADER_H

#include <string>

// 不要写 using namespace std;
// 因为包含此头文件的代码会被污染

class MyClass {
    std::string name_;  // 必须写 std::
};

#endif

// 源文件中：可以用 using 声明，慎用 using 指令
// mysource.cpp
#include "myheader.h"
#include <iostream>

using std::cout;     // 推荐：只引入需要的
using std::endl;

void MyClass::print() {
    cout << name_ << endl;
}
```

***

### 7. 命名空间别名

```cpp
namespace VeryLongNamespaceName {
    void func();
}

// 起别名
namespace VLN = VeryLongNamespaceName;

VLN::func();  // 等价于 VeryLongNamespaceName::func()
```

#### 1. 实际应用

```cpp
namespace fs = std::filesystem;           // C++17
namespace chrono = std::chrono;           // C++11
namespace bp = boost::process;            // Boost 库

fs::path p = "/usr/local";
auto now = chrono::system_clock::now();
```

***

### 8. 头文件与命名空间

#### 1. 头文件中的命名空间

```cpp
// mymath.h
#ifndef MYMATH_H
#define MYMATH_H

#include <vector>

namespace MyMath {
    int add(int a, int b);
    double average(const std::vector<int>& values);

    class Calculator {
    public:
        static int sub(int a, int b);
        int multiply(int a, int b);
    };
}

#endif
```

#### 2. 源文件中的命名空间

```cpp
// mymath.cpp
#include "mymath.h"

// 方式1：完全限定
int MyMath::add(int a, int b) { return a + b; }

// 方式2：重新打开命名空间
namespace MyMath {
    double average(const std::vector<int>& values) {
        if (values.empty()) return 0.0;
        int sum = 0;
        for (int v : values) sum += v;
        return static_cast<double>(sum) / values.size();
    }

    int Calculator::sub(int a, int b) { return a - b; }

    int Calculator::multiply(int a, int b) { return a * b; }
}
```

#### 3. 关键规则

| 规则 | 说明 |
|:----:|:----:|
| 头文件必须声明命名空间 | 否则使用者无法正确调用 |
| 命名空间可以跨文件 | 多个 .cpp 可以打开同一个命名空间 |
| 头文件不要用 using 指令 | 会污染包含者的全局空间 |
| 头文件不要放 using 声明 | 同样会污染 |

***

### 9. 库的打包与发布

#### 1. 静态库打包流程

```bash
# 1. 编译源文件为目标文件
g++ -c mymath.cpp -o mymath.o

# 2. 打包为静态库
ar rcs libmymath.a mymath.o

# 3. 使用静态库
g++ main.cpp -lmymath -L. -o main
```

#### 2. 动态库打包流程

```bash
# 1. 编译为位置无关代码
g++ -fPIC -c mymath.cpp -o mymath.o

# 2. 打包为动态库
g++ -shared -o libmymath.so mymath.o

# 3. 使用动态库
g++ main.cpp -lmymath -L. -o main
```

#### 3. 静态库 vs 动态库对比

| 对比项 | 静态库（.lib/.a） | 动态库（.dll/.so） |
|:------:|:----------------:|:----------------:|
| 链接时机 | 编译时 | 运行时 |
| 代码位置 | 打包进可执行文件 | 独立文件 |
| 可执行文件大小 | 较大 | 较小 |
| 更新方式 | 需重新编译 | 替换库文件即可 |
| 部署 | 单文件即可 | 需附带库文件 |
| 启动速度 | 较快 | 略慢（需加载） |
| 内存共享 | 不可 | 多进程共享 |

#### 4. 库的目录结构

```
MyLib/
├── include/
│   └── mymath.h        # 公开头文件（含命名空间声明）
├── src/
│   └── mymath.cpp      # 实现源文件
├── lib/
│   ├── libmymath.a     # 静态库
│   └── libmymath.so    # 动态库
└── CMakeLists.txt      # 构建脚本
```

#### 5. 使用第三方库的标准流程

```bash
# 1. 包含头文件（头文件里有命名空间声明）
# 2. 链接库文件
# 3. 使用时带上命名空间

# 示例：使用 OpenCV
g++ main.cpp -lopencv_core -lopencv_imgproc -o main
```

```cpp
#include <opencv2/opencv.hpp>

int main() {
    cv::Mat image = cv::imread("photo.jpg");  // cv 命名空间
    cv::imshow("Window", image);
    cv::waitKey(0);
    return 0;
}
```

***

### 10. 命名空间与名称修饰

编译器看到 `MyMath::add(int, int)` 后，会将其修饰为内部符号：

```cpp
namespace MyMath {
    int add(int a, int b);
}
// GCC 修饰后的符号名：_ZN6MyMath3addEii
```

**名称修饰规则**：

| 原始名字 | 修饰后（GCC） |
|:--------:|:------------:|
| `MyMath::add(int, int)` | `_ZN6MyMath3addEii` |
| `MyMath::Calculator::sub(int, int)` | `_ZN6MyMath9Calculator3subEii` |
| `A::B::C::func()` | `_ZN1A1B1C4funcEv` |

**查看修饰后名字**：

```bash
# Linux
nm libmymath.a | grep add
c++filt _ZN6MyMath3addEii   # 反修饰

# MSVC
dumpbin /symbols mymath.lib
```

***

### 11. 常见陷阱

#### 1. 陷阱1：头文件中 using namespace

```cpp
// bad_lib.h
using namespace std;  // 灾难！所有包含此头文件的代码都被污染

class MyClass {
    string name;  // 看起来没问题，但强制所有用户也用 using namespace std
};
```

#### 2. 陷阱2：命名空间不匹配

```cpp
// 声明在 MyMath 中
namespace MyMath {
    int add(int a, int b);
}

// 定义写错命名空间
namespace Math {  // 错误！不是同一个命名空间
    int add(int a, int b) { return a + b; }
}
// 链接错误：找不到 MyMath::add 的定义
```

#### 3. 陷阱3：参数依赖查找（ADL）的意外

```cpp
namespace NS {
    class MyClass {};
    void func(MyClass& obj);  // NS 中的 func
}

NS::MyClass obj;
func(obj);  // 能找到 NS::func！即使没有 using
// 这就是 ADL（参数依赖查找）：根据参数的命名空间查找函数
```

***

### 12. 极简总结

| 要点 | 内容 |
|:----:|:----:|
| 命名空间作用 | 防止名字冲突，给名字加前缀 |
| 三种使用方式 | 完全限定 > using 声明 > using 指令 |
| 匿名命名空间 | 替代 static，限制符号在当前翻译单元 |
| 嵌套简写 | C++17: `namespace A::B::C {}` |
| 头文件规则 | 绝对不要 `using namespace` |
| 库打包 | 头文件 + 编译产物（.a/.so/.lib/.dll） |
| 名称修饰 | 编译器将命名空间编入符号名，链接时匹配 |

***

### 相关阅读

- [static关键字](../01-基础概念/16-static关键字.md)
- [什么是ODR单定义规则](../03-编译与链接/01-什么是ODR单定义规则.md)
- [什么是名称修饰Name-Mangling](../03-编译与链接/09-什么是名称修饰Name-Mangling.md)