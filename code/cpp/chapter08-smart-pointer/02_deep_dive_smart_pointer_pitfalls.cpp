/** @file 02_deep_dive_smart_pointer_pitfalls.cpp
 *  @brief 智能指针陷阱：循环引用、this指针问题、自定义删除器模式、性能对比、何时不用智能指针
 *  @description 对应文档: 08-智能指针与内存管理 | 举一反三：避开智能指针的常见陷阱
 */

#include <iostream>
#include <memory>
#include <vector>
#include <chrono>
#include <string>

class Widget {
public:
    Widget(int id) : id_(id) {
        std::cout << "Widget(" << id_ << ") 构造\n";
    }
    ~Widget() {
        std::cout << "Widget(" << id_ << ") 析构\n";
    }
    void work() const {
        std::cout << "Widget(" << id_ << ") 正在工作\n";
    }
    int id() const { return id_; }
private:
    int id_;
};

void demo_circular_reference() {
    std::cout << "=== 循环引用 (内存泄漏的经典陷阱) ===\n";

    class NodeShared {
    public:
        NodeShared(std::string name) : name_(std::move(name)) {
            std::cout << "NodeShared(" << name_ << ") 构造\n";
        }
        ~NodeShared() {
            std::cout << "NodeShared(" << name_ << ") 析构\n";
        }
        void add_child(std::shared_ptr<NodeShared> child) {
            children_.push_back(std::move(child));
        }
        void set_parent(std::shared_ptr<NodeShared> parent) {
            parent_ = std::move(parent);
        }
        const std::string& name() const { return name_; }
    private:
        std::string name_;
        std::vector<std::shared_ptr<NodeShared>> children_;
        std::shared_ptr<NodeShared> parent_;
    };

    std::cout << "--- 使用 shared_ptr: 循环引用导致内存泄漏 ---\n";
    {
        auto parent = std::make_shared<NodeShared>("父节点");
        auto child = std::make_shared<NodeShared>("子节点");
        parent->add_child(child);
        child->set_parent(parent);
        std::cout << "parent use_count = " << parent.use_count() << "\n";
        std::cout << "child use_count = " << child.use_count() << "\n";
    }
    std::cout << "离开作用域, 但析构函数没有被调用! 内存泄漏!\n\n";

    class NodeFixed {
    public:
        NodeFixed(std::string name) : name_(std::move(name)) {
            std::cout << "NodeFixed(" << name_ << ") 构造\n";
        }
        ~NodeFixed() {
            std::cout << "NodeFixed(" << name_ << ") 析构\n";
        }
        void add_child(std::shared_ptr<NodeFixed> child) {
            children_.push_back(std::move(child));
        }
        void set_parent(std::weak_ptr<NodeFixed> parent) {
            parent_ = std::move(parent);
        }
        std::shared_ptr<NodeFixed> get_parent() const {
            return parent_.lock();
        }
        const std::string& name() const { return name_; }
    private:
        std::string name_;
        std::vector<std::shared_ptr<NodeFixed>> children_;
        std::weak_ptr<NodeFixed> parent_;
    };

    std::cout << "--- 使用 weak_ptr 打破循环引用 ---\n";
    {
        auto parent = std::make_shared<NodeFixed>("父节点");
        auto child = std::make_shared<NodeFixed>("子节点");
        parent->add_child(child);
        child->set_parent(parent);
        std::cout << "parent use_count = " << parent.use_count() << "\n";
        std::cout << "child use_count = " << child.use_count() << "\n";
    }
    std::cout << "离开作用域, 析构函数正确调用!\n";

    std::cout << "\n循环引用的解决原则:\n";
    std::cout << "1. 父子关系: 父用 shared_ptr 持有子, 子用 weak_ptr 引用父\n";
    std::cout << "2. 观察者模式: 主题用 weak_ptr 持有观察者\n";
    std::cout << "3. 缓存: 用 weak_ptr 持有缓存对象\n";

    std::cout << "\n";
}

void demo_this_pointer_problem() {
    std::cout << "=== this 指针的陷阱 ===\n";

    class BadHandler {
    public:
        BadHandler(int id) : id_(id) {
            std::cout << "BadHandler(" << id_ << ") 构造\n";
        }
        ~BadHandler() {
            std::cout << "BadHandler(" << id_ << ") 析构\n";
        }

        std::shared_ptr<BadHandler> get_self_bad() {
            // 严重错误! 创建新的 shared_ptr 管理同一对象
            // 会导致双重删除
            // return std::shared_ptr<BadHandler>(this);  // 绝对不要这样做!
            std::cout << "错误做法: shared_ptr<BadHandler>(this) 会导致双重删除\n";
            return nullptr;
        }
    private:
        int id_;
    };

    std::cout << "错误示例 (注释掉, 不会执行):\n";
    std::cout << "  auto sp1 = make_shared<BadHandler>(1);\n";
    std::cout << "  auto sp2 = sp1->get_self_bad();  // 新控制块! 双重删除!\n\n";

    class GoodHandler : public std::enable_shared_from_this<GoodHandler> {
    public:
        GoodHandler(int id) : id_(id) {
            std::cout << "GoodHandler(" << id_ << ") 构造\n";
        }
        ~GoodHandler() {
            std::cout << "GoodHandler(" << id_ << ") 析构\n";
        }

        std::shared_ptr<GoodHandler> get_self_good() {
            return shared_from_this();
        }
    private:
        int id_;
    };

    auto sp1 = std::make_shared<GoodHandler>(1);
    auto sp2 = sp1->get_self_good();
    std::cout << "正确做法: shared_from_this() use_count = " << sp1.use_count() << "\n";

    std::cout << "\nthis 指针陷阱总结:\n";
    std::cout << "1. 绝不要用 this 构造 shared_ptr\n";
    std::cout << "2. 使用 enable_shared_from_this 和 shared_from_this()\n";
    std::cout << "3. 调用 shared_from_this() 前对象必须已被 shared_ptr 管理\n";
    std::cout << "4. 构造函数中不能调用 shared_from_this()\n";

    std::cout << "\n";
}

void demo_custom_deleter_patterns() {
    std::cout << "=== 自定义删除器模式 ===\n";

    std::cout << "--- 模式1: 日志删除器 ---\n";
    {
        auto logging_deleter = [](int* p) {
            std::cout << "删除 int*, 值=" << *p << "\n";
            delete p;
        };
        std::unique_ptr<int, decltype(logging_deleter)> p(new int(42), logging_deleter);
    }

    std::cout << "\n--- 模式2: 数组删除器 (处理 void* 等不完整类型) ---\n";
    {
        auto array_deleter = [](void* p) {
            std::cout << "调用 operator delete[]\n";
            operator delete[](p);
        };
        std::unique_ptr<void, decltype(array_deleter)> p(operator new[](100), array_deleter);
    }

    std::cout << "\n--- 模式3: 空删除器 (不拥有资源) ---\n";
    {
        int value = 100;
        auto null_deleter = [](int*) {};
        std::shared_ptr<int> p(&value, null_deleter);
        std::cout << "*p = " << *p << " (不管理栈上变量的生命周期)\n";
    }

    std::cout << "\n--- 模式4: shared_ptr 管理C风格资源 ---\n";
    {
        struct FileCloser {
            void operator()(FILE* f) const {
                if (f) {
                    std::cout << "关闭文件\n";
                    fclose(f);
                }
            }
        };
        std::unique_ptr<FILE, FileCloser> file(fopen("test_deleter.txt", "w"));
        if (file) {
            fprintf(file.get(), "自定义删除器管理文件句柄\n");
        }
    }

    std::cout << "\n自定义删除器注意事项:\n";
    std::cout << "1. unique_ptr 的删除器是类型的一部分\n";
    std::cout << "2. shared_ptr 的删除器不是类型的一部分 (更灵活)\n";
    std::cout << "3. 删除器必须保证不抛出异常\n";

    std::cout << "\n";
}

void demo_performance_comparison() {
    std::cout << "=== 性能对比 ===\n";

    const int N = 100000;

    auto test_raw = [&]() {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < N; ++i) {
            int* p = new int(i);
            *p += 1;
            delete p;
        }
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    };

    auto test_unique = [&]() {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < N; ++i) {
            auto p = std::make_unique<int>(i);
            *p += 1;
        }
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    };

    auto test_shared = [&]() {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < N; ++i) {
            auto p = std::make_shared<int>(i);
            *p += 1;
        }
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    };

    auto raw_time = test_raw();
    auto unique_time = test_unique();
    auto shared_time = test_shared();

    std::cout << "创建/销毁 " << N << " 个 int 指针:\n";
    std::cout << "  原始指针:   " << raw_time << " us\n";
    std::cout << "  unique_ptr: " << unique_time << " us\n";
    std::cout << "  shared_ptr: " << shared_time << " us\n";

    std::cout << "\n大小对比:\n";
    std::cout << "  原始指针:   " << sizeof(int*) << " 字节\n";
    std::cout << "  unique_ptr: " << sizeof(std::unique_ptr<int>) << " 字节\n";
    std::cout << "  shared_ptr: " << sizeof(std::shared_ptr<int>) << " 字节\n";

    std::cout << "\n性能要点:\n";
    std::cout << "1. unique_ptr 与原始指针零开销抽象\n";
    std::cout << "2. shared_ptr 有引用计数开销 (原子操作)\n";
    std::cout << "3. make_shared 比先 new 再构造 shared_ptr 更快\n";

    std::cout << "\n";
}

void demo_when_not_to_use_smart_pointers() {
    std::cout << "=== 何时不应使用智能指针 ===\n";

    std::cout << "1. 局部作用域的简单对象:\n";
    {
        std::string s("直接使用栈对象, 不需要智能指针");
        std::cout << "   " << s << "\n";
    }

    std::cout << "\n2. 容器元素:\n";
    {
        std::vector<int> v = {1, 2, 3, 4, 5};
        std::cout << "   vector<int> 直接存储值, 不需要 vector<unique_ptr<int>>\n";
    }

    std::cout << "\n3. 不拥有对象时 (使用引用或原始指针):\n";
    {
        auto print_widget = [](const Widget& w) {
            std::cout << "   传递引用, 不需要 shared_ptr\n";
            w.work();
        };
        auto w = std::make_unique<Widget>(1);
        print_widget(*w);
    }

    std::cout << "\n4. 性能关键路径的频繁分配:\n";
    std::cout << "   考虑对象池或自定义分配器\n";

    std::cout << "\n5. 与C API交互时:\n";
    std::cout << "   使用智能指针管理所有权, 但传递 get() 给C API\n";

    std::cout << "\n6. 多线程队列中的对象传递:\n";
    std::cout << "   使用 unique_ptr + move 语义, 避免共享状态\n";

    std::cout << "\n智能指针使用原则:\n";
    std::cout << "- 默认: unique_ptr (零开销, 所有权清晰)\n";
    std::cout << "- 共享: shared_ptr (有开销, 需要时才用)\n";
    std::cout << "- 观察: weak_ptr (不延长生命周期)\n";
    std::cout << "- 不拥有: 原始指针或引用\n";
    std::cout << "- 栈对象: 不需要任何指针\n";

    std::cout << "\n";
}

int main() {
    demo_circular_reference();
    demo_this_pointer_problem();
    demo_custom_deleter_patterns();
    demo_performance_comparison();
    demo_when_not_to_use_smart_pointers();

    return 0;
}
