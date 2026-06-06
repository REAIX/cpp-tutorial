/** @file 03_example_new_delete.cpp
 *  @brief new/delete, new[]/delete[], placement new, nothrow new
 *  @description 对应文档: 02-CPP/01-basics
 */

#include <iostream>
#include <string>
#include <new>
#include <memory>

// ===== 1. 基本 new/delete =====
class Widget {
public:
    Widget(int id) : id_(id) {
        std::cout << "  Widget(" << id_ << ") 构造" << std::endl;
    }
    ~Widget() {
        std::cout << "  Widget(" << id_ << ") 析构" << std::endl;
    }
    int id() const { return id_; }
private:
    int id_;
};

void demo_basic_new_delete() {
    std::cout << "===== 基本 new/delete =====" << std::endl;

    // new 做两件事: 1.分配内存 2.调用构造函数
    // delete 做两件事: 1.调用析构函数 2.释放内存
    Widget* w = new Widget(1);
    std::cout << "使用: w->id() = " << w->id() << std::endl;
    delete w;

    // 内置类型
    int* pi = new int(42);
    std::cout << "new int(42): *pi = " << *pi << std::endl;
    delete pi;

    // 默认初始化 vs 值初始化
    int* p1 = new int;      // 默认初始化: 值未定义(垃圾值)
    int* p2 = new int();    // 值初始化: 值为 0
    int* p3 = new int{};    // 列表初始化: 值为 0 (C++11)
    std::cout << "new int:   " << *p1 << " (未定义值)" << std::endl;
    std::cout << "new int(): " << *p2 << " (零初始化)" << std::endl;
    std::cout << "new int{}: " << *p3 << " (零初始化)" << std::endl;
    delete p1;
    delete p2;
    delete p3;
}

// ===== 2. new[]/delete[] =====
void demo_array_new_delete() {
    std::cout << "\n===== new[]/delete[] =====" << std::endl;

    // 动态数组
    int* arr = new int[5]{10, 20, 30, 40, 50};
    std::cout << "new int[5]: ";
    for (int i = 0; i < 5; ++i) {
        std::cout << arr[i] << " ";
    }
    std::cout << std::endl;
    delete[] arr;  // 必须用 delete[], 不能用 delete!

    // 对象数组
    std::cout << "创建对象数组:" << std::endl;
    Widget* widgets = new Widget[3]{Widget(10), Widget(20), Widget(30)};
    std::cout << "使用对象数组" << std::endl;
    delete[] widgets;

    // 常见错误: new[] 配 delete (未定义行为)
    std::cout << "\n警告: new[] 必须配 delete[], new 必须配 delete!" << std::endl;
    std::cout << "  混用是未定义行为, 可能导致内存泄漏或崩溃" << std::endl;
}

// ===== 3. placement new =====
void demo_placement_new() {
    std::cout << "\n===== placement new =====" << std::endl;

    // placement new: 在已分配的内存上构造对象
    // 不分配内存, 只调用构造函数

    // 分配原始内存
    char buffer[sizeof(Widget)];
    std::cout << "在 buffer 上使用 placement new 构造对象:" << std::endl;

    // placement new
    Widget* w = new (buffer) Widget(99);
    std::cout << "对象地址: " << w << std::endl;
    std::cout << "buffer地址: " << static_cast<void*>(buffer) << std::endl;
    std::cout << "w->id() = " << w->id() << std::endl;

    // 必须显式调用析构函数 (不能用 delete)
    w->~Widget();
    // buffer 本身是栈上的数组, 不需要 delete

    std::cout << "\nplacement new 的用途:" << std::endl;
    std::cout << "  1. 自定义内存池/分配器" << std::endl;
    std::cout << "  2. 在共享内存中构造对象" << std::endl;
    std::cout << "  3. 实现就地构造 (emplace)" << std::endl;

    std::cout << "\n注意: placement new 构造的对象必须显式析构!" << std::endl;
    std::cout << "  不能用 delete, 因为内存不是 new 分配的" << std::endl;
}

// ===== 4. nothrow new =====
void demo_nothrow_new() {
    std::cout << "\n===== nothrow new =====" << std::endl;

    // 默认 new 失败时抛出 std::bad_alloc
    // nothrow new 失败时返回 nullptr

    int* p1 = new(std::nothrow) int(42);
    if (p1) {
        std::cout << "nothrow new 成功: *p1 = " << *p1 << std::endl;
        delete p1;
    } else {
        std::cout << "nothrow new 失败: 返回 nullptr" << std::endl;
    }

    // 尝试分配极大内存
    std::cout << "尝试分配超大内存..." << std::endl;
    void* p2 = new(std::nothrow) char[1024ULL * 1024 * 1024 * 1024];  // 1TB
    if (p2) {
        std::cout << "分配成功 (不太可能)" << std::endl;
        operator delete(p2);
    } else {
        std::cout << "分配失败, 返回 nullptr (nothrow)" << std::endl;
    }

    std::cout << "\nnothrow new vs 普通 new:" << std::endl;
    std::cout << "  普通 new:    失败抛 std::bad_alloc" << std::endl;
    std::cout << "  nothrow new: 失败返回 nullptr" << std::endl;
    std::cout << "  推荐: 嵌入式/实时系统用 nothrow, 一般场景用异常" << std::endl;
}

// ===== 5. 现代 C++ 推荐: 智能指针 =====
void demo_smart_pointers() {
    std::cout << "\n===== 现代 C++ 推荐: 智能指针 =====" << std::endl;

    // unique_ptr: 独占所有权
    auto up = std::make_unique<Widget>(100);
    std::cout << "unique_ptr: up->id() = " << up->id() << std::endl;

    // shared_ptr: 共享所有权
    auto sp1 = std::make_shared<Widget>(200);
    auto sp2 = sp1;  // 引用计数 +1
    std::cout << "shared_ptr 引用计数: " << sp1.use_count() << std::endl;

    std::cout << "\n智能指针优势:" << std::endl;
    std::cout << "  - 自动释放内存, 无需手动 delete" << std::endl;
    std::cout << "  - 异常安全, 即使抛异常也能正确释放" << std::endl;
    std::cout << "  - 明确所有权语义" << std::endl;

    std::cout << "\n最佳实践:" << std::endl;
    std::cout << "  - 优先使用 make_unique/make_shared" << std::endl;
    std::cout << "  - 默认用 unique_ptr, 需要共享时用 shared_ptr" << std::endl;
    std::cout << "  - 避免裸 new/delete" << std::endl;
}

int main() {
    std::cout << "========== new/delete 与动态内存 ==========\n" << std::endl;

    demo_basic_new_delete();
    demo_array_new_delete();
    demo_placement_new();
    demo_nothrow_new();
    demo_smart_pointers();

    return 0;
}
