/**
 * @file 01_deep_dive_coroutine_mechanism.cpp
 * @brief 协程机制深入探讨
 * @description 对应文档: 02-CPP/26-coroutine
 */

#include <iostream>
#include <coroutine>
#include <string>
#include <vector>
#include <cstdint>
#include <new>

void demo_coroutine_state() {
    std::cout << "\n=== 协程状态(协程帧) ===\n";

    std::cout << "协程帧包含:\n";
    std::cout << "  1. promise_type对象\n";
    std::cout << "  2. 协程参数(拷贝/移动)\n";
    std::cout << "  3. 局部变量(跨挂起点的)\n";
    std::cout << "  4. 挂起点信息(恢复地址)\n";
    std::cout << "  5. 引用计数(可选)\n";

    std::cout << "\n协程帧大小估算:\n";
    std::cout << "  简单协程: 通常100-200字节\n";
    std::cout << "  含大量局部变量: 可能更大\n";
    std::cout << "  默认使用operator new分配(堆上)\n";

    std::cout << "\n优化: 如果编译器能确定帧大小且生命周期明确,\n";
    std::cout << "  可能将帧分配在调用栈上(优化掉堆分配)\n";
}

struct TracingCoroutine {
    struct promise_type {
        int value;
        static int alive_count;

        TracingCoroutine get_return_object() {
            ++alive_count;
            std::cout << "  [promise] get_return_object() 帧数=" << alive_count << "\n";
            return TracingCoroutine{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() {
            std::cout << "  [promise] initial_suspend() -> 挂起\n";
            return {};
        }
        std::suspend_always final_suspend() noexcept {
            --alive_count;
            std::cout << "  [promise] final_suspend() -> 挂起 帧数=" << alive_count << "\n";
            return {};
        }
        std::suspend_always yield_value(int v) {
            value = v;
            std::cout << "  [promise] yield_value(" << v << ")\n";
            return {};
        }
        void return_void() {
            std::cout << "  [promise] return_void()\n";
        }
        void unhandled_exception() {
            std::cout << "  [promise] unhandled_exception()\n";
            std::terminate();
        }
    };

    std::coroutine_handle<promise_type> handle;

    ~TracingCoroutine() {
        if (handle && !handle.done()) handle.destroy();
    }
};

int TracingCoroutine::promise_type::alive_count = 0;

TracingCoroutine tracing_flow() {
    std::cout << "  [协程体] 第一步\n";
    co_yield 1;
    std::cout << "  [协程体] 第二步\n";
    co_yield 2;
    std::cout << "  [协程体] 第三步\n";
}

void demo_state_machine() {
    std::cout << "\n=== 协程状态机 ===\n";

    auto coro = tracing_flow();
    auto handle = coro.handle;

    std::cout << "初始状态(挂起在initial_suspend)\n";
    std::cout << "resume #1:\n";
    handle.resume();
    std::cout << "resume #2:\n";
    handle.resume();
    std::cout << "resume #3:\n";
    handle.resume();
    std::cout << "协程完成\n";
}

void demo_promise_customization() {
    std::cout << "\n=== promise_type定制点详解 ===\n";

    std::cout << "所有定制点:\n";
    std::cout << "  get_return_object()  -> 必须实现, 创建返回对象\n";
    std::cout << "  initial_suspend()    -> 必须实现, 协程体执行前\n";
    std::cout << "  final_suspend()      -> 必须实现, 协程体执行后\n";
    std::cout << "  unhandled_exception()-> 必须实现, 异常传播\n";
    std::cout << "  return_value()/return_void() -> co_return时调用\n";
    std::cout << "  yield_value()        -> co_yield时调用\n";
    std::cout << "  await_transform()    -> 可选, 转换co_await的操作数\n";

    std::cout << "\n举一反三:\n";
    std::cout << "  initial_suspend返回suspend_never -> 急切执行(立即运行协程体)\n";
    std::cout << "  initial_suspend返回suspend_always -> 惰性执行(需要手动resume)\n";
    std::cout << "  final_suspend返回suspend_always -> 协程结束后帧仍存在(可获取结果)\n";
    std::cout << "  final_suspend返回suspend_never -> 协程结束后帧自动销毁\n";
    std::cout << "  注意: final_suspend返回suspend_never时, 必须保证没有悬垂引用\n";
}

struct AllocatorCoroutine {
    struct promise_type {
        int value;

        AllocatorCoroutine get_return_object() {
            return AllocatorCoroutine{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        std::suspend_always yield_value(int v) { value = v; return {}; }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }

        void* operator new(std::size_t size) noexcept {
            std::cout << "  [分配器] 自定义new, 大小=" << size << "\n";
            return ::operator new(size, std::nothrow);
        }

        void operator delete(void* ptr, std::size_t size) {
            std::cout << "  [分配器] 自定义delete, 大小=" << size << "\n";
            ::operator delete(ptr);
        }

        static AllocatorCoroutine get_return_object_on_allocation_failure() {
            return AllocatorCoroutine{nullptr};
        }
    };

    std::coroutine_handle<promise_type> handle;

    ~AllocatorCoroutine() {
        if (handle && !handle.done()) handle.destroy();
    }
};

AllocatorCoroutine allocator_demo() {
    co_yield 42;
}

void demo_allocator_support() {
    std::cout << "\n=== 协程分配器支持 ===\n";

    auto coro = allocator_demo();
    if (coro.handle) {
        coro.handle.resume();
        std::cout << "值: " << coro.handle.promise().value << "\n";
        coro.handle.resume();
    } else {
        std::cout << "分配失败(返回空handle)\n";
    }

    std::cout << "\n分配器相关定制点:\n";
    std::cout << "  promise_type::operator new(size) -> 自定义帧分配\n";
    std::cout << "  promise_type::operator delete(ptr, size) -> 自定义帧释放\n";
    std::cout << "  promise_type::get_return_object_on_allocation_failure() -> 分配失败处理\n";
    std::cout << "  如果promise_type提供了operator new, 编译器会优先使用\n";
}

void demo_coroutine_overhead() {
    std::cout << "\n=== 协程开销分析 ===\n";

    std::cout << "1. 内存开销:\n";
    std::cout << "   协程帧: 通常100-300字节(取决于局部变量)\n";
    std::cout << "   普通函数: 仅栈帧, 无堆分配\n";

    std::cout << "\n2. 时间开销:\n";
    std::cout << "   创建: 一次堆分配(约50-200ns)\n";
    std::cout << "   挂起/恢复: 保存/恢复寄存器(约5-20ns)\n";
    std::cout << "   销毁: 一次堆释放\n";

    std::cout << "\n3. 与替代方案对比:\n";
    std::cout << "   vs 回调: 协程有分配开销, 但代码更清晰\n";
    std::cout << "   vs 线程: 协程更轻量(帧vs栈, KB级vsMB级)\n";
    std::cout << "   vs 手写状态机: 协程有分配开销, 但可读性远胜\n";

    std::cout << "\n4. 优化建议:\n";
    std::cout << "   避免在热路径创建大量短命协程\n";
    std::cout << "   使用自定义分配器(内存池)\n";
    std::cout << "   考虑对称转移(symmetic transfer)减少调度\n";
}

void demo_symmetric_transfer() {
    std::cout << "\n=== 对称转移(Symmetric Transfer) ===\n";

    std::cout << "对称转移: 一个协程直接将控制权转给另一个协程\n";
    std::cout << "避免通过中间调度器, 减少上下文切换开销\n";

    std::cout << "\n实现方式:\n";
    std::cout << "  await_suspend返回std::coroutine_handle<>(目标协程)\n";
    std::cout << "  编译器直接resume目标协程, 无需返回调用者\n";

    std::cout << "\n非对称转移:\n";
    std::cout << "  await_suspend返回void -> 返回调用者(resume的调用者)\n";
    std::cout << "  await_suspend返回bool -> true挂起返回调用者, false继续\n";

    std::cout << "\n举一反三:\n";
    std::cout << "  Generator的yield: 非对称(返回消费者)\n";
    std::cout << "  链式协程: 对称(直接转到下一个协程)\n";
    std::cout << "  生产者-消费者: 可用对称转移优化\n";
}

int main() {
    std::cout << "========== 协程机制深入探讨 ==========\n";
    std::cout << "注意: GCC可能需要 -fcoroutines 编译选项\n";

    demo_coroutine_state();
    demo_state_machine();
    demo_promise_customization();
    demo_allocator_support();
    demo_coroutine_overhead();
    demo_symmetric_transfer();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
