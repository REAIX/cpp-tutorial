/**
 * @file 03_example_double_checked_locking.cpp
 * @brief DCLP模式: 错误实现与正确实现
 * @description 对应文档: 02-CPP/32-内存模型
 */

#include <iostream>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

class BrokenSingleton {
    static BrokenSingleton* instance_;
    static std::mutex mtx_;
    int value_;

    BrokenSingleton() : value_(42) {
        std::cout << "  BrokenSingleton构造 (value_=" << value_ << ")\n";
    }

public:
    static BrokenSingleton* getInstance() {
        if (!instance_) {
            std::lock_guard<std::mutex> lock(mtx_);
            if (!instance_) {
                instance_ = new BrokenSingleton();
            }
        }
        return instance_;
    }

    int getValue() const { return value_; }

    static void reset() { instance_ = nullptr; }
};

BrokenSingleton* BrokenSingleton::instance_ = nullptr;
std::mutex BrokenSingleton::mtx_;

void demo_broken_dclp() {
    std::cout << "\n=== demo_broken_dclp ===\n";
    std::cout << "双重检查锁定模式(DCLP) - 错误实现\n\n";

    std::cout << "问题分析:\n";
    std::cout << "  new BrokenSingleton() 分三步:\n";
    std::cout << "    1. 分配内存\n";
    std::cout << "    2. 调用构造函数\n";
    std::cout << "    3. 将地址赋给instance_\n";
    std::cout << "  编译器可能重排为 1->3->2\n";
    std::cout << "  线程B可能在步骤3后看到instance_非空, 但对象未构造完成!\n\n";

    BrokenSingleton::reset();
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([i]() {
            auto* p = BrokenSingleton::getInstance();
            std::cout << "  线程" << i << ": instance=" << p
                      << ", value=" << p->getValue() << "\n";
        });
    }
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "\n在C++11之前, DCLP是未定义行为(UB)\n";
    std::cout << "因为C++11之前没有正式的内存模型\n";
}

class CorrectSingletonMutex {
    static std::atomic<CorrectSingletonMutex*> instance_;
    static std::mutex mtx_;
    int value_;

    CorrectSingletonMutex() : value_(42) {
        std::cout << "  CorrectSingletonMutex构造\n";
    }

public:
    static CorrectSingletonMutex* getInstance() {
        CorrectSingletonMutex* tmp = instance_.load(std::memory_order_acquire);
        if (!tmp) {
            std::lock_guard<std::mutex> lock(mtx_);
            tmp = instance_.load(std::memory_order_relaxed);
            if (!tmp) {
                tmp = new CorrectSingletonMutex();
                instance_.store(tmp, std::memory_order_release);
            }
        }
        return tmp;
    }

    int getValue() const { return value_; }

    static void reset() { instance_.store(nullptr, std::memory_order_release); }
};

std::atomic<CorrectSingletonMutex*> CorrectSingletonMutex::instance_{nullptr};
std::mutex CorrectSingletonMutex::mtx_;

void demo_correct_dclp_mutex() {
    std::cout << "\n=== demo_correct_dclp_mutex ===\n";
    std::cout << "DCLP正确实现 - 使用atomic + mutex\n\n";

    std::cout << "关键修复:\n";
    std::cout << "  1. instance_改为atomic<Singleton*>\n";
    std::cout << "  2. 外层检查用acquire读\n";
    std::cout << "  3. 内层用release写\n";
    std::cout << "  4. release保证构造函数的写入对acquire线程可见\n\n";

    CorrectSingletonMutex::reset();
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([i]() {
            auto* p = CorrectSingletonMutex::getInstance();
            std::cout << "  线程" << i << ": value=" << p->getValue() << "\n";
        });
    }
    for (auto& t : threads) {
        t.join();
    }
}

class SingletonMeyers {
    int value_;

    SingletonMeyers() : value_(42) {
        std::cout << "  Meyers单例构造\n";
    }

public:
    SingletonMeyers(const SingletonMeyers&) = delete;
    SingletonMeyers& operator=(const SingletonMeyers&) = delete;

    static SingletonMeyers& getInstance() {
        static SingletonMeyers instance;
        return instance;
    }

    int getValue() const { return value_; }
};

void demo_meyers_singleton() {
    std::cout << "\n=== demo_meyers_singleton ===\n";
    std::cout << "Meyers单例 (C++11推荐方式)\n\n";

    std::cout << "C++11保证: 局部static变量的初始化是线程安全的\n";
    std::cout << "编译器内部使用类似DCLP的机制, 但保证正确\n\n";

    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([i]() {
            auto& obj = SingletonMeyers::getInstance();
            std::cout << "  线程" << i << ": value=" << obj.getValue() << "\n";
        });
    }
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "\nMeyers单例优势:\n";
    std::cout << "  1. 无需手动管理atomic和mutex\n";
    std::cout << "  2. 标准保证线程安全\n";
    std::cout << "  3. 代码最简洁\n";
    std::cout << "  4. 延迟初始化\n";
}

class SingletonCallOnce {
    static std::once_flag flag_;
    static SingletonCallOnce* instance_;
    int value_;

    SingletonCallOnce() : value_(42) {
        std::cout << "  CallOnce单例构造\n";
    }

public:
    static SingletonCallOnce* getInstance() {
        std::call_once(flag_, []() {
            instance_ = new SingletonCallOnce();
        });
        return instance_;
    }

    int getValue() const { return value_; }

    static void reset() {
        instance_ = nullptr;
    }
};

std::once_flag SingletonCallOnce::flag_;
SingletonCallOnce* SingletonCallOnce::instance_ = nullptr;

void demo_call_once_singleton() {
    std::cout << "\n=== demo_call_once_singleton ===\n";
    std::cout << "std::call_once实现单例\n\n";

    std::cout << "std::call_once保证函数只执行一次, 线程安全\n";
    std::cout << "内部使用比mutex更高效的机制\n\n";

    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([i]() {
            auto* p = SingletonCallOnce::getInstance();
            std::cout << "  线程" << i << ": value=" << p->getValue() << "\n";
        });
    }
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "\nDCLP模式总结:\n";
    std::cout << "  错误DCLP: 指针检查+mutex, 无内存序保证 (C++11前是UB)\n";
    std::cout << "  正确DCLP: atomic指针 + acquire/release + mutex\n";
    std::cout << "  推荐方式1: Meyers单例 (局部static)\n";
    std::cout << "  推荐方式2: std::call_once\n";
    std::cout << "  实际开发中优先使用Meyers单例或call_once\n";
}

int main() {
    std::cout << "双重检查锁定模式(DCLP)演示\n";

    demo_broken_dclp();
    demo_correct_dclp_mutex();
    demo_meyers_singleton();
    demo_call_once_singleton();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
