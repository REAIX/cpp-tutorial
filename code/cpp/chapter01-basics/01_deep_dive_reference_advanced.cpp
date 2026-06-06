/** @file 01_deep_dive_reference_advanced.cpp
 *  @brief 引用折叠、万能引用、悬垂引用与引用生命周期延长
 *  @description 对应文档: 02-CPP/01-basics
 */

#include <iostream>
#include <string>
#include <vector>
#include <utility>

// ===== 1. 引用折叠 (Reference Collapsing) =====
void demo_reference_collapsing() {
    std::cout << "===== 引用折叠 =====" << std::endl;

    // 引用折叠规则 (模板和 typedef 上下文中):
    // T& &   -> T&    (左值引用 + 左值引用 = 左值引用)
    // T& &&  -> T&    (左值引用 + 右值引用 = 左值引用)
    // T&& &  -> T&    (右值引用 + 左值引用 = 左值引用)
    // T&& && -> T&&   (右值引用 + 右值引用 = 右值引用)
    // 规律: 只要有一个左值引用, 结果就是左值引用

    // 通过类型别名演示
    using LRef = int&;
    using RRef = int&&;

    int x = 42;

    LRef& r1 = x;    // int& & -> int&
    LRef&& r2 = x;   // int& && -> int&
    RRef& r3 = x;    // int&& & -> int&
    // RRef&& r4 = 42; // int&& && -> int&& (C++ 中合法但需要右值)

    std::cout << "LRef&   绑定到 x: " << r1 << " (int& & -> int&)" << std::endl;
    std::cout << "LRef&&  绑定到 x: " << r2 << " (int& && -> int&)" << std::endl;
    std::cout << "RRef&   绑定到 x: " << r3 << " (int&& & -> int&)" << std::endl;

    std::cout << "\n引用折叠的意义:" << std::endl;
    std::cout << "  - 使模板能同时接受左值和右值" << std::endl;
    std::cout << "  - 是 std::forward 实现的基础" << std::endl;
    std::cout << "  - 是完美转发的核心机制" << std::endl;
}

// ===== 2. 万能引用 (Universal Reference) =====
// 注意: "万能引用"是 Scott Meyers 的术语, 标准中称为"转发引用"

template<typename T>
void process(T&& arg) {
    // T&& 在模板推导上下文中是万能引用
    // 传入左值时: T 推导为 int&, arg 类型为 int& (引用折叠)
    // 传入右值时: T 推导为 int,  arg 类型为 int&&
}

void demo_universal_reference() {
    std::cout << "\n===== 万能引用 (转发引用) =====" << std::endl;

    int x = 42;
    const int cx = 100;

    // 传入左值
    process(x);    // T = int&,   arg 类型 = int& && -> int&
    process(cx);   // T = const int&, arg 类型 = const int& && -> const int&

    // 传入右值
    process(42);   // T = int,    arg 类型 = int&&
    process(std::move(x));  // T = int, arg 类型 = int&&

    std::cout << "万能引用的判断条件:" << std::endl;
    std::cout << "  1. 必须是 T&& 形式 (模板参数 + &&)" << std::endl;
    std::cout << "  2. 必须发生类型推导" << std::endl;
    std::cout << "  3. 两个条件同时满足才是万能引用" << std::endl;

    std::cout << "\n不是万能引用的情况:" << std::endl;
    std::cout << "  - vector<int>&&  (无类型推导, 是右值引用)" << std::endl;
    std::cout << "  - auto&& 在非推导上下文" << std::endl;

    // auto&& 也是万能引用
    auto&& ref1 = x;    // 左值: auto = int&, ref1 类型 = int&
    auto&& ref2 = 42;   // 右值: auto = int, ref2 类型 = int&&
    (void)ref1;
    (void)ref2;
    std::cout << "  - auto&& 是万能引用" << std::endl;
}

// ===== 3. 悬垂引用 (Dangling Reference) =====
int& dangerous_return() {
    int local = 42;
    // return local;  // 严重错误: 返回局部变量的引用
    static int s = local;
    return s;  // OK: 静态变量生命周期持续到程序结束
}

std::vector<int> get_vector() {
    return {1, 2, 3, 4, 5};
}

void demo_dangling_reference() {
    std::cout << "\n===== 悬垂引用 =====" << std::endl;

    // 场景1: 返回局部变量的引用
    std::cout << "场景1: 返回局部变量引用 -> 未定义行为" << std::endl;
    std::cout << "  永远不要返回局部变量的引用或指针!" << std::endl;

    // 场景2: 容器元素引用在容器修改后失效
    std::vector<int> vec = {1, 2, 3};
    int& first = vec[0];
    vec.push_back(4);  // 可能触发重新分配, first 失效!
    // std::cout << first;  // 未定义行为!
    std::cout << "场景2: vector push_back 可能导致引用失效" << std::endl;
    std::cout << "  vector 重新分配后, 所有引用/指针/迭代器失效" << std::endl;

    // 场景3: 临时对象的引用
    const std::string& ref = std::string("hello");  // OK: const引用延长生命周期
    std::cout << "场景3: const引用延长临时对象生命周期: " << ref << std::endl;

    // 但注意: 只延长直接绑定的临时对象
    const std::string& bad_ref = get_vector().empty() ? "empty" : "not empty";
    // 这里 bad_ref 可能绑定到条件表达式的临时 string, 生命周期被延长
    std::cout << "  条件表达式中的临时对象: " << bad_ref << std::endl;

    std::cout << "\n避免悬垂引用的规则:" << std::endl;
    std::cout << "  1. 不返回局部变量的引用/指针" << std::endl;
    std::cout << "  2. 容器修改后不使用旧引用" << std::endl;
    std::cout << "  3. const 引用延长临时对象生命周期(仅直接绑定)" << std::endl;
    std::cout << "  4. 使用智能指针替代裸指针/引用" << std::endl;
}

// ===== 4. 引用生命周期延长 =====
void demo_lifetime_extension() {
    std::cout << "\n===== 引用生命周期延长 =====" << std::endl;

    // const 引用可以延长临时对象的生命周期
    {
        const int& r = 42;  // 临时 int 对象生命周期延长到 r 的作用域结束
        std::cout << "const int& 绑定右值: " << r << std::endl;
    }

    {
        const std::string& r = std::string("hello");
        std::cout << "const string& 绑定临时对象: " << r << std::endl;
        // 临时 string 的生命周期延长到 r 的作用域结束
    }

    // 举一反三: 生命周期延长的陷阱
    std::cout << "\n生命周期延长的陷阱:" << std::endl;

    // 陷阱1: 成员引用绑定到临时对象
    struct RefHolder {
        const std::string& ref;
        RefHolder(const std::string& s) : ref(s) {}
    };

    // RefHolder holder(std::string("temp"));
    // holder.ref 是悬垂引用! 临时 string 在构造函数结束后销毁
    // 生命周期延长只对直接绑定有效, 不传递给成员

    std::string persistent = "persistent";
    RefHolder safe_holder(persistent);  // OK: 绑定到持久对象
    std::cout << "  成员引用绑定临时对象: 构造结束后悬垂!" << std::endl;
    std::cout << "  安全做法: 绑定到生命周期足够长的对象" << std::endl;

    // 陷阱2: 函数返回 const 引用参数
    auto make_ref = [](const std::string& s) -> const std::string& {
        return s;  // OK: 返回参数的引用, 调用者保证参数生命周期
    };

    const std::string& safe = make_ref(persistent);  // OK
    std::cout << "  返回参数引用: 调用者需保证参数生命周期" << std::endl;

    std::cout << "\n最佳实践:" << std::endl;
    std::cout << "  - 类成员避免使用引用, 优先用指针或值" << std::endl;
    std::cout << "  - const 引用延长生命周期仅限直接绑定" << std::endl;
    std::cout << "  - 不确定时用值语义, 更安全" << std::endl;
}

int main() {
    std::cout << "========== 引用高级主题 ==========\n" << std::endl;

    demo_reference_collapsing();
    demo_universal_reference();
    demo_dangling_reference();
    demo_lifetime_extension();

    return 0;
}
