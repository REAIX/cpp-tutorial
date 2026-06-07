# CPP编码规范与命名约定
> 📖 相关章节：[编码规范](../../04-工程实践/00-编码规范.md)、[设计模式](../../04-工程实践/03-设计模式.md)、[单元测试](../../04-工程实践/05-单元测试.md)、[代码审查](../../04-工程实践/07-代码审查.md)

> "编码规范是团队的交通规则——没有规则，各开各的，必然撞车。"

***

### 1. 核心定义

- **编码规范** = 团队协作的"交通规则"，让代码风格统一、可读、可维护
- **命名约定** = 变量/函数/类等的命名规则，是编码规范中最核心的部分

| | 编码规范 | 命名约定 |
|---|---|---|
| 关注点 | 代码风格、组织方式、最佳实践 | 标识符的命名规则 |
| 作用 | 统一团队代码风格 | 让名字传达含义 |
| 范围 | 缩进、括号、头文件、类设计…… | 类名、函数名、变量名、常量名…… |
| 关系 | 包含命名约定 | 是编码规范的子集 |

***

### 2. 生活类比

**编码规范 = 交通规则**

| 比喻 | 含义 |
|------|------|
| 交通规则 | 编码规范——没有规则，各开各的，必然撞车（代码冲突） |
| 靠右行驶 vs 靠左行驶 | 不同规范风格（Google vs LLVM）——选哪种都行，但团队必须统一 |
| 红绿灯 | 自动化工具（clang-format）——机器执行规则，不用人吵 |
| 违章罚款 | CI 检查不通过——不符合规范的代码合不进去 |

**命名约定 = 门牌号**

| 比喻 | 含义 |
|------|------|
| 门牌号有规则 | 变量名有规则——门牌号有规则才能找到人，变量名有规则才能读懂代码 |
| 1号楼2单元301 | `HttpRequest`——一看就知道是"请求"相关的"类" |
| 门牌号乱写 | `var1`、`tmp`、`data`——谁也不知道是什么 |

***

### 3. 三大主流规范对比

| 规范 | 来源 | 特点 | 适用场景 |
|------|------|------|---------|
| Google C++ Style Guide | Google | 严格、全面、偏保守（禁用异常、限制C++特性） | 大型团队、开源项目 |
| LLVM Coding Standards | LLVM项目 | 现代、灵活（允许异常、鼓励现代C++） | 编译器/工具链项目 |
| C++ Core Guidelines | Bjarne Stroustrup / Herb Sutter | 权威、原则导向（不规定具体风格，给原则） | 通用参考 |

**关键差异**：

| 争议点 | Google | LLVM | Core Guidelines |
|--------|--------|------|-----------------|
| 异常 | ❌ 禁用 | ✅ 允许 | ✅ 允许 |
| RTTI | ❌ 禁用 | ✅ 允许 | 谨慎使用 |
| 函数命名 | PascalCase | PascalCase | 不限定 |
| 成员变量 | trailing_ `_` | trailing_ `_` | 不限定 |
| 代码行宽 | 80字符 | 80字符 | 不限定 |
| `auto` | 限制使用 | 鼓励使用 | 鼓励使用 |

**选择建议**：团队选一种，全员遵守。没有"最好的规范"，只有"大家都遵守的规范"。

***

### 4. 命名约定详解

#### 1. 各类标识符命名规则

| 标识符类型 | 风格 | 正确示例 | 错误示例 |
|-----------|------|---------|---------|
| 类名/结构体 | PascalCase | `MyClass`、`HttpRequest` | `myClass`、`my_class` |
| 函数名 | snake_case 或 camelCase | `get_size` / `getSize` | `GetSize`、`getsize` |
| 变量名 | snake_case | `count`、`user_name` | `Count`、`userName` |
| 常量 | kConstant 或 ALL_CAPS | `kMaxSize`、`MAX_BUFFER` | `kmaxsize`、`maxBuffer` |
| 成员变量 | trailing_underscore 或 m_prefix | `name_`、`count_` / `m_name` | `Name`、`_name` |
| 命名空间 | snake_case | `my_project`、`http_server` | `MyProject`、`HTTPServer` |
| 模板参数 | PascalCase | `T`、`Container`、`Allocator` | `t`、`container` |
| 宏 | ALL_CAPS | `MAX_SIZE`、`DEBUG_MODE` | `MaxSize`、`debug_mode` |
| 枚举值 | kConstant 或 ALL_CAPS | `kColorRed`、`COLOR_RED` | `colorRed`、`red` |

#### 2. 代码示例

```cpp
#include <cstddef>
#include <string>
#include <vector>

namespace http_server {

constexpr size_t kMaxBufferSize = 4096;
constexpr int MAX_RETRY_COUNT = 3;

#define DEBUG_MODE 1

template <typename T, typename Allocator = std::allocator<T>>
class Container {
public:
    Container() = default;
    size_t get_size() const { return data_.size(); }
    void add_item(const T& item) { data_.push_back(item); }

private:
    std::vector<T, Allocator> data_;
    size_t count_ = 0;
};

enum class Color {
    kRed,
    kGreen,
    kBlue
};

class HttpRequest {
public:
    std::string get_url() const { return url_; }
    void set_url(const std::string& url) { url_ = url; }

private:
    std::string url_;
    int timeout_ = 30;
};

}
```

#### 3. 命名原则

| 原则 | 说明 | 好 | 差 |
|------|------|---|---|
| 名副其实 | 名字要表达真实含义 | `elapsed_time` | `t`、`tmp` |
| 避免缩写 | 除非广为人知 | `window` | `wnd`、`w` |
| 区分用途 | 不同东西不同名字 | `source` / `destination` | `data1` / `data2` |
| 可读可说 | 能口头交流 | `customer_id` | `cid`、`c_id` |
| 适度长度 | 不过长不过短 | `find_user_by_id` | `fubi`、`find_the_user_by_the_user_id` |

***

### 5. 头文件组织规范

#### 1. include guard vs pragma once

| | include guard | pragma once |
|---|---|---|
| 标准 | C++标准 | 编译器扩展（但所有主流编译器都支持） |
| 可靠性 | 100%可靠 | 实际可靠（极少数边界情况可能出问题） |
| 代码量 | 多（3行） | 少（1行） |
| 冲突 | 宏名可能冲突（用全路径可避免） | 不存在冲突 |

```cpp
#ifndef HTTP_SERVER_REQUEST_H_
#define HTTP_SERVER_REQUEST_H_

namespace http_server {
class Request {
};
}

#endif
```

```cpp
#pragma once

namespace http_server {
class Request {
};
}
```

**建议**：新项目用 `#pragma once`，简单高效。

#### 2. 包含顺序

从本地到系统，减少隐藏依赖：

```
1. 对应头文件（my_class.h）      ← 自己的头文件排第一
2. C 系统头文件（<unistd.h>）    ← 纯C的系统头
3. C++ 系统头文件（<string>）    ← C++标准库
4. 其他库头文件（<boost/xxx>）   ← 第三方库
5. 项目内头文件（"my_project/xxx.h"） ← 项目内其他头文件
```

```cpp
#include "my_class.h"

#include <cstdio>
#include <cstring>

#include <string>
#include <vector>
#include <memory>

#include <boost/asio.hpp>

#include "my_project/utils.h"
#include "my_project/config.h"
```

#### 3. 前向声明 vs 包含头文件

> 详见 [前向声明与包含头文件](../03-编译与链接/03-前向声明与包含头文件.md)

| | 前向声明 | 包含头文件 |
|---|---|---|
| 编译速度 | 快（不需要解析整个头文件） | 慢 |
| 适用场景 | 只用指针/引用 | 需要知道完整定义 |
| 风险 | 可能与定义不一致 | 无 |

```cpp
#pragma once
#include <memory>
#include <string>

namespace http_server {

class Connection;
class Response;

class HttpRequest {
public:
    void process(Connection* conn);
    std::unique_ptr<Response> create_response();

private:
    std::string url_;
};

}
```

***

### 6. 类设计规范

#### 1. Rule of 0/3/5/7

> 详见 [Rule-of-Five与Rule-of-Zero](../04-CPP核心特性/29-Rule-of-Five与Rule-of-Zero.md)

| 规则 | 含义 | 适用场景 |
|------|------|---------|
| Rule of 0 | 不需要任何特殊成员函数 | 使用智能指针/RAII类型管理资源 |
| Rule of 3 | 如果定义了析构/拷贝构造/拷贝赋值之一，就三个都定义 | 管理原始资源 |
| Rule of 5 | Rule of 3 + 移动构造 + 移动赋值 | 需要移动语义 |
| Rule of 7 | Rule of 5 + swap 函数 | 需要强异常安全保证 |

#### 2. 接口类命名

```cpp
class IShape {
public:
    virtual ~IShape() = default;
    virtual double area() const = 0;
    virtual double perimeter() const = 0;
};

class ISerializable {
public:
    virtual ~ISerializable() = default;
    virtual std::string serialize() const = 0;
    virtual void deserialize(const std::string& data) = 0;
};
```

| 风格 | 示例 | 常见于 |
|------|------|--------|
| I 前缀 | `IShape`、`ISerializable` | Windows/COM 生态 |
| Interface 后缀 | `ShapeInterface` | Java 风格 |
| 无特殊标记 | `Shape`（纯虚类本身就是接口） | 现代 C++ |

#### 3. 成员访问控制

```cpp
class UserManager {
public:
    UserManager() = default;

    std::string get_user_name(int id) const;
    void set_user_name(int id, const std::string& name);
    bool remove_user(int id);

protected:
    virtual void on_user_removed(int id);

private:
    struct UserInfo {
        int id;
        std::string name;
    };

    std::vector<UserInfo> users_;
    int next_id_ = 1;
};
```

**原则**：

| 原则 | 说明 |
|------|------|
| 数据 private | 成员变量一律 private，通过接口访问 |
| 接口 public | 对外提供的方法放 public |
| 钩子 protected | 供子类重写的虚函数放 protected |
| 实现细节 private | 内部辅助函数、嵌套类型放 private |

***

### 7. const正确性

#### 1. 核心原则：能用const就用const

```cpp
class TextProcessor {
public:
    const std::string& get_text() const {
        return text_;
    }

    void append(const std::string& suffix) {
        text_ += suffix;
    }

    size_t find_keyword(const std::string& keyword) const {
        return text_.find(keyword);
    }

private:
    std::string text_;
    mutable size_t cache_length_ = 0;
    mutable bool length_cached_ = false;
};
```

#### 2. const 传播

| 位置 | 用法 | 说明 |
|------|------|------|
| 变量 | `const int max = 100;` | 值不可修改 |
| 参数 | `void foo(const std::string& s)` | 函数内不可修改参数 |
| 返回值 | `const std::string& get() const` | 调用方不可修改返回的引用 |
| 成员函数 | `size_t size() const` | 不修改任何成员变量 |
| 迭代器 | `const_iterator` | 只读遍历 |

#### 3. const 正确 vs const 缺失

```cpp
class BadExample {
public:
    std::string& get_name() {
        return name_;
    }

    void print() {
        std::cout << name_ << "\n";
    }

private:
    std::string name_;
};

const BadExample obj;
obj.get_name();
obj.print();
```

```cpp
class GoodExample {
public:
    const std::string& get_name() const {
        return name_;
    }

    std::string& get_name() {
        return name_;
    }

    void print() const {
        std::cout << name_ << "\n";
    }

private:
    std::string name_;
};

const GoodExample obj;
obj.get_name();
obj.print();

GoodExample mutable_obj;
mutable_obj.get_name() = "new name";
```

#### 4. mutable 的使用时机

```cpp
class ExpensiveCache {
public:
    int compute() const {
        if (!cached_) {
            cached_value_ = do_expensive_work();
            cached_ = true;
        }
        return cached_value_;
    }

private:
    int do_expensive_work() const { return 42; }
    mutable int cached_value_ = 0;
    mutable bool cached_ = false;
};
```

**mutable 仅用于**：缓存/延迟计算——逻辑上不改变对象状态，但物理上需要修改成员。

| 用法 | 合理？ | 说明 |
|------|--------|------|
| 缓存计算结果 | ✅ | 逻辑上 const，只是延迟计算 |
| 日志记录 | ✅ | 不影响对象逻辑状态 |
| 修改业务数据 | ❌ | 不应该用 mutable 绕过 const |

***

### 8. 常见反模式

#### 1. 反模式1：全局变量滥用

```cpp
int g_count;
std::string g_name;
bool g_initialized;

void do_something() {
    g_count++;
    g_name = "hello";
}
```

```cpp
class Counter {
public:
    void increment() { ++count_; }
    int get() const { return count_; }
private:
    int count_ = 0;
};

class NameStore {
public:
    void set(const std::string& name) { name_ = name; }
    const std::string& get() const { return name_; }
private:
    std::string name_;
};
```

#### 2. 反模式2：过深的继承层次

```cpp
class Animal {};
class Mammal : public Animal {};
class Dog : public Mammal {};
class GuideDog : public Dog {};
class SmartGuideDog : public GuideDog {};
```

```cpp
class Animal {
public:
    virtual ~Animal() = default;
};

class Behavior {
public:
    virtual ~Behavior() = default;
    virtual void perform() = 0;
};

class Guiding : public Behavior {
public:
    void perform() override {}
};

class Dog : public Animal {
public:
    Dog(std::unique_ptr<Behavior> b) : behavior_(std::move(b)) {}
private:
    std::unique_ptr<Behavior> behavior_;
};
```

**原则**：继承层次不超过3层，优先用组合替代继承。

#### 3. 反模式3：上帝类

```cpp
class SystemManager {
public:
    void connect_database();
    void read_config();
    void process_request();
    void send_email();
    void log_message();
    void render_ui();
    void calculate_tax();
private:
    DBConnection db_;
    Config config_;
    EmailSender email_;
    Logger logger_;
    UIRenderer ui_;
    TaxCalculator tax_;
};
```

```cpp
class DatabaseService {
public:
    void connect();
};

class ConfigService {
public:
    void load();
};

class EmailService {
public:
    void send();
};
```

**原则**：一个类只有一个变更理由（单一职责原则）。

#### 4. 反模式4：魔数

```cpp
if (status == 3) {
    sleep(2000);
}
```

```cpp
constexpr int kStatusError = 3;
constexpr int kRetryDelayMs = 2000;

if (status == kStatusError) {
    sleep(kRetryDelayMs);
}
```

**原则**：所有硬编码数字都提取为命名常量。

***

### 9. 自动化工具

#### 1. clang-format：自动格式化代码

`.clang-format` 配置示例：

```yaml
BasedOnStyle: Google
IndentWidth: 4
ColumnLimit: 100
AllowShortFunctionsOnASingleLine: Inline
AllowShortIfStatementsOnASingleLine: false
BreakBeforeBraces: Attach
PointerAlignment: Left
SortIncludes: CaseInsensitive
```

**使用方式**：

```bash
clang-format -i src/*.cpp src/*.h
clang-format -i --style=file src/*.cpp
```

#### 2. clang-tidy：静态分析和规范检查

`.clang-tidy` 配置示例：

```yaml
Checks: >
  -*,
  bugprone-*,
  cppcoreguidelines-*,
  modernize-*,
  performance-*,
  readability-*,
  -modernize-use-trailing-return-type,
  -readability-magic-numbers
WarningsAsErrors: ''
HeaderFilterRegex: '.*'
FormatStyle: file
```

**使用方式**：

```bash
clang-tidy src/*.cpp -- -std=c++17
clang-tidy -p build src/main.cpp
```

#### 3. 两者对比

| | clang-format | clang-tidy |
|---|---|---|
| 功能 | 代码格式化 | 静态分析 + 自动修复 |
| 关注点 | 缩进、空格、换行 | 逻辑错误、规范违反、现代C++改进 |
| 修改代码 | 只改格式 | 可改逻辑（如 `auto` 替换、`override` 添加） |
| 速度 | 快 | 慢（需要编译信息） |
| 配置文件 | `.clang-format` | `.clang-tidy` |
| CI集成 | 格式检查 | 规范检查 |

**最佳实践**：

| 阶段 | 工具 | 说明 |
|------|------|------|
| 保存文件时 | clang-format | 编辑器自动格式化 |
| 提交前 | clang-tidy | 检查规范违反 |
| CI流水线 | 两者都用 | 格式 + 规范双重检查 |

***

### 10. 极简总结

**编码规范=团队的交通规则。核心原则：命名有意义、const能用就用、头文件精简、类设计遵循Rule of 0/3/5。用clang-format+clang-tidy自动化执行。**

| 要点 | 一句话 |
|------|--------|
| 命名 | 名副其实，风格统一（PascalCase类名、snake_case函数/变量） |
| const | 能用就用（变量、参数、返回值、成员函数） |
| 头文件 | `#pragma once`，包含顺序从本地到系统，优先前向声明 |
| 类设计 | Rule of 0/3/5，数据private，接口public，组合优于继承 |
| 反模式 | 避免全局变量、深继承、上帝类、魔数 |
| 自动化 | clang-format 格式化 + clang-tidy 检查，CI 强制执行 |

***

### 相关阅读

- [防御性编程与断言](./06-防御性编程与断言.md)
- [C语言安全编码实践](./05-C语言安全编码实践.md)
- [CPP工具链](../08-调试与性能/03-CPP工具链.md)

***