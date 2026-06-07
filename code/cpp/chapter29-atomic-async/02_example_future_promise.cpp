/** @file 02_example_future_promise.cpp @brief Future与Promise示例 @description 对应文档: 02-CPP/31-atomic-async
 *  编译命令: g++ -std=c++20 02_example_future_promise.cpp -o 02_example_future_promise
 */

#include <iostream>
#include <future>
#include <thread>
#include <vector>
#include <chrono>
#include <numeric>
#include <string>

int compute_square(int x) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return x * x;
}

void demo_async_basic() {
    std::cout << "\n=== std::async基础 ===\n";

    std::future<int> f1 = std::async(std::launch::async, compute_square, 5);
    std::cout << "async计算5的平方...\n";
    int result = f1.get();
    std::cout << "结果: " << result << "\n";

    std::future<int> f2 = std::async(std::launch::deferred, compute_square, 7);
    std::cout << "deferred: 延迟执行, get时才计算\n";
    result = f2.get();
    std::cout << "结果: " << result << "\n";

    std::cout << "\n启动策略:\n";
    std::cout << "  launch::async:    异步执行(新线程)\n";
    std::cout << "  launch::deferred: 延迟执行(get时)\n";
    std::cout << "  默认(async|deferred): 实现决定\n";
}

void demo_promise_future() {
    std::cout << "\n=== promise与future ===\n";

    std::promise<int> prom;
    std::future<int> fut = prom.get_future();

    std::thread t([&prom]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        prom.set_value(42);
        std::cout << "  设置promise值: 42\n";
    });

    std::cout << "等待future...\n";
    int val = fut.get();
    std::cout << "得到future值: " << val << "\n";
    t.join();

    std::cout << "\npromise可以设置异常:\n";
    std::promise<int> prom2;
    std::future<int> fut2 = prom2.get_future();
    prom2.set_exception(std::make_exception_ptr(std::runtime_error("计算失败")));
    try {
        fut2.get();
    } catch (const std::exception& e) {
        std::cout << "  捕获异常: " << e.what() << "\n";
    }
}

void demo_shared_future() {
    std::cout << "\n=== shared_future ===\n";

    std::promise<int> prom;
    std::shared_future<int> sf = prom.get_future().share();

    prom.set_value(100);

    auto reader = [sf](int id) {
        int val = sf.get();
        std::cout << "  读者" << id << ": " << val << "\n";
    };

    std::vector<std::jthread> threads;
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(reader, i);
    }

    std::cout << "shared_future: 多个线程可以get同一个future\n";
    std::cout << "future: 只能get一次\n";
    std::cout << "shared_future: 可以get多次(共享结果)\n";
}

void demo_packaged_task() {
    std::cout << "\n=== packaged_task ===\n";

    std::packaged_task<int(int, int)> task([](int a, int b) {
        return a + b;
    });

    std::future<int> fut = task.get_future();

    std::thread t(std::move(task), 10, 20);
    int result = fut.get();
    t.join();
    std::cout << "packaged_task结果: " << result << "\n";

    std::cout << "\npackaged_task用途:\n";
    std::cout << "  1. 包装可调用对象, 获取future\n";
    std::cout << "  2. 线程池任务提交\n";
    std::cout << "  3. 将普通函数转为异步任务\n";

    std::cout << "\n线程池模式:\n";
    std::vector<std::packaged_task<int()>> tasks;
    std::vector<std::future<int>> futures;

    for (int i = 0; i < 5; ++i) {
        std::packaged_task<int()> pt([i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            return i * i;
        });
        futures.push_back(pt.get_future());
        tasks.push_back(std::move(pt));
    }

    std::vector<std::jthread> workers;
    for (auto& task : tasks) {
        workers.emplace_back(std::move(task));
    }

    std::cout << "任务结果: ";
    for (auto& f : futures) {
        std::cout << f.get() << " ";
    }
    std::cout << "\n";
}

void demo_parallel_accumulate() {
    std::cout << "\n=== 并行累加 ===\n";

    std::vector<int> data(1000000, 1);

    auto start = std::chrono::high_resolution_clock::now();
    long long seq_sum = std::accumulate(data.begin(), data.end(), 0LL);
    auto seq_end = std::chrono::high_resolution_clock::now();

    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4;
    std::vector<std::future<long long>> partial_sums;
    size_t chunk_size = data.size() / num_threads;

    auto par_start = std::chrono::high_resolution_clock::now();
    for (unsigned int i = 0; i < num_threads; ++i) {
        size_t begin = i * chunk_size;
        size_t end = (i == num_threads - 1) ? data.size() : begin + chunk_size;
        partial_sums.push_back(std::async(std::launch::async,
            [&data](size_t b, size_t e) {
                long long sum = 0;
                for (size_t i = b; i < e; ++i) sum += data[i];
                return sum;
            }, begin, end));
    }

    long long par_sum = 0;
    for (auto& f : partial_sums) {
        par_sum += f.get();
    }
    auto par_end = std::chrono::high_resolution_clock::now();

    auto seq_ms = std::chrono::duration_cast<std::chrono::microseconds>(seq_end - start).count();
    auto par_ms = std::chrono::duration_cast<std::chrono::microseconds>(par_end - par_start).count();

    std::cout << "串行: sum=" << seq_sum << " 耗时=" << seq_ms << "us\n";
    std::cout << "并行(" << num_threads << "线程): sum=" << par_sum << " 耗时=" << par_ms << "us\n";
    std::cout << "结果一致: " << (seq_sum == par_sum ? "是" : "否") << "\n";
}

void demo_future_methods() {
    std::cout << "\n=== future方法 ===\n";

    std::future<int> f = std::async(std::launch::async, []() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return 42;
    });

    std::cout << "wait_for(50ms): ";
    auto status = f.wait_for(std::chrono::milliseconds(50));
    if (status == std::future_status::timeout) {
        std::cout << "超时(尚未就绪)\n";
    } else if (status == std::future_status::ready) {
        std::cout << "就绪\n";
    } else {
        std::cout << "延迟\n";
    }

    f.wait();
    std::cout << "wait()后: " << f.get() << "\n";

    std::cout << "\nfuture状态:\n";
    std::cout << "  timeout: 等待超时, 结果未就绪\n";
    std::cout << "  ready:   结果已就绪\n";
    std::cout << "  deferred: 延迟执行(尚未开始)\n";
}

int main() {
    std::cout << "========== Future与Promise示例 ==========\n";

    demo_async_basic();
    demo_promise_future();
    demo_shared_future();
    demo_packaged_task();
    demo_parallel_accumulate();
    demo_future_methods();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
