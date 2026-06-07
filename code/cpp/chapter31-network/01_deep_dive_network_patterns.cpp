/**
 * @file 01_deep_dive_network_patterns.cpp
 * @brief Reactor模式, Proactor概念, 异步I/O模型, 连接管理
 * @description 对应文档: 02-CPP/35-网络编程
 *  @note C 语言中使用原始 socket API 实现类似功能, 参见 C 章节 25-网络编程基础
 */

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <map>
#include <mutex>
#include <chrono>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET socket_t;
    #define INVALID_SOCKET_VAL INVALID_SOCKET
    #define CLOSE_SOCKET closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <sys/select.h>
    #include <fcntl.h>
    typedef int socket_t;
    #define INVALID_SOCKET_VAL (-1)
    #define CLOSE_SOCKET close
#endif

void demo_reactor_pattern() {
    std::cout << "\n=== demo_reactor_pattern ===\n";
    std::cout << "Reactor模式 (反应器模式)\n\n";

    std::cout << "Reactor模式组成:\n";
    std::cout << "  1. Handle (句柄): 文件描述符/socket\n";
    std::cout << "  2. Synchronous Event Demultiplexer: select/poll/epoll\n";
    std::cout << "  3. Event Handler: 事件处理接口\n";
    std::cout << "  4. Concrete Event Handler: 具体事件处理\n";
    std::cout << "  5. Initiation Dispatcher: 事件循环和分发\n\n";

    std::cout << "Reactor工作流程:\n";
    std::cout << "  1. 注册事件处理器到Reactor\n";
    std::cout << "  2. Reactor调用事件多路分离器等待事件\n";
    std::cout << "  3. 事件就绪时, Reactor回调对应处理器\n";
    std::cout << "  4. 处理器执行非阻塞I/O操作\n\n";

    class SimpleReactor {
    public:
        using EventHandler = std::function<void(socket_t)>;

        void register_read(socket_t fd, EventHandler handler) {
            std::lock_guard<std::mutex> lock(mutex_);
            read_handlers_[fd] = handler;
        }

        void unregister(socket_t fd) {
            std::lock_guard<std::mutex> lock(mutex_);
            read_handlers_.erase(fd);
        }

        void event_loop_once(int timeout_ms) {
            fd_set read_fds;
            FD_ZERO(&read_fds);
            socket_t max_fd = 0;

            {
                std::lock_guard<std::mutex> lock(mutex_);
                for (auto& [fd, _] : read_handlers_) {
                    FD_SET(fd, &read_fds);
                    if (fd > max_fd) max_fd = fd;
                }
            }

            timeval tv;
            tv.tv_sec = timeout_ms / 1000;
            tv.tv_usec = (timeout_ms % 1000) * 1000;

            int result = select(static_cast<int>(max_fd + 1), &read_fds, nullptr, nullptr, &tv);
            if (result > 0) {
                std::lock_guard<std::mutex> lock(mutex_);
                for (auto& [fd, handler] : read_handlers_) {
                    if (FD_ISSET(fd, &read_fds)) {
                        handler(fd);
                    }
                }
            }
        }

    private:
        std::map<socket_t, EventHandler> read_handlers_;
        std::mutex mutex_;
    };

    std::cout << "Reactor模式示例 (概念演示):\n";
    SimpleReactor reactor;
    std::cout << "  Reactor创建成功\n";
    std::cout << "  可注册socket读事件处理器\n";
    std::cout << "  事件循环使用select等待就绪事件\n";
    std::cout << "  就绪后回调对应处理器\n\n";

    std::cout << "Reactor模式优缺点:\n";
    std::cout << "  优点: 非阻塞, 单线程处理多连接, 响应快\n";
    std::cout << "  缺点: 仍需自己读写数据, 复杂逻辑难处理\n";
    std::cout << "  代表: Redis, Nginx, Netty (NIO模式)\n";
}

void demo_proactor_concept() {
    std::cout << "\n=== demo_proactor_concept ===\n";
    std::cout << "Proactor模式 (前摄器模式)\n\n";

    std::cout << "Proactor vs Reactor:\n";
    std::cout << "  Reactor: 事件就绪时通知, 用户自己读写\n";
    std::cout << "  Proactor: 读写完成后通知, 由系统完成I/O\n\n";

    std::cout << "Proactor工作流程:\n";
    std::cout << "  1. 发起异步读/写操作\n";
    std::cout << "  2. 系统在后台执行I/O操作\n";
    std::cout << "  3. I/O完成后, 系统通知完成处理器\n";
    std::cout << "  4. 完成处理器处理结果\n\n";

    std::cout << "平台实现:\n";
    std::cout << "  Windows: IOCP (真正的异步I/O)\n";
    std::cout << "  Linux: io_uring (接近真正的异步I/O)\n";
    std::cout << "  Boost.Asio: 在Linux上用epoll模拟Proactor\n\n";

    std::cout << "IOCP (I/O Completion Port) 详解:\n";
    std::cout << "  1. CreateIoCompletionPort: 创建完成端口\n";
    std::cout << "  2. 将socket关联到完成端口\n";
    std::cout << "  3. 发起异步WSARecv/WSASend\n";
    std::cout << "  4. 工作线程调用GetQueuedCompletionStatus等待\n";
    std::cout << "  5. I/O完成后, 线程被唤醒处理结果\n\n";

    std::cout << "Proactor模式优缺点:\n";
    std::cout << "  优点: 真正的异步, 不需要非阻塞I/O技巧\n";
    std::cout << "  缺点: 编程模型复杂, 调试困难\n";
    std::cout << "  代表: Windows IOCP, Boost.Asio\n";
}

void demo_async_io_models() {
    std::cout << "\n=== demo_async_io_models ===\n";
    std::cout << "异步I/O模型详解\n\n";

    std::cout << "模型对比:\n";
    std::cout << "  ┌──────────┬────────────┬──────────┬──────────┬──────────┐\n";
    std::cout << "  │ 模型     │ 平台       │ fd限制   │ 复杂度   │ 性能     │\n";
    std::cout << "  ├──────────┼────────────┼──────────┼──────────┼──────────┤\n";
    std::cout << "  │ select   │ 跨平台     │ 1024     │ 低       │ 低       │\n";
    std::cout << "  │ poll     │ 跨平台     │ 无       │ 低       │ 中       │\n";
    std::cout << "  │ epoll    │ Linux      │ 无       │ 中       │ 高       │\n";
    std::cout << "  │ kqueue   │ BSD/macOS  │ 无       │ 中       │ 高       │\n";
    std::cout << "  │ IOCP     │ Windows    │ 无       │ 高       │ 极高     │\n";
    std::cout << "  │ io_uring │ Linux 5.1+ │ 无       │ 高       │ 极高     │\n";
    std::cout << "  └──────────┴────────────┴──────────┴──────────┴──────────┘\n\n";

    std::cout << "epoll详解:\n";
    std::cout << "  epoll_create: 创建epoll实例\n";
    std::cout << "  epoll_ctl: 添加/修改/删除监听的fd\n";
    std::cout << "  epoll_wait: 等待事件\n\n";

    std::cout << "  LT (水平触发): 缓冲区有数据就一直通知\n";
    std::cout << "    适合: 阻塞I/O, 简单编程\n";
    std::cout << "  ET (边缘触发): 状态变化时才通知\n";
    std::cout << "    适合: 非阻塞I/O, 高性能\n";
    std::cout << "    要求: 必须一次性读完所有数据\n\n";

    std::cout << "io_uring详解:\n";
    std::cout << "  Submission Queue (SQ): 提交I/O请求\n";
    std::cout << "  Completion Queue (CQ): 接收完成通知\n";
    std::cout << "  共享内存环形缓冲区, 无需系统调用\n";
    std::cout << "  支持批量提交和完成\n";
    std::cout << "  liburing: 简化io_uring使用的库\n\n";

    std::cout << "C++跨平台选择:\n";
    std::cout << "  小项目: select (最简单)\n";
    std::cout << "  中项目: Boost.Asio (跨平台)\n";
    std::cout << "  大项目: 平台特定 + 抽象层\n";
}

void demo_connection_management() {
    std::cout << "\n=== demo_connection_management ===\n";
    std::cout << "连接管理\n\n";

    std::cout << "1. 连接建立:\n";
    std::cout << "   三次握手: SYN -> SYN+ACK -> ACK\n";
    std::cout << "   服务器: listen -> accept\n";
    std::cout << "   客户端: connect\n\n";

    std::cout << "2. 连接维护:\n";
    std::cout << "   心跳机制: 定期发送心跳包\n";
    std::cout << "   超时检测: 超时未收到心跳则断开\n";
    std::cout << "   流量控制: 滑动窗口\n";
    std::cout << "   拥塞控制: 慢启动, 拥塞避免\n\n";

    std::cout << "3. 连接关闭:\n";
    std::cout << "   四次挥手: FIN -> ACK -> FIN -> ACK\n";
    std::cout << "   TIME_WAIT: 主动关闭方等待2MSL\n";
    std::cout << "   SO_REUSEADDR: 允许重用TIME_WAIT地址\n";
    std::cout << "   SO_LINGER: 控制关闭行为\n\n";

    std::cout << "4. 半关闭:\n";
    std::cout << "   shutdown(fd, SHUT_WR): 关闭写端\n";
    std::cout << "   仍可接收数据\n";
    std::cout << "   HTTP管线化使用此特性\n\n";

    std::cout << "5. 连接池:\n";
    std::cout << "   预创建连接, 复用连接\n";
    std::cout << "   减少连接建立开销\n";
    std::cout << "   适合短连接频繁的场景\n\n";

    std::cout << "6. 常见问题:\n";
    std::cout << "   TIME_WAIT过多: 用SO_REUSEADDR + 短超时\n";
    std::cout << "   CLOSE_WAIT过多: 检查是否正确关闭socket\n";
    std::cout << "   连接泄漏: 使用RAII管理socket生命周期\n";
    std::cout << "   惊群效应: 多线程accept同一socket\n";

    std::cout << "\n连接管理最佳实践:\n";
    std::cout << "  1. 用RAII包装socket (构造打开, 析构关闭)\n";
    std::cout << "  2. 设置合理的超时时间\n";
    std::cout << "  3. 实现心跳保活机制\n";
    std::cout << "  4. 限制最大连接数\n";
    std::cout << "  5. 监控连接状态指标\n";
}

int main() {
    std::cout << "网络编程模式深入\n";

#ifdef _WIN32
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif

    demo_reactor_pattern();
    demo_proactor_concept();
    demo_async_io_models();
    demo_connection_management();

#ifdef _WIN32
    WSACleanup();
#endif

    std::cout << "\n所有演示完成!\n";
    return 0;
}
