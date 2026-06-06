/** @file 02_deep_dive_exception_pitfalls.cpp
 *  @brief 析构函数中抛异常、栈展开期间的异常、异常中的内存泄漏、性能影响
 *  @description 对应文档: 02-CPP/07-exception
 */

#include <iostream>
#include <string>
#include <stdexcept>
#include <memory>
#include <vector>

// ===== 1. 析构函数中抛出异常 =====
class BadDestructor {
public:
    BadDestructor(const std::string& name) : name_(name) {}

    ~BadDestructor() {
        // 危险! 析构函数中抛出异常
        // 如果在栈展开期间析构函数抛出异常, 程序会 std::terminate
        std::cout << "  ~BadDestructor(" << name_ << ")" << std::endl;
        // throw std::runtime_error("析构函数中的异常");  // 绝对不要这样做!
    }

private:
    std::string name_;
};

class SafeDestructor {
public:
    SafeDestructor(const std::string& name) : name_(name) {}

    ~SafeDestructor() noexcept {
        // 安全: 析构函数中捕获所有异常
        try {
            cleanup();
        } catch (const std::exception& e) {
            std::cerr << "  析构函数中捕获异常: " << e.what() << std::endl;
            // 记录日志, 但不传播异常
        } catch (...) {
            std::cerr << "  析构函数中捕获未知异常" << std::endl;
        }
    }

private:
    void cleanup() {
        // 可能抛出异常的清理操作
        // 模拟: 某些情况下清理可能失败
        std::cout << "  SafeDestructor::cleanup(" << name_ << ")" << std::endl;
    }

    std::string name_;
};

void demo_destructor_exception() {
    std::cout << "===== 析构函数中抛出异常 =====" << std::endl;

    std::cout << "规则: 析构函数绝不能抛出异常!" << std::endl;
    std::cout << "  - C++11 起, 析构函数隐式 noexcept" << std::endl;
    std::cout << "  - 如果析构函数抛出异常, std::terminate 被调用" << std::endl;
    std::cout << "  - 在栈展开期间, 两个同时活跃的异常导致 terminate" << std::endl;

    std::cout << "\n安全做法:" << std::endl;
    std::cout << "  - 析构函数中 try-catch 捕获所有异常" << std::endl;
    std::cout << "  - 提供单独的 close/cleanup 方法处理可能失败的操作" << std::endl;
    std::cout << "  - 析构函数中只做不会失败的操作" << std::endl;

    SafeDestructor sd("测试");
}

// ===== 2. 栈展开期间的异常 =====
class Tracker {
public:
    explicit Tracker(const std::string& name) : name_(name) {
        std::cout << "  构造: " << name_ << std::endl;
    }
    ~Tracker() {
        std::cout << "  析构: " << name_ << std::endl;
    }
private:
    std::string name_;
};

void demo_stack_unwinding() {
    std::cout << "\n===== 栈展开期间的异常 =====" << std::endl;

    std::cout << "正常栈展开:" << std::endl;
    try {
        Tracker t1("t1");
        Tracker t2("t2");
        {
            Tracker t3("t3");
            throw std::runtime_error("测试异常");
        }
    } catch (const std::runtime_error& e) {
        std::cout << "  捕获: " << e.what() << std::endl;
    }
    // 析构顺序: t3 -> t2 -> t1 (栈展开, LIFO)

    std::cout << "\n危险: 栈展开期间析构函数抛出异常" << std::endl;
    std::cout << "  如果 t3 的析构函数抛出异常:" << std::endl;
    std::cout << "  1. 已经有一个异常在传播 (runtime_error)" << std::endl;
    std::cout << "  2. 析构函数又抛出第二个异常" << std::endl;
    std::cout << "  3. 两个同时活跃的异常 -> std::terminate" << std::endl;
    std::cout << "  4. 程序直接终止, 不执行任何清理" << std::endl;

    std::cout << "\n栈展开要点:" << std::endl;
    std::cout << "  - 异常抛出后, 从 throw 到 catch 之间的局部对象被析构" << std::endl;
    std::cout << "  - 析构顺序与构造顺序相反 (LIFO)" << std::endl;
    std::cout << "  - 析构函数必须 noexcept" << std::endl;
}

// ===== 3. 异常中的内存泄漏 =====
void demo_memory_leak_in_exception() {
    std::cout << "\n===== 异常中的内存泄漏 =====" << std::endl;

    // 泄漏场景1: 裸指针 + 异常
    std::cout << "场景1: 裸指针泄漏" << std::endl;
    try {
        int* p1 = new int(42);
        int* p2 = new int(100);  // 如果这里抛出 bad_alloc
        // p1 泄漏! 因为 delete p1 不会执行
        delete p1;
        delete p2;
    } catch (const std::bad_alloc& e) {
        std::cout << "  捕获 bad_alloc: " << e.what() << std::endl;
        std::cout << "  p1 可能已泄漏!" << std::endl;
    }

    // 解决方案: 智能指针
    std::cout << "\n解决方案: 智能指针" << std::endl;
    try {
        auto p1 = std::make_unique<int>(42);
        auto p2 = std::make_unique<int>(100);  // 即使抛出异常
        // p1 也会被正确释放 (栈展开)
        std::cout << "  智能指针: *p1 = " << *p1 << ", *p2 = " << *p2 << std::endl;
    } catch (const std::bad_alloc& e) {
        std::cout << "  捕获 bad_alloc, 但无泄漏" << std::endl;
    }

    // 泄漏场景2: 构造函数中的资源获取
    std::cout << "\n场景2: 构造函数中的部分初始化" << std::endl;
    std::cout << "  class Widget {" << std::endl;
    std::cout << "    Widget() : f1(open()), f2(open()) {}" << std::endl;
    std::cout << "    如果 f2(open()) 抛异常, f1 不会关闭!" << std::endl;
    std::cout << "  };" << std::endl;
    std::cout << "  解决: 用 RAII 成员管理每个资源" << std::endl;

    std::cout << "\n避免异常中内存泄漏的规则:" << std::endl;
    std::cout << "  1. 使用智能指针代替裸指针" << std::endl;
    std::cout << "  2. 使用 RAII 管理所有资源" << std::endl;
    std::cout << "  3. 使用标准容器代替手动内存管理" << std::endl;
    std::cout << "  4. 构造函数中用 RAII 成员管理资源" << std::endl;
}

// ===== 4. 异常的性能影响 =====
void demo_exception_performance() {
    std::cout << "\n===== 异常的性能影响 =====" << std::endl;

    std::cout << "异常的运行时开销:" << std::endl;
    std::cout << "  1. 代码大小: 异常处理代码增加二进制大小" << std::endl;
    std::cout << "     - DWARF 异常展开表 (.eh_frame)" << std::endl;
    std::cout << "     - 通常增加 10-20% 的代码大小" << std::endl;

    std::cout << "\n  2. 正常路径 (无异常抛出):" << std::endl;
    std::cout << "     - 几乎零开销 (与返回错误码相当)" << std::endl;
    std::cout << "     - 不需要检查返回值" << std::endl;
    std::cout << "     - 编译器优化不受影响" << std::endl;

    std::cout << "\n  3. 异常路径 (异常被抛出):" << std::endl;
    std::cout << "     - 栈展开: 比返回错误码慢几个数量级" << std::endl;
    std::cout << "     - 需要查找匹配的 catch 块" << std::endl;
    std::cout << "     - 析构所有局部对象" << std::endl;

    std::cout << "\n  4. 内存开销:" << std::endl;
    std::cout << "     - 异常展开表占用内存" << std::endl;
    std::cout << "     - 异常对象本身在堆上分配 (某些实现)" << std::endl;

    std::cout << "\n异常 vs 错误码:" << std::endl;
    std::cout << "  异常优势:" << std::endl;
    std::cout << "    - 正常路径无开销" << std::endl;
    std::cout << "    - 错误不会被忽略" << std::endl;
    std::cout << "    - 自动资源清理 (RAII)" << std::endl;
    std::cout << "    - 可以携带丰富的错误信息" << std::endl;

    std::cout << "\n  错误码优势:" << std::endl;
    std::cout << "    - 确定性性能 (无异常展开开销)" << std::endl;
    std::cout << "    - 更小的二进制" << std::endl;
    std::cout << "    - 适合实时/嵌入式系统" << std::endl;
    std::cout << "    - 跨语言边界更安全" << std::endl;

    std::cout << "\n  选择建议:" << std::endl;
    std::cout << "    - 一般应用: 使用异常" << std::endl;
    std::cout << "    - 实时/嵌入式: 使用错误码" << std::endl;
    std::cout << "    - 游戏引擎: 混合使用" << std::endl;
    std::cout << "    - 异常用于不可恢复的错误" << std::endl;
    std::cout << "    - 错误码用于可预期的失败" << std::endl;
}

// ===== 5. 举一反三: 异常安全检查清单 =====
void demo_exception_safety_checklist() {
    std::cout << "\n===== 举一反三: 异常安全检查清单 =====" << std::endl;

    std::cout << "代码审查时检查:" << std::endl;

    std::cout << "\n  1. 析构函数是否 noexcept?" << std::endl;
    std::cout << "     - 析构函数中不能抛出异常" << std::endl;
    std::cout << "     - 必要时用 try-catch 包裹" << std::endl;

    std::cout << "\n  2. 资源是否用 RAII 管理?" << std::endl;
    std::cout << "     - 智能指针代替裸指针" << std::endl;
    std::cout << "     - 标准容器代替手动数组" << std::endl;
    std::cout << "     - lock_guard 代替手动加解锁" << std::endl;

    std::cout << "\n  3. 移动操作是否 noexcept?" << std::endl;
    std::cout << "     - 影响容器性能和异常安全" << std::endl;
    std::cout << "     - vector 扩容依赖 noexcept 移动" << std::endl;

    std::cout << "\n  4. 构造函数是否异常安全?" << std::endl;
    std::cout << "     - 部分初始化时资源不泄漏" << std::endl;
    std::cout << "     - 用 RAII 成员管理每个资源" << std::endl;

    std::cout << "\n  5. swap 是否 noexcept?" << std::endl;
    std::cout << "     - copy-and-swap 惯用法的基础" << std::endl;
    std::cout << "     - 强异常安全保证的关键" << std::endl;

    std::cout << "\n  6. catch 顺序是否正确?" << std::endl;
    std::cout << "     - 派生类在前, 基类在后" << std::endl;
    std::cout << "     - catch(...) 在最后" << std::endl;

    std::cout << "\n  7. 异常类型是否合适?" << std::endl;
    std::cout << "     - 使用标准异常或自定义异常类" << std::endl;
    std::cout << "     - 不要抛出内置类型 (int, const char*)" << std::endl;
    std::cout << "     - 异常携带足够的上下文信息" << std::endl;
}

int main() {
    std::cout << "========== 异常处理陷阱 ==========\n" << std::endl;

    demo_destructor_exception();
    demo_stack_unwinding();
    demo_memory_leak_in_exception();
    demo_exception_performance();
    demo_exception_safety_checklist();

    return 0;
}
