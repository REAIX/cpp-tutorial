/** @file 03_example_object_lifecycle.cpp
 *  @brief 对象创建/销毁顺序、RAII、基于作用域的资源管理
 *  @description 对应文档: 02-CPP/03-class-object
 */

#include <iostream>
#include <string>
#include <memory>
#include <fstream>
#include <vector>
#include <functional>

// ===== 1. 对象创建与销毁顺序 =====
class Tracker {
public:
    Tracker(const std::string& name) : name_(name) {
        std::cout << "  构造: " << name_ << std::endl;
    }
    ~Tracker() {
        std::cout << "  析构: " << name_ << std::endl;
    }
private:
    std::string name_;
};

void demo_construction_destruction_order() {
    std::cout << "===== 对象创建与销毁顺序 =====" << std::endl;

    std::cout << "栈上对象 (后构造先析构, LIFO):" << std::endl;
    {
        Tracker t1("t1");
        Tracker t2("t2");
        Tracker t3("t3");
    }  // t3 先析构, t2 其次, t1 最后

    std::cout << "\n成员变量 (按声明顺序构造, 按逆序析构):" << std::endl;
    struct Outer {
        Tracker a;
        Tracker b;
        Tracker c;
        Outer() : c("c"), b("b"), a("a") {}  // 不管初始化列表顺序, 按声明顺序构造
    };
    Outer outer;  // 构造顺序: a, b, c (声明顺序)

    std::cout << "\n全局/静态对象:" << std::endl;
    std::cout << "  - 全局对象: main() 之前构造, main() 之后析构" << std::endl;
    std::cout << "  - 静态局部对象: 首次执行到声明处构造, 程序结束时析构" << std::endl;
    std::cout << "  - 不同翻译单元的全局对象构造顺序未定义!" << std::endl;
}

// ===== 2. RAII (Resource Acquisition Is Initialization) =====

// RAII 核心思想:
// - 资源获取即初始化: 在构造函数中获取资源
// - 资源释放即析构: 在析构函数中释放资源
// - 利用栈展开(stack unwinding)机制保证资源释放

// 示例: 手动管理文件 (不推荐)
void manual_file_handling() {
    std::cout << "\n手动文件管理 (不推荐):" << std::endl;
    std::ofstream file("test_raii.txt");
    if (!file.is_open()) {
        std::cout << "  打开文件失败" << std::endl;
        return;
    }
    file << "Hello RAII";
    // 如果这里抛异常, file.close() 不会执行!
    // 但 ofstream 本身是 RAII 的, 所以实际上没问题
    file.close();
    std::cout << "  文件已写入并关闭" << std::endl;
}

// 示例: 自定义 RAII 类
class FileRAII {
public:
    explicit FileRAII(const std::string& path) {
        file_.open(path);
        if (!file_.is_open()) {
            throw std::runtime_error("无法打开文件: " + path);
        }
        std::cout << "  RAII: 打开文件 " << path << std::endl;
    }

    ~FileRAII() {
        if (file_.is_open()) {
            file_.close();
            std::cout << "  RAII: 关闭文件" << std::endl;
        }
    }

    FileRAII(const FileRAII&) = delete;
    FileRAII& operator=(const FileRAII&) = delete;

    void write(const std::string& content) {
        file_ << content;
    }

private:
    std::ofstream file_;
};

// 示例: 自定义 RAII 锁
class SimpleLock {
public:
    explicit SimpleLock(bool& mutex) : mutex_(mutex) {
        mutex_ = true;
        std::cout << "  RAII: 加锁" << std::endl;
    }
    ~SimpleLock() {
        mutex_ = false;
        std::cout << "  RAII: 解锁" << std::endl;
    }
    SimpleLock(const SimpleLock&) = delete;
    SimpleLock& operator=(const SimpleLock&) = delete;
private:
    bool& mutex_;
};

void demo_raii() {
    std::cout << "\n===== RAII (资源获取即初始化) =====" << std::endl;

    manual_file_handling();

    {
        FileRAII file("test_raii2.txt");
        file.write("Hello RAII World");
    }  // 自动关闭, 即使发生异常也会关闭

    bool mutex = false;
    {
        SimpleLock lock(mutex);
        std::cout << "  临界区操作, mutex=" << mutex << std::endl;
    }  // 自动解锁
    std::cout << "  离开临界区, mutex=" << mutex << std::endl;

    std::cout << "\nRAII 要点:" << std::endl;
    std::cout << "  - 构造时获取资源, 析构时释放资源" << std::endl;
    std::cout << "  - 异常安全: 栈展开保证析构函数执行" << std::endl;
    std::cout << "  - 禁止拷贝 (或实现深拷贝/共享语义)" << std::endl;
    std::cout << "  - C++ 最重要的惯用法之一" << std::endl;
}

// ===== 3. 基于作用域的资源管理 =====
void demo_scope_based_management() {
    std::cout << "\n===== 基于作用域的资源管理 =====" << std::endl;

    // unique_ptr: 独占所有权的 RAII
    {
        auto ptr = std::make_unique<Tracker>("unique_ptr 对象");
        std::cout << "  使用 unique_ptr 管理的对象" << std::endl;
    }  // 自动析构

    // shared_ptr: 共享所有权的 RAII
    {
        auto sp1 = std::make_shared<Tracker>("shared_ptr 对象");
        {
            auto sp2 = sp1;  // 引用计数 +1
            std::cout << "  引用计数: " << sp1.use_count() << std::endl;
        }  // sp2 析构, 引用计数 -1
        std::cout << "  引用计数: " << sp1.use_count() << std::endl;
    }  // 引用计数归零, 自动析构

    // scope_guard 模式 (C++17 之前用 RAII, C++20 有 std::scope_exit)
    class ScopeGuard {
    public:
        explicit ScopeGuard(std::function<void()> cleanup) : cleanup_(cleanup) {}
        ~ScopeGuard() { cleanup_(); }
        ScopeGuard(const ScopeGuard&) = delete;
        ScopeGuard& operator=(const ScopeGuard&) = delete;
    private:
        std::function<void()> cleanup_;
    };

    {
        std::cout << "  执行操作..." << std::endl;
        ScopeGuard guard([]() {
            std::cout << "  作用域结束, 执行清理" << std::endl;
        });
        std::cout << "  操作进行中" << std::endl;
    }  // guard 析构, 自动执行清理

    std::cout << "\n作用域管理要点:" << std::endl;
    std::cout << "  - 利用 { } 显式控制对象生命周期" << std::endl;
    std::cout << "  - unique_ptr: 独占, 离开作用域自动释放" << std::endl;
    std::cout << "  - shared_ptr: 共享, 引用计数归零释放" << std::endl;
    std::cout << "  - scope_guard: 通用清理机制" << std::endl;
}

int main() {
    std::cout << "========== 对象生命周期与 RAII ==========\n" << std::endl;

    demo_construction_destruction_order();
    demo_raii();
    demo_scope_based_management();

    return 0;
}
