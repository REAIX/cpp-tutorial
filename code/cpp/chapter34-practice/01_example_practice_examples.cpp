/**
 * @file 01_example_practice_examples.cpp
 * @brief 综合实战: 迷你KV存储, RAII, 智能指针, 模板, 错误处理
 * @description 对应文档: 02-CPP/38-实战案例
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <functional>
#include <thread>
#include <chrono>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cassert>

class KvException : public std::runtime_error {
public:
    explicit KvException(const std::string& msg) : std::runtime_error(msg) {}
};

class KeyNotFoundException : public KvException {
    std::string key_;
public:
    explicit KeyNotFoundException(const std::string& key)
        : KvException("Key not found: " + key), key_(key) {}
    const std::string& key() const { return key_; }
};

class DuplicateKeyException : public KvException {
public:
    explicit DuplicateKeyException(const std::string& key)
        : KvException("Duplicate key: " + key) {}
};

template<typename V>
class KvEntry {
    std::string key_;
    V value_;
    std::chrono::system_clock::time_point created_at_;
    std::chrono::system_clock::time_point updated_at_;

public:
    KvEntry(std::string key, V value)
        : key_(std::move(key)), value_(std::move(value))
        , created_at_(std::chrono::system_clock::now())
        , updated_at_(created_at_) {}

    const std::string& key() const { return key_; }
    const V& value() const { return value_; }
    void set_value(V val) { value_ = std::move(val); updated_at_ = std::chrono::system_clock::now(); }
    auto created_at() const { return created_at_; }
    auto updated_at() const { return updated_at_; }
};

template<typename V>
class KvStore {
    std::map<std::string, std::shared_ptr<KvEntry<V>>> entries_;
    mutable std::mutex mutex_;
    size_t max_size_;

public:
    explicit KvStore(size_t max_size = 10000) : max_size_(max_size) {}

    void put(const std::string& key, V value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (entries_.size() >= max_size_ && entries_.find(key) == entries_.end()) {
            throw KvException("Store is full (max=" + std::to_string(max_size_) + ")");
        }
        auto it = entries_.find(key);
        if (it != entries_.end()) {
            it->second->set_value(std::move(value));
        } else {
            entries_[key] = std::make_shared<KvEntry<V>>(key, std::move(value));
        }
    }

    std::optional<V> get(const std::string& key) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = entries_.find(key);
        if (it == entries_.end()) return std::nullopt;
        return it->second->value();
    }

    V get_or_throw(const std::string& key) const {
        auto val = get(key);
        if (!val) throw KeyNotFoundException(key);
        return *val;
    }

    bool remove(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        return entries_.erase(key) > 0;
    }

    bool contains(const std::string& key) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return entries_.find(key) != entries_.end();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return entries_.size();
    }

    std::vector<std::string> keys() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::string> result;
        for (const auto& [k, _] : entries_) {
            result.push_back(k);
        }
        return result;
    }

    std::vector<std::pair<std::string, V>> entries() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::pair<std::string, V>> result;
        for (const auto& [k, v] : entries_) {
            result.emplace_back(k, v->value());
        }
        return result;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        entries_.clear();
    }

    template<typename Predicate>
    std::vector<std::pair<std::string, V>> query(Predicate pred) const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::pair<std::string, V>> result;
        for (const auto& [k, v] : entries_) {
            if (pred(k, v->value())) {
                result.emplace_back(k, v->value());
            }
        }
        return result;
    }
};

template<typename V>
class KvStoreWithTtl {
    struct TtlEntry {
        std::shared_ptr<KvEntry<V>> entry;
        std::chrono::system_clock::time_point expires_at;
    };

    std::map<std::string, TtlEntry> entries_;
    mutable std::mutex mutex_;

    bool is_expired(const TtlEntry& e) const {
        return std::chrono::system_clock::now() > e.expires_at;
    }

    void cleanup_expired() {
        auto it = entries_.begin();
        while (it != entries_.end()) {
            if (is_expired(it->second)) {
                it = entries_.erase(it);
            } else {
                ++it;
            }
        }
    }

public:
    void put(const std::string& key, V value, int ttl_seconds = 0) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto entry = std::make_shared<KvEntry<V>>(key, std::move(value));
        auto expires_at = (ttl_seconds > 0)
            ? std::chrono::system_clock::now() + std::chrono::seconds(ttl_seconds)
            : std::chrono::system_clock::time_point::max();
        entries_[key] = {entry, expires_at};
    }

    std::optional<V> get(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        cleanup_expired();
        auto it = entries_.find(key);
        if (it == entries_.end() || is_expired(it->second)) return std::nullopt;
        return it->second.entry->value();
    }

    size_t size() {
        std::lock_guard<std::mutex> lock(mutex_);
        cleanup_expired();
        return entries_.size();
    }
};

template<typename V>
class KvSnapshot {
    std::vector<std::pair<std::string, V>> data_;
    std::chrono::system_clock::time_point timestamp_;

public:
    explicit KvSnapshot(std::vector<std::pair<std::string, V>> data)
        : data_(std::move(data)), timestamp_(std::chrono::system_clock::now()) {}

    const auto& data() const { return data_; }
    auto timestamp() const { return timestamp_; }
};

template<typename V>
class PersistentKvStore : public KvStore<V> {
    std::string filepath_;

    static std::string serialize_value(const V& val) {
        std::ostringstream oss;
        oss << val;
        return oss.str();
    }

    static V deserialize_value(const std::string& s) {
        std::istringstream iss(s);
        V val;
        iss >> val;
        return val;
    }

public:
    explicit PersistentKvStore(const std::string& filepath, size_t max_size = 10000)
        : KvStore<V>(max_size), filepath_(filepath) {}

    bool save() {
        try {
            std::ofstream file(filepath_);
            if (!file) return false;
            auto entries = this->entries();
            for (const auto& [k, v] : entries) {
                file << k << "\t" << serialize_value(v) << "\n";
            }
            return true;
        } catch (...) {
            return false;
        }
    }

    bool load() {
        try {
            std::ifstream file(filepath_);
            if (!file) return false;
            this->clear();
            std::string line;
            while (std::getline(file, line)) {
                auto pos = line.find('\t');
                if (pos != std::string::npos) {
                    std::string key = line.substr(0, pos);
                    V value = deserialize_value(line.substr(pos + 1));
                    this->put(key, value);
                }
            }
            return true;
        } catch (...) {
            return false;
        }
    }
};

void demo_basic_kv_store() {
    std::cout << "\n=== demo_basic_kv_store ===\n";
    std::cout << "基本KV存储操作\n\n";

    KvStore<std::string> store;

    store.put("name", "张三");
    store.put("city", "北京");
    store.put("job", "工程师");

    std::cout << "存储大小: " << store.size() << "\n";
    std::cout << "获取name: " << store.get("name").value_or("(未找到)") << "\n";
    std::cout << "获取city: " << store.get("city").value_or("(未找到)") << "\n";

    store.put("name", "李四");
    std::cout << "更新name: " << store.get("name").value_or("(未找到)") << "\n";

    store.remove("city");
    std::cout << "删除city后: " << (store.contains("city") ? "存在" : "不存在") << "\n";
    std::cout << "存储大小: " << store.size() << "\n";

    std::cout << "\n所有键: ";
    for (const auto& k : store.keys()) {
        std::cout << k << " ";
    }
    std::cout << "\n";
}

void demo_template_kv_store() {
    std::cout << "\n=== demo_template_kv_store ===\n";
    std::cout << "模板KV存储 - 支持不同值类型\n\n";

    KvStore<int> int_store;
    int_store.put("age", 25);
    int_store.put("score", 95);
    std::cout << "int存储: age=" << int_store.get("age").value_or(0)
              << ", score=" << int_store.get("score").value_or(0) << "\n";

    KvStore<double> double_store;
    double_store.put("pi", 3.14159);
    double_store.put("e", 2.71828);
    std::cout << "double存储: pi=" << double_store.get("pi").value_or(0.0) << "\n";
}

void demo_query_and_error_handling() {
    std::cout << "\n=== demo_query_and_error_handling ===\n";
    std::cout << "查询与错误处理\n\n";

    KvStore<int> store;
    store.put("apples", 10);
    store.put("bananas", 20);
    store.put("cherries", 5);
    store.put("dates", 30);

    auto expensive = store.query([](const std::string& key, int val) {
        return val > 15;
    });
    std::cout << "价格>15的水果:\n";
    for (const auto& [k, v] : expensive) {
        std::cout << "  " << k << ": " << v << "\n";
    }

    auto with_a = store.query([](const std::string& key, int) {
        return key.find('a') != std::string::npos;
    });
    std::cout << "名称含'a'的水果:\n";
    for (const auto& [k, v] : with_a) {
        std::cout << "  " << k << ": " << v << "\n";
    }

    std::cout << "\n错误处理:\n";
    try {
        store.get_or_throw("nonexistent");
    } catch (const KeyNotFoundException& e) {
        std::cout << "  捕获异常: " << e.what() << "\n";
        std::cout << "  缺失的键: " << e.key() << "\n";
    }

    try {
        KvStore<int> small_store(3);
        small_store.put("a", 1);
        small_store.put("b", 2);
        small_store.put("c", 3);
        small_store.put("d", 4);
    } catch (const KvException& e) {
        std::cout << "  捕获异常: " << e.what() << "\n";
    }
}

void demo_ttl_and_persistence() {
    std::cout << "\n=== demo_ttl_and_persistence ===\n";
    std::cout << "TTL过期与持久化\n\n";

    std::cout << "TTL存储:\n";
    KvStoreWithTtl<std::string> ttl_store;
    ttl_store.put("session", "abc123", 1);
    ttl_store.put("config", "default", 0);
    std::cout << "  session: " << ttl_store.get("session").value_or("(过期)") << "\n";
    std::cout << "  config: " << ttl_store.get("config").value_or("(过期)") << "\n";

    std::cout << "  等待2秒...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "  session: " << ttl_store.get("session").value_or("(已过期)") << "\n";
    std::cout << "  config: " << ttl_store.get("config").value_or("(过期)") << "\n";

    std::cout << "\n持久化存储:\n";
    PersistentKvStore<std::string> pstore("kv_data.txt");
    pstore.put("key1", "value1");
    pstore.put("key2", "value2");
    bool saved = pstore.save();
    std::cout << "  保存: " << (saved ? "成功" : "失败") << "\n";

    PersistentKvStore<std::string> pstore2("kv_data.txt");
    bool loaded = pstore2.load();
    std::cout << "  加载: " << (loaded ? "成功" : "失败") << "\n";
    std::cout << "  key1: " << pstore2.get("key1").value_or("(未找到)") << "\n";
    std::cout << "  key2: " << pstore2.get("key2").value_or("(未找到)") << "\n";

    std::cout << "\n综合案例使用的C++特性:\n";
    std::cout << "  1. RAII: mutex的lock_guard, 文件流自动关闭\n";
    std::cout << "  2. 智能指针: shared_ptr管理KvEntry生命周期\n";
    std::cout << "  3. 模板: KvStore支持不同值类型\n";
    std::cout << "  4. 异常: 自定义异常层次结构\n";
    std::cout << "  5. optional: 安全的空值表示\n";
    std::cout << "  6. lambda: 查询谓词\n";
    std::cout << "  7. chrono: 时间戳和TTL\n";
    std::cout << "  8. 结构化绑定: 遍历map\n";
}

int main() {
    std::cout << "综合实战: 迷你KV存储\n";

    demo_basic_kv_store();
    demo_template_kv_store();
    demo_query_and_error_handling();
    demo_ttl_and_persistence();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
