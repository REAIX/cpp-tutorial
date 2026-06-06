/** @file 01_deep_dive_smart_pointer_patterns.cpp
 *  @brief 智能指针进阶模式：PIMPL、工厂模式、aliasing constructor、enable_shared_from_this
 *  @description 对应文档: 08-智能指针与内存管理 | 举一反三：掌握智能指针的高级设计模式
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <functional>

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

class Window {
public:
    Window(const std::string& title);
    ~Window();
    Window(Window&&) noexcept;
    Window& operator=(Window&&) noexcept;
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    void show() const;
    void resize(int w, int h);
    std::string title() const;
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

struct Window::Impl {
    std::string title;
    int width = 800;
    int height = 600;
    Impl(const std::string& t) : title(t) {}
};

Window::Window(const std::string& title)
    : pImpl(std::make_unique<Impl>(title)) {
    std::cout << "Window(\"" << title << "\") 构造\n";
}

Window::~Window() {
    std::cout << "Window 析构\n";
}

Window::Window(Window&&) noexcept = default;
Window& Window::operator=(Window&&) noexcept = default;

void Window::show() const {
    std::cout << "显示窗口: " << pImpl->title
              << " (" << pImpl->width << "x" << pImpl->height << ")\n";
}

void Window::resize(int w, int h) {
    pImpl->width = w;
    pImpl->height = h;
}

std::string Window::title() const {
    return pImpl->title;
}

enum class WidgetType { Simple, Advanced, Premium };

std::unique_ptr<Widget> create_widget(WidgetType type) {
    switch (type) {
        case WidgetType::Simple:   return std::make_unique<Widget>(1);
        case WidgetType::Advanced: return std::make_unique<Widget>(2);
        case WidgetType::Premium:  return std::make_unique<Widget>(3);
    }
    return nullptr;
}

struct Person {
    std::string name;
    int age;
    Person(std::string n, int a) : name(std::move(n)), age(a) {
        std::cout << "Person(" << name << ") 构造\n";
    }
    ~Person() {
        std::cout << "Person(" << name << ") 析构\n";
    }
};

class Connection : public std::enable_shared_from_this<Connection> {
public:
    Connection(int id) : id_(id) {
        std::cout << "Connection(" << id_ << ") 构造\n";
    }
    ~Connection() {
        std::cout << "Connection(" << id_ << ") 析构\n";
    }

    void process() {
        std::cout << "Connection(" << id_ << ") 处理请求\n";
    }

    std::shared_ptr<Connection> get_self() {
        return shared_from_this();
    }

    void register_callback() {
        auto self = shared_from_this();
        std::cout << "注册回调, use_count = " << self.use_count() << "\n";
        callbacks_.push_back([self]() {
            self->process();
        });
    }

    void trigger_callbacks() {
        for (const auto& cb : callbacks_) {
            cb();
        }
    }
private:
    int id_;
    std::vector<std::function<void()>> callbacks_;
};

void demo_pimpl_with_unique_ptr() {
    std::cout << "=== PIMPL 惯用法 (编译防火墙) ===\n";

    Window win("主窗口");
    win.show();
    win.resize(1024, 768);
    win.show();

    std::cout << "\nPIMPL 的好处:\n";
    std::cout << "1. 头文件不需要暴露私有成员和实现细节\n";
    std::cout << "2. 修改实现不需要重新编译使用者\n";
    std::cout << "3. 减少头文件依赖, 加快编译速度\n";
    std::cout << "4. unique_ptr 是 PIMPL 的最佳搭档\n";

    std::cout << "\n";
}

void demo_factory_returning_unique_ptr() {
    std::cout << "=== 工厂函数返回 unique_ptr ===\n";

    auto w1 = create_widget(WidgetType::Simple);
    auto w2 = create_widget(WidgetType::Advanced);
    auto w3 = create_widget(WidgetType::Premium);

    w1->work();
    w2->work();
    w3->work();

    std::cout << "\n工厂函数返回 unique_ptr 的好处:\n";
    std::cout << "1. 所有权清晰: 调用者获得独占所有权\n";
    std::cout << "2. 如果需要共享, 调用者可以转为 shared_ptr\n";
    std::cout << "3. unique_ptr 可隐式转为 shared_ptr\n";

    std::shared_ptr<Widget> shared_w = create_widget(WidgetType::Simple);
    std::cout << "unique_ptr 转为 shared_ptr, use_count = " << shared_w.use_count() << "\n";

    std::cout << "\n";
}

void demo_aliasing_constructor() {
    std::cout << "=== shared_ptr aliasing constructor ===\n";

    auto person = std::make_shared<Person>("张三", 30);
    std::cout << "person use_count = " << person.use_count() << "\n";

    std::shared_ptr<std::string> name_ptr(person, &person->name);
    std::cout << "*name_ptr = " << *name_ptr << "\n";
    std::cout << "person use_count = " << person.use_count() << "\n";
    std::cout << "name_ptr use_count = " << name_ptr.use_count() << "\n";

    person.reset();
    std::cout << "person.reset() 后, name_ptr 仍有效: " << *name_ptr << "\n";
    std::cout << "name_ptr use_count = " << name_ptr.use_count() << "\n";

    std::cout << "\naliasing constructor 的用途:\n";
    std::cout << "1. 指向对象的成员, 同时保持对象存活\n";
    std::cout << "2. 引用计数与原对象共享\n";
    std::cout << "3. 即使原 shared_ptr 被重置, 成员指针仍有效\n";

    std::cout << "\n";
}

void demo_enable_shared_from_this() {
    std::cout << "=== enable_shared_from_this ===\n";

    auto conn = std::make_shared<Connection>(1);
    conn->register_callback();
    conn->trigger_callbacks();

    auto self = conn->get_self();
    std::cout << "shared_from_this() use_count = " << self.use_count() << "\n";

    std::cout << "\nenable_shared_from_this 的要点:\n";
    std::cout << "1. 在成员函数中需要 shared_ptr<this> 时使用\n";
    std::cout << "2. 绝不能在构造函数中调用 shared_from_this()\n";
    std::cout << "3. 对象必须已经被 shared_ptr 管理\n";
    std::cout << "4. 错误做法: shared_ptr<T>(this) 会导致双重删除\n";

    std::cout << "\n";
}

void demo_unique_ptr_to_shared_ptr() {
    std::cout << "=== unique_ptr 转换为 shared_ptr ===\n";

    auto unique_w = std::make_unique<Widget>(50);
    std::shared_ptr<Widget> shared_w = std::move(unique_w);

    std::cout << "转换后 unique_w.get() = " << unique_w.get() << "\n";
    std::cout << "shared_w.use_count() = " << shared_w.use_count() << "\n";
    shared_w->work();

    std::cout << "\n注意: shared_ptr 不能转为 unique_ptr\n";
    std::cout << "设计原则: 工厂函数返回 unique_ptr, 让调用者决定是否共享\n";

    std::cout << "\n";
}

int main() {
    demo_pimpl_with_unique_ptr();
    demo_factory_returning_unique_ptr();
    demo_aliasing_constructor();
    demo_enable_shared_from_this();
    demo_unique_ptr_to_shared_ptr();

    return 0;
}
