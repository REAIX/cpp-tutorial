/**
 * @file 03_example_proactor_concepts.cpp
 * @brief Proactor模式: Proactor vs Reactor对比, 概念性实现
 * @description 对应文档: 高性能网络与异步IO / 第3节 Proactor模式与异步IO
 *
 * Proactor模式是真正的异步IO模型, 与Reactor的同步非阻塞IO不同。
 * Windows IOCP是Proactor的典型实现。
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

// ============================================================
// 演示1: Proactor vs Reactor 核心区别
// ============================================================
void demo_proactor_vs_reactor() {
    std::cout << "\n========================================\n";
    std::cout << "  演示1: Proactor vs Reactor 核心区别\n";
    std::cout << "========================================\n\n";

    std::cout << "【Reactor模式 - 同步非阻塞IO】\n\n";

    std::cout << "  工作流程:\n";
    std::cout << "  1. 应用注册感兴趣的事件 (fd可读/可写)\n";
    std::cout << "  2. Reactor等待事件就绪 (epoll_wait/select)\n";
    std::cout << "  3. 事件就绪后通知应用\n";
    std::cout << "  4. 应用自己执行IO操作 (recv/send)\n";
    std::cout << "  5. 应用处理数据\n\n";

    std::cout << "  特点: \"告诉我什么时候可以IO, 我自己来做\"\n\n";

    std::cout << "  伪代码:\n";
    std::cout << "    // 注册事件\n";
    std::cout << "    reactor.register(fd, READ_EVENT, on_readable);\n\n";

    std::cout << "    // 事件回调: 应用自己执行IO\n";
    std::cout << "    void on_readable(fd) {\n";
    std::cout << "        data = recv(fd);      // 应用自己读\n";
    std::cout << "        process(data);        // 应用自己处理\n";
    std::cout << "    }\n\n";

    std::cout << "【Proactor模式 - 真正的异步IO】\n\n";

    std::cout << "  工作流程:\n";
    std::cout << "  1. 应用发起异步IO操作 (WSARecv/WSASend)\n";
    std::cout << "  2. 操作立即返回, 应用继续做其他事\n";
    std::cout << "  3. 操作系统在后台执行IO\n";
    std::cout << "  4. IO完成后, 操作系统通知应用\n";
    std::cout << "  5. 应用处理已完成的IO结果\n\n";

    std::cout << "  特点: \"帮我做IO, 做完了告诉我\"\n\n";

    std::cout << "  伪代码:\n";
    std::cout << "    // 发起异步读操作\n";
    std::cout << "    proactor.async_read(fd, buffer, on_read_complete);\n\n";

    std::cout << "    // 完成回调: IO已经完成, 数据已在buffer中\n";
    std::cout << "    void on_read_complete(fd, buffer, bytes_read) {\n";
    std::cout << "        process(buffer);      // 直接处理, 无需再recv\n";
    std::cout << "    }\n\n";

    std::cout << "【核心区别对比表】\n\n";

    std::cout << "  方面          Reactor              Proactor\n";
    std::cout << "  ──────────────────────────────────────────────────\n";
    std::cout << "  IO执行者      应用程序             操作系统\n";
    std::cout << "  通知时机      IO可以执行时         IO已完成时\n";
    std::cout << "  IO模型        同步非阻塞           真正异步\n";
    std::cout << "  典型实现      epoll/select         IOCP/io_uring\n";
    std::cout << "  缓冲区管理    应用自己管理         需提前提供buffer\n";
    std::cout << "  编程复杂度    较低                 较高\n";
    std::cout << "  Windows支持   select(性能差)       IOCP(高性能)\n";
    std::cout << "  Linux支持     epoll(高性能)        io_uring(5.1+)\n";
}

// ============================================================
// 演示2: Proactor模式概念实现
// ============================================================
void demo_proactor_conceptual() {
    std::cout << "\n========================================\n";
    std::cout << "  演示2: Proactor模式概念实现\n";
    std::cout << "========================================\n\n";

    std::cout << "【Proactor的核心组件】\n\n";

    std::cout << "  1. AsyncOperation (异步操作)\n";
    std::cout << "     - 描述一个异步IO请求 (读/写/接受连接)\n";
    std::cout << "     - 包含: fd, buffer, 操作类型, 完成回调\n\n";

    std::cout << "  2. Proactor (完成事件分发器)\n";
    std::cout << "     - 等待IO完成通知\n";
    std::cout << "     - 将完成事件分发给对应的回调\n";
    std::cout << "     - Windows: GetQueuedCompletionStatus\n\n";

    std::cout << "  3. CompletionHandler (完成处理器)\n";
    std::cout << "     - 处理IO完成后的逻辑\n";
    std::cout << "     - 类似Reactor中的EventHandler\n\n";

    std::cout << "【模拟Proactor工作流程】\n\n";

    // 模拟异步操作
    struct AsyncOperation {
        enum Type { READ, WRITE, ACCEPT };
        Type type;
        socket_t fd;
        std::vector<char> buffer;
        std::function<void(socket_t, const std::vector<char>&, size_t)> callback;
    };

    // 模拟Proactor (简化版, 用同步IO模拟异步完成)
    class SimpleProactor {
    public:
        /// 发起异步读操作
        void async_read(socket_t fd, size_t buffer_size,
                       std::function<void(socket_t, const std::vector<char>&, size_t)> callback) {
            std::cout << "[Proactor] 发起异步读: fd=" << fd
                      << ", buffer_size=" << buffer_size << "\n";

            // 在真实实现中, 这里会调用WSARecv等异步API
            // 这里用同步IO模拟
            pending_ops_.push({AsyncOperation::READ, fd,
                              std::vector<char>(buffer_size), std::move(callback)});
        }

        /// 发起异步写操作
        void async_write(socket_t fd, const std::vector<char>& data,
                        std::function<void(socket_t, const std::vector<char>&, size_t)> callback) {
            std::cout << "[Proactor] 发起异步写: fd=" << fd
                      << ", data_size=" << data.size() << "\n";

            pending_ops_.push({AsyncOperation::WRITE, fd, data, std::move(callback)});
        }

        /// 处理已完成的操作 (模拟GetQueuedCompletionStatus)
        void process_completions() {
            while (!pending_ops_.empty()) {
                auto& op = pending_ops_.front();

                if (op.type == AsyncOperation::READ) {
                    // 模拟异步读完成
                    ssize_t n = ::recv(op.fd, op.buffer.data(),
                                      static_cast<int>(op.buffer.size()), 0);
                    if (n > 0) {
                        std::cout << "[Proactor] 异步读完成: " << n << " 字节\n";
                        op.callback(op.fd, op.buffer, static_cast<size_t>(n));
                    }
                } else if (op.type == AsyncOperation::WRITE) {
                    // 模拟异步写完成
                    ssize_t n = ::send(op.fd, op.buffer.data(),
                                      static_cast<int>(op.buffer.size()), 0);
                    if (n > 0) {
                        std::cout << "[Proactor] 异步写完成: " << n << " 字节\n";
                        op.callback(op.fd, op.buffer, static_cast<size_t>(n));
                    }
                }

                pending_ops_.pop();
            }
        }

    private:
        std::queue<AsyncOperation> pending_ops_;
    };

    std::cout << "  Proactor模式的关键区别:\n";
    std::cout << "  - 应用不直接调用recv/send\n";
    std::cout << "  - 而是调用async_read/async_write\n";
    std::cout << "  - IO由操作系统在后台完成\n";
    std::cout << "  - 完成后回调被调用, 数据已就绪\n\n";

    std::cout << "  真正的Proactor实现 (Windows IOCP):\n";
    std::cout << "  1. CreateIoCompletionPort()  创建完成端口\n";
    std::cout << "  2. WSARecv()                 发起异步读\n";
    std::cout << "  3. GetQueuedCompletionStatus() 等待完成\n";
    std::cout << "  4. 处理完成结果\n";
}

// ============================================================
// 演示3: Boost.Asio的Proactor实现
// ============================================================
void demo_boost_asio_proactor() {
    std::cout << "\n========================================\n";
    std::cout << "  演示3: Boost.Asio的Proactor实现\n";
    std::cout << "========================================\n\n";

    std::cout << "【Boost.Asio架构】\n\n";

    std::cout << "  Boost.Asio采用Proactor模式设计:\n\n";

    std::cout << "  io_context (Proactor)\n";
    std::cout << "    ├── async_read_some()  → 发起异步读\n";
    std::cout << "    ├── async_write_some() → 发起异步写\n";
    std::cout << "    └── run()              → 等待完成事件\n\n";

    std::cout << "  平台适配:\n";
    std::cout << "    Windows: 使用IOCP (真正异步)\n";
    std::cout << "    Linux:   使用epoll (模拟异步, 实际是Reactor)\n";
    std::cout << "    macOS:   使用kqueue (模拟异步)\n\n";

    std::cout << "【Boost.Asio使用示例 (伪代码)】\n\n";

    std::cout << "  #include <boost/asio.hpp>\n";
    std::cout << "  using namespace boost::asio;\n\n";

    std::cout << "  void session(ip::tcp::socket sock) {\n";
    std::cout << "      auto buffer = std::make_shared<std::array<char, 1024>>();\n\n";

    std::cout << "      // 发起异步读\n";
    std::cout << "      sock.async_read_some(\n";
    std::cout << "          buffer(*buffer),\n";
    std::cout << "          [&sock, buffer](error_code ec, size_t len) {\n";
    std::cout << "              if (!ec) {\n";
    std::cout << "                  // 数据已在buffer中, 直接处理\n";
    std::cout << "                  // 发起异步写 (回显)\n";
    std::cout << "                  async_write(sock, buffer(*buffer, len),\n";
    std::cout << "                      [](error_code ec, size_t) { ... });\n";
    std::cout << "              }\n";
    std::cout << "          });\n";
    std::cout << "  }\n\n";

    std::cout << "【C++20协程 + Asio (未来方向)】\n\n";

    std::cout << "  awaitable<void> session(tcp::socket sock) {\n";
    std::cout << "      char data[1024];\n";
    std::cout << "      while (true) {\n";
    std::cout << "          size_t n = co_await sock.async_read_some(\n";
    std::cout << "              buffer(data), use_awaitable);\n";
    std::cout << "          co_await async_write(sock, buffer(data, n),\n";
    std::cout << "              use_awaitable);\n";
    std::cout << "      }\n";
    std::cout << "  }\n\n";

    std::cout << "  协程让异步代码像同步一样直观!\n";
}

// ============================================================
// 主函数
// ============================================================
int main() {
    std::cout << "============================================================\n";
    std::cout << "  第8章: 高性能网络与异步IO - 第3节 Proactor模式概念\n";
    std::cout << "============================================================\n";

    WinsockInit winsock_init;

    demo_proactor_vs_reactor();
    demo_proactor_conceptual();
    demo_boost_asio_proactor();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
