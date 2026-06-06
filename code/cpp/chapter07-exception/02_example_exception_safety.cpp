/** @file 02_example_exception_safety.cpp
 *  @brief 异常安全等级、RAII实现异常安全
 *  @description 对应文档: 02-CPP/07-exception
 */

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include <mutex>

// ===== 1. 异常安全等级 =====

// 四个异常安全等级:
// 1. 不抛出 (No-throw): 保证不抛出异常 (析构函数, swap, 内存释放)
// 2. 强保证 (Strong): 操作失败时状态回滚到调用前
// 3. 基本保证 (Basic): 操作失败时对象处于有效状态, 无资源泄漏
// 4. 无保证 (No): 异常发生时行为未定义

void demo_exception_safety_levels() {
    std::cout << "===== 异常安全等级 =====" << std::endl;

    std::cout << "1. 不抛出保证 (No-throw guarantee)" << std::endl;
    std::cout << "   - 析构函数, swap, 内存释放, mutex 操作" << std::endl;
    std::cout << "   - 标记: noexcept" << std::endl;

    std::cout << "\n2. 强保证 (Strong exception guarantee)" << std::endl;
    std::cout << "   - 操作原子性: 成功或回滚" << std::endl;
    std::cout << "   - 示例: std::vector::push_back (可能不满足)" << std::endl;

    std::cout << "\n3. 基本保证 (Basic exception guarantee)" << std::endl;
    std::cout << "   - 无资源泄漏" << std::endl;
    std::cout << "   - 对象处于有效但可能改变的状态" << std::endl;

    std::cout << "\n4. 无保证 (No exception safety)" << std::endl;
    std::cout << "   - 异常发生时可能资源泄漏或状态不一致" << std::endl;
    std::cout << "   - 应该避免!" << std::endl;
}

// ===== 2. 不安全的代码 =====
class UnsafeStack {
public:
    void push(int value) {
        int* new_data = new int[size_ + 1];  // 可能抛出 bad_alloc

        // 如果下面拷贝时抛出异常, new_data 泄漏!
        for (size_t i = 0; i < size_; ++i) {
            new_data[i] = data_[i];
        }
        new_data[size_] = value;

        delete[] data_;  // 如果上面抛出异常, 这行不执行
        data_ = new_data;
        ++size_;
    }

    UnsafeStack() : data_(nullptr), size_(0) {}
    ~UnsafeStack() { delete[] data_; }

private:
    int* data_;
    size_t size_;
};

// ===== 3. RAII 实现异常安全 =====
class SafeStack {
public:
    SafeStack() = default;

    void push(int value) {
        // 强异常安全: copy-and-swap 惯用法
        std::vector<int> new_data = data_;  // 先拷贝
        new_data.push_back(value);           // 修改拷贝
        data_.swap(new_data);               // swap 不抛出异常
    }

    int top() const {
        if (data_.empty()) {
            throw std::out_of_range("栈为空");
        }
        return data_.back();
    }

    void pop() {
        if (data_.empty()) {
            throw std::out_of_range("栈为空");
        }
        data_.pop_back();
    }

    bool empty() const { return data_.empty(); }
    size_t size() const { return data_.size(); }

private:
    std::vector<int> data_;
};

void demo_raii_exception_safety() {
    std::cout << "\n===== RAII 实现异常安全 =====" << std::endl;

    SafeStack stack;
    stack.push(1);
    stack.push(2);
    stack.push(3);

    std::cout << "  栈顶: " << stack.top() << std::endl;
    stack.pop();
    std::cout << "  pop 后栈顶: " << stack.top() << std::endl;

    // 即使 push 中途抛出异常, 也不会泄漏资源
    // 因为 vector 的 RAII 保证了内存正确释放

    std::cout << "\nRAII 保证异常安全:" << std::endl;
    std::cout << "  - 构造时获取资源, 析构时释放" << std::endl;
    std::cout << "  - 栈展开保证析构函数执行" << std::endl;
    std::cout << "  - 即使异常发生, 资源也不会泄漏" << std::endl;
}

// ===== 4. Copy-and-Swap 惯用法 =====
class StringTable {
public:
    StringTable() = default;

    void add(const std::string& key, const std::string& value) {
        // 强异常安全: 先创建新数据, 再交换
        auto new_entries = entries_;  // 拷贝
        new_entries.push_back({key, value});  // 修改拷贝
        entries_.swap(new_entries);  // noexcept swap
    }

    void remove(size_t index) {
        if (index >= entries_.size()) {
            throw std::out_of_range("索引越界");
        }
        auto new_entries = entries_;
        new_entries.erase(new_entries.begin() + static_cast<std::ptrdiff_t>(index));
        entries_.swap(new_entries);
    }

    size_t size() const { return entries_.size(); }

    void display() const {
        for (const auto& [k, v] : entries_) {
            std::cout << "    " << k << " = " << v << std::endl;
        }
    }

private:
    std::vector<std::pair<std::string, std::string>> entries_;
};

void demo_copy_and_swap() {
    std::cout << "\n===== Copy-and-Swap 惯用法 =====" << std::endl;

    StringTable table;
    table.add("name", "张三");
    table.add("age", "25");
    table.add("city", "北京");

    std::cout << "  添加后 (" << table.size() << " 条):" << std::endl;
    table.display();

    table.remove(1);
    std::cout << "  删除后 (" << table.size() << " 条):" << std::endl;
    table.display();

    std::cout << "\nCopy-and-Swap 步骤:" << std::endl;
    std::cout << "  1. 创建数据的拷贝" << std::endl;
    std::cout << "  2. 在拷贝上执行修改" << std::endl;
    std::cout << "  3. 用 noexcept swap 交换原数据和新数据" << std::endl;
    std::cout << "  如果步骤1或2抛异常, 原数据不变 (强保证)" << std::endl;
    std::cout << "  步骤3不抛异常, 保证交换成功" << std::endl;
}

// ===== 5. 智能指针与异常安全 =====
class Resource {
public:
    Resource(const std::string& name) : name_(name) {
        std::cout << "  获取资源: " << name_ << std::endl;
    }
    ~Resource() {
        std::cout << "  释放资源: " << name_ << std::endl;
    }
    void use() const {
        std::cout << "  使用资源: " << name_ << std::endl;
    }
private:
    std::string name_;
};

void process_with_raii() {
    auto r1 = std::make_unique<Resource>("文件A");
    auto r2 = std::make_unique<Resource>("网络B");

    r1->use();
    r2->use();

    // 如果这里抛出异常, r1 和 r2 的析构函数仍会执行
    // 资源不会泄漏

    throw std::runtime_error("模拟异常");
}

void demo_smart_pointer_safety() {
    std::cout << "\n===== 智能指针与异常安全 =====" << std::endl;

    try {
        process_with_raii();
    } catch (const std::runtime_error& e) {
        std::cout << "  捕获: " << e.what() << std::endl;
    }
    // 即使抛出异常, 两个 Resource 对象也被正确释放

    std::cout << "\n智能指针保证异常安全:" << std::endl;
    std::cout << "  - unique_ptr: 独占所有权, 析构自动释放" << std::endl;
    std::cout << "  - shared_ptr: 共享所有权, 引用计数归零释放" << std::endl;
    std::cout << "  - 栈展开时析构函数执行, 资源不泄漏" << std::endl;
    std::cout << "  - 比手动 new/delete 更安全" << std::endl;
}

int main() {
    std::cout << "========== 异常安全 ==========\n" << std::endl;

    demo_exception_safety_levels();
    demo_raii_exception_safety();
    demo_copy_and_swap();
    demo_smart_pointer_safety();

    return 0;
}
