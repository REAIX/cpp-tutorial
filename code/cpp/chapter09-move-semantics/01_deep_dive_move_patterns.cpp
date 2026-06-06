/** @file 01_deep_dive_move_patterns.cpp
 *  @brief 移动语义进阶：RVO/NRVO、仅移动类型、容器中的移动、move与noexcept
 *  @description 对应文档: 09-移动语义与完美转发 | 举一反三：掌握移动语义的深层机制
 */

#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <algorithm>

class HeavyObject {
public:
    HeavyObject(int id) : id_(id), data_(1000, 'X') {
        std::cout << "HeavyObject(" << id_ << ") 构造\n";
    }
    HeavyObject(const HeavyObject& other) : id_(other.id_), data_(other.data_) {
        std::cout << "HeavyObject(" << id_ << ") 拷贝构造\n";
    }
    HeavyObject(HeavyObject&& other) noexcept : id_(other.id_), data_(std::move(other.data_)) {
        other.id_ = 0;
        std::cout << "HeavyObject(" << id_ << ") 移动构造\n";
    }
    HeavyObject& operator=(const HeavyObject& other) {
        if (this != &other) {
            id_ = other.id_;
            data_ = other.data_;
            std::cout << "HeavyObject(" << id_ << ") 拷贝赋值\n";
        }
        return *this;
    }
    HeavyObject& operator=(HeavyObject&& other) noexcept {
        if (this != &other) {
            id_ = other.id_;
            data_ = std::move(other.data_);
            other.id_ = 0;
            std::cout << "HeavyObject(" << id_ << ") 移动赋值\n";
        }
        return *this;
    }
    ~HeavyObject() {
        std::cout << "HeavyObject(" << id_ << ") 析构\n";
    }
    int id() const { return id_; }
private:
    int id_ = 0;
    std::string data_;
};

void demo_rvo_nrvo() {
    std::cout << "=== 返回值优化 (RVO/NRVO) ===\n";

    std::cout << "--- RVO (Return Value Optimization) ---\n";
    {
        auto create_rvo = []() -> HeavyObject {
            return HeavyObject(1);  // 直接在调用者的栈上构造
        };
        HeavyObject obj = create_rvo();
        std::cout << "RVO: 可能完全没有拷贝/移动!\n";
    }

    std::cout << "\n--- NRVO (Named Return Value Optimization) ---\n";
    {
        auto create_nrvo = []() -> HeavyObject {
            HeavyObject obj(2);
            // ... 对 obj 做一些操作 ...
            return obj;  // 命名对象的优化
        };
        HeavyObject obj = create_nrvo();
        std::cout << "NRVO: 命名对象也可能被优化掉\n";
    }

    std::cout << "\n--- NRVO 失效的情况 ---\n";
    {
        auto create_conditional = [](bool flag) -> HeavyObject {
            HeavyObject a(3);
            HeavyObject b(4);
            if (flag) return a;
            return b;
        };
        HeavyObject obj = create_conditional(true);
        std::cout << "条件返回: NRVO 失效, 但移动语义兜底\n";
    }

    std::cout << "\nRVO/NRVO 要点:\n";
    std::cout << "  1. C++17 保证: 返回纯右值时必定省略拷贝/移动\n";
    std::cout << "  2. NRVO 不保证, 但主流编译器都会优化\n";
    std::cout << "  3. 不要为了'优化'而使用 std::move 返回局部变量\n";
    std::cout << "     return std::move(obj); 会阻止 NRVO!\n";
    std::cout << "  4. 正确做法: return obj; 让编译器决定\n";

    std::cout << "\n";
}

void demo_move_only_types() {
    std::cout << "=== 仅移动类型 ===\n";

    std::cout << "--- std::unique_ptr 是典型的仅移动类型 ---\n";
    {
        auto p1 = std::make_unique<int>(42);
        // auto p2 = p1;  // 编译错误! 不可拷贝
        auto p2 = std::move(p1);  // 只能移动
        std::cout << "p1.get() = " << p1.get() << "\n";
        std::cout << "p2.get() = " << p2.get() << ", *p2 = " << *p2 << "\n";
    }

    std::cout << "\n--- 自定义仅移动类型 ---\n";
    {
        class MoveOnly {
        public:
            MoveOnly() = default;
            MoveOnly(const MoveOnly&) = delete;
            MoveOnly& operator=(const MoveOnly&) = delete;
            MoveOnly(MoveOnly&&) noexcept = default;
            MoveOnly& operator=(MoveOnly&&) noexcept = default;
        };

        MoveOnly a;
        // MoveOnly b = a;  // 编译错误
        MoveOnly c = std::move(a);  // OK
        (void)c;

        std::cout << "仅移动类型的定义模式:\n";
        std::cout << "  1. 拷贝构造/赋值 = delete\n";
        std::cout << "  2. 移动构造/赋值 = default 或自定义\n";
        std::cout << "  3. 移动操作标记 noexcept\n";
    }

    std::cout << "\n仅移动类型的典型应用:\n";
    std::cout << "  - 独占资源的句柄 (文件、锁、网络连接)\n";
    std::cout << "  - unique_ptr, thread, fstream, mutex\n";
    std::cout << "  - 不可复制的状态机\n";

    std::cout << "\n";
}

void demo_move_in_containers() {
    std::cout << "=== 容器中的移动 ===\n";

    std::cout << "--- vector 扩容时的移动 ---\n";
    {
        std::vector<HeavyObject> vec;
        vec.reserve(2);
        std::cout << "预留容量 2\n";

        vec.emplace_back(1);
        vec.emplace_back(2);
        std::cout << "添加2个元素, 容量=" << vec.capacity() << "\n";

        std::cout << "添加第3个元素, 触发扩容:\n";
        vec.emplace_back(3);
        std::cout << "扩容后容量=" << vec.capacity() << "\n";
    }

    std::cout << "\n--- vector 元素的移动 ---\n";
    {
        std::vector<std::unique_ptr<int>> ptrs;
        ptrs.push_back(std::make_unique<int>(10));
        ptrs.push_back(std::make_unique<int>(20));
        ptrs.push_back(std::make_unique<int>(30));

        std::vector<std::unique_ptr<int>> moved_ptrs;
        for (auto& p : ptrs) {
            moved_ptrs.push_back(std::move(p));
        }
        std::cout << "移动后原容器元素全部为 nullptr\n";
    }

    std::cout << "\n--- sort 中的移动 ---\n";
    {
        std::vector<HeavyObject> vec;
        vec.emplace_back(3);
        vec.emplace_back(1);
        vec.emplace_back(2);

        std::cout << "排序时元素通过移动交换:\n";
        // 自定义比较避免额外构造
        std::sort(vec.begin(), vec.end(),
            [](const HeavyObject& a, const HeavyObject& b) {
                return a.id() < b.id();
            });
        std::cout << "排序完成\n";
    }

    std::cout << "\n";
}

void demo_move_and_noexcept() {
    std::cout << "=== move 与 noexcept ===\n";

    std::cout << "--- noexcept 移动操作的重要性 ---\n";
    {
        class NoNoexceptMove {
        public:
            NoNoexceptMove() = default;
            NoNoexceptMove(NoNoexceptMove&& other)  // 没有 noexcept
                : data_(std::move(other.data_)) {
                std::cout << "NoNoexceptMove 移动构造\n";
            }
            NoNoexceptMove(const NoNoexceptMove& other)
                : data_(other.data_) {
                std::cout << "NoNoexceptMove 拷贝构造\n";
            }
            std::string data_ = "data";
        };

        std::vector<NoNoexceptMove> vec;
        vec.emplace_back();
        vec.emplace_back();
        std::cout << "没有 noexcept 的移动: vector 扩容时可能使用拷贝!\n";
    }

    std::cout << "\n";
    {
        class NoexceptMove {
        public:
            NoexceptMove() = default;
            NoexceptMove(NoexceptMove&& other) noexcept
                : data_(std::move(other.data_)) {
                std::cout << "NoexceptMove 移动构造 (noexcept)\n";
            }
            NoexceptMove(const NoexceptMove& other)
                : data_(other.data_) {
                std::cout << "NoexceptMove 拷贝构造\n";
            }
            std::string data_ = "data";
        };

        std::vector<NoexceptMove> vec;
        vec.emplace_back();
        vec.emplace_back();
        std::cout << "有 noexcept 的移动: vector 扩容时使用移动!\n";
    }

    std::cout << "\nnoexcept 的重要性:\n";
    std::cout << "  1. vector 扩容时, 如果移动构造不是 noexcept,\n";
    std::cout << "     则使用拷贝构造 (强异常安全保证)\n";
    std::cout << "  2. 移动操作应该尽量标记 noexcept\n";
    std::cout << "  3. swap 操作也应该标记 noexcept\n";
    std::cout << "  4. noexcept 是接口契约的一部分\n";

    std::cout << "\n";
}

void demo_move_common_mistakes() {
    std::cout << "=== 移动语义常见错误 ===\n";

    std::cout << "错误1: 在 return 语句中使用 std::move\n";
    std::cout << "  return std::move(local);  // 阻止 NRVO!\n";
    std::cout << "  正确: return local;  // 让编译器优化\n\n";

    std::cout << "错误2: 移动后使用源对象\n";
    {
        std::string s = "Hello";
        std::string s2 = std::move(s);
        // std::cout << s[0];  // 未定义行为! s 处于有效但未指定状态
        std::cout << "  移动后不要对源对象做假设\n";
        s = "新值";  // 赋新值是安全的
        std::cout << "  赋新值后: s = " << s << "\n";
    }

    std::cout << "\n错误3: 在容器中 push_back 后使用 std::move 的对象\n";
    {
        auto p = std::make_unique<int>(42);
        std::vector<std::unique_ptr<int>> vec;
        vec.push_back(std::move(p));
        // *p;  // p 已经是 nullptr
        std::cout << "  push_back(std::move(p)) 后 p = " << p.get() << "\n";
    }

    std::cout << "\n错误4: 移动赋值未处理自赋值\n";
    std::cout << "  obj = std::move(obj);  // 如果未检查 this == &other, 会出问题\n";

    std::cout << "\n";
}

int main() {
    demo_rvo_nrvo();
    demo_move_only_types();
    demo_move_in_containers();
    demo_move_and_noexcept();
    demo_move_common_mistakes();

    return 0;
}
