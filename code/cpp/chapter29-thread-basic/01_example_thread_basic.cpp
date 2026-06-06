/** 
@file 01_example_thread_basic.cpp 
@brief 多线程基础示例 @description 对应文档: 02-CPP/29-thread-basic
 *  @note C 语言中使用 pthread API 实现类似功能, 参见 C 章节 24-进程与线程
 *  编译命令: g++ -std=c++20 01_example_thread_basic.cpp -o 01_example_thread_basic
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <string>

void hello_thread() {
    std::cout << "  Hello from thread " << std::this_thread::get_id() << "\n";
}

void hello_with_name(const std::string& name) {
    std::cout << "  Hello from " << name
              << " (thread " << std::this_thread::get_id() << ")\n";
}

void work_with_duration(int milliseconds) {
    std::cout << "  工作" << milliseconds << "ms开始 (线程"
              << std::this_thread::get_id() << ")\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    std::cout << "  工作" << milliseconds << "ms结束 (线程"
              << std::this_thread::get_id() << ")\n";
}

void demo_thread_creation() {
    std::cout << "\n=== 线程创建 ===\n";

    std::cout << "主线程ID: " << std::this_thread::get_id() << "\n";

    std::thread t1(hello_thread);
    t1.join();

    std::thread t2(hello_with_name, "Worker-1");
    t2.join();

    std::thread t3([]() {
        std::cout << "  Lambda线程 " << std::this_thread::get_id() << "\n";
    });
    t3.join();
}

void demo_join_detach() {
    std::cout << "\n=== join与detach ===\n";

    std::cout << "join: 等待线程完成\n";
    std::thread t1([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        std::cout << "  t1完成\n";
    });
    std::cout << "t1 joinable: " << t1.joinable() << "\n";
    t1.join();
    std::cout << "t1 join后 joinable: " << t1.joinable() << "\n";

    std::cout << "\ndetach: 分离线程, 后台运行\n";
    std::thread t2([]() {
        std::cout << "  分离线程运行中\n";
    });
    t2.detach();
    std::cout << "t2 detach后 joinable: " << t2.joinable() << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::cout << "\n注意: 必须调用join()或detach(), 否则析构时std::terminate\n";
}

void demo_multiple_threads() {
    std::cout << "\n=== 多线程并发 ===\n";

    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(i * 10));
            std::cout << "  线程" << i << " 完成\n";
        });
    }

    for (auto& t : threads) {
        t.join();
    }
    std::cout << "所有线程完成\n";
}

void demo_jthread() {
    std::cout << "\n=== jthread (C++20) ===\n";

    std::cout << "jthread: 析构时自动join的线程\n";

    {
        std::jthread jt([]() {
            std::cout << "  jthread工作中\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            std::cout << "  jthread完成\n";
        });
        std::cout << "jthread joinable: " << jt.joinable() << "\n";
    }
    std::cout << "jthread离开作用域, 自动join\n";

    std::cout << "\njthread支持停止请求:\n";
    {
        std::jthread jt([](std::stop_token st) {
            int count = 0;
            while (!st.stop_requested() && count < 100) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                ++count;
                std::cout << "  jthread计数: " << count << "\n";
            }
            if (st.stop_requested()) {
                std::cout << "  jthread收到停止请求\n";
            }
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(55));
        jt.request_stop();
        std::cout << "已发送停止请求\n";
    }
}

void demo_thread_id() {
    std::cout << "\n=== 线程ID ===\n";

    std::cout << "主线程ID: " << std::this_thread::get_id() << "\n";

    std::thread t1([]() {
        std::cout << "  子线程ID: " << std::this_thread::get_id() << "\n";
    });
    std::cout << "t1的ID: " << t1.get_id() << "\n";
    t1.join();

    std::cout << "\n线程ID可以用于:\n";
    std::cout << "  1. 调试和日志\n";
    std::cout << "  2. 线程特定的数据映射\n";
    std::cout << "  3. 判断是否在主线程\n";
}

int main() {
    std::cout << "========== 多线程基础示例 ==========\n";

    demo_thread_creation();
    demo_join_detach();
    demo_multiple_threads();
    demo_jthread();
    demo_thread_id();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
