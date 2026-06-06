/**
 * @file 02_deep_dive_career_growth.cpp
 * @brief C++职业成长: 面试准备, 开源贡献, 持续学习策略
 * @description 对应文档: 02-CPP/38-实战案例
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <algorithm>
#include <chrono>
#include <sstream>
#include <functional>
#include <memory>
#include <cassert>

template<typename K, typename V>
class LRUCache {
    struct Node {
        K key;
        V value;
        Node* prev = nullptr;
        Node* next = nullptr;
    };

    size_t capacity_;
    std::map<K, Node*> cache_;
    Node* head_ = nullptr;
    Node* tail_ = nullptr;

    void move_to_front(Node* node) {
        if (node == head_) return;
        if (node->prev) node->prev->next = node->next;
        if (node->next) node->next->prev = node->prev;
        if (node == tail_) tail_ = node->prev;
        node->next = head_;
        node->prev = nullptr;
        if (head_) head_->prev = node;
        head_ = node;
    }

    void remove_tail() {
        if (!tail_) return;
        cache_.erase(tail_->key);
        Node* old = tail_;
        tail_ = tail_->prev;
        if (tail_) tail_->next = nullptr;
        else head_ = nullptr;
        delete old;
    }

public:
    explicit LRUCache(size_t cap) : capacity_(cap) {}

    ~LRUCache() {
        while (head_) {
            Node* next = head_->next;
            delete head_;
            head_ = next;
        }
    }

    std::optional<V> get(const K& key) {
        auto it = cache_.find(key);
        if (it == cache_.end()) return std::nullopt;
        move_to_front(it->second);
        return it->second->value;
    }

    void put(const K& key, const V& value) {
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            it->second->value = value;
            move_to_front(it->second);
            return;
        }
        if (cache_.size() >= capacity_) remove_tail();
        Node* node = new Node{key, value, nullptr, head_};
        if (head_) head_->prev = node;
        head_ = node;
        if (!tail_) tail_ = node;
        cache_[key] = node;
    }

    size_t size() const { return cache_.size(); }
};

void demo_interview_preparation() {
    std::cout << "\n=== demo_interview_preparation ===\n";
    std::cout << "C++面试准备\n\n";

    std::cout << "1. 核心知识体系:\n\n";

    std::cout << "   基础篇:\n";
    std::cout << "   - 指针与引用的区别\n";
    std::cout << "   - 堆与栈的区别\n";
    std::cout << "   - 虚函数机制 (vtable, vptr)\n";
    std::cout << "   - 构造/析构顺序\n";
    std::cout << "   - 拷贝构造与赋值运算符\n\n";

    std::cout << "   进阶篇:\n";
    std::cout << "   - 移动语义与完美转发\n";
    std::cout << "   - 智能指针 (unique_ptr, shared_ptr, weak_ptr)\n";
    std::cout << "   - RAII原则\n";
    std::cout << "   - 模板与SFINAE\n";
    std::cout << "   - 右值引用\n\n";

    std::cout << "   高级篇:\n";
    std::cout << "   - 内存模型与原子操作\n";
    std::cout << "   - 无锁编程\n";
    std::cout << "   - 线程池设计\n";
    std::cout << "   - 模板元编程\n";
    std::cout << "   - C++20/23新特性\n\n";

    std::cout << "2. 常见面试题演示:\n\n";

    std::cout << "   Q: 实现线程安全的单例模式?\n";
    std::cout << "   A: Meyers单例 (C++11保证线程安全)\n";
    class Singleton {
    public:
        static Singleton& instance() {
            static Singleton inst;
            return inst;
        }
        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;
    private:
        Singleton() = default;
    };
    auto& s1 = Singleton::instance();
    auto& s2 = Singleton::instance();
    std::cout << "   单例验证: " << (&s1 == &s2 ? "同一实例" : "不同实例") << "\n\n";

    std::cout << "   Q: unique_ptr如何实现不可拷贝?\n";
    std::cout << "   A: 删除拷贝构造和拷贝赋值, 只保留移动\n";
    auto ptr = std::make_unique<int>(42);
    std::cout << "   unique_ptr值: " << *ptr << "\n";
    auto ptr2 = std::move(ptr);
    std::cout << "   移动后ptr2: " << *ptr2 << ", ptr=" << (ptr ? "非空" : "空") << "\n\n";

    std::cout << "   Q: 虚函数的开销?\n";
    std::cout << "   A: 每个对象多一个vptr(8字节), 虚调用多一次间接寻址\n";
    std::cout << "   实际影响: 极小, 除非在极热路径\n\n";

    std::cout << "3. 编程题准备:\n";
    std::cout << "   - 生产者-消费者问题\n";
    std::cout << "   - 读写锁实现\n";
    std::cout << "   - LRU缓存\n";
    std::cout << "   - 线程安全队列\n";
    std::cout << "   - 对象池\n";
}

void demo_coding_skills() {
    std::cout << "\n=== demo_coding_skills ===\n";
    std::cout << "编码技能提升\n\n";

    std::cout << "1. LRU缓存实现 (经典面试题):\n\n";

    LRUCache<std::string, int> lru(3);
    lru.put("a", 1);
    lru.put("b", 2);
    lru.put("c", 3);
    std::cout << "  get(a)=" << lru.get("a").value_or(-1) << "\n";
    lru.put("d", 4);
    std::cout << "  get(b)=" << lru.get("b").value_or(-1) << " (应被淘汰)\n";
    std::cout << "  get(c)=" << lru.get("c").value_or(-1) << "\n";
    std::cout << "  缓存大小: " << lru.size() << "\n\n";

    std::cout << "2. 技能提升路径:\n";
    std::cout << "   初级: 语法, STL, 基本OOP, 调试\n";
    std::cout << "   中级: 模板, 移动语义, 多线程, 设计模式\n";
    std::cout << "   高级: 内存模型, 元编程, 架构设计, 性能优化\n";
    std::cout << "   专家: 编译器, 标准库实现, 语言设计\n";
}

void demo_open_source_contribution() {
    std::cout << "\n=== demo_open_source_contribution ===\n";
    std::cout << "开源贡献指南\n\n";

    std::cout << "1. 选择项目:\n";
    std::cout << "   C++知名开源项目:\n";
    std::cout << "   - LLVM/Clang: 编译器基础设施\n";
    std::cout << "   - Boost: C++标准库的试验场\n";
    std::cout << "   - Folly: Facebook的C++库\n";
    std::cout << "   - spdlog: 高性能日志库\n";
    std::cout << "   - nlohmann/json: JSON库\n";
    std::cout << "   - range-v3: 范围库\n";
    std::cout << "   - fmt: 格式化库\n\n";

    std::cout << "2. 贡献方式:\n";
    std::cout << "   - 修复Bug: 从issue列表找good-first-issue\n";
    std::cout << "   - 添加测试: 提高测试覆盖率\n";
    std::cout << "   - 改进文档: 修正错误, 补充示例\n";
    std::cout << "   - 代码审查: 审查他人的PR\n";
    std::cout << "   - 新功能: 先提proposal, 讨论后再实现\n\n";

    std::cout << "3. 贡献流程:\n";
    std::cout << "   1. Fork项目\n";
    std::cout << "   2. 创建特性分支\n";
    std::cout << "   3. 编写代码和测试\n";
    std::cout << "   4. 遵循代码规范\n";
    std::cout << "   5. 提交PR, 清晰描述变更\n";
    std::cout << "   6. 响应代码审查意见\n\n";

    std::cout << "4. 开源贡献的价值:\n";
    std::cout << "   - 提升编码能力\n";
    std::cout << "   - 学习最佳实践\n";
    std::cout << "   - 建立技术影响力\n";
    std::cout << "   - 扩展职业网络\n";
    std::cout << "   - 面试加分项\n";
}

void demo_continuous_learning() {
    std::cout << "\n=== demo_continuous_learning ===\n";
    std::cout << "持续学习策略\n\n";

    std::cout << "1. 学习资源:\n\n";

    std::cout << "   书籍:\n";
    std::cout << "   入门: 《C++ Primer》\n";
    std::cout << "   进阶: 《Effective C++》《Effective Modern C++》\n";
    std::cout << "   高级: 《C++ Concurrency in Action》\n";
    std::cout << "   深入: 《Inside the C++ Object Model》\n";
    std::cout << "   模板: 《C++ Templates》\n";
    std::cout << "   元编程: 《C++ Template Metaprogramming》\n\n";

    std::cout << "   在线资源:\n";
    std::cout << "   - cppreference.com: 标准库参考\n";
    std::cout << "   - CppCon YouTube: 年度大会演讲\n";
    std::cout << "   - isocpp.org: C++标准委员会博客\n";
    std::cout << "   - Compiler Explorer: 在线编译器\n";
    std::cout << "   - Quick Bench: 在线基准测试\n\n";

    std::cout << "2. 学习方法:\n\n";

    std::cout << "   刻意练习:\n";
    std::cout << "   - 每天编码1小时\n";
    std::cout << "   - LeetCode/Codeforces刷题\n";
    std::cout << "   - 实现小型项目\n";
    std::cout << "   - 代码审查 (Review他人代码)\n\n";

    std::cout << "   深度学习:\n";
    std::cout << "   - 阅读标准库源码\n";
    std::cout << "   - 阅读优秀开源项目代码\n";
    std::cout << "   - 写技术博客/笔记\n";
    std::cout << "   - 参与C++标准讨论\n\n";

    std::cout << "3. C++标准演进:\n";
    std::cout << "   C++11: 现代C++的起点 (移动语义, lambda, auto)\n";
    std::cout << "   C++14: 完善C++11 (泛型lambda, auto返回类型推导)\n";
    std::cout << "   C++17: 结构化绑定, optional, variant, filesystem\n";
    std::cout << "   C++20: Concepts, Ranges, Coroutines, Modules\n";
    std::cout << "   C++23: std::expected, std::print, std::flat_map\n";
    std::cout << "   C++26: 反射(提案), 契约(提案), 线程池(提案)\n\n";

    std::cout << "4. 职业发展路径:\n";
    std::cout << "   初级工程师 -> 中级工程师 -> 高级工程师\n";
    std::cout << "   -> 技术专家 / 架构师 -> 技术总监 / CTO\n\n";

    std::cout << "   C++应用领域:\n";
    std::cout << "   - 系统编程: 操作系统, 驱动, 数据库\n";
    std::cout << "   - 游戏开发: 游戏引擎, 实时渲染\n";
    std::cout << "   - 金融: 高频交易, 风控系统\n";
    std::cout << "   - 嵌入式: IoT, 汽车, 航空\n";
    std::cout << "   - AI/ML: 推理引擎, 深度学习框架\n";
    std::cout << "   - 网络: Web服务器, CDN, 流媒体\n";

    std::cout << "\n5. 学习计划模板:\n";
    std::cout << "   第1-3月: C++基础 + STL + 调试技巧\n";
    std::cout << "   第4-6月: 面向对象 + 模板 + 智能指针\n";
    std::cout << "   第7-9月: 多线程 + 内存模型 + 并发模式\n";
    std::cout << "   第10-12月: 项目实战 + 性能优化 + 开源贡献\n";
    std::cout << "   持续: 关注新标准, 参与社区, 写作分享\n";
}

int main() {
    std::cout << "C++职业成长指南\n";

    demo_interview_preparation();
    demo_coding_skills();
    demo_open_source_contribution();
    demo_continuous_learning();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
