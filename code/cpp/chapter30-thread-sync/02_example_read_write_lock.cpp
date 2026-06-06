/** @file 02_example_read_write_lock.cpp @brief 读写锁示例 @description 对应文档: 02-CPP/30-thread-sync
 *  编译命令: g++ -std=c++20 02_example_read_write_lock.cpp -o 02_example_read_write_lock
 */

#include <iostream>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <string>
#include <map>
#include <chrono>

class ThreadSafeCache {
    std::map<std::string, std::string> data_;
    mutable std::shared_mutex mutex_;

public:
    void set(const std::string& key, const std::string& value) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        data_[key] = value;
        std::cout << "  写入: " << key << " = " << value << "\n";
    }

    std::string get(const std::string& key) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto it = data_.find(key);
        std::string result = (it != data_.end()) ? it->second : "(未找到)";
        std::cout << "  读取: " << key << " = " << result << "\n";
        return result;
    }

    bool contains(const std::string& key) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return data_.count(key) > 0;
    }

    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return data_.size();
    }

    void remove(const std::string& key) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        data_.erase(key);
        std::cout << "  删除: " << key << "\n";
    }
};

void demo_shared_mutex() {
    std::cout << "\n=== shared_mutex (C++17) ===\n";

    std::cout << "shared_mutex提供两种锁模式:\n";
    std::cout << "  共享锁(shared_lock): 多个读者可同时持有\n";
    std::cout << "  独占锁(unique_lock): 只有一个写者可持有\n";

    std::cout << "\n适用场景:\n";
    std::cout << "  读多写少的数据结构\n";
    std::cout << "  缓存系统\n";
    std::cout << "  配置管理\n";
}

void demo_read_write_pattern() {
    std::cout << "\n=== 读写模式 ===\n";

    ThreadSafeCache cache;

    cache.set("host", "localhost");
    cache.set("port", "8080");
    cache.set("debug", "true");

    std::vector<std::jthread> readers;
    for (int i = 0; i < 3; ++i) {
        readers.emplace_back([&cache, i]() {
            for (int j = 0; j < 3; ++j) {
                std::string key = (j == 0) ? "host" : (j == 1) ? "port" : "debug";
                cache.get(key);
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        });
    }
    readers.clear();

    std::cout << "\n更新配置:\n";
    cache.set("port", "9090");
    cache.get("port");
}

void demo_shared_lock() {
    std::cout << "\n=== shared_lock用法 ===\n";

    std::shared_mutex rw_mtx;
    int shared_data = 0;

    auto reader = [&rw_mtx, &shared_data](int id) {
        std::shared_lock<std::shared_mutex> lock(rw_mtx);
        std::cout << "  读者" << id << ": data=" << shared_data << "\n";
    };

    auto writer = [&rw_mtx, &shared_data](int value) {
        std::unique_lock<std::shared_mutex> lock(rw_mtx);
        shared_data = value;
        std::cout << "  写者: data=" << shared_data << "\n";
    };

    std::vector<std::jthread> threads;
    threads.emplace_back(writer, 42);
    threads.emplace_back(reader, 1);
    threads.emplace_back(reader, 2);
    threads.emplace_back(reader, 3);
    threads.clear();

    std::cout << "\nshared_lock特点:\n";
    std::cout << "  1. 多个shared_lock可以同时持有\n";
    std::cout << "  2. 与unique_lock互斥\n";
    std::cout << "  3. 写者等待所有读者释放后才能获取独占锁\n";
    std::cout << "  4. 读者等待写者释放后才能获取共享锁\n";
}

void demo_upgrade_lock() {
    std::cout << "\n=== 锁升级 ===\n";

    std::shared_mutex rw_mtx;
    std::map<std::string, int> data = {{"a", 1}, {"b", 2}};

    auto read_or_create = [&rw_mtx, &data](const std::string& key, int default_val) {
        {
            std::shared_lock<std::shared_mutex> read_lock(rw_mtx);
            auto it = data.find(key);
            if (it != data.end()) {
                std::cout << "  读取: " << key << " = " << it->second << "\n";
                return it->second;
            }
        }

        {
            std::unique_lock<std::shared_mutex> write_lock(rw_mtx);
            auto it = data.find(key);
            if (it != data.end()) {
                std::cout << "  二次检查: " << key << " = " << it->second << "\n";
                return it->second;
            }
            data[key] = default_val;
            std::cout << "  创建: " << key << " = " << default_val << "\n";
            return default_val;
        }
    };

    read_or_create("a", 0);
    read_or_create("c", 3);

    std::cout << "\n注意: shared_mutex不支持直接升级(shared->unique)\n";
    std::cout << "需要先释放shared_lock, 再获取unique_lock\n";
    std::cout << "中间可能有其他写者修改, 需要二次检查\n";
}

int main() {
    std::cout << "========== 读写锁示例 ==========\n";

    demo_shared_mutex();
    demo_read_write_pattern();
    demo_shared_lock();
    demo_upgrade_lock();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
