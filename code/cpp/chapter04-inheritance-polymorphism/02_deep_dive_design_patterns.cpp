/** @file 02_deep_dive_design_patterns.cpp
 *  @brief 模板方法、策略模式、NVI、CRTP基础
 *  @description 对应文档: 02-CPP/04-inheritance-polymorphism
 */

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <functional>

// ===== 1. 模板方法模式 (Template Method) =====
class DataProcessor {
public:
    virtual ~DataProcessor() = default;

    // 模板方法: 定义算法骨架, 不可重写
    void process() {
        read_data();
        transform_data();
        write_data();
    }

protected:
    // 子步骤: 由派生类实现
    virtual void read_data() = 0;
    virtual void transform_data() = 0;
    virtual void write_data() = 0;
};

class CsvProcessor : public DataProcessor {
protected:
    void read_data() override {
        std::cout << "  读取 CSV 数据" << std::endl;
    }

    void transform_data() override {
        std::cout << "  转换 CSV 格式" << std::endl;
    }

    void write_data() override {
        std::cout << "  写入处理后的 CSV" << std::endl;
    }
};

class JsonProcessor : public DataProcessor {
protected:
    void read_data() override {
        std::cout << "  读取 JSON 数据" << std::endl;
    }

    void transform_data() override {
        std::cout << "  转换 JSON 格式" << std::endl;
    }

    void write_data() override {
        std::cout << "  写入处理后的 JSON" << std::endl;
    }
};

void demo_template_method() {
    std::cout << "===== 模板方法模式 =====" << std::endl;

    std::cout << "CSV 处理:" << std::endl;
    CsvProcessor csv;
    csv.process();

    std::cout << "\nJSON 处理:" << std::endl;
    JsonProcessor json;
    json.process();

    std::cout << "\n模板方法模式要点:" << std::endl;
    std::cout << "  - 基类定义算法骨架 (非虚函数)" << std::endl;
    std::cout << "  - 派生类实现各个步骤 (虚函数)" << std::endl;
    std::cout << "  - 避免重复的算法结构" << std::endl;
    std::cout << "  - 开闭原则: 对扩展开放, 对修改关闭" << std::endl;
}

// ===== 2. NVI (Non-Virtual Interface) 惯用法 =====
class Sorter {
public:
    // 公共接口: 非虚函数
    void sort(std::vector<int>& data) {
        // 前置操作
        std::cout << "  排序前: ";
        for (int v : data) std::cout << v << " ";
        std::cout << std::endl;

        // 调用虚函数实现
        do_sort(data);

        // 后置操作
        std::cout << "  排序后: ";
        for (int v : data) std::cout << v << " ";
        std::cout << std::endl;
    }

    virtual ~Sorter() = default;

protected:
    // 实际实现: 虚函数, 派生类可重写
    virtual void do_sort(std::vector<int>& data) = 0;
};

class BubbleSorter : public Sorter {
protected:
    void do_sort(std::vector<int>& data) override {
        std::cout << "  (冒泡排序)" << std::endl;
        for (size_t i = 0; i < data.size(); ++i) {
            for (size_t j = 0; j + 1 < data.size() - i; ++j) {
                if (data[j] > data[j + 1]) {
                    std::swap(data[j], data[j + 1]);
                }
            }
        }
    }
};

class StdSorter : public Sorter {
protected:
    void do_sort(std::vector<int>& data) override {
        std::cout << "  (std::sort)" << std::endl;
        std::sort(data.begin(), data.end());
    }
};

void demo_nvi() {
    std::cout << "\n===== NVI (Non-Virtual Interface) =====" << std::endl;

    std::vector<int> v1 = {5, 3, 1, 4, 2};
    BubbleSorter bs;
    bs.sort(v1);

    std::vector<int> v2 = {9, 7, 5, 3, 1};
    StdSorter ss;
    ss.sort(v2);

    std::cout << "\nNVI 惯用法:" << std::endl;
    std::cout << "  - 公共接口是非虚函数" << std::endl;
    std::cout << "  - 实现细节是 protected 虚函数" << std::endl;
    std::cout << "  - 基类控制调用时机 (前置/后置条件)" << std::endl;
    std::cout << "  - 比直接暴露虚函数更安全" << std::endl;
    std::cout << "  - Herb Sutter: '虚函数应该几乎总是 private'" << std::endl;
}

// ===== 3. 策略模式 (通过多态实现) =====
class CompressionStrategy {
public:
    virtual ~CompressionStrategy() = default;
    virtual std::string compress(const std::string& data) = 0;
    virtual std::string name() const = 0;
};

class ZipCompression : public CompressionStrategy {
public:
    std::string compress(const std::string& data) override {
        return "[ZIP:" + data + "]";
    }
    std::string name() const override { return "ZIP"; }
};

class GzipCompression : public CompressionStrategy {
public:
    std::string compress(const std::string& data) override {
        return "[GZIP:" + data + "]";
    }
    std::string name() const override { return "GZIP"; }
};

class Compressor {
public:
    void set_strategy(std::unique_ptr<CompressionStrategy> strategy) {
        strategy_ = std::move(strategy);
    }

    std::string compress(const std::string& data) {
        if (strategy_) {
            std::cout << "  使用 " << strategy_->name() << " 压缩" << std::endl;
            return strategy_->compress(data);
        }
        return data;
    }

private:
    std::unique_ptr<CompressionStrategy> strategy_;
};

void demo_strategy_pattern() {
    std::cout << "\n===== 策略模式 (多态实现) =====" << std::endl;

    Compressor compressor;
    compressor.set_strategy(std::make_unique<ZipCompression>());
    std::cout << "  结果: " << compressor.compress("Hello") << std::endl;

    compressor.set_strategy(std::make_unique<GzipCompression>());
    std::cout << "  结果: " << compressor.compress("Hello") << std::endl;

    std::cout << "\n策略模式 vs 模板方法:" << std::endl;
    std::cout << "  策略模式: 组合, 运行时切换算法" << std::endl;
    std::cout << "  模板方法: 继承, 编译时确定算法骨架" << std::endl;
    std::cout << "  策略模式更灵活, 但有虚函数开销" << std::endl;
}

// ===== 4. CRTP (Curiously Recurring Template Pattern) =====
// 编译期多态, 无虚函数开销

template<typename Derived>
class ShapeBase {
public:
    double area() const {
        return static_cast<const Derived*>(this)->compute_area();
    }

    void describe() const {
        std::cout << "  面积 = " << area() << std::endl;
    }
};

class CrtpCircle : public ShapeBase<CrtpCircle> {
public:
    CrtpCircle(double r) : radius_(r) {}

    double compute_area() const {
        return 3.14159265 * radius_ * radius_;
    }

private:
    double radius_;
};

class CrtpSquare : public ShapeBase<CrtpSquare> {
public:
    CrtpSquare(double s) : side_(s) {}

    double compute_area() const {
        return side_ * side_;
    }

private:
    double side_;
};

// CRTP 的另一个用途: 静态多态 + 统一接口
template<typename Derived>
class Counter {
public:
    void increment() {
        ++count_;
        static_cast<Derived*>(this)->on_increment();
    }

    int get_count() const { return count_; }

protected:
    int count_ = 0;
};

class MyCounter : public Counter<MyCounter> {
public:
    void on_increment() {
        std::cout << "  计数增加到 " << get_count() << std::endl;
    }
};

void demo_crtp() {
    std::cout << "\n===== CRTP (奇异递归模板模式) =====" << std::endl;

    CrtpCircle c(5.0);
    c.describe();

    CrtpSquare s(4.0);
    s.describe();

    std::cout << "\nCRTP 计数器:" << std::endl;
    MyCounter mc;
    mc.increment();
    mc.increment();
    mc.increment();

    std::cout << "\nCRTP vs 虚函数多态:" << std::endl;
    std::cout << "  CRTP 优势:" << std::endl;
    std::cout << "    - 无虚函数开销 (无 vptr, 无间接调用)" << std::endl;
    std::cout << "    - 编译期多态, 可内联" << std::endl;
    std::cout << "    - 编译期类型安全" << std::endl;
    std::cout << "  CRTP 劣势:" << std::endl;
    std::cout << "    - 不能在运行时切换类型" << std::endl;
    std::cout << "    - 不能用基类指针存储不同类型" << std::endl;
    std::cout << "    - 增加编译时间和代码膨胀" << std::endl;

    std::cout << "\nCRTP 常见用途:" << std::endl;
    std::cout << "  1. 静态多态 (替代虚函数)" << std::endl;
    std::cout << "  2. 代码复用 (基类调用派生类方法)" << std::endl;
    std::cout << "  3. 编译期接口约束 (类似 Concepts)" << std::endl;
}

int main() {
    std::cout << "========== 设计模式与多态 ==========\n" << std::endl;

    demo_template_method();
    demo_nvi();
    demo_strategy_pattern();
    demo_crtp();

    return 0;
}
