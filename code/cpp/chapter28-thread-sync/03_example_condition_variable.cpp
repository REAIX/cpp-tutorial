/** @file 03_example_condition_variable.cpp @brief 条件变量使用模式 @description 对应文档: 02-CPP/30-thread-sync
 *  编译命令: g++ -std=c++20 03_example_condition_variable.cpp -o 03_example_condition_variable
 */

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
#include <vector>
#include <atomic>

void demo_basic_cv() {
    std::cout << "\n=== 条件变量基础 ===\n";

    std::mutex mtx;
    std::condition_variable cv;
    bool ready = false;

    auto worker = [&mtx, &cv, &ready]() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&ready]() { return ready; });
        std::cout << "  工作者: 收到通知, 开始工作\n";
    };

    std::thread t(worker);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    {
        std::lock_guard<std::mutex> lock(mtx);
        ready = true;
        std::cout << "  主线程: 设置ready=true\n";
    }
    cv.notify_one();
    t.join();
    std::cout << "完成\n";
}

void demo_wait_with_predicate() {
    std::cout << "\n=== wait带谓词 ===\n";

    std::mutex mtx;
    std::condition_variable cv;
    std::queue<int> queue;
    bool done = false;

    auto consumer = [&mtx, &cv, &queue, &done]() {
        std::unique_lock<std::mutex> lock(mtx);
        while (!done || !queue.empty()) {
            cv.wait(lock, [&queue, &done]() { return !queue.empty() || done; });
            while (!queue.empty()) {
                int val = queue.front();
                queue.pop();
                std::cout << "  消费: " << val << "\n";
            }
        }
        std::cout << "  消费者结束\n";
    };

    std::thread c(consumer);

    for (int i = 1; i <= 5; ++i) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            queue.push(i);
        }
        cv.notify_one();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    {
        std::lock_guard<std::mutex> lock(mtx);
        done = true;
    }
    cv.notify_one();
    c.join();
}

void demo_notify_one_vs_all() {
    std::cout << "\n=== notify_one vs notify_all ===\n";

    std::mutex mtx;
    std::condition_variable cv;
    int event = 0;

    auto waiter = [&mtx, &cv, &event](int id) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&event]() { return event > 0; });
        std::cout << "  等待者" << id << " 被唤醒, event=" << event << "\n";
    };

    std::cout << "notify_one: 只唤醒一个等待线程\n";
    event = 0;
    std::vector<std::jthread> threads;
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(waiter, i);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    {
        std::lock_guard<std::mutex> lock(mtx);
        event = 1;
    }
    cv.notify_one();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    {
        std::lock_guard<std::mutex> lock(mtx);
        event = 2;
    }
    cv.notify_all();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "\nnotify_all: 唤醒所有等待线程\n";
    std::cout << "选择:\n";
    std::cout << "  notify_one: 只需一个线程处理(如任务队列)\n";
    std::cout << "  notify_all: 所有线程都需要响应(如状态变更)\n";
}

void demo_spurious_wakeup() {
    std::cout << "\n=== 虚假唤醒 ===\n";

    std::cout << "虚假唤醒: 条件变量可能在没有notify的情况下唤醒\n";
    std::cout << "这是允许的行为, 不是bug\n";

    std::cout << "\n错误写法(不检查条件):\n";
    std::cout << "  cv.wait(lock);  // 可能虚假唤醒\n";

    std::cout << "\n正确写法1(循环检查):\n";
    std::cout << "  while (!condition()) cv.wait(lock);\n";

    std::cout << "\n正确写法2(使用谓词):\n";
    std::cout << "  cv.wait(lock, []{ return condition(); });\n";

    std::cout << "\n举一反三:\n";
    std::cout << "  虚假唤醒的原因: 操作系统实现细节\n";
    std::cout << "  某些平台上更常见(如Linux的futex)\n";
    std::cout << "  始终使用谓词版本的wait, 最安全\n";
}

void demo_wait_for_wait_until() {
    std::cout << "\n=== 定时等待 ===\n";

    std::mutex mtx;
    std::condition_variable cv;
    bool ready = false;

    std::cout << "wait_for: 等待指定时长\n";
    {
        std::unique_lock<std::mutex> lock(mtx);
        auto result = cv.wait_for(lock, std::chrono::milliseconds(100), [&ready]() { return ready; });
        std::cout << "  wait_for(100ms)结果: " << (result ? "条件满足" : "超时") << "\n";
    }

    std::cout << "\nwait_until: 等待到指定时间点\n";
    {
        std::unique_lock<std::mutex> lock(mtx);
        auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(50);
        auto result = cv.wait_until(lock, deadline, [&ready]() { return ready; });
        std::cout << "  wait_until(50ms后)结果: " << (result ? "条件满足" : "超时") << "\n";
    }

    std::cout << "\n用途:\n";
    std::cout << "  超时控制: 避免无限等待\n";
    std::cout << "  定期检查: 周期性执行任务\n";
    std::cout << "  优雅退出: 超时后检查退出标志\n";
}

template<typename T>
class BoundedBuffer {
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable not_full_;
    std::condition_variable not_empty_;
    size_t capacity_;
public:
    explicit BoundedBuffer(size_t cap) : capacity_(cap) {}

    void put(T value) {
        std::unique_lock<std::mutex> lock(mutex_);
        not_full_.wait(lock, [this]() { return queue_.size() < capacity_; });
        queue_.push(std::move(value));
        not_empty_.notify_one();
    }

    T take() {
        std::unique_lock<std::mutex> lock(mutex_);
        not_empty_.wait(lock, [this]() { return !queue_.empty(); });
        T value = std::move(queue_.front());
        queue_.pop();
        not_full_.notify_one();
        return value;
    }
};

void demo_bounded_buffer() {
    std::cout << "\n=== 有界缓冲区 ===\n";

    BoundedBuffer<int> buffer(5);

    auto producer = [&buffer]() {
        for (int i = 1; i <= 10; ++i) {
            buffer.put(i);
            std::cout << "  生产: " << i << "\n";
        }
    };

    auto consumer = [&buffer]() {
        for (int i = 1; i <= 10; ++i) {
            int val = buffer.take();
            std::cout << "  消费: " << val << "\n";
        }
    };

    std::thread p(producer);
    std::thread c(consumer);
    p.join();
    c.join();
    std::cout << "有界缓冲区操作完成\n";
}

int main() {
    std::cout << "========== 条件变量使用模式 ==========\n";

    demo_basic_cv();
    demo_wait_with_predicate();
    demo_notify_one_vs_all();
    demo_spurious_wakeup();
    demo_wait_for_wait_until();
    demo_bounded_buffer();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
