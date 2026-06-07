/**
 * @file 06_example_thread_pool_server.cpp
 * @brief 高并发服务器架构: 线程池 + Socket服务器架构
 * @description 对应文档: 高性能网络与异步IO / 第6节 高并发服务器架构
 */

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>
#include <memory>

#ifdef _WIN32
    #ifndef _WINSOCK2API_
        #include <winsock2.h>
        #include <ws2tcpip.h>
    #endif
    #ifdef _MSC_VER
        #pragma comment(lib, "ws2_32.lib")
    #endif
    typedef int socklen_t;
    typedef SOCKET socket_t;
    #ifndef __MINGW32__
        typedef int ssize_t;
    #endif
    #define INVALID_SOCKET_VAL INVALID_SOCKET
    #define CLOSE_SOCKET closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
    #include <errno.h>
    typedef int socket_t;
    #define INVALID_SOCKET_VAL (-1)
    #define CLOSE_SOCKET close
#endif

// ============================================================
// Winsock RAII
// ============================================================
class WinsockInit {
#ifdef _WIN32
    WSADATA wsa_data_;
#endif
public:
    WinsockInit() {
#ifdef _WIN32
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data_) != 0) {
            std::cerr << "[错误] WSAStartup失败\n";
            std::exit(1);
        }
#endif
    }
    ~WinsockInit() {
#ifdef _WIN32
        WSACleanup();
#endif
    }
    WinsockInit(const WinsockInit&) = delete;
    WinsockInit& operator=(const WinsockInit&) = delete;
};

int get_socket_error() {
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

// ============================================================
// 线程池实现
// ============================================================
class ThreadPool {
public:
    explicit ThreadPool(size_t num_threads) : stop_(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this]() {
                worker_loop();
            });
        }
        std::cout << "[线程池] 创建 " << num_threads << " 个工作线程\n";
    }

    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }
        cv_.notify_all();

        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        std::cout << "[线程池] 所有工作线程已退出\n";
    }

    /// 提交任务到线程池
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
        using ReturnType = std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            [f = std::forward<F>(f), ...args = std::forward<Args>(args)]() mutable -> ReturnType {
                return f(args...);
            }
        );

        std::future<ReturnType> result = task->get_future();

        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (stop_) {
                throw std::runtime_error("线程池已停止");
            }
            tasks_.push([task]() { (*task)(); });
        }

        cv_.notify_one();
        return result;
    }

    /// 获取待处理任务数
    size_t pending_tasks() const {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return tasks_.size();
    }

private:
    void worker_loop() {
        while (true) {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                cv_.wait(lock, [this]() {
                    return stop_ || !tasks_.empty();
                });

                if (stop_ && tasks_.empty()) {
                    return;
                }

                task = std::move(tasks_.front());
                tasks_.pop();
            }

            task();
        }
    }

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    mutable std::mutex queue_mutex_;
    std::condition_variable cv_;
    bool stop_;
};

// ============================================================
// 演示1: 线程池基础
// ============================================================
void demo_thread_pool_basics() {
    std::cout << "\n========================================\n";
    std::cout << "  演示1: 线程池基础\n";
    std::cout << "========================================\n\n";

    std::cout << "【为什么需要线程池?】\n\n";

    std::cout << "  传统方式: 每个连接一个线程\n";
    std::cout << "    - 线程创建/销毁开销大\n";
    std::cout << "    - 线程数随连接数增长, 资源消耗大\n";
    std::cout << "    - 线程切换开销随线程数增加\n\n";

    std::cout << "  线程池方式: 固定数量工作线程\n";
    std::cout << "    - 线程预先创建, 无创建开销\n";
    std::cout << "    - 线程数固定, 资源可控\n";
    std::cout << "    - 任务队列缓冲, 削峰填谷\n\n";

    std::cout << "【线程池核心组件】\n\n";

    std::cout << "  1. 工作线程 (Worker Threads)\n";
    std::cout << "     从任务队列取任务执行\n";
    std::cout << "     空闲时阻塞等待新任务\n\n";

    std::cout << "  2. 任务队列 (Task Queue)\n";
    std::cout << "     存储待执行的任务\n";
    std::cout << "     线程安全的FIFO队列\n\n";

    std::cout << "  3. 同步机制\n";
    std::cout << "     mutex: 保护任务队列\n";
    std::cout << "     condition_variable: 通知工作线程\n\n";

    // 实际演示
    ThreadPool pool(4);

    std::cout << "\n[测试] 提交8个计算任务到线程池...\n";

    std::vector<std::future<int>> results;
    for (int i = 0; i < 8; ++i) {
        results.push_back(pool.submit([i]() -> int {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            return i * i;
        }));
    }

    std::cout << "[测试] 等待所有任务完成...\n";
    for (size_t i = 0; i < results.size(); ++i) {
        std::cout << "  任务" << i << " 结果: " << results[i].get() << "\n";
    }
    std::cout << "[测试] 所有任务完成\n\n";
}

// ============================================================
// 演示2: 线程池 + Socket服务器
// ============================================================
void demo_thread_pool_server() {
    std::cout << "========================================\n";
    std::cout << "  演示2: 线程池 + Socket服务器\n";
    std::cout << "========================================\n\n";

    const uint16_t port = 15007;

    // 创建线程池 (4个工作线程)
    ThreadPool pool(4);

    // 创建监听socket
    socket_t listen_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == INVALID_SOCKET_VAL) {
        std::cerr << "[错误] 创建socket失败\n";
        return;
    }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (::bind(listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        std::cerr << "[错误] bind失败\n";
        CLOSE_SOCKET(listen_fd);
        return;
    }

    if (::listen(listen_fd, 10) != 0) {
        std::cerr << "[错误] listen失败\n";
        CLOSE_SOCKET(listen_fd);
        return;
    }

    std::cout << "[服务器] 线程池Echo服务器启动, 端口: " << port << "\n";
    std::cout << "[服务器] 架构: 主线程accept → 线程池处理IO\n\n";

    std::atomic<bool> server_done{false};
    std::atomic<int> total_requests{0};

    // 主线程: accept循环
    std::thread accept_thread([&]() {
        int client_count = 0;
        while (!server_done && client_count < 10) {
            // 使用select设置超时, 避免阻塞在accept上
            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(listen_fd, &read_fds);

            struct timeval tv{};
            tv.tv_sec = 0;
            tv.tv_usec = 200000;  // 200ms

            int ret = ::select(static_cast<int>(listen_fd + 1),
                               &read_fds, nullptr, nullptr, &tv);

            if (ret <= 0) continue;

            sockaddr_in client_addr{};
            socklen_t client_len = sizeof(client_addr);
            socket_t client_fd = ::accept(listen_fd,
                reinterpret_cast<sockaddr*>(&client_addr), &client_len);

            if (client_fd == INVALID_SOCKET_VAL) continue;

            client_count++;
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
            std::cout << "[服务器] 新连接 #" << client_count << ": "
                      << ip << ":" << ntohs(client_addr.sin_port) << "\n";

            // 将连接处理提交到线程池
            pool.submit([client_fd, &total_requests, id = client_count]() {
                std::cout << "[线程池] 开始处理连接 #" << id
                          << " (线程ID: "
                          << std::this_thread::get_id() << ")\n";

                char buf[1024];
                while (true) {
                    ssize_t n = ::recv(client_fd, buf, sizeof(buf) - 1, 0);
                    if (n <= 0) break;

                    buf[n] = '\0';
                    std::cout << "[线程池] 连接#" << id << " 收到: \""
                              << buf << "\"\n";

                    // 模拟业务处理
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));

                    ::send(client_fd, buf, static_cast<int>(n), 0);
                    total_requests++;
                }

                CLOSE_SOCKET(client_fd);
                std::cout << "[线程池] 连接#" << id << " 处理完成\n";
            });
        }
    });

    // 客户端测试
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    auto run_client = [&](int id) {
        socket_t sock = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET_VAL) return;

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

        if (::connect(sock, reinterpret_cast<sockaddr*>(&server_addr),
                      sizeof(server_addr)) == 0) {
            char buf[1024];
            std::string msg = "客户端" + std::to_string(id) + "的消息";
            ::send(sock, msg.c_str(), static_cast<int>(msg.size()), 0);
            std::cout << "[客户端" << id << "] 发送: \"" << msg << "\"\n";

            ssize_t n = ::recv(sock, buf, sizeof(buf) - 1, 0);
            if (n > 0) {
                buf[n] = '\0';
                std::cout << "[客户端" << id << "] 收到: \"" << buf << "\"\n";
            }
        }
        CLOSE_SOCKET(sock);
    };

    // 启动多个客户端
    std::vector<std::thread> clients;
    for (int i = 0; i < 4; ++i) {
        clients.emplace_back([&, i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(i * 100));
            run_client(i + 1);
        });
    }

    for (auto& c : clients) c.join();

    std::cout << "\n[统计] 总处理请求数: " << total_requests << "\n";

    server_done = true;
    accept_thread.join();
    CLOSE_SOCKET(listen_fd);
    std::cout << "\n";
}

// ============================================================
// 演示3: 高并发服务器架构模式
// ============================================================
void demo_server_architectures() {
    std::cout << "========================================\n";
    std::cout << "  演示3: 高并发服务器架构模式\n";
    std::cout << "========================================\n\n";

    std::cout << "【架构1: 单线程Reactor】\n";
    std::cout << "  accept + IO + 业务逻辑 全在一个线程\n";
    std::cout << "  优点: 简单, 无锁\n";
    std::cout << "  缺点: 无法利用多核\n";
    std::cout << "  适用: Redis, 少量连接\n\n";

    std::cout << "【架构2: Reactor + 线程池 (本节演示)】\n";
    std::cout << "  主线程: accept + IO\n";
    std::cout << "  线程池: 业务逻辑\n";
    std::cout << "  优点: IO和业务分离\n";
    std::cout << "  缺点: 主线程仍是瓶颈\n";
    std::cout << "  适用: 中等并发\n\n";

    std::cout << "【架构3: 主从Reactor多线程 (muduo)】\n";
    std::cout << "  主Reactor: accept\n";
    std::cout << "  从Reactor(多个): IO\n";
    std::cout << "  线程池: 业务逻辑\n";
    std::cout << "  优点: 充分利用多核\n";
    std::cout << "  缺点: 架构复杂\n";
    std::cout << "  适用: 高并发\n\n";

    std::cout << "【架构4: IOCP (Windows)】\n";
    std::cout << "  多个工作线程等待完成端口\n";
    std::cout << "  操作系统负责负载均衡\n";
    std::cout << "  优点: 最高性能, 操作系统优化\n";
    std::cout << "  缺点: 仅Windows\n";
    std::cout << "  适用: Windows高并发\n\n";

    std::cout << "【架构5: 微服务/分布式】\n";
    std::cout << "  每个服务独立部署\n";
    std::cout << "  通过RPC/消息队列通信\n";
    std::cout << "  优点: 可水平扩展\n";
    std::cout << "  缺点: 运维复杂\n";
    std::cout << "  适用: 超大规模\n\n";

    std::cout << "【线程池大小选择】\n\n";

    std::cout << "  CPU密集型: 线程数 = CPU核心数 + 1\n";
    std::cout << "  IO密集型:  线程数 = CPU核心数 × 2\n";
    std::cout << "  混合型:    根据实际负载调整\n";
    std::cout << "  经验公式:  线程数 = CPU核心数 × (1 + 等待时间/计算时间)\n";
}

// ============================================================
// 主函数
// ============================================================
int main() {
    std::cout << "============================================================\n";
    std::cout << "  第8章: 高性能网络与异步IO - 第6节 线程池服务器\n";
    std::cout << "============================================================\n";

    WinsockInit winsock_init;

    demo_thread_pool_basics();
    demo_thread_pool_server();
    demo_server_architectures();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
