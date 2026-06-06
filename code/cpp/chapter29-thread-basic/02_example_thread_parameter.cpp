/**
 * @file 02_example_thread_parameter.cpp
 * @brief 线程参数传递示例
 * @description 对应文档: 02-CPP/29-thread-basic
 *  @note C 语言中使用 pthread API 实现类似功能, 参见 C 章节 24-进程与线程
 */

#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <memory>
#include <functional>

void pass_by_value(int x, std::string s) {
    std::cout << "  按值传递: x=" << x << " s=" << s
              << " (线程" << std::this_thread::get_id() << ")\n";
    x = 999;
    s = "modified";
}

void pass_by_ref(int& x, std::string& s) {
    std::cout << "  按引用传递: x=" << x << " s=" << s << "\n";
    x = 100;
    s = "modified_by_ref";
}

void pass_by_const_ref(const int& x, const std::string& s) {
    std::cout << "  按const引用: x=" << x << " s=" << s << "\n";
}

void pass_by_move(std::unique_ptr<int> ptr, std::string s) {
    if (ptr) {
        std::cout << "  移动传递: *ptr=" << *ptr << " s=" << s << "\n";
    }
}

class Worker {
    std::string name_;
public:
    explicit Worker(std::string name) : name_(std::move(name)) {}

    void operator()() {
        std::cout << "  Worker " << name_ << " 运行中 (线程"
                  << std::this_thread::get_id() << ")\n";
    }

    void process(int task_id) {
        std::cout << "  Worker " << name_ << " 处理任务" << task_id << "\n";
    }
};

void demo_pass_by_value() {
    std::cout << "\n=== 按值传递 ===\n";

    int x = 42;
    std::string s = "hello";

    std::thread t(pass_by_value, x, s);
    t.join();

    std::cout << "主线程: x=" << x << " s=" << s << "\n";
    std::cout << "注意: 子线程的修改不影响主线程(值拷贝)\n";
}

void demo_pass_by_ref() {
    std::cout << "\n=== 按引用传递(std::ref) ===\n";

    int x = 42;
    std::string s = "hello";

    std::thread t(pass_by_ref, std::ref(x), std::ref(s));
    t.join();

    std::cout << "主线程: x=" << x << " s=" << s << "\n";
    std::cout << "注意: 使用std::ref()才能按引用传递\n";

    std::cout << "\nconst引用:\n";
    std::thread t2(pass_by_const_ref, std::cref(x), std::cref(s));
    t2.join();
}

void demo_pass_by_move() {
    std::cout << "\n=== 移动传递 ===\n";

    auto ptr = std::make_unique<int>(42);
    std::string s = "movable";

    std::thread t(pass_by_move, std::move(ptr), std::move(s));
    t.join();

    std::cout << "移动后: ptr=" << (ptr ? std::to_string(*ptr) : "null")
              << " s=\"" << s << "\"\n";
    std::cout << "注意: 移动后原对象变为空/默认状态\n";
}

void demo_callable_objects() {
    std::cout << "\n=== 可调用对象 ===\n";

    std::cout << "1. 函数对象(重载operator()):\n";
    Worker w("Alice");
    std::thread t1(w);
    t1.join();

    std::cout << "\n2. 成员函数:\n";
    Worker w2("Bob");
    std::thread t2(&Worker::process, &w2, 42);
    t2.join();

    std::cout << "\n3. Lambda:\n";
    int capture_val = 100;
    std::thread t3([capture_val]() {
        std::cout << "  Lambda捕获: " << capture_val << "\n";
    });
    t3.join();

    std::cout << "\n4. Lambda引用捕获:\n";
    int result = 0;
    std::thread t4([&result]() {
        result = 42;
        std::cout << "  Lambda设置result=" << result << "\n";
    });
    t4.join();
    std::cout << "主线程result=" << result << "\n";

    std::cout << "\n5. Lambda移动捕获:\n";
    auto sp = std::make_shared<int>(99);
    std::thread t5([sp = std::move(sp)]() {
        std::cout << "  Lambda移动捕获: " << *sp << "\n";
    });
    t5.join();
}

void demo_parameter_pitfalls() {
    std::cout << "\n=== 参数传递陷阱 ===\n";

    std::cout << "陷阱1: 忘记std::ref\n";
    std::cout << "  std::thread t(func, x);  // 按值传递, 不是引用!\n";
    std::cout << "  std::thread t(func, std::ref(x));  // 按引用传递\n";

    std::cout << "\n陷阱2: 悬垂引用\n";
    std::cout << "  int& ref = local_var;\n";
    std::cout << "  std::thread t([&ref]() { use(ref); });\n";
    std::cout << "  // 如果local_var先于线程销毁, ref悬垂!\n";

    std::cout << "\n陷阱3: 指针失效\n";
    std::cout << "  std::thread t(func, ptr);\n";
    std::cout << "  // 如果ptr指向的内存在线程使用前释放\n";

    std::cout << "\n陷阱4: 字符串字面量\n";
    std::cout << "  std::thread t(func, \"hello\");  // 可能绑定到const char*\n";
    std::cout << "  // 如果func期望std::string, 可能出问题\n";
    std::cout << "  std::thread t(func, std::string(\"hello\"));  // 安全\n";

    std::cout << "\n最佳实践:\n";
    std::cout << "  1. 优先按值传递(安全)\n";
    std::cout << "  2. 需要引用时用std::ref, 确保生命周期\n";
    std::cout << "  3. 独占资源用std::move\n";
    std::cout << "  4. 共享资源用std::shared_ptr\n";
}

int main() {
    std::cout << "========== 线程参数传递示例 ==========\n";

    demo_pass_by_value();
    demo_pass_by_ref();
    demo_pass_by_move();
    demo_callable_objects();
    demo_parameter_pitfalls();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
