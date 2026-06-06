# 什么是C++20 Modules
> 📖 相关章节：[STL容器](../../02-CPP/14-STL容器.md)、[STL算法](../../02-CPP/15-STL算法与迭代器.md)、[正则表达式](../../02-CPP/16-正则表达式.md)、[文件系统](../../02-CPP/19-文件系统库.md)

> "Modules are the most impactful change to how we organize C++ code since the invention of headers." — Bjarne Stroustrup

***

### 1. 要点直击

C++20 Modules 是一种替代头文件的全新代码组织机制，通过 `module` 声明和 `import` 导入实现真正的隔离编译，消除宏污染、加快编译速度、提升封装性。

***

### 2. 模块基础：module 声明与 import 导入

模块的核心语法由两个关键字构成：`module` 用于声明一个模块，`import` 用于导入一个模块。

```cpp
// math_module.cppm —— 模块接口文件
export module math_module;

export int add(int a, int b) {
    return a + b;
}

export double pi() {
    return 3.14159265358979;
}
```

```cpp
// main.cpp —— 消费者
import math_module;
#include <iostream>

int main() {
    std::cout << add(1, 2) << "\n";
    std::cout << pi() << "\n";
    return 0;
}
```

关键规则：

| 规则 | 说明 |
|------|------|
| `export module` | 声明模块名并导出，放在模块接口文件顶部 |
| `module` | 声明模块名但不导出，用于模块实现文件 |
| `export` | 修饰需要对外暴露的声明 |
| `import` | 导入一个模块，只引入导出的名字 |
| 无 `export` | 模块内部符号，外部不可见 |

```cpp
// internal_helper —— 不导出，模块外部不可见
int multiply(int a, int b) {
    return a * b;
}

export int square(int x) {
    return multiply(x, x);
}
```

***

### 3. 接口与实现的分离

模块支持接口与实现分离，类似头文件与源文件的拆分方式，但语义更清晰。

```cpp
// shape.cppm —— 模块接口单元
export module shape;

export class Circle {
    double radius_;
public:
    Circle(double r);
    double area() const;
};
```

```cpp
// shape_impl.cpp —— 模块实现单元
module shape;

Circle::Circle(double r) : radius_(r) {}

double Circle::area() const {
    return 3.14159265358979 * radius_ * radius_;
}
```

接口与实现分离的要点：

| 方面 | 说明 |
|------|------|
| 接口单元 | 文件扩展名通常为 `.cppm` 或 `.ixx`，含 `export module` |
| 实现单元 | 文件扩展名通常为 `.cpp`，含 `module`（无 export） |
| 一个模块 | 只能有一个接口单元，可以有多个实现单元 |
| 编译顺序 | 接口单元先编译，实现单元后编译 |

***

### 4. 模块分区（Partition）

大型模块可以拆分为多个分区，每个分区是模块的一部分，由模块名加分区名标识。

```cpp
// container_vector.cppm —— 接口分区
export module container:vector;

export class Vector {
    float data_[3];
public:
    Vector(float x, float y, float z);
    float& operator[](int i);
};
```

```cpp
// container_list.cppm —— 另一个接口分区
export module container:list;

export class LinkedList {
public:
    void push_back(int val);
    int front() const;
};
```

```cpp
// container.cppm —— 主接口单元，汇聚所有分区
export module container;

export import :vector;
export import :list;
```

```cpp
// main.cpp
import container;

int main() {
    Vector v(1.0f, 2.0f, 3.0f);
    LinkedList lst;
    lst.push_back(42);
    return 0;
}
```

分区类型对比：

| 分区类型 | 语法 | 说明 |
|----------|------|------|
| 接口分区 | `export module M:P` | 可被外部 import |
| 实现分区 | `module M:P` | 仅模块内部可见 |
| 主接口汇聚 | `export import :P` | 将分区重新导出 |

***

### 5. 头文件单元（Header Unit）

头文件单元允许将传统头文件作为模块导入，是迁移的桥梁。

```cpp
// 传统方式
#include <iostream>
#include <vector>

// 头文件单元方式
import <iostream>;
import <vector>;
```

```cpp
// 自定义头文件也可作为头文件单元
// mylib.h
#pragma once
inline int helper() { return 42; }

// 使用
import "mylib.h";

int main() {
    return helper();
}
```

头文件单元与 `#include` 的区别：

| 特性 | `#include` | `import <header>` |
|------|-----------|-------------------|
| 宏定义 | 可见 | 不可见（隔离） |
| 重复包含 | 需要头文件卫士 | 天然幂等 |
| 编译模型 | 文本包含 | 预编译 BMI |
| 符号可见性 | 全部 | 仅导出符号 |

> ⚠️ 注意：头文件单元不导入宏定义，如果依赖宏则仍需 `#include`。

***

### 6. 全局模块片段（Global Module Fragment）

全局模块片段用于在模块中 `#include` 传统头文件，这些内容不属于模块，但模块内部可用。

```cpp
module;

#include <iostream>
#include <string>
#include <vector>

export module myapp;

export void print_greeting(const std::string& name) {
    std::cout << "Hello, " << name << "!\n";
}

export std::vector<int> make_sequence(int n) {
    std::vector<int> v(n);
    for (int i = 0; i < n; ++i)
        v[i] = i * i;
    return v;
}
```

全局模块片段的规则：

| 规则 | 说明 |
|------|------|
| 位置 | 必须是文件最顶部，`module;` 之前不能有其他声明 |
| 用途 | 放置 `#include` 指令 |
| 可见性 | 包含的内容在模块内可用，但不会被导出 |
| 限制 | 不能在全局模块片段中声明模块自己的实体 |

```cpp
module;

#include <cstring>

export module utils;

export bool is_empty(const char* s) {
    return s == nullptr || std::strlen(s) == 0;
}
```

***

### 7. Modules vs 头文件：全面对比

| 对比维度 | 头文件（#include） | 模块（import） |
|----------|-------------------|----------------|
| 编译模型 | 文本替换，每次重新解析 | 预编译 BMI，一次编译多次使用 |
| 宏污染 | 宏泄漏到所有包含文件 | 宏不会泄漏，完全隔离 |
| 重复包含 | 需要 `#pragma once` 或卫士 | 天然幂等，import 多次等价一次 |
| 封装性 | 所有声明可见 | 仅 export 的声明可见 |
| 编译速度 | 大型项目极慢 | 显著提升（实测 3-10 倍） |
| 依赖顺序 | 依赖 include 顺序 | 不依赖导入顺序 |
| 工具支持 | 成熟 | 仍在发展中 |
| 迁移成本 | 无 | 需要改造代码和构建系统 |

编译速度对比示例（大型项目实测）：

```
项目规模：50 万行代码
头文件模式：完整编译 120 秒
模块模式：  完整编译  25 秒
增量编译：头文件 45 秒 → 模块 8 秒
```

***

### 8. 从头文件迁移到模块

迁移策略分为渐进式和一步到位两种。推荐渐进式。

**策略一：从叶子模块开始**

```
迁移顺序：
1. 底层工具库（无依赖）→ 改造为模块
2. 中间层库（依赖底层）→ 改造为模块
3. 顶层应用 → 最后改造
```

**策略二：头文件单元过渡**

```cpp
// 阶段一：用 import <header> 替换 #include
// 之前
#include <string>
#include <vector>

// 之后
import <string>;
import <vector>;
```

**策略三：混合模式**

```cpp
// 模块中可以 #include（在全局模块片段中）
module;

#include <第三方库.h>

export module mymod;

// 传统 .cpp 文件也可以 import 模块
import mymod;
```

迁移注意事项：

| 问题 | 解决方案 |
|------|----------|
| 宏依赖 | 保留 `#include` 或在全局模块片段中包含 |
| 第三方库 | 使用头文件单元 `import <header>` |
| include guard | 模块天然不需要，直接删除 |
| 匿名命名空间 | 模块中无 export 的声明天然内部可见 |
| 链接兼容性 | 模块和头文件编译的代码可以互相链接 |

***

### 9. 编译器支持现状

| 编译器 | 最低版本 | 支持程度 | 备注 |
|--------|----------|----------|------|
| GCC | 11+ | 较完整 | 使用 `-fmodules-ts` |
| Clang | 16+ | 较完整 | 使用 `-fmodules` |
| MSVC | 19.28+ | 最成熟 | Visual Studio 2019 16.10+ |

各编译器编译命令：

```bash
# GCC
g++ -std=c++20 -fmodules-ts -c math_module.cppm -o math_module.o
g++ -std=c++20 -fmodules-ts main.cpp math_module.o -o main

# MSVC
cl /std:c++20 /EHsc /c math_module.cppm
cl /std:c++20 /EHsc main.cpp math_module.obj

# Clang
clang++ -std=c++20 -fmodules -c math_module.cppm --precompile -o math_module.pcm
clang++ -std=c++20 -fmodules math_module.pcm main.cpp -o main
```

> ⚠️ 平台注意：MSVC 默认使用 `.ixx` 作为模块接口扩展名，GCC/Clang 常用 `.cppm`。不同编译器的 BMI（预编译模块接口）格式不兼容。

***

### 10. CMake 集成

CMake 3.28+ 原生支持 C++20 模块，自动扫描依赖并生成正确的构建顺序。

```cmake
cmake_minimum_required(VERSION 3.28)
project(ModuleDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(demo
    main.cpp
)

target_sources(demo
    PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES FILES
        math_module.cppm
        shape.cppm
        shape_impl.cpp
)
```

CMake 模块相关特性：

| CMake 版本 | 特性 |
|------------|------|
| 3.28+ | 原生 C++ 模块支持（`FILE_SET TYPE CXX_MODULES`） |
| 3.25+ | 实验性支持（需 `CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP=1`） |
| < 3.25 | 不支持，需手动管理 BMI 依赖 |

手动管理 BMI 的 Makefile 示例：

```makefile
CXX = g++
FLAGS = -std=c++20 -fmodules-ts

all: main

math_module.gcm: math_module.cppm
	$(CXX) $(FLAGS) -c $< -o $@

main.o: main.cpp math_module.gcm
	$(CXX) $(FLAGS) -c $< -o $@

main: main.o
	$(CXX) $(FLAGS) $^ -o $@
```

***

### 11. 极简总结

| 概念 | 关键语法 | 核心作用 |
|------|----------|----------|
| 模块声明 | `export module M;` | 定义模块并导出 |
| 导入 | `import M;` | 引入模块的导出符号 |
| 导出 | `export int f();` | 标记对外可见的声明 |
| 分区 | `export module M:P;` | 拆分大模块 |
| 头文件单元 | `import <header>;` | 兼容传统头文件 |
| 全局模块片段 | `module; #include <...>` | 在模块中使用 #include |
| 实现单元 | `module M;` | 分离实现 |

迁移路线图：

```
1. 确认编译器支持 C++20 模块
2. CMake 升级到 3.28+
3. 从底层库开始，用头文件单元过渡
4. 逐步将 .h/.cpp 改为 .cppm 模块接口
5. 利用 CMake FILE_SET 管理模块依赖
6. 最终享受编译加速和封装性提升
```

核心收益：**编译速度提升 3-10 倍、宏隔离、封装性增强、依赖关系显式化**。

***

### 相关阅读

- [C++编译时间优化](../03-编译与链接/04-C++编译时间优化.md)
- [什么是C++23新特性](./15-什么是C++23新特性.md)
- [constexpr与consteval](./05-constexpr与consteval.md)

***