# 什么是读写锁 Read-Write Lock
> 📖 相关章节：[多线程基础](../../02-CPP/27-多线程基础.md)、[线程同步](../../02-CPP/28-线程同步.md)、[原子操作](../../02-CPP/29-原子操作与异步编程.md)、[无锁编程](../../02-CPP/33-无锁编程.md)

> 读写锁允许多个读者同时进入，但写者独占——读多写少场景的并发利器。

***

### 1. 核心提炼

读写锁（Read-Write Lock）是一种区分读操作和写操作的同步原语：多个线程可以同时持有读锁（共享访问），但写锁独占（排他访问），适用于读多写少的场景以提升并发度。

***

### 2. 共享读与排他写

读写锁的核心思想是区分两种访问模式：

```
读锁（共享锁 / Shared Lock）:
  线程A ──R──→ ┌────────────────┐
  线程B ──R──→ │  同时读取数据   │  ✅ 允许并发
  线程C ──R──→ │  互不干扰       │
               └────────────────┘

写锁（排他锁 / Exclusive Lock）:
  线程A ──W──→ ┌────────────────┐
  线程B ──R──→ │  等待...        │  ❌ 必须等写者释放
  线程C ──W──→ │  等待...        │
               └────────────────┘
```

| 状态 | 新请求读锁 | 新请求写锁 |
|------|-----------|-----------|
| 无锁 | ✅ 获取 | ✅ 获取 |
| 已有读锁 | ✅ 获取（读可共享） | ❌ 等待 |
| 已有写锁 | ❌ 等待 | ❌ 等待 |

```cpp
#include <iostream>
#include <shared_mutex>
#include <thread>
#include <vector>
#include <map>
#include <string>

template<typename K, typename V>
class ThreadSafeMap {
    mutable std::shared_mutex rw_lock_;
    std::map<K, V> data_;

public:
    V get(const K& key) const {
        std::shared_lock<std::shared_mutex> read_lock(rw_lock_);
        auto it = data_.find(key);
        if (it != data_.end()) {
            return it->second;
        }
        return V{};
    }

    void set(const K& key, const V& value) {
        std::unique_lock<std::shared_mutex> write_lock(rw_lock_);
        data_[key] = value;
    }

    size_t size() const {
        std::shared_lock<std::shared_mutex> read_lock(rw_lock_);
        return data_.size();
    }
};

int main() {
    ThreadSafeMap<std::string, int> cache;

    auto reader = [&](int id) {
        for (int i = 0; i < 1000; ++i) {
            int val = cache.get("key");
            (void)val;
        }
        std::cout << "读者 " << id << " 完成\n";
    };

    auto writer = [&](int id) {
        for (int i = 0; i < 100; ++i) {
            cache.set("key", id * 100 + i);
        }
        std::cout << "写者 " << id << " 完成\n";
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i) {
        threads.emplace_back(reader, i);
    }
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back(writer, i);
    }
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "最终 map 大小: " << cache.size() << "\n";
    return 0;
}
```

> 读锁之间不互斥，多个读者可以并发访问——这是读写锁相比互斥锁的核心优势。

***

### 3. C++17 shared_mutex

C++17 引入了 `std::shared_mutex`，是标准库提供的读写锁原语。C++14 已有 `std::shared_timed_mutex`，C++17 的 `shared_mutex` 去掉了定时功能，性能更优。

| 类 | C++ 版本 | 特性 |
|----|---------|------|
| `std::shared_mutex` | C++17 | 基本读写锁，不可定时 |
| `std::shared_timed_mutex` | C++14 | 支持超时的读写锁 |

```cpp
#include <iostream>
#include <shared_mutex>
#include <thread>
#include <vector>
#include <chrono>

class RWProtectedData {
    mutable std::shared_mutex rw_lock_;
    int value_ = 0;

public:
    int read() const {
        std::shared_lock<std::shared_mutex> lock(rw_lock_);
        return value_;
    }

    void write(int new_val) {
        std::unique_lock<std::shared_mutex> lock(rw_lock_);
        value_ = new_val;
    }

    void increment() {
        std::unique_lock<std::shared_mutex> lock(rw_lock_);
        ++value_;
    }
};

int main() {
    RWProtectedData data;

    auto reader = [&](int id) {
        for (int i = 0; i < 10000; ++i) {
            int val = data.read();
            (void)val;
        }
    };

    auto writer = [&](int id) {
        for (int i = 0; i < 1000; ++i) {
            data.increment();
        }
    };

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i) {
        threads.emplace_back(reader, i);
    }
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back(writer, i);
    }
    for (auto& t : threads) {
        t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "最终值: " << data.read() << " (期望 2000)\n";
    std::cout << "耗时: " << ms << " ms\n";
    return 0;
}
```

> `shared_mutex` 在 Linux 上通常基于 `pthread_rwlock_t` 实现，Windows 上基于 SRWLock 实现。

***

### 4. shared_lock 与 unique_lock

`std::shared_lock` 和 `std::unique_lock` 是 RAII 包装器，分别管理读锁和写锁的生命周期。

| 锁类型 | RAII 包装 | 对应操作 | 互斥关系 |
|--------|----------|---------|---------|
| 读锁 | `std::shared_lock` | `lock_shared()` / `unlock_shared()` | 与读锁兼容，与写锁互斥 |
| 写锁 | `std::unique_lock` | `lock()` / `unlock()` | 与读锁、写锁均互斥 |

```cpp
#include <iostream>
#include <shared_mutex>
#include <vector>
#include <thread>

class DataStore {
    mutable std::shared_mutex rw_;
    std::vector<int> data_;

public:
    void append(int val) {
        std::unique_lock<std::shared_mutex> write_lock(rw_);
        data_.push_back(val);
    }

    void batch_append(const std::vector<int>& vals) {
        std::unique_lock<std::shared_mutex> write_lock(rw_);
        for (int v : vals) {
            data_.push_back(v);
        }
    }

    int get(size_t index) const {
        std::shared_lock<std::shared_mutex> read_lock(rw_);
        if (index < data_.size()) {
            return data_[index];
        }
        return -1;
    }

    size_t size() const {
        std::shared_lock<std::shared_mutex> read_lock(rw_);
        return data_.size();
    }

    std::vector<int> snapshot() const {
        std::shared_lock<std::shared_mutex> read_lock(rw_);
        return data_;
    }
};

int main() {
    DataStore store;

    store.append(1);
    store.append(2);
    store.batch_append({3, 4, 5});

    std::cout << "大小: " << store.size() << "\n";
    std::cout << "元素[2]: " << store.get(2) << "\n";

    auto snap = store.snapshot();
    std::cout << "快照: ";
    for (int v : snap) {
        std::cout << v << " ";
    }
    std::cout << "\n";

    return 0;
}
```

**defer_lock 延迟加锁**：

```cpp
#include <shared_mutex>
#include <iostream>

int main() {
    std::shared_mutex rw;

    {
        std::shared_lock<std::shared_mutex> read_lock(rw, std::defer_lock);
        read_lock.lock();
        std::cout << "读锁已获取\n";
    }

    {
        std::unique_lock<std::shared_mutex> write_lock(rw, std::defer_lock);
        write_lock.lock();
        std::cout << "写锁已获取\n";
    }

    return 0;
}
```

> `shared_lock` 支持 `try_lock`、`try_lock_for`、`try_lock_until`（需配合 `shared_timed_mutex`）。

***

### 5. 写者饥饿问题

许多读写锁实现（包括 POSIX 默认的 `pthread_rwlock_t`）偏向读者：只要还有读者持有读锁，新来的读者可以继续获取读锁，写者可能无限等待——这就是写者饥饿。

```
时间线:
T1: 读者A 获取读锁
T2: 读者B 获取读锁      ← 写者W 尝试获取写锁，被阻塞
T3: 读者C 获取读锁      ← 新读者可以继续进入！写者仍在等
T4: 读者D 获取读锁      ← 写者继续等...
T5: 读者E 获取读锁      ← 写者可能永远等下去...
...
```

| 策略 | 读者优先 | 写者优先 | 公平 |
|------|---------|---------|------|
| 新读者能否在写者等待时进入 | ✅ | ❌ | 看情况 |
| 写者是否可能饥饿 | ⚠️ 可能 | 不会 | 不会 |
| 读者是否可能饥饿 | 不会 | ⚠️ 可能 | 不会 |
| 吞吐量 | 读多时最高 | 写多时较高 | 折中 |

```cpp
#include <iostream>
#include <shared_mutex>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>

class FairRWLock {
    std::shared_mutex rw_;
    std::atomic<int> waiting_writers_{0};

public:
    void read_lock() {
        while (waiting_writers_.load(std::memory_order_acquire) > 0) {
            std::this_thread::yield();
        }
        rw_.lock_shared();
    }

    void read_unlock() {
        rw_.unlock_shared();
    }

    void write_lock() {
        waiting_writers_.fetch_add(1, std::memory_order_acq_rel);
        rw_.lock();
    }

    void write_unlock() {
        rw_.unlock();
        waiting_writers_.fetch_sub(1, std::memory_order_acq_rel);
    }
};

int main() {
    FairRWLock lock;
    int data = 0;
    std::atomic<int> read_count{0};
    std::atomic<int> write_count{0};

    auto reader = [&]() {
        for (int i = 0; i < 10000; ++i) {
            lock.read_lock();
            int val = data;
            (void)val;
            read_count.fetch_add(1, std::memory_order_relaxed);
            lock.read_unlock();
        }
    };

    auto writer = [&]() {
        for (int i = 0; i < 100; ++i) {
            lock.write_lock();
            ++data;
            write_count.fetch_add(1, std::memory_order_relaxed);
            lock.write_unlock();
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i) {
        threads.emplace_back(reader);
    }
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back(writer);
    }
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "读操作: " << read_count.load() << "\n";
    std::cout << "写操作: " << write_count.load() << "\n";
    std::cout << "数据值: " << data << " (期望 200)\n";
    return 0;
}
```

> POSIX `pthread_rwlock_t` 可通过 `pthread_rwlockattr_setkind_np` 设置为写者优先：`PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP`。

***

### 6. 可升级锁（Upgradeable Lock）

有时线程需要先读数据，根据条件决定是否升级为写锁。直接释放读锁再获取写锁存在窗口期，数据可能被其他线程修改。

```
升级过程:
1. 获取读锁 → 读取数据
2. 发现需要修改 → 请求升级为写锁
3. 升级成功 → 修改数据
4. 释放写锁

问题: 如果两个线程同时请求升级 → 死锁!
```

```cpp
#include <iostream>
#include <shared_mutex>
#include <thread>

class UpgradableRWLock {
    std::shared_mutex rw_;
    std::mutex upgrade_mutex_;

public:
    void read_lock() {
        rw_.lock_shared();
    }

    void read_unlock() {
        rw_.unlock_shared();
    }

    void write_lock() {
        rw_.lock();
    }

    void write_unlock() {
        rw_.unlock();
    }

    bool try_upgrade() {
        if (!upgrade_mutex_.try_lock()) {
            return false;
        }

        rw_.unlock_shared();
        rw_.lock();
        upgrade_mutex_.unlock();
        return true;
    }

    void downgrade() {
        rw_.unlock();
        rw_.lock_shared();
    }
};

int main() {
    UpgradableRWLock lock;
    int cache = 0;
    bool valid = false;

    auto read_or_update = [&](int expected) {
        lock.read_lock();
        if (!valid || cache != expected) {
            if (lock.try_upgrade()) {
                cache = expected;
                valid = true;
                std::cout << "升级成功，写入: " << expected << "\n";
                lock.downgrade();
            } else {
                lock.read_unlock();
                lock.write_lock();
                cache = expected;
                valid = true;
                std::cout << "重新获取写锁，写入: " << expected << "\n";
                lock.write_unlock();
                return;
            }
        }
        lock.read_unlock();
    };

    read_or_update(42);
    read_or_update(42);
    return 0;
}
```

| 方案 | 安全性 | 性能 | 复杂度 |
|------|--------|------|--------|
| 释放读锁→获取写锁 | ❌ 有窗口期 | 高 | 低 |
| 直接获取写锁（不先读） | ✅ | 低（不必要排他） | 低 |
| 可升级锁 | ✅ | 中 | 高 |
| 乐观读（类似 StampedLock） | ✅ | 最高 | 高 |

> C++ 标准库没有原生的可升级锁支持。Boost.Thread 提供了 `upgrade_lock` 和 `upgrade_to_unique_lock`。

***

### 7. pthread_rwlock 详解

POSIX 线程库提供 `pthread_rwlock_t`，是 Linux/macOS 下读写锁的底层实现。

```cpp
#include <iostream>
#include <pthread.h>
#include <vector>
#include <thread>
#include <cstring>

class PThreadRWLock {
    pthread_rwlock_t lock_;

public:
    PThreadRWLock() {
        pthread_rwlock_init(&lock_, nullptr);
    }

    ~PThreadRWLock() {
        pthread_rwlock_destroy(&lock_);
    }

    PThreadRWLock(const PThreadRWLock&) = delete;
    PThreadRWLock& operator=(const PThreadRWLock&) = delete;

    void read_lock() {
        pthread_rwlock_rdlock(&lock_);
    }

    void read_unlock() {
        pthread_rwlock_unlock(&lock_);
    }

    void write_lock() {
        pthread_rwlock_wrlock(&lock_);
    }

    void write_unlock() {
        pthread_rwlock_unlock(&lock_);
    }

    bool try_read_lock() {
        return pthread_rwlock_tryrdlock(&lock_) == 0;
    }

    bool try_write_lock() {
        return pthread_rwlock_trywrlock(&lock_) == 0;
    }
};

int main() {
    PThreadRWLock rw;
    int data = 0;

    auto reader = [&](int id) {
        for (int i = 0; i < 10000; ++i) {
            rw.read_lock();
            int val = data;
            (void)val;
            rw.read_unlock();
        }
        std::cout << "读者 " << id << " 完成\n";
    };

    auto writer = [&](int id) {
        for (int i = 0; i < 100; ++i) {
            rw.write_lock();
            ++data;
            rw.write_unlock();
        }
        std::cout << "写者 " << id << " 完成\n";
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(reader, i);
    }
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back(writer, i);
    }
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "最终值: " << data << " (期望 200)\n";
    return 0;
}
```

**Linux 下设置写者优先**：

```cpp
#include <pthread.h>

void set_writer_preferred(pthread_rwlock_t& rw) {
    pthread_rwlockattr_t attr;
    pthread_rwlockattr_init(&attr);
    pthread_rwlockattr_setkind_np(&attr,
        PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
    pthread_rwlock_init(&rw, &attr);
    pthread_rwlockattr_destroy(&attr);
}
```

| pthread_rwlockattr 策略 | 常量 | 说明 |
|------------------------|------|------|
| 读者优先（默认） | `PTHREAD_RWLOCK_PREFER_READER_NP` | 新读者可插队 |
| 写者优先 | `PTHREAD_RWLOCK_PREFER_WRITER_NP` | 写者优先但可能递归 |
| 写者优先（非递归） | `PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP` | 推荐设置 |

> `_NP` 后缀表示 Non-Portable，仅 Linux/glibc 支持。macOS 和其他 POSIX 实现不一定有此选项。

***

### 8. 性能对比

不同锁方案在读写比例不同场景下的性能差异显著。

```cpp
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>

constexpr int READ_ITERS = 1000000;
constexpr int WRITE_ITERS = 10000;
constexpr int NUM_READERS = 8;
constexpr int NUM_WRITERS = 2;

template<typename LockType>
void benchmark_mutex(const char* name) {
    LockType mtx;
    int data = 0;

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;

    for (int i = 0; i < NUM_READERS; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < READ_ITERS; ++j) {
                std::lock_guard<LockType> lock(mtx);
                (void)data;
            }
        });
    }

    for (int i = 0; i < NUM_WRITERS; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < WRITE_ITERS; ++j) {
                std::lock_guard<LockType> lock(mtx);
                ++data;
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << name << ": " << ms << " ms, data=" << data << "\n";
}

void benchmark_rwmutex(const char* name) {
    std::shared_mutex rw;
    int data = 0;

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;

    for (int i = 0; i < NUM_READERS; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < READ_ITERS; ++j) {
                std::shared_lock<std::shared_mutex> lock(rw);
                (void)data;
            }
        });
    }

    for (int i = 0; i < NUM_WRITERS; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < WRITE_ITERS; ++j) {
                std::unique_lock<std::shared_mutex> lock(rw);
                ++data;
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << name << ": " << ms << " ms, data=" << data << "\n";
}

int main() {
    std::cout << "读:写 = " << NUM_READERS * READ_ITERS << ":" << NUM_WRITERS * WRITE_ITERS << "\n\n";
    benchmark_mutex<std::mutex>("std::mutex       ");
    benchmark_rwmutex(         "std::shared_mutex ");
    return 0;
}
```

**典型性能对比（读多写少，8 读 2 写）**：

| 锁类型 | 相对耗时 | 说明 |
|--------|---------|------|
| `std::mutex` | 1.0x (基准) | 所有操作互斥，读不能并发 |
| `std::shared_mutex` | 0.3~0.6x | 读可并发，吞吐量更高 |
| 自旋读写锁 | 0.2~0.5x | 临界区极短时更快 |
| RCU | 0.05~0.1x | 读零开销，但实现复杂 |

> 读写锁的优势随读比例增加而增大。当读写比接近 1:1 时，读写锁反而比互斥锁慢（额外开销大于并发收益）。

***

### 9. 实际应用示例

**场景一：配置中心**

```cpp
#include <iostream>
#include <shared_mutex>
#include <string>
#include <map>
#include <thread>
#include <vector>

class ConfigCenter {
    mutable std::shared_mutex rw_;
    std::map<std::string, std::string> config_;

public:
    std::string get(const std::string& key) const {
        std::shared_lock<std::shared_mutex> lock(rw_);
        auto it = config_.find(key);
        return (it != config_.end()) ? it->second : "";
    }

    void set(const std::string& key, const std::string& value) {
        std::unique_lock<std::shared_mutex> lock(rw_);
        config_[key] = value;
    }

    void reload(const std::map<std::string, std::string>& new_config) {
        std::unique_lock<std::shared_mutex> lock(rw_);
        config_ = new_config;
    }

    std::map<std::string, std::string> snapshot() const {
        std::shared_lock<std::shared_mutex> lock(rw_);
        return config_;
    }
};

int main() {
    ConfigCenter cfg;

    cfg.set("host", "127.0.0.1");
    cfg.set("port", "8080");

    auto reader = [&](int id) {
        for (int i = 0; i < 10000; ++i) {
            std::string host = cfg.get("host");
            (void)host;
        }
    };

    auto updater = [&]() {
        std::map<std::string, std::string> new_cfg;
        new_cfg["host"] = "192.168.1.1";
        new_cfg["port"] = "9090";
        cfg.reload(new_cfg);
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i) {
        threads.emplace_back(reader, i);
    }
    threads.emplace_back(updater);

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "host=" << cfg.get("host") << "\n";
    std::cout << "port=" << cfg.get("port") << "\n";
    return 0;
}
```

**场景二：带缓存的查询服务**

```cpp
#include <iostream>
#include <shared_mutex>
#include <unordered_map>
#include <string>
#include <optional>
#include <thread>
#include <vector>

template<typename K, typename V>
class Cache {
    mutable std::shared_mutex rw_;
    std::unordered_map<K, V> store_;

public:
    std::optional<V> lookup(const K& key) const {
        std::shared_lock<std::shared_mutex> lock(rw_);
        auto it = store_.find(key);
        if (it != store_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    void insert(const K& key, const V& value) {
        std::unique_lock<std::shared_mutex> lock(rw_);
        store_[key] = value;
    }

    void evict(const K& key) {
        std::unique_lock<std::shared_mutex> lock(rw_);
        store_.erase(key);
    }

    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(rw_);
        return store_.size();
    }
};

int main() {
    Cache<int, std::string> cache;

    for (int i = 0; i < 100; ++i) {
        cache.insert(i, "value_" + std::to_string(i));
    }

    auto reader = [&](int id) {
        for (int i = 0; i < 10000; ++i) {
            auto val = cache.lookup(id * 10 + i % 10);
            (void)val;
        }
    };

    auto writer = [&]() {
        for (int i = 100; i < 200; ++i) {
            cache.insert(i, "new_" + std::to_string(i));
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i) {
        threads.emplace_back(reader, i);
    }
    threads.emplace_back(writer);

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "缓存大小: " << cache.size() << "\n";
    return 0;
}
```

> 配置中心、缓存、路由表等"读远多于写"的数据结构是读写锁的最佳应用场景。

***

### 10. 读写锁的局限与替代方案

| 局限 | 说明 | 替代方案 |
|------|------|---------|
| 写者饥饿 | 读者持续进入导致写者无法获取锁 | 写者优先策略 / 公平锁 |
| 不可升级 | C++ 标准不支持读锁升级为写锁 | 乐观读 / 重新获取写锁 |
| 开销较大 | 读写锁内部状态比互斥锁复杂 | 读写比低时直接用互斥锁 |
| 递归问题 | 同一线程重复获取读锁可能死锁 | 避免递归加锁 |
| 不适合写多 | 写操作频繁时退化为互斥锁 | 用互斥锁或无锁结构 |

**乐观读（StampedLock 思路）**：

```cpp
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>

class OptimisticRWLock {
    std::atomic<uint64_t> stamp_{0};

public:
    uint64_t optimistic_read() {
        uint64_t s = stamp_.load(std::memory_order_acquire);
        if (s & 1) {
            return 0;
        }
        return s;
    }

    bool validate(uint64_t stamp) {
        std::atomic_thread_fence(std::memory_order_acquire);
        return stamp_.load(std::memory_order_relaxed) == stamp;
    }

    void write_lock() {
        uint64_t expected = stamp_.load(std::memory_order_relaxed);
        while (!stamp_.compare_exchange_weak(expected, expected + 1,
                                              std::memory_order_acq_rel)) {
            if (expected & 1) {
                expected = stamp_.load(std::memory_order_acquire);
            }
        }
    }

    void write_unlock() {
        stamp_.fetch_add(1, std::memory_order_release);
    }
};

int main() {
    OptimisticRWLock lock;
    int data = 0;

    auto optimistic_reader = [&]() {
        for (int i = 0; i < 100000; ++i) {
            uint64_t stamp;
            int val;
            do {
                stamp = lock.optimistic_read();
                val = data;
            } while (!lock.validate(stamp));
            (void)val;
        }
    };

    auto writer = [&]() {
        for (int i = 0; i < 1000; ++i) {
            lock.write_lock();
            ++data;
            lock.write_unlock();
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i) {
        threads.emplace_back(optimistic_reader);
    }
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back(writer);
    }
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "最终值: " << data << " (期望 2000)\n";
    return 0;
}
```

| 方案 | 读性能 | 写性能 | 一致性 | 复杂度 |
|------|--------|--------|--------|--------|
| `std::mutex` | 低 | 中 | 强 | 低 |
| `std::shared_mutex` | 中 | 中 | 强 | 低 |
| 乐观读（StampedLock） | 极高 | 中 | 最终 | 高 |
| RCU | 零开销 | 低 | 最终 | 极高 |
| Copy-on-Write | 高 | 低（需复制） | 强 | 中 |

> 乐观读在读操作远多于写操作时性能极佳——读完全无锁，仅在写发生时重试。Java 的 `StampedLock` 是此模式的经典实现。

***

### 11. 极简总结

| 概念 | 核心要点 |
|------|---------|
| 共享读/排他写 | 多读者可并发，写者独占，读多写少场景提升并发度 |
| `shared_mutex` | C++17 标准读写锁，Linux 基于 pthread_rwlock，Windows 基于 SRWLock |
| `shared_lock` | RAII 读锁包装器，调用 `lock_shared()` |
| `unique_lock` | RAII 写锁包装器，调用 `lock()` |
| 写者饥饿 | 读者持续进入导致写者无法获取锁，需写者优先策略 |
| 可升级锁 | 读锁升级为写锁，C++ 标准不支持，Boost 提供 |
| `pthread_rwlock` | POSIX 读写锁，Linux 可设写者优先 |
| 性能对比 | 读写比 > 10:1 时读写锁优势明显，否则不如互斥锁 |
| 乐观读 | 读无锁 + 版本号验证，类似 Java StampedLock |
| 替代方案 | 读写比低用互斥锁，读零开销用 RCU，强一致用 COW |

**核心原则**：
- 读写比 > 10:1 时优先考虑读写锁
- 注意写者饥饿问题，生产环境建议写者优先
- C++ 标准不支持锁升级，需自行实现或用 Boost
- 临界区极短时考虑乐观读替代方案
- 不确定时用 `std::mutex`，profile 后再优化

***

### 相关阅读

- [什么是自旋锁Spin-Lock](19-什么是自旋锁Spin-Lock.md)
- [scoped-lock与lock-guard](./15-scoped-lock与lock-guard.md)
- [锁的粒度与性能](13-锁的粒度与性能.md)

***