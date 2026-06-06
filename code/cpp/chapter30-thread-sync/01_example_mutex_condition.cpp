/**
 * @file 01_example_mutex_condition.cpp
 * @brief 互斥量与条件变量示例
 * @description 对应文档: 02-CPP/30-thread-sync
 */

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
#include <vector>
#include <atomic>

std::mutex g_mutex;
int g_shared = 0;

void demo_mutex_basic() {
    std::cout << "\n=== mutex基础 ===\n";

    auto increment = [](int count) {
        for (int i = 0; i < count; ++i) {
            std::lock_guard<std::mutex> lock(g_mutex);
            ++g_shared;
        }
    };

    g_shared = 0;
    std::thread t1(increment, 100000);
    std::thread t2(increment, 100000);
    t1.join();
    t2.join();
    std::cout << "mutex保护计数: " << g_shared << " (期望200000)\n";
}

void demo_lock_guard() {
    std::cout << "\n=== lock_guard ===\n";

    std::mutex mtx;
    int counter = 0;

    auto work = [&mtx, &counter]() {
        for (int i = 0; i < 10000; ++i) {
            std::lock_guard<std::mutex> lock(mtx);
            ++counter;
        }
    };

    std::thread t1(work);
    std::thread t2(work);
    t1.join();
    t2.join();
    std::cout << "lock_guard计数: " << counter << "\n";
    std::cout << "特点: RAII, 构造加锁, 析构解锁, 不可手动解锁\n";
}

void demo_unique_lock() {
    std::cout << "\n=== unique_lock ===\n";

    std::mutex mtx;

    {
        std::unique_lock<std::mutex> lock(mtx);
        std::cout << "  手动解锁前: 拥有锁\n";
        lock.unlock();
        std::cout << "  手动解锁后: 不拥有锁\n";
        lock.lock();
        std::cout << "  重新加锁\n";
    }
    std::cout << "  离开作用域, 自动解锁\n";

    std::cout << "\nunique_lock vs lock_guard:\n";
    std::cout << "  lock_guard: 轻量, 不可手动控制\n";
    std::cout << "  unique_lock: 灵活, 可手动加/解锁, 可移动\n";
    std::cout << "  unique_lock可与condition_variable配合\n";
    std::cout << "  unique_lock支持defer_lock, try_lock等策略\n";

    {
        std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
        std::cout << "  defer_lock: 不立即加锁\n";
        lock.lock();
        std::cout << "  手动加锁成功\n";
    }
}

template<typename T>
class ThreadSafeQueue {
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;

public:
    void push(T value) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(value));
        }
        cv_.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]() { return !queue_.empty(); });
        T value = std::move(queue_.front());
        queue_.pop();
        return value;
    }

    bool try_pop(T& value, int timeout_ms) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms),
                         [this]() { return !queue_.empty(); })) {
            value = std::move(queue_.front());
            queue_.pop();
            return true;
        }
        return false;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
};

void demo_producer_consumer() {
    std::cout << "\n=== 生产者-消费者模式 ===\n";

    ThreadSafeQueue<int> queue;
    const int item_count = 10;
    std::atomic<bool> done{false};

    auto producer = [&queue, &done]() {
        for (int i = 1; i <= item_count; ++i) {
            queue.push(i);
            std::cout << "  生产: " << i << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        done = true;
        queue.push(-1);
    };

    auto consumer = [&queue]() {
        while (true) {
            int value = queue.pop();
            if (value == -1) break;
            std::cout << "  消费: " << value << "\n";
        }
    };

    std::thread p(producer);
    std::thread c(consumer);
    p.join();
    c.join();
    std::cout << "生产者-消费者完成\n";
}

int main() {
    std::cout << "========== 互斥量与条件变量示例 ==========\n";

    demo_mutex_basic();
    demo_lock_guard();
    demo_unique_lock();
    demo_producer_consumer();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
