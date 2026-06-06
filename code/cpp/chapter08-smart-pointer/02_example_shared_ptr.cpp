/** @file 02_example_shared_ptr.cpp
 *  @brief shared_ptr基础：创建、引用计数、weak_ptr、与unique_ptr对比
 *  @description 对应文档: 08-智能指针与内存管理
 */

#include <iostream>
#include <memory>
#include <vector>
#include <string>

class Node {
public:
    Node(std::string name) : name_(std::move(name)) {
        std::cout << "Node(" << name_ << ") 构造\n";
    }
    ~Node() {
        std::cout << "Node(" << name_ << ") 析构\n";
    }
    void add_child(std::shared_ptr<Node> child) {
        children_.push_back(std::move(child));
    }
    const std::string& name() const { return name_; }
    void print_children() const {
        std::cout << name_ << " 的子节点: ";
        for (const auto& c : children_) {
            std::cout << c->name() << " ";
        }
        if (children_.empty()) std::cout << "(无)";
        std::cout << "\n";
    }
private:
    std::string name_;
    std::vector<std::shared_ptr<Node>> children_;
};

void demo_shared_ptr_basics() {
    std::cout << "=== shared_ptr 基础用法 ===\n";

    auto p1 = std::make_shared<int>(42);
    std::cout << "*p1 = " << *p1 << "\n";
    std::cout << "p1.use_count() = " << p1.use_count() << "\n";

    auto p2 = p1;
    std::cout << "复制后 p1.use_count() = " << p1.use_count() << "\n";
    std::cout << "复制后 p2.use_count() = " << p2.use_count() << "\n";

    {
        auto p3 = p1;
        std::cout << "内部作用域 p1.use_count() = " << p1.use_count() << "\n";
    }
    std::cout << "离开内部作用域 p1.use_count() = " << p1.use_count() << "\n";

    std::cout << "\nshared_ptr 共享所有权:\n";
    std::cout << "1. 多个 shared_ptr 可以指向同一对象\n";
    std::cout << "2. 引用计数跟踪有多少个 shared_ptr 指向该对象\n";
    std::cout << "3. 最后一个 shared_ptr 销毁时, 对象被删除\n";

    std::cout << "\n";
}

void demo_make_shared() {
    std::cout << "=== make_shared 创建 shared_ptr ===\n";

    auto p1 = std::make_shared<std::string>("Hello");
    std::cout << "make_shared<string>(\"Hello\"): " << *p1 << "\n";

    auto p2 = std::make_shared<std::vector<int>>(5, 7);
    std::cout << "make_shared<vector<int>>(5, 7): 大小=" << p2->size() << "\n";

    struct Point {
        double x, y;
        Point(double x, double y) : x(x), y(y) {}
    };
    auto p3 = std::make_shared<Point>(3.0, 4.0);
    std::cout << "make_shared<Point>(3.0, 4.0): (" << p3->x << ", " << p3->y << ")\n";

    std::cout << "\nmake_shared 的优势:\n";
    std::cout << "1. 单次内存分配 (对象和控制块一起分配)\n";
    std::cout << "2. 异常安全\n";
    std::cout << "3. 更好的缓存局部性\n";

    std::cout << "\n";
}

void demo_shared_ptr_in_container() {
    std::cout << "=== shared_ptr 在容器中使用 ===\n";

    std::vector<std::shared_ptr<Node>> nodes;
    nodes.push_back(std::make_shared<Node>("root"));
    nodes.push_back(std::make_shared<Node>("child1"));
    nodes.push_back(std::make_shared<Node>("child2"));

    nodes[0]->add_child(nodes[1]);
    nodes[0]->add_child(nodes[2]);

    for (const auto& n : nodes) {
        std::cout << n->name() << " use_count=" << n.use_count() << "\n";
    }

    std::cout << "\n";
}

void demo_weak_ptr() {
    std::cout << "=== weak_ptr 弱引用 ===\n";

    std::weak_ptr<int> wp;
    {
        auto sp = std::make_shared<int>(42);
        wp = sp;
        std::cout << "shared_ptr 存在时:\n";
        std::cout << "  wp.use_count() = " << wp.use_count() << "\n";
        std::cout << "  wp.expired() = " << (wp.expired() ? "true" : "false") << "\n";

        if (auto locked = wp.lock()) {
            std::cout << "  wp.lock() 成功: *locked = " << *locked << "\n";
        }
    }

    std::cout << "\nshared_ptr 销毁后:\n";
    std::cout << "  wp.expired() = " << (wp.expired() ? "true" : "false") << "\n";
    if (auto locked = wp.lock()) {
        std::cout << "  wp.lock() 成功\n";
    } else {
        std::cout << "  wp.lock() 失败, 对象已销毁\n";
    }

    std::cout << "\nweak_ptr 的用途:\n";
    std::cout << "1. 打破 shared_ptr 循环引用\n";
    std::cout << "2. 观察对象但不延长其生命周期\n";
    std::cout << "3. 缓存场景: 缓存不阻止对象销毁\n";

    std::cout << "\n";
}

void demo_shared_vs_unique() {
    std::cout << "=== shared_ptr vs unique_ptr 对比 ===\n";

    std::cout << "unique_ptr:\n";
    std::cout << "  - 独占所有权, 零开销\n";
    std::cout << "  - 不可复制, 只能移动\n";
    std::cout << "  - 适合: 明确单一所有者的场景\n";
    std::cout << "  - 大小: 与原始指针相同\n\n";

    std::cout << "shared_ptr:\n";
    std::cout << "  - 共享所有权, 有引用计数开销\n";
    std::cout << "  - 可复制\n";
    std::cout << "  - 适合: 所有权不明确的共享场景\n";
    std::cout << "  - 大小: 两个指针 (对象指针 + 控制块指针)\n\n";

    std::cout << "选择原则:\n";
    std::cout << "  - 默认使用 unique_ptr\n";
    std::cout << "  - 只有需要共享所有权时才用 shared_ptr\n";
    std::cout << "  - 用 weak_ptr 观察共享对象\n";

    std::cout << "\n";
}

void demo_shared_ptr_reset() {
    std::cout << "=== shared_ptr 的 reset() ===\n";

    auto p = std::make_shared<int>(10);
    std::cout << "初始: *p = " << *p << ", use_count = " << p.use_count() << "\n";

    p.reset(new int(20));
    std::cout << "reset(new int(20)) 后: *p = " << *p << ", use_count = " << p.use_count() << "\n";

    auto q = p;
    std::cout << "q = p 后: p.use_count = " << p.use_count() << "\n";

    p.reset();
    std::cout << "p.reset() 后: p.get() = " << p.get() << ", q.use_count = " << q.use_count() << "\n";

    std::cout << "\n";
}

int main() {
    demo_shared_ptr_basics();
    demo_make_shared();
    demo_shared_ptr_in_container();
    demo_weak_ptr();
    demo_shared_vs_unique();
    demo_shared_ptr_reset();

    return 0;
}
