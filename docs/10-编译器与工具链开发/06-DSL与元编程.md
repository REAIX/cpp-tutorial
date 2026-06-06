# DSL与元编程

> 设计与实现领域特定语言：让代码更贴近问题域

---

> **A language that doesn't affect the way you think about programming is not worth knowing.** — Alan Perlis
> （不影响你编程思维方式的语言，不值得学习。）

> **DSL：为特定领域量身定制的语言，让表达更自然、更高效。**

---

> **🎯 工欲善其事，必先利其器。**
>
> （好的 DSL 就是一把利器，让领域专家也能直接编程。）

---

> 💡 **通俗理解 - DSL 是什么？**

想象你在点餐：
1. **通用语言**："我想要一份主食，其中包含面粉制成的面条，加入牛肉和蔬菜，用辣椒调味"
2. **DSL**："牛肉辣面"

**DSL 就是领域专家的"行话"：**
- SQL：数据库查询的行话 — `SELECT * FROM users WHERE age > 18`
- 正则表达式：文本匹配的行话 — `[a-z]+@[a-z]+\.[a-z]+`
- HTML：网页结构的行话 — `<div class="header">Hello</div>`
- CSS：网页样式的行话 — `body { color: red; }`

---

> 🔬 **抽象理解 - DSL 的本质**：
> - **DSL**：针对特定问题域优化的编程语言，牺牲通用性换取表达力
> - **嵌入式 DSL**：在宿主语言内实现的 DSL，复用宿主语言的基础设施
> - **独立 DSL**：拥有独立语法和编译器的 DSL，表达力更强但开发成本更高
> - **元编程**：编写能生成或操作代码的代码，是 DSL 实现的核心技术

---

## 前置知识
- [代码生成与目标平台](05-代码生成与目标平台.md)
## 后续内容
- [构建工具链实战](07-构建工具链实战.md)

---

## 目录

- [1. 领域特定语言(DSL)设计](#1-领域特定语言dsl设计)
- [2. 嵌入式DSL vs 独立DSL](#2-嵌入式dsl-vs-独立dsl)
- [3. C++中实现DSL](#3-c中实现dsl)
- [4. DSL解析器生成](#4-dsl解析器生成)
- [5. DSL到LLVM IR的编译](#5-dsl到llvm-ir的编译)
- [6. 本章小结](#6-本章小结)

---

## 1. 领域特定语言(DSL)设计

### 1.1 DSL 的分类

```
编程语言的谱系：

通用语言 (GPL)                              领域特定语言 (DSL)
◄────────────────────────────────────────────────────────────►

C/C++/Java     Python/Ruby     SQL/Regex     MATLAB/R     Excel公式
  ↑               ↑               ↑            ↑            ↑
完全通用        多范式          数据查询      科学计算     表格计算

DSL 的两种类型：
┌──────────────────────────────────────────────────────┐
│  外部 DSL（独立 DSL）                                 │
│  - 有独立的语法和解析器                               │
│  - 例：SQL, Regex, HTML, CSS, Makefile               │
│  - 优点：语法自由，表达力强                           │
│  - 缺点：需要独立开发编译器/解释器                    │
├──────────────────────────────────────────────────────┤
│  内部 DSL（嵌入式 DSL）                               │
│  - 在宿主语言内实现                                   │
│  - 例：C++ 的运算符重载、构建器模式                   │
│  - 例：Ruby on Rails 的 ActiveRecord                  │
│  - 优点：复用宿主语言工具链                           │
│  - 缺点：受宿主语言语法限制                           │
└──────────────────────────────────────────────────────┘
```

### 1.2 DSL 设计原则

```
DSL 设计的核心原则：

1. 领域驱动
   - 语法应反映领域概念
   - 领域专家应能直接阅读和编写

2. 最小表达力
   - 只提供领域需要的表达能力
   - 不需要的特性不实现（减少复杂性）

3. 声明式优先
   - 描述"做什么"而非"怎么做"
   - 让 DSL 运行时决定实现方式

4. 安全性
   - 静态检查尽可能多的错误
   - 不合法的程序应该无法表达

5. 可组合
   - DSL 构件可以自由组合
   - 组合后的行为可预测

DSL 设计流程：
┌──────────┐   ┌──────────┐   ┌──────────┐   ┌──────────┐
│ 1.分析    │──▶│ 2.设计    │──▶│ 3.实现    │──▶│ 4.评估    │
│ 领域特征  │   │ 语法语义  │   │ 解析执行  │   │ 可用性    │
└──────────┘   └──────────┘   └──────────┘   └──────────┘
```

### 1.3 DSL 设计示例：配置语言

```
设计一个应用配置 DSL：

需求：
- 定义配置项（键值对）
- 支持分组
- 支持环境变量引用
- 支持默认值
- 支持类型检查

设计语法：
app "MyApp" {
    version = "1.0.0"

    database {
        host = env("DB_HOST", "localhost")
        port = 5432
        name = "mydb"
        pool_size = env("DB_POOL", 10)
    }

    server {
        host = "0.0.0.0"
        port = env("PORT", 8080)
        workers = 4
    }

    logging {
        level = "info"       // debug, info, warn, error
        file = "/var/log/myapp.log"
    }
}
```

---

## 2. 嵌入式DSL vs 独立DSL

### 2.1 对比分析

```
┌──────────────────┬──────────────────────┬──────────────────────┐
│     维度          │    嵌入式 DSL         │    独立 DSL           │
├──────────────────┼──────────────────────┼──────────────────────┤
│ 开发成本          │ 低（复用宿主语言）    │ 高（需要完整编译器）  │
│ 语法自由度        │ 受限（宿主语言语法）  │ 完全自由              │
│ 错误诊断          │ 依赖宿主语言          │ 可定制                │
│ 工具支持          │ 继承宿主语言工具      │ 需要独立开发          │
│ 学习曲线          │ 低（只需学 DSL 部分） │ 高（全新语言）        │
│ 调试体验          │ 与宿主语言一致        │ 需要源码映射          │
│ 可组合性          │ 天然支持              │ 需要设计              │
│ 性能              │ 宿主语言级别          │ 可针对性优化          │
│ 典型案例          │ C++ 运算符重载        │ SQL                   │
│                  │ Rust 宏               │ Regex                 │
│                  │ Kotlin DSL            │ HTML/CSS              │
└──────────────────┴──────────────────────┴──────────────────────┘
```

### 2.2 嵌入式 DSL 示例

```cpp
// 嵌入式 DSL 示例：HTTP 路由定义
// 使用 C++ 运算符重载和链式调用

#include <string>
#include <functional>
#include <map>
#include <iostream>

// HTTP 请求和响应
struct Request {
    std::string method;
    std::string path;
    std::map<std::string, std::string> headers;
    std::string body;
};

struct Response {
    int status;
    std::string body;
    std::map<std::string, std::string> headers;

    static Response ok(const std::string& body) {
        return {200, body, {{"Content-Type", "text/plain"}}};
    }
    static Response json(const std::string& body) {
        return {200, body, {{"Content-Type", "application/json"}}};
    }
    static Response notFound() {
        return {404, "Not Found", {}};
    }
};

using Handler = std::function<Response(const Request&)>;

// 路由条目
struct Route {
    std::string method;
    std::string path;
    Handler handler;
};

// 路由构建器（嵌入式 DSL）
class Router {
private:
    std::vector<Route> routes;

public:
    // GET 路由
    Router& GET(const std::string& path, Handler handler) {
        routes.push_back({"GET", path, handler});
        return *this;
    }

    // POST 路由
    Router& POST(const std::string& path, Handler handler) {
        routes.push_back({"POST", path, handler});
        return *this;
    }

    // PUT 路由
    Router& PUT(const std::string& path, Handler handler) {
        routes.push_back({"PUT", path, handler});
        return *this;
    }

    // DELETE 路由
    Router& DELETE(const std::string& path, Handler handler) {
        routes.push_back({"DELETE", path, handler});
        return *this;
    }

    // 中间件
    Router& use(std::function<void(Request&)> middleware) {
        // 注册中间件
        return *this;
    }

    // 路由分组
    Router& group(const std::string& prefix, std::function<void(Router&)> configure) {
        Router subRouter;
        configure(subRouter);
        for (auto& route : subRouter.routes) {
            route.path = prefix + route.path;
            routes.push_back(route);
        }
        return *this;
    }

    // 查找匹配的路由
    Response handle(const Request& req) {
        for (const auto& route : routes) {
            if (route.method == req.method && route.path == req.path) {
                return route.handler(req);
            }
        }
        return Response::notFound();
    }
};

// 使用 DSL 定义路由
int main() {
    Router router;

    // DSL 风格的路由定义
    router
        .GET("/", [](const Request& req) {
            return Response::ok("Hello, World!");
        })
        .GET("/users", [](const Request& req) {
            return Response::json("[{\"id\":1,\"name\":\"Alice\"}]");
        })
        .POST("/users", [](const Request& req) {
            return Response::json("{\"status\":\"created\"}");
        })
        .group("/api/v1", [](Router& r) {
            r.GET("/products", [](const Request& req) {
                    return Response::json("[{\"id\":1,\"name\":\"Widget\"}]");
                })
              .POST("/products", [](const Request& req) {
                    return Response::json("{\"status\":\"created\"}");
                });
        });

    // 测试路由
    Request req1{"GET", "/", {}, ""};
    auto resp1 = router.handle(req1);
    std::cout << "GET / → " << resp1.status << ": " << resp1.body << "\n";

    Request req2{"GET", "/api/v1/products", {}, ""};
    auto resp2 = router.handle(req2);
    std::cout << "GET /api/v1/products → " << resp2.status << ": " << resp2.body << "\n";

    return 0;
}
```

### 2.3 独立 DSL 示例

```
独立 DSL：一个简单的模板语言

语法设计：
template "UserPage" {
    header {
        title = "用户信息"
        css = ["style.css"]
    }

    body {
        div(class="container") {
            h1 { "用户: {{user.name}}" }
            p { "邮箱: {{user.email}}" }
            if(user.isAdmin) {
                div(class="admin-panel") {
                    "管理员面板"
                }
            }
            for(order in user.orders) {
                div(class="order") {
                    "订单 #{{order.id}}: {{order.amount}}元"
                }
            }
        }
    }
}

需要开发的组件：
1. 词法分析器
2. 语法分析器
3. AST 定义
4. 模板引擎（解释器或编译器）
```

---

## 3. C++中实现DSL

### 3.1 运算符重载实现 DSL

```cpp
// 运算符重载 DSL：数学表达式构建器
// 可以构建表达式树，然后编译或解释执行

#include <memory>
#include <string>
#include <map>
#include <cmath>
#include <iostream>
#include <sstream>

// 表达式基类
class Expr {
public:
    virtual ~Expr() = default;
    virtual double evaluate(const std::map<std::string, double>& vars) const = 0;
    virtual std::string toString() const = 0;
    virtual void print(int indent = 0) const = 0;
};

// 常量表达式
class Constant : public Expr {
    double value;
public:
    Constant(double v) : value(v) {}
    double evaluate(const std::map<std::string, double>&) const override {
        return value;
    }
    std::string toString() const override {
        return std::to_string(value);
    }
    void print(int indent) const override {
        std::cout << std::string(indent, ' ') << value << "\n";
    }
};

// 变量表达式
class Variable : public Expr {
    std::string name;
public:
    Variable(const std::string& n) : name(n) {}
    double evaluate(const std::map<std::string, double>& vars) const override {
        auto it = vars.find(name);
        if (it == vars.end()) return 0.0;
        return it->second;
    }
    std::string toString() const override { return name; }
    void print(int indent) const override {
        std::cout << std::string(indent, ' ') << name << "\n";
    }
    const std::string& getName() const { return name; }
};

// 二元运算表达式
class BinaryExpr : public Expr {
    char op;
    std::shared_ptr<Expr> left, right;
public:
    BinaryExpr(char o, std::shared_ptr<Expr> l, std::shared_ptr<Expr> r)
        : op(o), left(std::move(l)), right(std::move(r)) {}

    double evaluate(const std::map<std::string, double>& vars) const override {
        double l = left->evaluate(vars);
        double r = right->evaluate(vars);
        switch (op) {
            case '+': return l + r;
            case '-': return l - r;
            case '*': return l * r;
            case '/': return l / r;
            case '^': return std::pow(l, r);
            default:  return 0;
        }
    }

    std::string toString() const override {
        return "(" + left->toString() + " " + op + " " + right->toString() + ")";
    }

    void print(int indent) const override {
        std::cout << std::string(indent, ' ') << op << "\n";
        left->print(indent + 2);
        right->print(indent + 2);
    }
};

// 一元运算表达式
class UnaryExpr : public Expr {
    char op;
    std::shared_ptr<Expr> operand;
public:
    UnaryExpr(char o, std::shared_ptr<Expr> e) : op(o), operand(std::move(e)) {}

    double evaluate(const std::map<std::string, double>& vars) const override {
        double v = operand->evaluate(vars);
        return op == '-' ? -v : v;
    }

    std::string toString() const override {
        return std::string(1, op) + operand->toString();
    }

    void print(int indent) const override {
        std::cout << std::string(indent, ' ') << op << "\n";
        operand->print(indent + 2);
    }
};

// 函数调用表达式
class FuncCall : public Expr {
    std::string funcName;
    std::shared_ptr<Expr> arg;
public:
    FuncCall(const std::string& name, std::shared_ptr<Expr> a)
        : funcName(name), arg(std::move(a)) {}

    double evaluate(const std::map<std::string, double>& vars) const override {
        double v = arg->evaluate(vars);
        if (funcName == "sin") return std::sin(v);
        if (funcName == "cos") return std::cos(v);
        if (funcName == "sqrt") return std::sqrt(v);
        if (funcName == "abs") return std::abs(v);
        return 0;
    }

    std::string toString() const override {
        return funcName + "(" + arg->toString() + ")";
    }

    void print(int indent) const override {
        std::cout << std::string(indent, ' ') << funcName << "\n";
        arg->print(indent + 2);
    }
};

// ==================== 运算符重载 ====================

// 表达式包装器（支持运算符重载）
class ExprWrapper {
public:
    std::shared_ptr<Expr> expr;

    ExprWrapper(double v) : expr(std::make_shared<Constant>(v)) {}
    ExprWrapper(const std::string& name) : expr(std::make_shared<Variable>(name)) {}
    ExprWrapper(std::shared_ptr<Expr> e) : expr(std::move(e)) {}

    double evaluate(const std::map<std::string, double>& vars = {}) const {
        return expr->evaluate(vars);
    }

    std::string toString() const { return expr->toString(); }
    void print() const { expr->print(); }

    // 算术运算符
    ExprWrapper operator+(const ExprWrapper& rhs) const {
        return ExprWrapper(std::make_shared<BinaryExpr>('+', expr, rhs.expr));
    }
    ExprWrapper operator-(const ExprWrapper& rhs) const {
        return ExprWrapper(std::make_shared<BinaryExpr>('-', expr, rhs.expr));
    }
    ExprWrapper operator*(const ExprWrapper& rhs) const {
        return ExprWrapper(std::make_shared<BinaryExpr>('*', expr, rhs.expr));
    }
    ExprWrapper operator/(const ExprWrapper& rhs) const {
        return ExprWrapper(std::make_shared<BinaryExpr>('/', expr, rhs.expr));
    }
    ExprWrapper operator^(const ExprWrapper& rhs) const {
        return ExprWrapper(std::make_shared<BinaryExpr>('^', expr, rhs.expr));
    }
    ExprWrapper operator-() const {
        return ExprWrapper(std::make_shared<UnaryExpr>('-', expr));
    }
};

// 辅助函数
ExprWrapper sin(const ExprWrapper& e) {
    return ExprWrapper(std::make_shared<FuncCall>("sin", e.expr));
}
ExprWrapper cos(const ExprWrapper& e) {
    return ExprWrapper(std::make_shared<FuncCall>("cos", e.expr));
}
ExprWrapper sqrt(const ExprWrapper& e) {
    return ExprWrapper(std::make_shared<FuncCall>("sqrt", e.expr));
}

// 使用 DSL
int main() {
    // 创建变量
    ExprWrapper x("x");
    ExprWrapper y("y");

    // 使用运算符重载构建表达式
    // 数学公式：f(x, y) = x^2 + y^2 + sin(x) * cos(y)
    auto f = x^2 + y^2 + sin(x) * cos(y);

    std::cout << "表达式: " << f.toString() << "\n";
    std::cout << "AST:\n";
    f.print();

    // 计算值
    std::map<std::string, double> vars = {{"x", 1.0}, {"y", 2.0}};
    std::cout << "f(1, 2) = " << f.evaluate(vars) << "\n";

    return 0;
}
```

### 3.2 模板元编程实现 DSL

```cpp
// 模板元编程 DSL：编译期单位检查系统
// 确保物理量的单位在编译期正确

#include <ratio>
#include <iostream>

// 单位维度：长度(L)、质量(M)、时间(T)
template<int L, int M, int T>
struct Dimension {
    static constexpr int length = L;
    static constexpr int mass = M;
    static constexpr int time = T;
};

// 带单位的物理量
template<typename Dim, typename Rep = double>
class Quantity {
private:
    Rep value_;

public:
    constexpr explicit Quantity(Rep v) : value_(v) {}
    constexpr Rep value() const { return value_; }

    // 同单位加法
    constexpr Quantity operator+(const Quantity& rhs) const {
        return Quantity(value_ + rhs.value_);
    }

    constexpr Quantity operator-(const Quantity& rhs) const {
        return Quantity(value_ - rhs.value_);
    }

    // 不同单位乘法：维度相加
    template<typename Dim2>
    constexpr Quantity<Dimension<Dim::length + Dim2::length,
                                Dim::mass + Dim2::mass,
                                Dim::time + Dim2::time>, Rep>
    operator*(const Quantity<Dim2, Rep>& rhs) const {
        using ResultDim = Dimension<Dim::length + Dim2::length,
                                    Dim::mass + Dim2::mass,
                                    Dim::time + Dim2::time>;
        return Quantity<ResultDim, Rep>(value_ * rhs.value());
    }

    // 不同单位除法：维度相减
    template<typename Dim2>
    constexpr Quantity<Dimension<Dim::length - Dim2::length,
                                Dim::mass - Dim2::mass,
                                Dim::time - Dim2::time>, Rep>
    operator/(const Quantity<Dim2, Rep>& rhs) const {
        using ResultDim = Dimension<Dim::length - Dim2::length,
                                    Dim::mass - Dim2::mass,
                                    Dim::time - Dim2::time>;
        return Quantity<ResultDim, Rep>(value_ / rhs.value());
    }
};

// 预定义单位
using Length   = Dimension<1, 0, 0>;   // 米
using Mass     = Dimension<0, 1, 0>;   // 千克
using Time     = Dimension<0, 0, 1>;   // 秒
using Velocity = Dimension<1, 0, -1>;  // 米/秒
using Force    = Dimension<1, 1, -2>;  // 千克·米/秒² = 牛顿
using Area     = Dimension<2, 0, 0>;   // 米²

// 单位字面量（C++14 用户定义字面量）
constexpr Quantity<Length> operator""_m(long double v) {
    return Quantity<Length>(static_cast<double>(v));
}
constexpr Quantity<Mass> operator""_kg(long double v) {
    return Quantity<Mass>(static_cast<double>(v));
}
constexpr Quantity<Time> operator""_s(long double v) {
    return Quantity<Time>(static_cast<double>(v));
}

// 使用示例
int main() {
    // 正确的物理量运算
    auto distance = 100.0_m;       // 100 米
    auto time = 10.0_s;            // 10 秒
    auto mass = 5.0_kg;            // 5 千克

    // 速度 = 距离 / 时间
    auto velocity = distance / time;  // 类型：Quantity<Velocity>
    std::cout << "速度: " << velocity.value() << " m/s\n";

    // 力 = 质量 * 速度 / 时间
    auto force = mass * velocity / time;  // 类型：Quantity<Force>
    std::cout << "力: " << force.value() << " N\n";

    // 面积 = 距离 * 距离
    auto area = distance * distance;  // 类型：Quantity<Area>
    std::cout << "面积: " << area.value() << " m²\n";

    // 编译期错误：不能将长度加到质量上
    // auto invalid = distance + mass;  // 编译错误！

    return 0;
}
```

### 3.3 宏实现 DSL

```cpp
// 宏 DSL：测试框架
// 使用宏实现声明式测试定义

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <cmath>

// 测试框架核心
class TestFramework {
public:
    struct TestCase {
        std::string name;
        std::function<void()> func;
    };

    static TestFramework& instance() {
        static TestFramework framework;
        return framework;
    }

    void addTest(const std::string& name, std::function<void()> func) {
        tests_.push_back({name, func});
    }

    int runAll() {
        int passed = 0, failed = 0;
        for (const auto& test : tests_) {
            try {
                test.func();
                std::cout << "  ✓ " << test.name << "\n";
                passed++;
            } catch (const std::exception& e) {
                std::cout << "  ✗ " << test.name << ": " << e.what() << "\n";
                failed++;
            }
        }
        std::cout << "\n结果: " << passed << " 通过, " << failed << " 失败\n";
        return failed > 0 ? 1 : 0;
    }

private:
    std::vector<TestCase> tests_;
};

// 断言辅助
class Assert {
public:
    static void isTrue(bool condition, const std::string& msg = "") {
        if (!condition) {
            throw std::runtime_error("断言失败: 期望 true" +
                (msg.empty() ? "" : " (" + msg + ")"));
        }
    }

    static void areEqual(double expected, double actual, double epsilon = 1e-9) {
        if (std::abs(expected - actual) > epsilon) {
            throw std::runtime_error("断言失败: 期望 " +
                std::to_string(expected) + "，实际 " +
                std::to_string(actual));
        }
    }

    template<typename T>
    static void areEqual(const T& expected, const T& actual) {
        if (expected != actual) {
            throw std::runtime_error("断言失败: 值不相等");
        }
    }

    static void throws(std::function<void()> func) {
        bool threw = false;
        try { func(); } catch (...) { threw = true; }
        if (!threw) {
            throw std::runtime_error("断言失败: 期望抛出异常");
        }
    }
};

// ==================== DSL 宏定义 ====================

// 定义测试套件
#define TEST_SUITE(name) \
    namespace test_suite_##name {

// 定义测试用例
#define TEST_CASE(name) \
    void test_##name(); \
    static bool reg_##name = \
        (TestFramework::instance().addTest(#name, test_##name), true); \
    void test_##name()

// 断言宏
#define ASSERT_TRUE(expr)     Assert::isTrue((expr), #expr)
#define ASSERT_FALSE(expr)    Assert::isTrue(!(expr), #expr)
#define ASSERT_EQ(a, b)       Assert::areEqual((a), (b))
#define ASSERT_NEQ(a, b)      Assert::isTrue((a) != (b), #a " != " #b)
#define ASSERT_THROWS(expr)   Assert::throws([&](){ expr; })

// 结束测试套件
#define END_SUITE }

// ==================== 使用 DSL 编写测试 ====================

TEST_SUITE(MathTests)

TEST_CASE(addition) {
    ASSERT_EQ(2 + 2, 4);
    ASSERT_EQ(0 + 0, 0);
    ASSERT_EQ(-1 + 1, 0);
}

TEST_CASE(multiplication) {
    ASSERT_EQ(3 * 4, 12);
    ASSERT_EQ(0 * 100, 0);
    ASSERT_EQ(-2 * -3, 6);
}

TEST_CASE(division) {
    ASSERT_EQ(10.0 / 2.0, 5.0);
    ASSERT_THROWS(throw std::runtime_error("error"));
}

END_SUITE

TEST_SUITE(StringTests)

TEST_CASE(concatenation) {
    std::string a = "hello";
    std::string b = " world";
    ASSERT_EQ(a + b, std::string("hello world"));
}

TEST_CASE(empty_string) {
    std::string s;
    ASSERT_TRUE(s.empty());
    ASSERT_EQ(s.length(), 0u);
}

END_SUITE

// 运行所有测试
int main() {
    std::cout << "=== 运行测试 ===\n";
    return TestFramework::instance().runAll();
}
```

---

## 4. DSL解析器生成

### 4.1 使用 ANTLR 生成 DSL 解析器

```
// ConfigDSL.g4 - 配置文件 DSL 文法
grammar ConfigDSL;

config: 'app' STRING '{' section* '}' ;

section: IDENTIFIER '{' assignment* '}' ;

assignment: IDENTIFIER '=' value ';' ;

value: STRING
     | NUMBER
     | 'env' '(' STRING (',' value)? ')'
     | list
     ;

list: '[' value (',' value)* ']' ;

STRING: '"' (~["\\] | '\\' .)* '"' ;
NUMBER: [0-9]+ ('.' [0-9]+)? ;
IDENTIFIER: [a-zA-Z_][a-zA-Z0-9_]* ;
WS: [ \t\r\n]+ -> skip ;
COMMENT: '//' ~[\r\n]* -> skip ;
```

### 4.2 手写 DSL 解析器

```cpp
// 手写配置 DSL 解析器
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <sstream>
#include <iostream>
#include <fstream>

// AST 节点
struct ConfigValue {
    enum Type { STRING, NUMBER, ENV, LIST } type;
    std::string strVal;
    double numVal;
    std::string envVar;
    std::shared_ptr<ConfigValue> defaultVal;
    std::vector<std::shared_ptr<ConfigValue>> listVal;

    static std::shared_ptr<ConfigValue> makeString(const std::string& s) {
        auto v = std::make_shared<ConfigValue>();
        v->type = STRING; v->strVal = s; return v;
    }
    static std::shared_ptr<ConfigValue> makeNumber(double n) {
        auto v = std::make_shared<ConfigValue>();
        v->type = NUMBER; v->numVal = n; return v;
    }
    static std::shared_ptr<ConfigValue> makeEnv(const std::string& var,
                                                 std::shared_ptr<ConfigValue> def = nullptr) {
        auto v = std::make_shared<ConfigValue>();
        v->type = ENV; v->envVar = var; v->defaultVal = def; return v;
    }
};

struct ConfigAssignment {
    std::string key;
    std::shared_ptr<ConfigValue> value;
};

struct ConfigSection {
    std::string name;
    std::vector<ConfigAssignment> assignments;
};

struct ConfigFile {
    std::string appName;
    std::vector<ConfigSection> sections;
};

// 词法分析器
class ConfigLexer {
private:
    std::string source;
    size_t pos;

    enum class TokenType {
        APP, IDENTIFIER, STRING, NUMBER,
        LBRACE, RBRACE, EQUALS, SEMICOLON,
        COMMA, LPAREN, RPAREN, LBRACKET, RBRACKET,
        ENV, END_OF_FILE, ERROR
    };

    struct Token {
        TokenType type;
        std::string value;
    };

    void skipWhitespace() {
        while (pos < source.size() && isspace(source[pos])) pos++;
        // 跳过注释
        if (pos + 1 < source.size() && source[pos] == '/' && source[pos+1] == '/') {
            while (pos < source.size() && source[pos] != '\n') pos++;
            skipWhitespace();
        }
    }

public:
    ConfigLexer(const std::string& src) : source(src), pos(0) {}

    Token nextToken() {
        skipWhitespace();
        if (pos >= source.size()) return {TokenType::END_OF_FILE, ""};

        char ch = source[pos];

        // 字符串
        if (ch == '"') {
            pos++; // 跳过开头的 "
            std::string value;
            while (pos < source.size() && source[pos] != '"') {
                if (source[pos] == '\\') pos++;
                value += source[pos++];
            }
            pos++; // 跳过结尾的 "
            return {TokenType::STRING, value};
        }

        // 数字
        if (isdigit(ch)) {
            std::string value;
            while (pos < source.size() && (isdigit(source[pos]) || source[pos] == '.')) {
                value += source[pos++];
            }
            return {TokenType::NUMBER, value};
        }

        // 标识符/关键字
        if (isalpha(ch) || ch == '_') {
            std::string value;
            while (pos < source.size() && (isalnum(source[pos]) || source[pos] == '_')) {
                value += source[pos++];
            }
            if (value == "app") return {TokenType::APP, value};
            if (value == "env") return {TokenType::ENV, value};
            return {TokenType::IDENTIFIER, value};
        }

        // 符号
        pos++;
        switch (ch) {
            case '{': return {TokenType::LBRACE, "{"};
            case '}': return {TokenType::RBRACE, "}"};
            case '=': return {TokenType::EQUALS, "="};
            case ';': return {TokenType::SEMICOLON, ";"};
            case ',': return {TokenType::COMMA, ","};
            case '(': return {TokenType::LPAREN, "("};
            case ')': return {TokenType::RPAREN, ")"};
            case '[': return {TokenType::LBRACKET, "["};
            case ']': return {TokenType::RBRACKET, "]"};
            default:  return {TokenType::ERROR, std::string(1, ch)};
        }
    }
};

// 语法分析器
class ConfigParser {
private:
    ConfigLexer lexer;
    ConfigLexer::Token current;

    void advance() { current = lexer.nextToken(); }

    bool match(ConfigLexer::TokenType type) {
        if (current.type == type) { advance(); return true; }
        return false;
    }

    void expect(ConfigLexer::TokenType type, const std::string& msg) {
        if (current.type != type) {
            throw std::runtime_error("解析错误: " + msg);
        }
        advance();
    }

public:
    ConfigParser(const std::string& source) : lexer(source) {
        advance(); // 读取第一个 Token
    }

    ConfigFile parse() {
        ConfigFile config;

        expect(ConfigLexer::TokenType::APP, "期望 'app'");
        config.appName = current.value;
        expect(ConfigLexer::TokenType::STRING, "期望应用名称");
        expect(ConfigLexer::TokenType::LBRACE, "期望 '{'");

        while (current.type != ConfigLexer::TokenType::RBRACE) {
            config.sections.push_back(parseSection());
        }

        expect(ConfigLexer::TokenType::RBRACE, "期望 '}'");
        return config;
    }

    ConfigSection parseSection() {
        ConfigSection section;
        section.name = current.value;
        expect(ConfigLexer::TokenType::IDENTIFIER, "期望节名称");
        expect(ConfigLexer::TokenType::LBRACE, "期望 '{'");

        while (current.type != ConfigLexer::TokenType::RBRACE) {
            section.assignments.push_back(parseAssignment());
        }

        expect(ConfigLexer::TokenType::RBRACE, "期望 '}'");
        return section;
    }

    ConfigAssignment parseAssignment() {
        ConfigAssignment assign;
        assign.key = current.value;
        expect(ConfigLexer::TokenType::IDENTIFIER, "期望键名");
        expect(ConfigLexer::TokenType::EQUALS, "期望 '='");
        assign.value = parseValue();
        expect(ConfigLexer::TokenType::SEMICOLON, "期望 ';'");
        return assign;
    }

    std::shared_ptr<ConfigValue> parseValue() {
        switch (current.type) {
            case ConfigLexer::TokenType::STRING:
                return ConfigValue::makeString(current.value);
                advance();

            case ConfigLexer::TokenType::NUMBER:
                return ConfigValue::makeNumber(std::stod(current.value));
                advance();

            case ConfigLexer::TokenType::ENV: {
                advance(); // 消费 env
                expect(ConfigLexer::TokenType::LPAREN, "期望 '('");
                std::string varName = current.value;
                expect(ConfigLexer::TokenType::STRING, "期望环境变量名");
                std::shared_ptr<ConfigValue> defaultVal;
                if (match(ConfigLexer::TokenType::COMMA)) {
                    defaultVal = parseValue();
                }
                expect(ConfigLexer::TokenType::RPAREN, "期望 ')'");
                return ConfigValue::makeEnv(varName, defaultVal);
            }

            default:
                throw std::runtime_error("解析错误: 期望值");
        }
    }
};
```

---

## 5. DSL到LLVM IR的编译

### 5.1 编译流程

```
DSL 源代码 → 词法分析 → 语法分析 → AST → 语义分析 → LLVM IR → 机器码

示例：数学表达式 DSL

源代码：
let f(x, y) = x^2 + y^2;

编译流程：
1. 词法分析：[let] [f] [(] [x] [,] [y] [)] [=] [x] [^] [2] [+] [y] [^] [2] [;]
2. 语法分析：AST
3. 语义分析：类型检查
4. IR 生成：
   define double @f(double %x, double %y) {
   entry:
     %x2 = fmul double %x, %x
     %y2 = fmul double %y, %y
     %sum = fadd double %x2, %y2
     ret double %sum
   }
5. 代码生成：x86-64 汇编
```

### 5.2 DSL 编译器实现

```cpp
// 数学表达式 DSL 到 LLVM IR 的编译器

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/Support/TargetSelect.h"

using namespace llvm;

// DSL 编译器
class MathDSLCompiler {
private:
    LLVMContext context;
    std::unique_ptr<Module> module;
    std::unique_ptr<IRBuilder<>> builder;
    std::map<std::string, Value*> variables;

public:
    MathDSLCompiler()
        : module(std::make_unique<Module>("math_dsl", context)),
          builder(std::make_unique<IRBuilder<>>(context)) {}

    // 编译函数定义
    // let f(x, y) = expr;
    Function* compileFunction(const std::string& name,
                              const std::vector<std::string>& params,
                              ExprAST* body) {
        // 创建函数类型
        std::vector<Type*> paramTypes(params.size(), Type::getDoubleTy(context));
        FunctionType* funcType = FunctionType::get(
            Type::getDoubleTy(context), paramTypes, false);

        // 创建函数
        Function* func = Function::Create(
            funcType, Function::ExternalLinkage, name, module.get());

        // 设置参数名
        unsigned idx = 0;
        for (auto& arg : func->args()) {
            arg.setName(params[idx++]);
        }

        // 创建入口基本块
        BasicBlock* entry = BasicBlock::Create(context, "entry", func);
        builder->SetInsertPoint(entry);

        // 将参数存入变量表
        variables.clear();
        for (auto& arg : func->args()) {
            AllocaInst* alloca = builder->CreateAlloca(
                Type::getDoubleTy(context), nullptr, arg.getName().str());
            builder->CreateStore(&arg, alloca);
            variables[arg.getName().str()] = alloca;
        }

        // 编译函数体
        Value* result = compileExpr(body);
        builder->CreateRet(result);

        // 验证函数
        verifyFunction(*func);

        return func;
    }

    // 编译表达式
    Value* compileExpr(ExprAST* expr) {
        // 根据表达式类型生成 IR
        // 这里简化处理，展示核心逻辑

        // 常量
        // return ConstantFP::get(context, APFloat(value));

        // 变量
        // auto* alloca = variables[name];
        // return builder->CreateLoad(Type::getDoubleTy(context), alloca, name);

        // 二元运算
        // Value* L = compileExpr(left);
        // Value* R = compileExpr(right);
        // switch (op) {
        //     case '+': return builder->CreateFAdd(L, R, "addtmp");
        //     case '-': return builder->CreateFSub(L, R, "subtmp");
        //     case '*': return builder->CreateFMul(L, R, "multmp");
        //     case '/': return builder->CreateFDiv(L, R, "divtmp");
        // }

        // 函数调用
        // std::vector<Value*> args;
        // for (auto& arg : arguments) args.push_back(compileExpr(arg));
        // return builder->CreateCall(callee, args, "calltmp");

        return ConstantFP::get(context, APFloat(0.0));
    }

    // JIT 执行
    double execute(const std::string& funcName,
                  const std::vector<double>& args) {
        InitializeNativeTarget();
        InitializeNativeTargetAsmPrinter();

        auto jit = LLJITBuilder().create();
        if (!jit) return 0.0;

        (*jit)->addIRModule(ThreadSafeModule(
            std::move(module), std::make_unique<LLVMContext>()));

        auto sym = (*jit)->lookup(funcName);
        if (!sym) return 0.0;

        auto* funcPtr = sym->toPtr<double(const double*)>();

        return funcPtr(args.data());
    }

    // 打印 IR
    void printIR() {
        module->print(outs(), nullptr);
    }
};
```

### 5.3 完整 DSL 工具链示例

```
完整的 DSL 工具链：

1. DSL 源代码 (.math)
   let f(x, y) = x^2 + y^2;
   let g(x) = sin(x) + cos(x);

2. 编译器前端（解析 + 语义分析）
   mathc --parse f.math       # 仅解析
   mathc --check f.math       # 语义检查

3. IR 生成
   mathc --emit-llvm f.math   # 输出 LLVM IR
   mathc --emit-bc f.math     # 输出位码

4. 优化
   mathc -O2 f.math -o f.bc   # 优化编译

5. 目标代码生成
   mathc --target=x86-64 f.math -o f.s
   mathc --target=wasm32 f.math -o f.wasm

6. JIT 执行
   mathc --jit f.math --eval "f(3, 4)"   # 输出: 25

7. 交互式 REPL
   mathc --repl
   >> let f(x, y) = x^2 + y^2;
   >> f(3, 4)
   25.0
   >> let g(x) = sqrt(f(x, 0));
   >> g(5)
   5.0
```

---

## 6. 本章小结

### 核心要点回顾

| 技术 | 关键内容 |
|------|---------|
| DSL设计 | 领域驱动、最小表达力、声明式优先、安全可组合 |
| 嵌入式DSL | 在宿主语言内实现，复用工具链，受语法限制 |
| 独立DSL | 独立语法和编译器，表达力强，开发成本高 |
| 运算符重载 | C++ 实现嵌入式 DSL 的主要手段 |
| 模板元编程 | 编译期计算和类型检查，零运行时开销 |
| 宏 | 声明式语法定义，简化重复代码 |
| DSL编译 | 解析 → AST → LLVM IR → 机器码/JIT |

### 关键理解

1. **DSL 是领域专家和开发者之间的桥梁**：好的 DSL 让领域专家也能直接编程
2. **嵌入式 DSL 是 C++ 的强项**：运算符重载 + 模板 + 宏提供了强大的 DSL 构建能力
3. **独立 DSL 需要完整的工具链**：从词法分析到代码生成的全套编译器基础设施
4. **LLVM 是 DSL 编译的理想后端**：只需生成 IR，即可获得多平台支持和优化

### 延伸思考

- 如何设计一个既支持嵌入式又支持独立模式的 DSL？
- C++20 的 Concepts 和 Ranges 如何改善嵌入式 DSL 的体验？
- 如何为 DSL 构建语言服务器（LSP），提供智能补全和错误提示？

> **下一章**：[构建工具链实战](07-构建工具链实战.md) — 综合运用所学知识，构建完整的编译器工具链
