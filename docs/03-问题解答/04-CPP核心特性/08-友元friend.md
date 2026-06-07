# 友元（friend）完全指南
> 📖 相关章节：[运算符重载与友元](../../02-CPP/06-运算符重载与友元.md)、[类与对象](../../02-CPP/03-类与对象.md)

## 1. 友元是什么？

### 1. 核心定义

**友元（friend）** 是 C++ 中一种**打破封装机制**的特殊机制：

> **允许一个函数或类访问另一个类的 private 和 protected 成员**

### 2. 生活类比

```
你的日记本 = 类的 private 成员
你本人    = 类本身（可以看自己的日记）
朋友      = friend（你授权的人，也能看你的日记）
陌生人    = 普通外部代码（不能看你的日记）

关键：友谊是单向的、不传递的、不被继承的
```

***

## 2. 友元的三大类型

| 类型 | 语法 | 说明 |
|:----:|------|------|
| **友元函数** | `friend 返回值 函数名(参数);` | 外部函数可访问私有成员 |
| **友元类** | `friend class 类名;` | 另一个类的所有成员函数都可访问 |
| **友元成员函数** | `friend 返回值 类名::函数名(参数);` | 只允许某个类的特定成员函数访问 |

***

## 3. 语法格式与如何识别

### 1. 友元函数

```cpp
class BankAccount {
private:
    double balance;        // 私有：余额
    std::string password;  // 私有：密码

public:
    BankAccount(double bal, const std::string& pwd)
        : balance(bal), password(pwd) {}

    // 声明友元函数 👇 在类内部声明
    friend void displayBalance(const BankAccount& acct);
    friend bool verifyPassword(const BankAccount& acct, const std::string& input);

    // 获取余额（公开接口）
    double getBalance() const { return balance; }
};

// 定义在类外部（不加 friend 关键字！不加作用域！）
void displayBalance(const BankAccount& acct) {
    // 可以直接访问 private 成员
    std::cout << "余额: $" << acct.balance << "\n";
}

bool verifyPassword(const BankAccount& acct, const std::string& input) {
    // 可以直接访问 private 成员
    return acct.password == input;
}

int main() {
    BankAccount myAccount(1000.0, "secret123");

    displayBalance(myAccount);       // 输出: 余额: $1000

    if (verifyPassword(myAccount, "secret123")) {
        std::cout << "密码正确!\n";
    }

    // 普通函数不能访问
    // std::cout << myAccount.balance;  // 编译错误！
}
```

**怎么看谁是谁的友元？**

```
class A {
    friend void foo();   // ← foo() 是 A 的友元
                         //   即：foo() 可以访问 A 的私有成员
                         //   但：A 不能访问 foo() 的局部变量（foo 不是类）
};
```

### 2. 友元类

```cpp
class Teacher;  // 前向声明

class Student {
private:
    std::string name;
    int score;
    std::string secretDiary;

public:
    Student(const std::string& n, int s) : name(n), score(s) {}

    // 声明友元类 👇 Teacher 是 Student 的友元
    friend class Teacher;

    // 普通人只能通过公开接口访问
    std::string getName() const { return name; }
};

// Teacher 类可以访问 Student 的所有私有成员
class Teacher {
public:
    void gradeStudent(Student& s, int newScore) {
        s.score = newScore;          // 直接修改私有成员
    }

    void readStudentDiary(const Student& s) {
        std::cout << "偷看日记: " << s.secretDiary << "\n";  // 能访问！
    }

    void printStudentInfo(const Student& s) {
        std::cout << "姓名: " << s.name
                  << ", 分数: " << s.score << "\n";  // 都能访问
    }
};

int main() {
    Student alice("Alice", 85);
    Teacher mrSmith;

    mrSmith.gradeStudent(alice, 95);
    mrSmith.printStudentInfo(alice);

    return 0;
}
```

**关键理解：单向关系**

```
Student 说："Teacher 是我的朋友"
→ Teacher 能看 Student 的秘密
→ 但 Student 不能看 Teacher 的秘密！（除非 Teacher 也声明 friend）
```

### 3. 友元成员函数（精确控制）

```cpp
class Engineer;  // 前向声明

class SecuritySystem {
private:
    int accessLevel;
    std::vector<std::string> allowedUsers;

public:
    SecuritySystem(int level) : accessLevel(level) {}

    // 只允许 Engineer 的特定成员函数访问
    friend void Engineer::repairSystem(SecuritySystem& sys);

    void showStatus() {
        std::cout << "安全等级: " << accessLevel << "\n";
    }
};

class Engineer {
public:
    // 这个函数是 SecuritySystem 的友元
    void repairSystem(SecuritySystem& sys) {
        sys.accessLevel = 10;           // 可访问私有成员
        sys.allowedUsers.push_back("Engineer");  // 可访问
        std::cout << "系统已修复!\n";
    }

    // 这个函数不是友元，不能访问
    void hackSystem(SecuritySystem& sys) {
        // sys.accessLevel = 0;  // 编译错误！
    }
};

int main() {
    SecuritySystem sys(5);
    Engineer eng;

    eng.repairSystem(sys);  // OK - 是友元函数
    // eng.hackSystem(sys);  // 会编译失败

    return 0;
}
```

***

## 4. 友元的核心作用

### 1. 作用1：运算符重载（最常见用途）⭐

```cpp
class Complex {
private:
    double real, imag;

public:
    Complex(double r = 0, double i = 0) : real(r), imag(i) {}

    // 左操作数是 ostream，必须是友元！
    friend std::ostream& operator<<(std::ostream& os, const Complex& c) {
        os << c.real;
        if (c.imag >= 0) os << "+";
        os << c.imag << "i";
        return os;
    }

    // 左操作数是 istream，必须是友元！
    friend std::istream& operator>>(std::istream& is, Complex& c) {
        is >> c.real >> c.imag;
        return is;
    }
};

int main() {
    Complex c(3, 4);
    std::cout << c << "\n";         // 输出: 3+4i
    std::cin >> c;                  // 输入: 5 6
    return 0;
}
```

**为什么必须用友元？**
```
cout << c;
↑     ↑
左操作数是 cout（ostream对象），不是Complex对象
所以不能写成 ostream 的成员函数，只能是 Complex 的友元函数
```

### 2. 作用2：两个类需要紧密协作

```cpp
class State;  // 前向声明

class Monitor {
private:
    std::vector<double> temperatureData;
    bool isRunning;

public:
    Monitor() : isRunning(false) {}

    // State 需要深入访问 Monitor 的内部状态
    friend class State;

    void record(double temp) {
        temperatureData.push_back(temp);
    }
};

class State {
public:
    // 保存/恢复 Monitor 的完整内部状态
    void saveState(Monitor& m, std::ofstream& out) {
        out << m.isRunning << " ";                    // 访问私有
        out << m.temperatureData.size() << " ";        // 访问私有
        for (double temp : m.temperatureData) {        // 访问私有
            out << temp << " ";
        }
    }

    void loadState(Monitor& m, std::ifstream& in) {
        in >> m.isRunning;                             // 访问私有
        size_t size;
        in >> size;
        m.temperatureData.resize(size);               // 访问私有
        for (size_t i = 0; i < size; i++) {
            in >> m.temperatureData[i];               // 访问私有
        }
    }
};
```

### 3. 作用3：辅助/工具函数

```cpp
class Matrix {
private:
    int rows, cols;
    std::vector<std::vector<double>> data;

public:
    Matrix(int r, int c) : rows(r), cols(c), data(r, std::vector<double>(c, 0)) {}

    // 数学计算工具函数作为友元
    friend Matrix addMatrices(const Matrix& a, const Matrix& b);
    friend Matrix multiplyMatrices(const Matrix& a, const Matrix& b);
    friend double determinant(const Matrix& m);
    friend Matrix transpose(const Matrix& m);
    friend void printMatrix(const Matrix& m);

    double& at(int r, int c) { return data[r][c]; }
    double at(int r, int c) const { return data[r][c]; }
};

Matrix addMatrices(const Matrix& a, const Matrix& b) {
    assert(a.rows == b.rows && a.cols == b.cols);
    Matrix result(a.rows, a.cols);
    for (int i = 0; i < a.rows; i++) {
        for (int j = 0; j < a.cols; j++) {
            result.data[i][j] = a.data[i][j] + b.data[i][j];  // 直接访问
        }
    }
    return result;
}

void printMatrix(const Matrix& m) {
    for (int i = 0; i < m.rows; i++) {
        for (int j = 0; j < m.cols; j++) {
            printf("%8.2f", m.data[i][j]);  // 直接访问
        }
        printf("\n");
    }
}
```

***

## 5. 友元的三大特性（重要！）

### 1. 特性1：单向性（不是互为朋友）

```cpp
class A {
private:
    int secretA = 100;
    friend class B;   // B 是 A 的朋友 → B 能看 A 的秘密
};

class B {
private:
    int secretB = 200;
    // A 不是 B 的朋友！A 不能看 B 的秘密
};

void test() {
    A objA;
    B objB;

    // B 能访问 A 的私有成员 ✅
    objB.someFunction(objA);  // 内部可以读 objA.secretA

    // A 不能访问 B 的私有成员 ❌
    // objA.someFunction(objB);  // 不能读 objB.secretB
}
```

### 2. 特性2：不传递性（朋友的朋友不是朋友）

```cpp
class A {
    friend class B;   // B 是 A 的朋友
};

class B {
    friend class C;   // C 是 B 的朋友
    // 但是 C 不是 A 的朋友！
};

class C {
    void func(A& a) {
        // a.secret;  // 错误！C 不能访问 A 的私有成员
    }
};
```

### 3. 特性3：不被继承（儿子的朋友 ≠ 爸爸的朋友）

```cpp
class Base {
private:
    int baseSecret = 100;
    friend class FriendClass;
};

class Derived : public Base {
private:
    int derivedSecret = 200;
    // FriendClass 只能访问 Base 的私有成员
    // 不能自动访问 Derived 的私有成员！
};

class FriendClass {
    void func(Base& b, Derived& d) {
        // b.baseSecret;    // 可以，Base 声明了友元
        // d.derivedSecret; // 不行！Derived 没有声明友元
        // d.baseSecret;    // 可以，继承来的，且Base声明了友元
    }
};
```

***

## 6. 实用技巧与最佳实践

### 1. 技巧1：友元声明的位置

```cpp
class MyClass {
private:
    int data;

public:
    // 方式1：放在 public 区域（推荐，更明显）
    friend void helperFunc(MyClass& obj);

protected:
    // 方式2：也可以放在这里
    friend class HelperClass;

private:
    // 方式3：甚至可以放这里（但不太直观）
    friend std::ostream& operator<<(std::ostream&, const MyClass&);
};
```

**建议**：统一放在类声明的**最前面或 public 区域**，方便查找。

### 2. 技巧2：友元 + 模板

```cpp
template<typename T>
class Box {
private:
    T content;

public:
    Box(const T& c) : content(c) {}

    // 模板友元函数
    template<typename U>
    friend void printBox(const Box<U>& box);

    // 模板友元类
    template<typename U>
    friend class BoxInspector;
};

template<typename T>
void printBox(const Box<T>& box) {
    std::cout << "Box contains: " << box.content << "\n";  // 访问私有
}

template<typename T>
class BoxInspector {
public:
    T peek(const Box<T>& box) {
        return box.content;  // 访问私有
    }
};
```

### 3. 技巧3：最小权限原则

```cpp
// 不推荐：把整个类设为友元（暴露太多）
friend class Engine;

// 推荐：只把需要的特定函数设为友元
friend void Engine::ignite(FuelTank& tank);
friend void Engine::checkFuelLevel(const FuelTank& tank);
```

### 4. 技巧4：友元 vs 公开 setter/getter

```cpp
// 传统方式：大量 getter/setter
class Person {
private:
    std::string name;
    int age;
    double salary;

public:
    std::string getName() const { return name; }
    void setName(const std::string& n) { name = n; }
    int getAge() const { return age; }
    void setAge(int a) { age = a; }
    double getSalary() const { return salary; }
    void setSalary(double s) { salary = s; }
};

// 友元方式：减少接口暴露
class Person {
private:
    std::string name;
    int age;
    double salary;

    // 只有真正需要的函数才能访问
    friend void HRSystem::processSalary(Person& p);
    friend void Database::savePerson(const Person& p);
    friend std::ostream& operator<<(std::ostream&, const Person&);
};
```

**对比**：

| 方式 | 封装性 | 代码量 | 适用场景 |
|------|:------:|:------:|----------|
| getter/setter | 弱 | 多 | 需要频繁外部访问 |
| 友元 | 强 | 少 | 特定函数/类需要深度访问 |

### 5. 技巧5：调试用友元

```cpp
class ComplicatedAlgorithm {
private:
    std::vector<int> internalBuffer;
    int stepCount;
    double convergenceRate;

public:
    void run() { /* 复杂算法... */ }

#ifdef DEBUG
    // 仅调试模式下声明为友元
    friend void debugPrintState(const ComplicatedAlgorithm& algo);
#endif
};

#ifdef DEBUG
void debugPrintState(const ComplicatedAlgorithm& algo) {
    printf("步骤: %d\n", algo.stepCount);
    printf("收敛率: %.4f\n", algo.convergenceRate);
    printf("缓冲区大小: %zu\n", algo.internalBuffer.size());
}
#endif
```

***

## 7. 友元使用决策流程图

```
需要让外部代码访问私有成员？
    │
    ├── 可以通过公开接口实现吗？
    │     ├── 是 → 用 getter/setter / 公开方法
    │     └── 否 ↓
    │
    ├── 是运算符重载吗？（<< >> 等）
    │     ├── 是 → 必须用友元函数 ⭐
    │     └── 否 ↓
    │
    ├── 是两个类紧密协作吗？
    │     ├── 是 → 考虑友元类
    │     └── 否 ↓
    │
    ├── 只是临时/一个函数需要？
    │     ├── 是 → 用友元函数
    │     └── 否 ↓
    │
    └── 整个类都需要深度访问？
          ├── 是 → 友元类（但要考虑是否设计有问题）
          └── 否 → 友元成员函数（更精确控制）
```

***

## 8. 何时用 / 何时不用的总结

### 1. 适合使用友元的场景

| 场景 | 示例 |
|------|------|
| **运算符重载** | `operator<<`, `operator>>` |
| **两个类紧密协作** | State/Monitor, Engine/FuelTank |
| **工厂模式** | Factory 需要访问私有构造函数 |
| **测试代码** | 测试类需要检查内部状态 |
| **性能敏感** | 避免getter开销 |
| **遗留代码集成** | 与旧代码交互 |

### 2. 不建议使用友元的场景

| 场景 | 原因 |
|------|------|
| **常规数据访问** | 应该用 getter/setter |
| **为了省事** | 破坏封装性 |
| **公共 API** | 用户不应依赖私有实现 |
| **循环依赖** | A 是 B 的友元，B 也是 A 的友元 |

***

## 9. 极简口诀

```
友元打破封装墙，private 也能望
运算符重载经常用，流插入提取帮上忙
单向不传不继承，朋友朋友不一样
精准控制用成员，整类授权要思量
能不用时尽量免，封装才是硬道理
```

***

### 相关阅读

- [运算符重载格式](07-运算符重载格式.md)
- [CPP命名空间与库打包](./00-CPP命名空间与库打包.md)
- [override与final关键字](09-override与final关键字.md)