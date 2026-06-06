/** @file 04_deep_dive_crtp_mixin.cpp
 *  @brief CRTP Mixin模式：接口扩展、代码复用、组合式设计
 *  @description 对应文档: 07-模板元编程与编译期计算 / CRTP与静态多态(深入)
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <cmath>

// ============================================================
// 1. CRTP Mixin 基本概念
// ============================================================

// Mixin = 混入，通过继承向类添加功能
// CRTP Mixin = 使用 CRTP 实现的混入模式
// 优势：可以在不修改原类的情况下，通过继承"混入"新功能

void demo_mixin_concept() {
    std::cout << "=== CRTP Mixin 基本概念 ===\n";
    std::cout << "Mixin(混入): 通过继承向类添加功能\n";
    std::cout << "CRTP Mixin: 使用 CRTP 实现的混入模式\n\n";
    std::cout << "核心优势:\n";
    std::cout << "  1. 不修改原类即可扩展功能\n";
    std::cout << "  2. 编译期组合，零运行期开销\n";
    std::cout << "  3. 可叠加多个 Mixin\n\n";
}

// ============================================================
// 2. 基本 Mixin：为类添加功能
// ============================================================

// Mixin1：添加打印功能
template<typename Derived>
class Printable {
public:
    void print() const {
        static_cast<const Derived*>(this)->do_print();
    }

    void println() const {
        print();
        std::cout << "\n";
    }
};

// Mixin2：添加比较功能
template<typename Derived>
class Comparable {
public:
    bool operator==(const Derived& other) const {
        return static_cast<const Derived*>(this)->do_equal(other);
    }
    bool operator!=(const Derived& other) const {
        return !(*this == other);
    }
};

// Mixin3：添加序列化功能
template<typename Derived>
class Serializable {
public:
    std::string to_json() const {
        return static_cast<const Derived*>(this)->do_to_json();
    }
};

// 组合多个 Mixin
class Person : public Printable<Person>,
               public Comparable<Person>,
               public Serializable<Person> {
    std::string name_;
    int age_;
public:
    Person(std::string name, int age) : name_(std::move(name)), age_(age) {}

    const std::string& name() const { return name_; }
    int age() const { return age_; }

    // Printable 接口
    void do_print() const {
        std::cout << "Person(" << name_ << ", " << age_ << ")";
    }

    // Comparable 接口
    bool do_equal(const Person& other) const {
        return name_ == other.name_ && age_ == other.age_;
    }

    // Serializable 接口
    std::string do_to_json() const {
        return "{\"name\":\"" + name_ + "\",\"age\":" + std::to_string(age_) + "}";
    }
};

void demo_basic_mixins() {
    std::cout << "=== 基本 Mixin ===\n";

    Person p1("Alice", 30);
    Person p2("Bob", 25);
    Person p3("Alice", 30);

    // 使用 Printable Mixin
    std::cout << "Printable: ";
    p1.println();

    // 使用 Comparable Mixin
    std::cout << "Comparable: p1 == p3? " << (p1 == p3 ? "是" : "否") << "\n";
    std::cout << "Comparable: p1 == p2? " << (p1 == p2 ? "是" : "否") << "\n";

    // 使用 Serializable Mixin
    std::cout << "Serializable: " << p1.to_json() << "\n";

    std::cout << "\n";
}

// ============================================================
// 3. 叠加 Mixin：链式继承
// ============================================================

// Mixin 可以叠加，形成功能链
// 最内层是核心类，外层逐步添加功能

// 核心类
class Number {
    double value_;
public:
    explicit Number(double v) : value_(v) {}
    double value() const { return value_; }
    void set_value(double v) { value_ = v; }
};

// Mixin层1：添加算术运算
template<typename Base>
class Arithmetic : public Base {
public:
    using Base::Base;  // 继承构造函数

    Arithmetic operator+(const Arithmetic& other) const {
        return Arithmetic(this->value() + other.value());
    }
    Arithmetic operator-(const Arithmetic& other) const {
        return Arithmetic(this->value() - other.value());
    }
    Arithmetic operator*(const Arithmetic& other) const {
        return Arithmetic(this->value() * other.value());
    }
    Arithmetic operator/(const Arithmetic& other) const {
        return Arithmetic(this->value() / other.value());
    }
};

// Mixin层2：添加比较运算
template<typename Base>
class Ordered : public Base {
public:
    using Base::Base;

    bool operator<(const Ordered& other) const {
        return this->value() < other.value();
    }
    bool operator>(const Ordered& other) const {
        return this->value() > other.value();
    }
    bool operator<=(const Ordered& other) const {
        return !(this->value() > other.value());
    }
    bool operator>=(const Ordered& other) const {
        return !(this->value() < other.value());
    }
};

// Mixin层3：添加格式化输出
template<typename Base>
class Formattable : public Base {
public:
    using Base::Base;

    std::string to_fixed(int precision = 2) const {
        std::string result = std::to_string(this->value());
        auto dot = result.find('.');
        if (dot != std::string::npos && result.size() > dot + precision + 1) {
            result = result.substr(0, dot + precision + 1);
        }
        return result;
    }
};

// 组合所有 Mixin
using SmartNumber = Formattable<Ordered<Arithmetic<Number>>>;

void demo_stacked_mixins() {
    std::cout << "=== 叠加 Mixin ===\n";

    SmartNumber a(3.14159);
    SmartNumber b(2.71828);

    // 算术运算（来自 Arithmetic Mixin）
    // 注意: 运算结果类型是 Arithmetic<Number>，需显式转为 SmartNumber
    SmartNumber sum = SmartNumber(a.value() + b.value());
    SmartNumber diff = SmartNumber(a.value() - b.value());
    SmartNumber prod = SmartNumber(a.value() * b.value());

    // 比较运算（来自 Ordered Mixin）
    std::cout << "a > b? " << (a > b ? "是" : "否") << "\n";

    // 格式化输出（来自 Formattable Mixin）
    std::cout << "a + b = " << sum.to_fixed(4) << "\n";
    std::cout << "a - b = " << diff.to_fixed(4) << "\n";
    std::cout << "a * b = " << prod.to_fixed(4) << "\n";

    std::cout << "\n叠加顺序:\n";
    std::cout << "  Number (核心)\n";
    std::cout << "    -> Arithmetic<Number> (算术运算)\n";
    std::cout << "      -> Ordered<Arithmetic<Number>> (比较运算)\n";
    std::cout << "        -> Formattable<...> (格式化输出)\n";

    std::cout << "\n";
}

// ============================================================
// 4. CRTP Mixin 实现观察者模式
// ============================================================

// Observable Mixin：为任何类添加观察者功能
template<typename Derived>
class Observable {
    std::vector<std::function<void(const Derived&)>> observers_;

public:
    void subscribe(std::function<void(const Derived&)> observer) {
        observers_.push_back(std::move(observer));
    }

protected:
    void notify_observers() const {
        for (const auto& obs : observers_) {
            obs(static_cast<const Derived&>(*this));
        }
    }
};

// 可观察的温度传感器
class TemperatureSensor : public Observable<TemperatureSensor> {
    double temperature_ = 0.0;

public:
    double temperature() const { return temperature_; }

    void set_temperature(double t) {
        double old = temperature_;
        temperature_ = t;
        if (old != t) {
            notify_observers();  // 温度变化时通知观察者
        }
    }
};

// 可观察的银行账户
class BankAccount : public Observable<BankAccount> {
    std::string owner_;
    double balance_ = 0.0;

public:
    explicit BankAccount(std::string owner) : owner_(std::move(owner)) {}

    const std::string& owner() const { return owner_; }
    double balance() const { return balance_; }

    void deposit(double amount) {
        balance_ += amount;
        notify_observers();
    }

    void withdraw(double amount) {
        if (amount <= balance_) {
            balance_ -= amount;
            notify_observers();
        }
    }
};

void demo_observable_mixin() {
    std::cout << "=== Observable Mixin ===\n";

    // 温度传感器
    TemperatureSensor sensor;
    sensor.subscribe([](const TemperatureSensor& s) {
        std::cout << "  [温度警报] 当前温度: " << s.temperature() << "°C\n";
    });
    sensor.subscribe([](const TemperatureSensor& s) {
        if (s.temperature() > 30.0) {
            std::cout << "  [高温警告] 温度超过30°C!\n";
        }
    });

    std::cout << "设置温度为25°C:\n";
    sensor.set_temperature(25.0);

    std::cout << "\n设置温度为35°C:\n";
    sensor.set_temperature(35.0);

    // 银行账户
    BankAccount account("Alice");
    account.subscribe([](const BankAccount& a) {
        std::cout << "  [账户通知] " << a.owner()
                  << " 余额: " << a.balance() << "\n";
    });

    std::cout << "\n存款1000:\n";
    account.deposit(1000.0);

    std::cout << "取款300:\n";
    account.withdraw(300.0);

    std::cout << "\n";
}

// ============================================================
// 5. CRTP Mixin 实现日志功能
// ============================================================

// Loggable Mixin：为类添加日志功能
template<typename Derived>
class Loggable {
    std::string log_prefix_;
    bool logging_enabled_ = true;

public:
    void set_log_prefix(std::string prefix) {
        log_prefix_ = std::move(prefix);
    }

    void enable_logging(bool enabled) {
        logging_enabled_ = enabled;
    }

protected:
    void log(const std::string& message) const {
        if (logging_enabled_) {
            std::cout << "[" << log_prefix_ << "] " << message << "\n";
        }
    }

    void log_action(const std::string& action) const {
        log(action + " on " + static_cast<const Derived*>(this)->object_name());
    }
};

// 带日志的文件处理器
class FileHandler : public Loggable<FileHandler> {
    std::string filename_;
    bool open_ = false;

public:
    explicit FileHandler(std::string filename) : filename_(std::move(filename)) {
        set_log_prefix("FileHandler");
    }

    std::string object_name() const { return filename_; }

    void open() {
        log_action("打开文件");
        open_ = true;
    }

    void close() {
        log_action("关闭文件");
        open_ = false;
    }

    void write(const std::string& data) {
        if (open_) {
            log("写入数据: " + data);
        } else {
            log("错误: 文件未打开!");
        }
    }
};

// 带日志的网络连接
class NetworkConnection : public Loggable<NetworkConnection> {
    std::string host_;
    int port_;
    bool connected_ = false;

public:
    NetworkConnection(std::string host, int port)
        : host_(std::move(host)), port_(port) {
        set_log_prefix("Network");
    }

    std::string object_name() const { return host_ + ":" + std::to_string(port_); }

    void connect() {
        log_action("连接");
        connected_ = true;
    }

    void disconnect() {
        log_action("断开连接");
        connected_ = false;
    }

    void send(const std::string& data) {
        if (connected_) {
            log("发送数据: " + data);
        } else {
            log("错误: 未连接!");
        }
    }
};

void demo_loggable_mixin() {
    std::cout << "=== Loggable Mixin ===\n";

    FileHandler file("test.txt");
    file.open();
    file.write("Hello, World!");
    file.close();

    std::cout << "\n";

    NetworkConnection net("example.com", 8080);
    net.connect();
    net.send("GET / HTTP/1.1");
    net.disconnect();

    std::cout << "\n禁用日志后:\n";
    file.enable_logging(false);
    file.open();
    file.write("这条不会显示");
    file.close();

    std::cout << "\n";
}

// ============================================================
// 6. CRTP Mixin 实现缓存功能
// ============================================================

// Cacheable Mixin：为计算类添加缓存
template<typename Derived>
class Cacheable {
    mutable bool cached_ = false;
    mutable double cached_result_ = 0.0;

public:
    double cached_compute() const {
        if (!cached_) {
            cached_result_ = static_cast<const Derived*>(this)->compute();
            cached_ = true;
        }
        return cached_result_;
    }

    void invalidate_cache() {
        cached_ = false;
    }

    bool has_cache() const { return cached_; }
};

// 可缓存的复杂计算
class ExpensiveComputation : public Cacheable<ExpensiveComputation> {
    double param_;

public:
    explicit ExpensiveComputation(double p) : param_(p) {}

    double param() const { return param_; }
    void set_param(double p) {
        if (p != param_) {
            param_ = p;
            invalidate_cache();  // 参数变化时自动失效缓存
        }
    }

    double compute() const {
        // 模拟耗时计算
        std::cout << "  (执行复杂计算...)\n";
        double result = 0.0;
        for (int i = 0; i < 100; ++i) {
            result += std::sin(param_ * i) * std::cos(param_ * i);
        }
        return result;
    }
};

void demo_cacheable_mixin() {
    std::cout << "=== Cacheable Mixin ===\n";

    ExpensiveComputation comp(1.5);

    std::cout << "第一次计算:\n";
    double r1 = comp.cached_compute();
    std::cout << "  结果: " << r1 << "\n";

    std::cout << "\n第二次计算(使用缓存):\n";
    double r2 = comp.cached_compute();
    std::cout << "  结果: " << r2 << "\n";
    std::cout << "  缓存命中: " << (r1 == r2 ? "是" : "否") << "\n";

    std::cout << "\n修改参数后:\n";
    comp.set_param(2.0);
    double r3 = comp.cached_compute();
    std::cout << "  结果: " << r3 << "\n";

    std::cout << "\n";
}

// ============================================================
// 7. Mixin 组合的灵活性
// ============================================================

// 核心实体
class Task {
    std::string description_;
    bool done_ = false;
public:
    explicit Task(std::string desc) : description_(std::move(desc)) {}
    const std::string& description() const { return description_; }
    bool is_done() const { return done_; }
    void complete() { done_ = true; }
    std::string object_name() const { return "Task: " + description_; }

    void do_print() const {
        std::cout << "Task(" << description_ << (done_ ? ", 完成" : ", 未完成") << ")";
    }

    bool do_equal(const Task& other) const {
        return description_ == other.description_;
    }

    std::string do_to_json() const {
        return "{\"desc\":\"" + description_ + "\",\"done\":" + (done_ ? "true" : "false") + "}";
    }
};

// 选择性组合 Mixin
// 注意: CRTP 的模板参数必须是最终派生类
class BasicTask : public Printable<BasicTask> {
    Task task_;
public:
    explicit BasicTask(std::string desc) : task_(std::move(desc)) {}
    const std::string& description() const { return task_.description(); }
    bool is_done() const { return task_.is_done(); }
    void complete() { task_.complete(); }
    void do_print() const { task_.do_print(); }
};

class FullTask : public Printable<FullTask>,
                 public Comparable<FullTask>,
                 public Serializable<FullTask>,
                 public Loggable<FullTask> {
    Task task_;
public:
    explicit FullTask(std::string desc) : task_(std::move(desc)) {
        set_log_prefix("FullTask");
    }
    const std::string& description() const { return task_.description(); }
    bool is_done() const { return task_.is_done(); }
    void complete() {
        task_.complete();
        log_action("完成");
    }
    void do_print() const { task_.do_print(); }
    bool do_equal(const FullTask& other) const { return task_.do_equal(other.task_); }
    std::string do_to_json() const { return task_.do_to_json(); }
    std::string object_name() const { return task_.object_name(); }
};

void demo_mixin_composition() {
    std::cout << "=== Mixin 组合灵活性 ===\n";

    std::cout << "BasicTask (仅 Printable):\n";
    BasicTask bt("简单任务");
    bt.print();
    std::cout << "\n";

    std::cout << "\nFullTask (Printable + Comparable + Serializable + Loggable):\n";
    FullTask ft("完整任务");
    ft.println();
    ft.complete();
    std::cout << "JSON: " << ft.to_json() << "\n";

    std::cout << "\nMixin 组合原则:\n";
    std::cout << "  1. 按需组合，不需要的功能不混入\n";
    std::cout << "  2. 每个 Mixin 职责单一\n";
    std::cout << "  3. Mixin 之间尽量解耦\n";
    std::cout << "  4. 使用 using Base::Base 继承构造函数\n";

    std::cout << "\n";
}

// ============================================================
// main
// ============================================================

int main() {
    std::cout << "============================================\n";
    std::cout << "  CRTP Mixin 模式\n";
    std::cout << "============================================\n\n";

    demo_mixin_concept();
    demo_basic_mixins();
    demo_stacked_mixins();
    demo_observable_mixin();
    demo_loggable_mixin();
    demo_cacheable_mixin();
    demo_mixin_composition();

    std::cout << "============================================\n";
    std::cout << "  核心要点总结:\n";
    std::cout << "  1. Mixin: 通过继承混入功能\n";
    std::cout << "  2. CRTP Mixin: 零开销的编译期组合\n";
    std::cout << "  3. 叠加Mixin: 链式继承逐层增强\n";
    std::cout << "  4. 常见Mixin: Printable, Loggable, Observable\n";
    std::cout << "  5. 按需组合，职责单一\n";
    std::cout << "============================================\n";

    return 0;
}
