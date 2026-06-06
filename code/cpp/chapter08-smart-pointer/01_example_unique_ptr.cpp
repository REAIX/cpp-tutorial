/** @file 01_example_unique_ptr.cpp
 *  @brief unique_ptr基础：创建、移动语义、自定义删除器、数组unique_ptr
 *  @description 对应文档: 08-智能指针与内存管理
 */

#include <iostream>
#include <memory>
#include <vector>
#include <cstdio>

class Widget {
public:
    Widget(int id) : id_(id) {
        std::cout << "Widget(" << id_ << ") 构造\n";
    }
    ~Widget() {
        std::cout << "Widget(" << id_ << ") 析构\n";
    }
    void greet() const {
        std::cout << "Widget(" << id_ << ") 说你好!\n";
    }
    int id() const { return id_; }
private:
    int id_;
};

void demo_unique_ptr_basics() {
    std::cout << "=== unique_ptr 基础用法 ===\n";

    auto p1 = std::make_unique<Widget>(1);
    p1->greet();
    std::cout << "p1 指向的 Widget id: " << p1->id() << "\n";
    std::cout << "p1.get() 返回原始指针: " << p1.get() << "\n";

    std::cout << "\nunique_ptr 独占所有权, 不能复制:\n";
    // auto p2 = p1;  // 编译错误! unique_ptr 不可复制
    auto p2 = std::move(p1);
    std::cout << "移动后 p1.get() = " << p1.get() << " (变为 nullptr)\n";
    std::cout << "移动后 p2.get() = " << p2.get() << "\n";

    if (p1 == nullptr) {
        std::cout << "p1 现在是空指针\n";
    }

    std::cout << "\n";
}

void demo_make_unique() {
    std::cout << "=== make_unique 创建 unique_ptr ===\n";

    auto p1 = std::make_unique<int>(42);
    std::cout << "make_unique<int>(42): *p1 = " << *p1 << "\n";

    auto p2 = std::make_unique<std::vector<int>>(5, 10);
    std::cout << "make_unique<vector<int>>(5, 10): 大小=" << p2->size() << "\n";

    auto p3 = std::make_unique<std::string>(5, 'A');
    std::cout << "make_unique<string>(5, 'A'): " << *p3 << "\n";

    std::cout << "\nmake_unique 的优势:\n";
    std::cout << "1. 更简洁, 无需写 new\n";
    std::cout << "2. 异常安全, 避免内存泄漏\n";
    std::cout << "3. 与 make_shared 风格一致\n";
    std::cout << "\n";
}

void demo_unique_ptr_in_container() {
    std::cout << "=== unique_ptr 在容器中使用 ===\n";

    std::vector<std::unique_ptr<Widget>> widgets;
    widgets.push_back(std::make_unique<Widget>(10));
    widgets.push_back(std::make_unique<Widget>(20));
    widgets.push_back(std::make_unique<Widget>(30));

    std::cout << "遍历容器中的 unique_ptr:\n";
    for (const auto& w : widgets) {
        w->greet();
    }

    std::cout << "\n转移 unique_ptr 到另一个容器:\n";
    std::vector<std::unique_ptr<Widget>> moved_widgets;
    for (auto& w : widgets) {
        moved_widgets.push_back(std::move(w));
    }
    std::cout << "原容器大小: " << widgets.size() << " (元素被移走, 变为 nullptr)\n";
    std::cout << "新容器大小: " << moved_widgets.size() << "\n";

    std::cout << "\n";
}

void demo_custom_deleter() {
    std::cout << "=== 自定义删除器 ===\n";

    auto file_closer = [](FILE* f) {
        if (f) {
            std::cout << "自定义删除器: 关闭文件\n";
            fclose(f);
        }
    };

    {
        std::unique_ptr<FILE, decltype(file_closer)> file(fopen("test_unique_ptr.txt", "w"), file_closer);
        if (file) {
            fprintf(file.get(), "Hello from unique_ptr with custom deleter!\n");
            std::cout << "文件已写入\n";
        }
    }

    std::cout << "离开作用域, 自定义删除器自动关闭文件\n";

    std::cout << "\n使用函数指针作为删除器:\n";
    auto int_deleter = [](int* p) {
        std::cout << "自定义删除 int*, 值=" << *p << "\n";
        delete p;
    };
    std::unique_ptr<int, decltype(int_deleter)> pi(new int(999), int_deleter);

    std::cout << "\n";
}

void demo_array_unique_ptr() {
    std::cout << "=== 数组 unique_ptr ===\n";

    std::unique_ptr<int[]> arr = std::make_unique<int[]>(5);
    for (int i = 0; i < 5; ++i) {
        arr[i] = i * 10;
    }

    std::cout << "数组 unique_ptr 元素: ";
    for (int i = 0; i < 5; ++i) {
        std::cout << arr[i] << " ";
    }
    std::cout << "\n";

    std::cout << "\n注意: unique_ptr<T[]> 使用 operator[] 而不是 operator->\n";
    std::cout << "C++17 起, 优先使用 std::array 或 std::vector 代替 unique_ptr<T[]>\n";

    std::cout << "\n";
}

void demo_unique_ptr_ownership_transfer() {
    std::cout << "=== unique_ptr 所有权转移 ===\n";

    auto factory = []() -> std::unique_ptr<Widget> {
        return std::make_unique<Widget>(100);
    };

    auto p = factory();
    std::cout << "从工厂函数获取 unique_ptr\n";
    p->greet();

    auto process = [](std::unique_ptr<Widget> w) {
        std::cout << "处理 Widget id=" << w->id() << "\n";
    };

    process(std::move(p));
    std::cout << "转移后 p.get() = " << p.get() << "\n";

    std::cout << "\nunique_ptr 所有权模型:\n";
    std::cout << "1. 始终只有一个 unique_ptr 拥有对象\n";
    std::cout << "2. 所有权通过 std::move 转移\n";
    std::cout << "3. 工厂函数返回 unique_ptr 是所有权转移的常见模式\n";

    std::cout << "\n";
}

void demo_release_and_reset() {
    std::cout << "=== release() 和 reset() ===\n";

    auto p = std::make_unique<int>(42);
    std::cout << "初始: *p = " << *p << "\n";

    int* raw = p.release();
    std::cout << "release() 后: p.get() = " << p.get() << ", *raw = " << *raw << "\n";
    std::cout << "release() 放弃所有权, 返回原始指针, 调用者负责删除\n";
    delete raw;

    p = std::make_unique<int>(100);
    std::cout << "\n重新赋值: *p = " << *p << "\n";

    p.reset(new int(200));
    std::cout << "reset(new int(200)) 后: *p = " << *p << "\n";

    p.reset();
    std::cout << "reset() 无参数后: p.get() = " << p.get() << "\n";

    std::cout << "\n";
}

int main() {
    demo_unique_ptr_basics();
    demo_make_unique();
    demo_unique_ptr_in_container();
    demo_custom_deleter();
    demo_array_unique_ptr();
    demo_unique_ptr_ownership_transfer();
    demo_release_and_reset();

    return 0;
}
