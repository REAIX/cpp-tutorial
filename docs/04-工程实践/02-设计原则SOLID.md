# 设计原则SOLID

> 掌握面向对象设计原则

---

> **Principles are the rules of the game.** — someone
> （原则是游戏的规则。）

> **SOLID：写出好代码的五大原则。**
> （SOLID: five principles for writing good code.）

---

> **🎯 无规矩不成方圆，无原则不成好代码。**
> 
> （遵循原则，写出高质量代码。）

---

> 💡 **通俗理解 - SOLID原则是什么？**

想象你要建一座房子：
- **不按原则**：房子可能建得很随意，后面修修补补
- **按原则**：房子结构稳固，容易扩建

**SOLID就是建筑设计的"五大原则"！**

- **S** - 单一职责：就像"一个房间只做一个用途"
- **O** - 开闭原则：就像"房子可以加层，但不破坏原有结构"
- **L** - 里氏替换：就像"可以用高级房间替换普通房间"
- **I** - 接口隔离：就像"不同房间有不同的门"
- **D** - 依赖倒置：就像"房子依赖抽象设计图，而不是具体砖块"

> 🔬 **抽象理解 - 设计原则的本质**：
> - **SRP（单一职责）**：每个类应该只有一个"改变的理由"，即一个类只负责一项职责
> - **OCP（开闭原则）**：软件实体应该"对扩展开放，对修改关闭"
> - **LSP（里氏替换）**：子类必须能够替换其基类而不影响程序正确性
> - **ISP（接口隔离）**：使用多个专门的接口比使用单一的总接口更好
> - **DIP（依赖倒置）**：高层模块不应该依赖低层模块，两者都应该依赖抽象
> - **原则的价值**：是"好的设计"的经验总结，提高代码的可维护性、可扩展性、可复用性

---

## 前置知识
- [单元测试](05-单元测试.md)
## 后续内容
- [设计模式](03-设计模式.md)
---

## 目录

- [1. SOLID原则概述](#1-solid原则概述)
- [2. 单一职责原则(SRP)](#2-单一职责原则srp)
- [3. 开闭原则(OCP)](#3-开闭原则ocp)
- [4. 里氏替换原则(LSP)](#4-里氏替换原则lsp)
- [5. 接口隔离原则(ISP)](#5-接口隔离原则isp)
- [6. 依赖倒置原则(DIP)](#6-依赖倒置原则dip)
- [7. 原则综合应用](#7-原则综合应用)

---

## 1. SOLID原则概述

### 1. 概念与定义

**SOLID原则**：C++中用于指导面向对象设计的五个原则的首字母缩写，由Robert C. Martin提出。SOLID原则可以提高代码的可维护性、可扩展性、可测试性、可读性和低耦合性。

**单一职责原则（Single Responsibility Principle, SRP）**：C++中用于指导类设计的原则。单一职责原则要求一个类只做一件事。例如`class User { /* ... */ };`用于表示用户，`class UserRepository { /* ... */ };`用于表示用户数据库操作，`class EmailService { /* ... */ };`用于表示邮件服务。

**开闭原则（Open/Closed Principle, OCP）**：C++中用于指导类设计的原则。开闭原则要求对扩展开放，对修改关闭。例如使用继承和多态实现扩展，而不是修改现有代码。

**里氏替换原则（Liskov Substitution Principle, LSP）**：C++中用于指导类设计的原则。里氏替换原则要求子类可以替换父类。例如`class Square : public Rectangle { /* ... */ };`用于表示正方形，`Square`可以替换`Rectangle`。

**接口隔离原则（Interface Segregation Principle, ISP）**：C++中用于指导接口设计的原则。接口隔离原则要求接口要小而专一。例如`class Printable { /* ... */ };`用于表示可打印的接口，`class Scannable { /* ... */ };`用于表示可扫描的接口。

**依赖倒置原则（Dependency Inversion Principle, DIP）**：C++中用于指导类设计的原则。依赖倒置原则要求依赖抽象而非具体。例如`class Database { /* ... */ };`用于表示数据库抽象，`class MySQLDatabase : public Database { /* ... */ };`用于表示MySQL数据库实现。

### 2. 什么是SOLID

SOLID是五个面向对象设计原则的首字母缩写，由Robert C. Martin提出。

```
┌─────────────────────────────────────────────────────────────┐
│                      SOLID 五原则                           │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  S - Single Responsibility Principle                        │
│      单一职责原则：一个类只做一件事                           │
│                                                             │
│  O - Open/Closed Principle                                  │
│      开闭原则：对扩展开放，对修改关闭                          │
│                                                             │
│  L - Liskov Substitution Principle                          │
│      里氏替换原则：子类可以替换父类                           │
│                                                             │
│  I - Interface Segregation Principle                        │
│      接口隔离原则：接口要小而专一                             │
│                                                             │
│  D - Dependency Inversion Principle                         │
│      依赖倒置原则：依赖抽象而非具体                           │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### 3. 为什么需要SOLID

| 好处 | 说明 |
|------|------|
| **可维护性** | 代码易于修改和维护 |
| **可扩展性** | 易于添加新功能 |
| **可测试性** | 代码易于单元测试 |
| **可读性** | 代码结构清晰易懂 |
| **低耦合** | 模块之间依赖最小化 |

---

## 2. 单一职责原则(SRP)

### 1. 定义

**一个类应该只有一个引起它变化的原因。**

简单来说：一个类只做一件事。

### 2. 违反SRP的例子

```cpp
#include <iostream>
#include <string>
#include <vector>

// ❌ 违反SRP：User类做了太多事情
class User {
public:
    std::string name;
    std::string email;
    
    // 用户数据管理
    void setName(const std::string& n) { name = n; }
    void setEmail(const std::string& e) { email = e; }
    
    // 数据库操作（不应该在这里）
    void save() {
        std::cout << "保存用户到数据库\n";
    }
    
    void load(int id) {
        std::cout << "从数据库加载用户\n";
    }
    
    // 发送邮件（不应该在这里）
    void sendEmail(const std::string& message) {
        std::cout << "发送邮件到 " << email << ": " << message << "\n";
    }
    
    // 生成报告（不应该在这里）
    void generateReport() {
        std::cout << "生成用户报告\n";
    }
};

// 问题：
// 1. 数据库变化需要修改User类
// 2. 邮件格式变化需要修改User类
// 3. 报告格式变化需要修改User类
// 4. 类太臃肿，难以维护
```

### 3. 遵循SRP的重构

```cpp
#include <iostream>
#include <string>
#include <memory>

// ✅ 遵循SRP：每个类只做一件事

// 用户数据类：只管理用户数据
class User {
public:
    std::string name;
    std::string email;
    
    User(const std::string& n, const std::string& e) 
        : name(n), email(e) {}
};

// 用户仓储类：只负责数据库操作
class UserRepository {
public:
    void save(const User& user) {
        std::cout << "保存用户 " << user.name << " 到数据库\n";
    }
    
    User load(int id) {
        std::cout << "从数据库加载用户ID: " << id << "\n";
        return User("Loaded", "loaded@example.com");
    }
};

// 邮件服务类：只负责发送邮件
class EmailService {
public:
    void send(const User& user, const std::string& message) {
        std::cout << "发送邮件到 " << user.email 
                  << ": " << message << "\n";
    }
};

// 报告生成类：只负责生成报告
class UserReportGenerator {
public:
    void generate(const User& user) {
        std::cout << "生成用户报告: " << user.name << "\n";
    }
};

int main() {
    User user("张三", "zhangsan@example.com");
    
    UserRepository repo;
    repo.save(user);
    
    EmailService email;
    email.send(user, "欢迎注册！");
    
    UserReportGenerator report;
    report.generate(user);
    
    return 0;
}
```

### 4. SRP的判断标准

```
问自己：
1. 这个类有几个职责？
2. 有几个原因会导致这个类变化？
3. 如果需求变化，需要修改这个类吗？

如果答案 > 1，可能违反了SRP
```

---

## 3. 开闭原则(OCP)

### 1. 定义

**软件实体应该对扩展开放，对修改关闭。**

简单来说：添加新功能时，不修改现有代码，而是扩展新代码。

### 2. 违反OCP的例子

```cpp
#include <iostream>
#include <string>
#include <vector>

// ❌ 违反OCP：每添加一种形状都要修改AreaCalculator

class Rectangle {
public:
    double width, height;
    Rectangle(double w, double h) : width(w), height(h) {}
};

class Circle {
public:
    double radius;
    Circle(double r) : radius(r) {}
};

class AreaCalculator {
public:
    // 每添加新形状都要修改这个函数！
    double calculateArea(void* shape, const std::string& type) {
        if (type == "rectangle") {
            Rectangle* r = static_cast<Rectangle*>(shape);
            return r->width * r->height;
        }
        else if (type == "circle") {
            Circle* c = static_cast<Circle*>(shape);
            return 3.14159 * c->radius * c->radius;
        }
        // 添加Triangle需要在这里加else if...
        return 0;
    }
};
```

### 3. 遵循OCP的重构

```cpp
#include <iostream>
#include <string>
#include <vector>
#include <memory>

// ✅ 遵循OCP：通过抽象和继承扩展

// 抽象基类
class Shape {
public:
    virtual ~Shape() = default;
    virtual double area() const = 0;
    virtual std::string name() const = 0;
};

// 矩形
class Rectangle : public Shape {
public:
    double width, height;
    
    Rectangle(double w, double h) : width(w), height(h) {}
    
    double area() const override {
        return width * height;
    }
    
    std::string name() const override {
        return "Rectangle";
    }
};

// 圆形
class Circle : public Shape {
public:
    double radius;
    
    Circle(double r) : radius(r) {}
    
    double area() const override {
        return 3.14159 * radius * radius;
    }
    
    std::string name() const override {
        return "Circle";
    }
};

// 三角形（新增形状不需要修改现有代码）
class Triangle : public Shape {
public:
    double base, height;
    
    Triangle(double b, double h) : base(b), height(h) {}
    
    double area() const override {
        return 0.5 * base * height;
    }
    
    std::string name() const override {
        return "Triangle";
    }
};

// 面积计算器：不需要修改！
class AreaCalculator {
public:
    double totalArea(const std::vector<std::unique_ptr<Shape>>& shapes) {
        double total = 0;
        for (const auto& shape : shapes) {
            total += shape->area();
        }
        return total;
    }
    
    void printAreas(const std::vector<std::unique_ptr<Shape>>& shapes) {
        for (const auto& shape : shapes) {
            std::cout << shape->name() 
                      << " area: " << shape->area() << "\n";
        }
    }
};

int main() {
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.push_back(std::make_unique<Rectangle>(3, 4));
    shapes.push_back(std::make_unique<Circle>(5));
    shapes.push_back(std::make_unique<Triangle>(6, 8));
    
    AreaCalculator calc;
    calc.printAreas(shapes);
    std::cout << "总面积: " << calc.totalArea(shapes) << "\n";
    
    return 0;
}
```

---

## 4. 里氏替换原则(LSP)

### 1. 定义

**子类对象必须能够替换其父类对象，且程序行为正确。**

简单来说：子类不能破坏父类的行为约定。

### 2. 违反LSP的例子

```cpp
#include <iostream>

// ❌ 违反LSP：Square破坏了Rectangle的行为

class Rectangle {
protected:
    double width, height;
    
public:
    virtual void setWidth(double w) { width = w; }
    virtual void setHeight(double h) { height = h; }
    
    double getWidth() const { return width; }
    double getHeight() const { return height; }
    
    double area() const { return width * height; }
};

// 正方形继承矩形：看似合理，实则违反LSP
class Square : public Rectangle {
public:
    // 正方形必须保持宽高相等
    void setWidth(double w) override {
        width = w;
        height = w;  // 同时修改高度！
    }
    
    void setHeight(double h) override {
        height = h;
        width = h;  // 同时修改宽度！
    }
};

// 问题：使用Rectangle的代码无法正常工作
void processRectangle(Rectangle& r) {
    r.setWidth(5);
    r.setHeight(4);
    
    // 期望面积是 5 * 4 = 20
    // 但如果是Square，实际是 4 * 4 = 16
    std::cout << "期望面积: 20, 实际面积: " << r.area() << "\n";
    
    // 违反了LSP！Square不能替换Rectangle
}
```

### 3. 遵循LSP的重构

```cpp
#include <iostream>
#include <memory>

// ✅ 遵循LSP：重新设计继承关系

// 抽象形状基类
class Shape {
public:
    virtual ~Shape() = default;
    virtual double area() const = 0;
};

// 矩形
class Rectangle : public Shape {
protected:
    double width, height;
    
public:
    Rectangle(double w, double h) : width(w), height(h) {}
    
    void setWidth(double w) { width = w; }
    void setHeight(double h) { height = h; }
    
    double getWidth() const { return width; }
    double getHeight() const { return height; }
    
    double area() const override { return width * height; }
};

// 正方形：不继承Rectangle，而是独立实现
class Square : public Shape {
protected:
    double side;
    
public:
    explicit Square(double s) : side(s) {}
    
    void setSide(double s) { side = s; }
    double getSide() const { return side; }
    
    double area() const override { return side * side; }
};

// 现在可以安全使用
void printArea(const Shape& shape) {
    std::cout << "面积: " << shape.area() << "\n";
}

int main() {
    Rectangle rect(5, 4);
    Square square(5);
    
    printArea(rect);    // 20
    printArea(square);  // 25
    
    return 0;
}
```

---

## 5. 接口隔离原则(ISP)

### 1. 定义

**客户端不应该依赖它不需要的接口。**

简单来说：接口要小而专一，不要创建"胖"接口。

### 2. 违反ISP的例子

```cpp
#include <iostream>

// ❌ 违反ISP：一个"胖"接口

// 多功能打印机接口
class IMachine {
public:
    virtual void print(const std::string& doc) = 0;
    virtual void scan(const std::string& doc) = 0;
    virtual void fax(const std::string& doc) = 0;
    virtual void copy(const std::string& doc) = 0;
};

// 多功能打印机：实现所有功能
class MultiFunctionPrinter : public IMachine {
public:
    void print(const std::string& doc) override {
        std::cout << "打印: " << doc << "\n";
    }
    void scan(const std::string& doc) override {
        std::cout << "扫描: " << doc << "\n";
    }
    void fax(const std::string& doc) override {
        std::cout << "传真: " << doc << "\n";
    }
    void copy(const std::string& doc) override {
        std::cout << "复印: " << doc << "\n";
    }
};

// 问题：普通打印机被迫实现不需要的方法
class OldPrinter : public IMachine {
public:
    void print(const std::string& doc) override {
        std::cout << "打印: " << doc << "\n";
    }
    
    // 被迫实现不需要的功能！
    void scan(const std::string& doc) override {
        throw std::runtime_error("不支持扫描");
    }
    void fax(const std::string& doc) override {
        throw std::runtime_error("不支持传真");
    }
    void copy(const std::string& doc) override {
        throw std::runtime_error("不支持复印");
    }
};
```

### 3. 遵循ISP的重构

```cpp
#include <iostream>
#include <string>

// ✅ 遵循ISP：接口分离

// 打印接口
class IPrinter {
public:
    virtual ~IPrinter() = default;
    virtual void print(const std::string& doc) = 0;
};

// 扫描接口
class IScanner {
public:
    virtual ~IScanner() = default;
    virtual void scan(const std::string& doc) = 0;
};

// 传真接口
class IFax {
public:
    virtual ~IFax() = default;
    virtual void fax(const std::string& doc) = 0;
};

// 复印接口
class ICopier {
public:
    virtual ~ICopier() = default;
    virtual void copy(const std::string& doc) = 0;
};

// 普通打印机：只实现打印
class SimplePrinter : public IPrinter {
public:
    void print(const std::string& doc) override {
        std::cout << "打印: " << doc << "\n";
    }
};

// 扫描仪：只实现扫描
class SimpleScanner : public IScanner {
public:
    void scan(const std::string& doc) override {
        std::cout << "扫描: " << doc << "\n";
    }
};

// 多功能打印机：实现所有接口
class MultiFunctionMachine : public IPrinter, 
                              public IScanner, 
                              public IFax, 
                              public ICopier {
public:
    void print(const std::string& doc) override {
        std::cout << "打印: " << doc << "\n";
    }
    void scan(const std::string& doc) override {
        std::cout << "扫描: " << doc << "\n";
    }
    void fax(const std::string& doc) override {
        std::cout << "传真: " << doc << "\n";
    }
    void copy(const std::string& doc) override {
        std::cout << "复印: " << doc << "\n";
    }
};

// 使用示例
void printDocument(IPrinter& printer, const std::string& doc) {
    printer.print(doc);
}

int main() {
    SimplePrinter printer;
    printDocument(printer, "Hello World");
    
    MultiFunctionMachine mfp;
    printDocument(mfp, "Multi-function print");
    
    return 0;
}
```

---

## 6. 依赖倒置原则(DIP)

### 1. 定义

**高层模块不应该依赖低层模块，两者都应该依赖其抽象。**

简单来说：依赖接口（抽象），而不是具体实现。

### 2. 违反DIP的例子

```cpp
#include <iostream>
#include <string>

// ❌ 违反DIP：高层模块直接依赖低层模块

// 低层模块：MySQL数据库
class MySQLDatabase {
public:
    void save(const std::string& data) {
        std::cout << "保存到MySQL: " << data << "\n";
    }
    
    std::string load(int id) {
        std::cout << "从MySQL加载ID: " << id << "\n";
        return "data";
    }
};

// 高层模块：用户服务
class UserService {
    MySQLDatabase database;  // 直接依赖具体实现！
    
public:
    void saveUser(const std::string& user) {
        database.save(user);
    }
    
    std::string loadUser(int id) {
        return database.load(id);
    }
};

// 问题：
// 1. 想换成PostgreSQL？必须修改UserService
// 2. 想做单元测试？无法mock数据库
// 3. 高层模块被低层模块"绑架"
```

### 3. 遵循DIP的重构

```cpp
#include <iostream>
#include <string>
#include <memory>

// ✅ 遵循DIP：依赖抽象

// 抽象接口
class IDatabase {
public:
    virtual ~IDatabase() = default;
    virtual void save(const std::string& data) = 0;
    virtual std::string load(int id) = 0;
};

// 低层模块：MySQL实现
class MySQLDatabase : public IDatabase {
public:
    void save(const std::string& data) override {
        std::cout << "保存到MySQL: " << data << "\n";
    }
    
    std::string load(int id) override {
        std::cout << "从MySQL加载ID: " << id << "\n";
        return "MySQL data";
    }
};

// 低层模块：PostgreSQL实现
class PostgreSQLDatabase : public IDatabase {
public:
    void save(const std::string& data) override {
        std::cout << "保存到PostgreSQL: " << data << "\n";
    }
    
    std::string load(int id) override {
        std::cout << "从PostgreSQL加载ID: " << id << "\n";
        return "PostgreSQL data";
    }
};

// 低层模块：Mock数据库（用于测试）
class MockDatabase : public IDatabase {
public:
    void save(const std::string& data) override {
        std::cout << "[Mock] 保存: " << data << "\n";
    }
    
    std::string load(int id) override {
        std::cout << "[Mock] 加载ID: " << id << "\n";
        return "Mock data";
    }
};

// 高层模块：依赖抽象
class UserService {
    std::shared_ptr<IDatabase> database;  // 依赖接口
    
public:
    // 依赖注入
    explicit UserService(std::shared_ptr<IDatabase> db) 
        : database(std::move(db)) {}
    
    void saveUser(const std::string& user) {
        database->save(user);
    }
    
    std::string loadUser(int id) {
        return database->load(id);
    }
};

int main() {
    // 使用MySQL
    auto mysql = std::make_shared<MySQLDatabase>();
    UserService service1(mysql);
    service1.saveUser("张三");
    
    // 切换到PostgreSQL（无需修改UserService）
    auto postgres = std::make_shared<PostgreSQLDatabase>();
    UserService service2(postgres);
    service2.saveUser("李四");
    
    // 使用Mock进行测试
    auto mock = std::make_shared<MockDatabase>();
    UserService testService(mock);
    testService.saveUser("测试用户");
    
    return 0;
}
```

---

## 7. 原则综合应用

### 1. 示例：日志系统

```cpp
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <fstream>

// 综合应用SOLID原则的日志系统

// ========== ISP：接口隔离 ==========

// 日志写入接口
class ILogWriter {
public:
    virtual ~ILogWriter() = default;
    virtual void write(const std::string& message) = 0;
};

// 日志格式化接口
class ILogFormatter {
public:
    virtual ~ILogFormatter() = default;
    virtual std::string format(const std::string& level, 
                               const std::string& message) = 0;
};

// ========== SRP：单一职责 ==========

// 控制台写入器
class ConsoleWriter : public ILogWriter {
public:
    void write(const std::string& message) override {
        std::cout << message << "\n";
    }
};

// 文件写入器
class FileWriter : public ILogWriter {
    std::ofstream file;
public:
    explicit FileWriter(const std::string& filename) 
        : file(filename) {}
    
    void write(const std::string& message) override {
        file << message << "\n";
    }
};

// 简单格式化器
class SimpleFormatter : public ILogFormatter {
public:
    std::string format(const std::string& level, 
                       const std::string& message) override {
        return "[" + level + "] " + message;
    }
};

// 带时间戳的格式化器
class TimestampFormatter : public ILogFormatter {
public:
    std::string format(const std::string& level, 
                       const std::string& message) override {
        // 简化：实际应获取当前时间
        return "[2024-01-01 10:00:00] [" + level + "] " + message;
    }
};

// ========== OCP + DIP：开闭原则 + 依赖倒置 ==========

// 日志器：高层模块
class Logger {
    std::vector<std::shared_ptr<ILogWriter>> writers;
    std::shared_ptr<ILogFormatter> formatter;
    
public:
    Logger(std::shared_ptr<ILogFormatter> fmt) 
        : formatter(std::move(fmt)) {}
    
    void addWriter(std::shared_ptr<ILogWriter> writer) {
        writers.push_back(std::move(writer));
    }
    
    void log(const std::string& level, const std::string& message) {
        std::string formatted = formatter->format(level, message);
        for (auto& writer : writers) {
            writer->write(formatted);
        }
    }
    
    void info(const std::string& message) { log("INFO", message); }
    void error(const std::string& message) { log("ERROR", message); }
    void warning(const std::string& message) { log("WARN", message); }
};

int main() {
    // 创建日志器（使用时间戳格式）
    auto formatter = std::make_shared<TimestampFormatter>();
    Logger logger(formatter);
    
    // 添加多个写入器
    logger.addWriter(std::make_shared<ConsoleWriter>());
    // logger.addWriter(std::make_shared<FileWriter>("app.log"));
    
    // 使用日志
    logger.info("应用程序启动");
    logger.warning("内存使用率较高");
    logger.error("数据库连接失败");
    
    return 0;
}
```

---

## 总结

### 2. SOLID原则速查表

| 原则 | 核心思想 | 关键词 |
|------|----------|--------|
| **SRP** | 一个类只做一件事 | 单一职责 |
| **OCP** | 扩展开放，修改关闭 | 抽象、继承 |
| **LSP** | 子类可替换父类 | 行为一致 |
| **ISP** | 接口要小而专一 | 接口分离 |
| **DIP** | 依赖抽象不依赖具体 | 依赖注入 |

### 3. 设计原则检查清单

```
□ SRP检查
  - 这个类有几个变化原因？
  - 类的名称能准确描述它的职责吗？

□ OCP检查
  - 添加新功能需要修改现有代码吗？
  - 是否使用了抽象和多态？

□ LSP检查
  - 子类能完全替换父类吗？
  - 子类是否改变了父类的行为约定？

□ ISP检查
  - 客户端是否被迫依赖不需要的方法？
  - 接口是否足够小？

□ DIP检查
  - 高层模块是否依赖低层模块？
  - 是否通过接口解耦？
```

---

## 参考资源

- [SOLID Principles](https://en.wikipedia.org/wiki/SOLID)
- [Clean Architecture](https://blog.cleancoder.com/uncle-bob/2012/08/13/the-clean-architecture.html)
- [Agile Software Development](https://www.pearson.com/en-us/subject-catalog/p/agile-software-development-principles-patterns-and-practices/P200000008886)

---

**上一章：** [第0章：单元测试](05-单元测试.md)\
**下一章：** [第2章：设计模式](03-设计模式.md)

***

### 4. 相关章节

- [编程范式概览与过程式编程](../03-问题解答/10-工程实践/13-编程范式概览与过程式编程.md) — SOLID原则在OOP范式中的实践
- [框架引擎中间件与架构概念指南](../03-问题解答/10-工程实践/15-框架引擎中间件与架构.md) — 框架/引擎/中间件/前后端/10种架构模式
- [依赖注入与控制反转](../03-问题解答/10-工程实践/18-依赖注入与控制反转.md) — DI容器、IoC原理、解耦实践
