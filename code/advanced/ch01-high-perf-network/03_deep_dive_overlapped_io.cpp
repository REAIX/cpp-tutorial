/**
 * @file 03_deep_dive_overlapped_io.cpp
 * @brief Windows重叠IO基础: WSARecv/WSASend与OVERLAPPED结构体
 * @description 对应文档: 高性能网络与异步IO / 第3节 Proactor模式与异步IO
 *
 * 本文件演示Windows重叠IO(Overlapped I/O)的基本用法:
 *   - OVERLAPPED结构体
 *   - WSARecv/WSASend异步操作
 *   - WSAGetOverlappedResult获取结果
 *
 * 注意: 重叠IO是Windows特有的异步IO机制, 是IOCP的基础。
 */

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <functional>

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
// 演示1: OVERLAPPED结构体详解
// ============================================================
void demo_overlapped_concepts() {
    std::cout << "\n========================================\n";
    std::cout << "  演示1: OVERLAPPED结构体详解\n";
    std::cout << "========================================\n\n";

#ifdef _WIN32
    std::cout << "【OVERLAPPED结构体定义】\n\n";

    std::cout << "  typedef struct _OVERLAPPED {\n";
    std::cout << "      ULONG_PTR Internal;      // 操作系统使用(状态码)\n";
    std::cout << "      ULONG_PTR InternalHigh;  // 操作系统使用(传输字节数)\n";
    std::cout << "      union {\n";
    std::cout << "          struct {\n";
    std::cout << "              DWORD Offset;    // 文件偏移量低32位\n";
    std::cout << "              DWORD OffsetHigh;// 文件偏移量高32位\n";
    std::cout << "          };\n";
    std::cout << "          PVOID Pointer;       // 保留\n";
    std::cout << "      };\n";
    std::cout << "      HANDLE hEvent;           // 事件对象(非IOCP时使用)\n";
    std::cout << "  } OVERLAPPED;\n\n";

    std::cout << "【OVERLAPPED的作用】\n\n";

    std::cout << "  1. 标识一次异步操作\n";
    std::cout << "     每个异步操作都需要一个独立的OVERLAPPED结构\n";
    std::cout << "     操作系统通过它跟踪操作状态\n\n";

    std::cout << "  2. 扩展为自定义结构\n";
    std::cout << "     可以将OVERLAPPED作为自定义结构的第一个成员:\n\n";

    std::cout << "     struct MyOverlapped {\n";
    std::cout << "         OVERLAPPED overlapped;  // 必须是第一个成员!\n";
    std::cout << "         WSABUF wsa_buf;         // 数据缓冲区\n";
    std::cout << "         int operation_type;     // 操作类型\n";
    std::cout << "         socket_t client_fd;     // 关联的socket\n";
    std::cout << "         // ... 其他自定义字段\n";
    std::cout << "     };\n\n";

    std::cout << "  3. 与IOCP配合\n";
    std::cout << "     IOCP通过OVERLAPPED识别完成的操作\n";
    std::cout << "     GetQueuedCompletionStatus返回OVERLAPPED指针\n";
    std::cout << "     通过CONTAINING_RECORD宏获取自定义结构\n\n";

    std::cout << "【WSABUF结构体】\n\n";

    std::cout << "  typedef struct _WSABUF {\n";
    std::cout << "      ULONG len;   // 缓冲区长度\n";
    std::cout << "      CHAR FAR *buf; // 缓冲区指针\n";
    std::cout << "  } WSABUF;\n\n";

    std::cout << "  与POSIX的iov结构类似, 用于scatter/gather IO\n";
#else
    std::cout << "【OVERLAPPED是Windows特有机制】\n\n";

    std::cout << "  当前平台非Windows, 以下是概念说明:\n\n";

    std::cout << "  OVERLAPPED结构体:\n";
    std::cout << "    - Windows异步IO的核心数据结构\n";
    std::cout << "    - 标识一次异步操作\n";
    std::cout << "    - 可扩展为自定义结构(放第一个成员)\n\n";

    std::cout << "  WSABUF结构体:\n";
    std::cout << "    - 类似POSIX的iovec\n";
    std::cout << "    - 用于scatter/gather IO\n";
    std::cout << "    - {len, buf} 二元组\n";
#endif
}

// ============================================================
// 演示2: 重叠IO基本操作 (Windows实际代码)
// ============================================================
void demo_overlapped_io() {
    std::cout << "\n========================================\n";
    std::cout << "  演示2: 重叠IO基本操作\n";
    std::cout << "========================================\n\n";

#ifdef _WIN32
    std::cout << "[信息] 当前平台: Windows, 演示真实重叠IO\n\n";

    // 创建服务器和客户端在同一进程演示
    const uint16_t port = 15006;

    // --- 服务器端 ---
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

    if (::listen(listen_fd, 5) != 0) {
        std::cerr << "[错误] listen失败\n";
        CLOSE_SOCKET(listen_fd);
        return;
    }

    std::cout << "[服务器] 重叠IO服务器启动, 端口: " << port << "\n";

    // 接受一个连接
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    socket_t client_fd = ::accept(listen_fd,
        reinterpret_cast<sockaddr*>(&client_addr), &client_len);

    if (client_fd == INVALID_SOCKET_VAL) {
        std::cerr << "[错误] accept失败\n";
        CLOSE_SOCKET(listen_fd);
        return;
    }

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
    std::cout << "[服务器] 接受连接: " << ip << ":"
              << ntohs(client_addr.sin_port) << "\n\n";

    // === 使用重叠IO进行异步读取 ===

    // 1. 创建事件对象 (非IOCP模式需要)
    HANDLE hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    if (hEvent == nullptr) {
        std::cerr << "[错误] CreateEvent失败\n";
        CLOSE_SOCKET(client_fd);
        CLOSE_SOCKET(listen_fd);
        return;
    }

    // 2. 准备OVERLAPPED结构
    OVERLAPPED overlapped{};
    overlapped.hEvent = hEvent;

    // 3. 准备接收缓冲区
    char recv_buf[1024];
    WSABUF wsa_buf{};
    wsa_buf.buf = recv_buf;
    wsa_buf.len = sizeof(recv_buf);

    // 4. 发起异步接收 (WSARecv)
    DWORD flags = 0;
    DWORD bytes_recv = 0;

    std::cout << "[服务器] 发起WSARecv异步读取...\n";
    int recv_ret = WSARecv(client_fd, &wsa_buf, 1, &bytes_recv, &flags,
                           &overlapped, nullptr);

    if (recv_ret == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err == WSA_IO_PENDING) {
            std::cout << "[服务器] WSA_IO_PENDING: 异步操作正在进行中\n";
            // 这是正常的! 异步操作已提交, 等待完成
        } else {
            std::cerr << "[错误] WSARecv失败, 错误码: " << err << "\n";
            CloseHandle(hEvent);
            CLOSE_SOCKET(client_fd);
            CLOSE_SOCKET(listen_fd);
            return;
        }
    } else {
        // 操作立即完成 (数据已在缓冲区中)
        std::cout << "[服务器] WSARecv立即完成, 收到 "
                  << bytes_recv << " 字节\n";
    }

    // --- 客户端发送数据 ---
    std::thread client_thread([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        socket_t sock = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET_VAL) return;

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

        if (::connect(sock, reinterpret_cast<sockaddr*>(&server_addr),
                      sizeof(server_addr)) == 0) {
            std::string msg = "Hello, Overlapped IO!";
            ::send(sock, msg.c_str(), static_cast<int>(msg.size()), 0);
            std::cout << "[客户端] 发送: \"" << msg << "\"\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        CLOSE_SOCKET(sock);
    });

    // 5. 等待异步操作完成
    std::cout << "[服务器] 等待异步操作完成 (WSAWaitForMultipleEvents)...\n";
    DWORD wait_ret = WSAWaitForMultipleEvents(1, &hEvent, TRUE, 5000, FALSE);

    if (wait_ret == WSA_WAIT_EVENT_0) {
        std::cout << "[服务器] 事件触发, 异步操作已完成\n";

        // 6. 获取异步操作结果
        DWORD bytes_transferred = 0;
        DWORD dwFlags = 0;
        BOOL result = WSAGetOverlappedResult(client_fd, &overlapped,
                                             &bytes_transferred, FALSE, &dwFlags);

        if (result && bytes_transferred > 0) {
            recv_buf[bytes_transferred] = '\0';
            std::cout << "[服务器] 异步读取完成: \"" << recv_buf
                      << "\" (" << bytes_transferred << " 字节)\n";

            // 使用重叠IO异步发送回显
            OVERLAPPED send_overlapped{};
            send_overlapped.hEvent = hEvent;
            ResetEvent(hEvent);

            WSABUF send_buf{};
            send_buf.buf = recv_buf;
            send_buf.len = bytes_transferred;

            DWORD bytes_sent = 0;
            int send_ret = WSASend(client_fd, &send_buf, 1, &bytes_sent, 0,
                                   &send_overlapped, nullptr);

            if (send_ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
                std::cerr << "[错误] WSASend失败\n";
            } else {
                // 等待发送完成
                WSAWaitForMultipleEvents(1, &hEvent, TRUE, 3000, FALSE);
                WSAGetOverlappedResult(client_fd, &send_overlapped,
                                      &bytes_sent, FALSE, &dwFlags);
                std::cout << "[服务器] 异步发送完成: " << bytes_sent << " 字节\n";
            }
        } else {
            std::cerr << "[错误] WSAGetOverlappedResult失败\n";
        }
    } else if (wait_ret == WSA_WAIT_TIMEOUT) {
        std::cout << "[服务器] 等待超时\n";
    } else {
        std::cerr << "[错误] WSAWaitForMultipleEvents失败\n";
    }

    client_thread.join();
    CloseHandle(hEvent);
    CLOSE_SOCKET(client_fd);
    CLOSE_SOCKET(listen_fd);

#else
    std::cout << "[信息] 当前平台非Windows, 展示重叠IO概念代码\n\n";

    std::cout << "【Windows重叠IO代码示例】\n\n";

    std::cout << "  // 1. 创建事件对象\n";
    std::cout << "  HANDLE hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);\n\n";

    std::cout << "  // 2. 初始化OVERLAPPED\n";
    std::cout << "  OVERLAPPED overlapped = {};\n";
    std::cout << "  overlapped.hEvent = hEvent;\n\n";

    std::cout << "  // 3. 准备缓冲区\n";
    std::cout << "  char buf[1024];\n";
    std::cout << "  WSABUF wsa_buf = {sizeof(buf), buf};\n\n";

    std::cout << "  // 4. 发起异步读\n";
    std::cout << "  DWORD flags = 0, bytes_recv = 0;\n";
    std::cout << "  int ret = WSARecv(fd, &wsa_buf, 1,\n";
    std::cout << "                    &bytes_recv, &flags, &overlapped, nullptr);\n\n";

    std::cout << "  // 5. 检查是否立即完成\n";
    std::cout << "  if (ret == SOCKET_ERROR) {\n";
    std::cout << "      if (WSAGetLastError() == WSA_IO_PENDING) {\n";
    std::cout << "          // 异步进行中, 等待完成\n";
    std::cout << "          WSAWaitForMultipleEvents(1, &hEvent, TRUE, INFINITE, FALSE);\n";
    std::cout << "      }\n";
    std::cout << "  }\n\n";

    std::cout << "  // 6. 获取结果\n";
    std::cout << "  DWORD bytes_transferred = 0;\n";
    std::cout << "  WSAGetOverlappedResult(fd, &overlapped,\n";
    std::cout << "                        &bytes_transferred, FALSE, &flags);\n\n";

    std::cout << "【Linux等价方案】\n";
    std::cout << "  - epoll + 非阻塞IO (Reactor模式, 非真正异步)\n";
    std::cout << "  - io_uring (Linux 5.1+, 真正异步, 类似重叠IO)\n";
    std::cout << "  - Boost.Asio在Linux上使用epoll模拟Proactor\n";
#endif
}

// ============================================================
// 演示3: 重叠IO → IOCP的演进
// ============================================================
void demo_overlapped_to_iocp() {
    std::cout << "\n========================================\n";
    std::cout << "  演示3: 重叠IO → IOCP的演进\n";
    std::cout << "========================================\n\n";

    std::cout << "【重叠IO的三种通知方式】\n\n";

    std::cout << "  1. 事件对象 (本节演示)\n";
    std::cout << "     每个OVERLAPPED关联一个事件\n";
    std::cout << "     WSAWaitForMultipleEvents等待\n";
    std::cout << "     缺点: 最多等待64个事件(WSA_WAIT_MAXIMUM_OBJECTS)\n\n";

    std::cout << "  2. 完成例程 (Completion Routine)\n";
    std::cout << "     WSARecv的最后一个参数传入回调函数\n";
    std::cout << "     WSAWaitForMultipleEventsEx触发回调\n";
    std::cout << "     缺点: 回调在等待线程执行, 不灵活\n\n";

    std::cout << "  3. 完成端口 (IOCP) ← 推荐!\n";
    std::cout << "     CreateIoCompletionPort创建完成端口\n";
    std::cout << "     将socket与完成端口关联\n";
    std::cout << "     GetQueuedCompletionStatus获取完成通知\n";
    std::cout << "     优点: 无连接数限制, 多线程处理, 最高性能\n\n";

    std::cout << "【IOCP编程模型】\n\n";

    std::cout << "  // 1. 创建完成端口\n";
    std::cout << "  HANDLE iocp = CreateIoCompletionPort(\n";
    std::cout << "      INVALID_HANDLE_VALUE, nullptr, 0, 0);\n\n";

    std::cout << "  // 2. 将socket关联到完成端口\n";
    std::cout << "  CreateIoCompletionPort(\n";
    std::cout << "      (HANDLE)client_fd, iocp,\n";
    std::cout << "      (ULONG_PTR)completion_key,  // 自定义标识\n";
    std::cout << "      0);\n\n";

    std::cout << "  // 3. 发起异步操作\n";
    std::cout << "  WSARecv(fd, &wsa_buf, 1, &bytes, &flags,\n";
    std::cout << "          &overlapped, nullptr);  // 不需要事件!\n\n";

    std::cout << "  // 4. 工作线程等待完成\n";
    std::cout << "  DWORD bytes_transferred;\n";
    std::cout << "  ULONG_PTR completion_key;\n";
    std::cout << "  OVERLAPPED* overlapped;\n";
    std::cout << "  GetQueuedCompletionStatus(iocp,\n";
    std::cout << "      &bytes_transferred, &completion_key,\n";
    std::cout << "      &overlapped, INFINITE);\n\n";

    std::cout << "  // 5. 处理完成结果\n";
    std::cout << "  // completion_key标识连接\n";
    std::cout << "  // overlapped标识操作\n";
    std::cout << "  // bytes_transferred是传输字节数\n\n";

    std::cout << "【IOCP最佳实践】\n\n";

    std::cout << "  1. 工作线程数 = CPU核心数 × 2\n";
    std::cout << "     多出的线程用于处理可能的阻塞操作\n\n";

    std::cout << "  2. 使用扩展OVERLAPPED结构\n";
    std::cout << "     包含操作类型、缓冲区等上下文信息\n\n";

    std::cout << "  3. 投递多个recv请求\n";
    std::cout << "     提高吞吐量, 减少IO延迟\n\n";

    std::cout << "  4. 避免在工作线程中执行耗时操作\n";
    std::cout << "     耗时操作应投递到独立线程池\n";
}

// ============================================================
// 主函数
// ============================================================
int main() {
    std::cout << "============================================================\n";
    std::cout << "  第8章: 高性能网络与异步IO - 第3节 Windows重叠IO\n";
    std::cout << "============================================================\n";

    WinsockInit winsock_init;

    demo_overlapped_concepts();
    demo_overlapped_io();
    demo_overlapped_to_iocp();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
