/**
 * @file 01_example_socket_tcp.cpp
 * @brief TCP服务器/客户端: 跨平台Winsock/POSIX, Echo服务器
 * @description 对应文档: 02-CPP/35-网络编程
 *  @note C 语言中使用原始 socket API 实现类似功能, 参见 C 章节 25-网络编程基础
 */

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>

#ifdef _WIN32
    #ifndef _WINSOCK2API_
        #include <winsock2.h>
        #include <ws2tcpip.h>
    #endif
    #pragma comment(lib, "ws2_32.lib")
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
    typedef int socket_t;
    #define INVALID_SOCKET_VAL (-1)
    #define CLOSE_SOCKET close
#endif

class SocketInit {
#ifdef _WIN32
    WSADATA wsa_data_;
#endif
public:
    SocketInit() {
#ifdef _WIN32
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data_) != 0) {
            std::cerr << "WSAStartup失败\n";
        } else {
            std::cout << "Winsock初始化成功\n";
        }
#else
        std::cout << "POSIX socket无需初始化\n";
#endif
    }

    ~SocketInit() {
#ifdef _WIN32
        WSACleanup();
        std::cout << "Winsock清理完成\n";
#endif
    }
};

class TcpSocket {
    socket_t fd_ = INVALID_SOCKET_VAL;

public:
    TcpSocket() = default;
    explicit TcpSocket(socket_t fd) : fd_(fd) {}

    ~TcpSocket() {
        close();
    }

    TcpSocket(const TcpSocket&) = delete;
    TcpSocket& operator=(const TcpSocket&) = delete;

    TcpSocket(TcpSocket&& other) noexcept : fd_(other.fd_) {
        other.fd_ = INVALID_SOCKET_VAL;
    }

    TcpSocket& operator=(TcpSocket&& other) noexcept {
        if (this != &other) {
            close();
            fd_ = other.fd_;
            other.fd_ = INVALID_SOCKET_VAL;
        }
        return *this;
    }

    bool create() {
        fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd_ == INVALID_SOCKET_VAL) {
            std::cerr << "创建socket失败\n";
            return false;
        }
        return true;
    }

    bool bind(uint16_t port) {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        int opt = 1;
        setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR,
                   reinterpret_cast<const char*>(&opt), sizeof(opt));

        if (::bind(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
            std::cerr << "bind失败\n";
            return false;
        }
        return true;
    }

    bool listen(int backlog = 5) {
        if (::listen(fd_, backlog) != 0) {
            std::cerr << "listen失败\n";
            return false;
        }
        return true;
    }

    TcpSocket accept() {
        sockaddr_in client_addr{};
        socklen_t addr_len = sizeof(client_addr);
        socket_t client_fd = ::accept(fd_, reinterpret_cast<sockaddr*>(&client_addr), &addr_len);
        if (client_fd == INVALID_SOCKET_VAL) {
            return TcpSocket();
        }
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
        std::cout << "  接受连接: " << ip << ":" << ntohs(client_addr.sin_port) << "\n";
        return TcpSocket(client_fd);
    }

    bool connect(const std::string& host, uint16_t port) {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);

        if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1) {
            std::cerr << "地址解析失败: " << host << "\n";
            return false;
        }

        if (::connect(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
            std::cerr << "connect失败\n";
            return false;
        }
        return true;
    }

    ssize_t send(const std::string& data) {
        return ::send(fd_, data.c_str(), static_cast<int>(data.size()), 0);
    }

    std::string recv(size_t max_size = 4096) {
        std::vector<char> buffer(max_size);
        ssize_t n = ::recv(fd_, buffer.data(), static_cast<int>(max_size - 1), 0);
        if (n <= 0) return "";
        buffer[n] = '\0';
        return std::string(buffer.data(), n);
    }

    void close() {
        if (fd_ != INVALID_SOCKET_VAL) {
            CLOSE_SOCKET(fd_);
            fd_ = INVALID_SOCKET_VAL;
        }
    }

    bool is_valid() const { return fd_ != INVALID_SOCKET_VAL; }
    socket_t native_handle() const { return fd_; }
};

void demo_echo_server_client() {
    std::cout << "\n=== demo_echo_server_client ===\n";
    std::cout << "TCP Echo服务器与客户端\n\n";

    SocketInit sock_init;

    const uint16_t port = 12345;

    TcpSocket server_sock;
    if (!server_sock.create()) return;
    if (!server_sock.bind(port)) return;
    if (!server_sock.listen()) return;
    std::cout << "Echo服务器监听端口: " << port << "\n";

    std::atomic<bool> server_done{false};

    std::thread server_thread([&]() {
        TcpSocket client = server_sock.accept();
        if (client.is_valid()) {
            for (int i = 0; i < 3; ++i) {
                std::string msg = client.recv();
                if (msg.empty()) break;
                std::cout << "  [服务器] 收到: \"" << msg << "\"\n";
                client.send(msg);
                std::cout << "  [服务器] 回显: \"" << msg << "\"\n";
            }
            client.close();
        }
        server_done = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    TcpSocket client_sock;
    if (!client_sock.create()) {
        server_sock.close();
        server_thread.join();
        return;
    }

    if (client_sock.connect("127.0.0.1", port)) {
        std::cout << "客户端连接成功\n";

        std::vector<std::string> messages = {"Hello, Echo!", "C++网络编程", "Goodbye!"};
        for (const auto& msg : messages) {
            client_sock.send(msg);
            std::cout << "  [客户端] 发送: \"" << msg << "\"\n";

            std::string echo = client_sock.recv();
            std::cout << "  [客户端] 收到回显: \"" << echo << "\"\n";
        }
    } else {
        std::cout << "客户端连接失败 (可能服务器未就绪)\n";
    }

    client_sock.close();
    server_thread.join();
    server_sock.close();

    std::cout << "\nEcho服务器/客户端演示完成\n";
}

void demo_socket_options() {
    std::cout << "\n=== demo_socket_options ===\n";
    std::cout << "Socket选项设置\n\n";

    SocketInit sock_init;

    TcpSocket sock;
    sock.create();

    std::cout << "常用Socket选项:\n\n";

    std::cout << "1. SO_REUSEADDR: 允许地址重用\n";
    std::cout << "   避免TIME_WAIT状态导致bind失败\n";
    std::cout << "   服务器必须设置\n\n";

    std::cout << "2. SO_KEEPALIVE: 保持连接活跃\n";
    std::cout << "   自动检测死连接\n";
    std::cout << "   默认2小时检测一次(可调整)\n\n";

    std::cout << "3. TCP_NODELAY: 禁用Nagle算法\n";
    std::cout << "   减少小包延迟\n";
    std::cout << "   实时应用必须设置\n\n";

    std::cout << "4. SO_RCVBUF/SO_SNDBUF: 收发缓冲区大小\n";
    std::cout << "   根据带宽延迟积调整\n\n";

    std::cout << "5. SO_RCVTIMEO/SO_SNDTIMEO: 收发超时\n";
    std::cout << "   防止无限阻塞\n";

    int optval = 1;
    setsockopt(sock.native_handle(), IPPROTO_TCP, TCP_NODELAY,
               reinterpret_cast<const char*>(&optval), sizeof(optval));
    std::cout << "\n已设置 TCP_NODELAY\n";

    setsockopt(sock.native_handle(), SOL_SOCKET, SO_KEEPALIVE,
               reinterpret_cast<const char*>(&optval), sizeof(optval));
    std::cout << "已设置 SO_KEEPALIVE\n";

    int bufsize = 65536;
    setsockopt(sock.native_handle(), SOL_SOCKET, SO_RCVBUF,
               reinterpret_cast<const char*>(&bufsize), sizeof(bufsize));
    std::cout << "已设置 SO_RCVBUF=65536\n";
}

void demo_async_io_concept() {
    std::cout << "\n=== demo_async_io_concept ===\n";
    std::cout << "异步I/O概念\n\n";

    std::cout << "同步I/O vs 异步I/O:\n";
    std::cout << "  同步: 调用阻塞直到操作完成\n";
    std::cout << "  异步: 调用立即返回, 操作完成后通知\n\n";

    std::cout << "异步I/O模型演进:\n\n";

    std::cout << "1. select:\n";
    std::cout << "   最多1024个文件描述符\n";
    std::cout << "   每次调用需要遍历所有fd\n";
    std::cout << "   跨平台, 但性能差\n\n";

    std::cout << "2. poll:\n";
    std::cout << "   无fd数量限制\n";
    std::cout << "   仍需遍历所有fd\n";
    std::cout << "   性能略优于select\n\n";

    std::cout << "3. epoll (Linux):\n";
    std::cout << "   事件驱动, 只返回就绪的fd\n";
    std::cout << "   O(1)复杂度(就绪fd数量)\n";
    std::cout << "   支持边缘触发(ET)和水平触发(LT)\n";
    std::cout << "   C10K问题的解决方案\n\n";

    std::cout << "4. IOCP (Windows):\n";
    std::cout << "   完成端口, 真正的异步I/O\n";
    std::cout << "   线程池 + 完成通知\n";
    std::cout << "   Windows上最高效的I/O模型\n\n";

    std::cout << "5. io_uring (Linux 5.1+):\n";
    std::cout << "   共享环形缓冲区\n";
    std::cout << "   无需系统调用提交请求\n";
    std::cout << "   最新的高性能I/O方案\n\n";

    std::cout << "C++跨平台方案:\n";
    std::cout << "  Boost.Asio: 统一异步I/O接口\n";
    std::cout << "  libuv: Node.js底层库\n";
    std::cout << "  POCO: 网络应用框架\n";
    std::cout << "  建议生产环境使用Boost.Asio\n";
}

int main() {
    std::cout << "TCP网络编程演示\n";

    demo_echo_server_client();
    demo_socket_options();
    demo_async_io_concept();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
