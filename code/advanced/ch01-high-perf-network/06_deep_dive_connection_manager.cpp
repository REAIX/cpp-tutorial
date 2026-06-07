/**
 * @file 06_deep_dive_connection_manager.cpp
 * @brief 高并发服务器架构深入: 连接管理, 超时处理, 优雅关闭
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
#include <unordered_map>
#include <mutex>
#include <memory>
#include <algorithm>

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
// 连接状态枚举
// ============================================================
enum class ConnectionState {
    CONNECTING,     // 正在连接
    CONNECTED,      // 已连接
    CLOSING,        // 正在关闭 (等待数据发送完)
    CLOSED,         // 已关闭
};

std::string connection_state_to_string(ConnectionState state) {
    switch (state) {
        case ConnectionState::CONNECTING: return "CONNECTING";
        case ConnectionState::CONNECTED:  return "CONNECTED";
        case ConnectionState::CLOSING:    return "CLOSING";
        case ConnectionState::CLOSED:     return "CLOSED";
    }
    return "UNKNOWN";
}

// ============================================================
// 连接信息
// ============================================================
struct ConnectionInfo {
    socket_t fd;
    std::string remote_ip;
    uint16_t remote_port;
    ConnectionState state;
    std::chrono::steady_clock::time_point connect_time;    // 连接时间
    std::chrono::steady_clock::time_point last_active_time; // 最后活跃时间
    uint64_t bytes_received;
    uint64_t bytes_sent;
    int id;

    ConnectionInfo()
        : fd(INVALID_SOCKET_VAL), remote_port(0),
          state(ConnectionState::CLOSED),
          bytes_received(0), bytes_sent(0), id(0) {}

    /// 检查连接是否超时
    bool is_idle_timeout(int timeout_seconds) const {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - last_active_time).count();
        return elapsed > timeout_seconds;
    }

    /// 获取连接持续时间(秒)
    int64_t duration_seconds() const {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::seconds>(
            now - connect_time).count();
    }

    /// 更新活跃时间
    void touch() {
        last_active_time = std::chrono::steady_clock::now();
    }
};

// ============================================================
// 连接管理器
// ============================================================
class ConnectionManager {
public:
    ConnectionManager() = default;

    /// 添加新连接
    int add_connection(socket_t fd, const std::string& ip, uint16_t port) {
        std::lock_guard<std::mutex> lock(mutex_);

        int id = next_id_++;
        ConnectionInfo info;
        info.fd = fd;
        info.remote_ip = ip;
        info.remote_port = port;
        info.state = ConnectionState::CONNECTED;
        info.connect_time = std::chrono::steady_clock::now();
        info.last_active_time = info.connect_time;
        info.id = id;

        connections_[fd] = std::move(info);

        std::cout << "[连接管理器] 新连接 #" << id
                  << ": " << ip << ":" << port
                  << " (当前连接数: " << connections_.size() << ")\n";
        return id;
    }

    /// 移除连接
    void remove_connection(socket_t fd) {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = connections_.find(fd);
        if (it != connections_.end()) {
            it->second.state = ConnectionState::CLOSED;
            std::cout << "[连接管理器] 连接 #" << it->second.id
                      << " 关闭 (存活: " << it->second.duration_seconds() << "秒"
                      << ", 收: " << it->second.bytes_received
                      << "字节, 发: " << it->second.bytes_sent << "字节)\n";
            CLOSE_SOCKET(fd);
            connections_.erase(it);
        }
    }

    /// 更新接收统计
    void on_data_received(socket_t fd, size_t bytes) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = connections_.find(fd);
        if (it != connections_.end()) {
            it->second.bytes_received += bytes;
            it->second.touch();
        }
    }

    /// 更新发送统计
    void on_data_sent(socket_t fd, size_t bytes) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = connections_.find(fd);
        if (it != connections_.end()) {
            it->second.bytes_sent += bytes;
            it->second.touch();
        }
    }

    /// 标记连接为关闭中 (优雅关闭)
    void mark_closing(socket_t fd) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = connections_.find(fd);
        if (it != connections_.end()) {
            it->second.state = ConnectionState::CLOSING;
        }
    }

    /// 检查并关闭超时连接
    int check_timeouts(int timeout_seconds) {
        std::lock_guard<std::mutex> lock(mutex_);

        std::vector<socket_t> timeout_fds;
        for (auto& [fd, info] : connections_) {
            if (info.is_idle_timeout(timeout_seconds)) {
                timeout_fds.push_back(fd);
            }
        }

        for (socket_t fd : timeout_fds) {
            auto& info = connections_[fd];
            std::cout << "[连接管理器] 连接 #" << info.id
                      << " 超时 (空闲 " << timeout_seconds << "秒)\n";
            info.state = ConnectionState::CLOSED;
            CLOSE_SOCKET(fd);
            connections_.erase(fd);
        }

        return static_cast<int>(timeout_fds.size());
    }

    /// 优雅关闭所有连接
    void graceful_shutdown(int wait_seconds = 5) {
        std::cout << "[连接管理器] 开始优雅关闭...\n";

        std::lock_guard<std::mutex> lock(mutex_);

        // 1. 标记所有连接为CLOSING
        for (auto& [fd, info] : connections_) {
            if (info.state == ConnectionState::CONNECTED) {
                info.state = ConnectionState::CLOSING;
                // 调用shutdown通知对端不再发送数据
#ifdef _WIN32
                ::shutdown(fd, SD_SEND);
#else
                ::shutdown(fd, SHUT_WR);
#endif
            }
        }

        // 2. 等待数据发送完成 (简化: 直接等待)
        std::cout << "[连接管理器] 等待 " << wait_seconds
                  << " 秒让数据发送完成...\n";

        // 3. 关闭所有连接
        for (auto& [fd, info] : connections_) {
            std::cout << "[连接管理器] 关闭连接 #" << info.id << "\n";
            info.state = ConnectionState::CLOSED;
            CLOSE_SOCKET(fd);
        }
        connections_.clear();

        std::cout << "[连接管理器] 优雅关闭完成\n";
    }

    /// 获取当前连接数
    size_t connection_count() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return connections_.size();
    }

    /// 打印所有连接状态
    void print_status() const {
        std::lock_guard<std::mutex> lock(mutex_);

        std::cout << "\n[连接管理器] 当前连接数: " << connections_.size() << "\n";
        if (connections_.empty()) return;

        std::cout << "  ID   状态         远程地址           存活  收/发字节\n";
        std::cout << "  ─────────────────────────────────────────────────\n";

        for (auto& [fd, info] : connections_) {
            std::cout << "  #" << std::setw(3) << info.id
                      << " " << std::setw(11) << connection_state_to_string(info.state)
                      << " " << std::setw(18) << (info.remote_ip + ":" + std::to_string(info.remote_port))
                      << " " << std::setw(4) << info.duration_seconds() << "秒"
                      << " " << info.bytes_received << "/" << info.bytes_sent
                      << "\n";
        }
    }

private:
    mutable std::mutex mutex_;
    std::unordered_map<socket_t, ConnectionInfo> connections_;
    int next_id_ = 1;
};

// ============================================================
// 演示1: 连接管理
// ============================================================
void demo_connection_management() {
    std::cout << "\n========================================\n";
    std::cout << "  演示1: 连接管理\n";
    std::cout << "========================================\n\n";

    std::cout << "【连接管理器的职责】\n\n";

    std::cout << "  1. 连接生命周期管理\n";
    std::cout << "     - 记录连接建立/关闭时间\n";
    std::cout << "     - 维护连接状态机\n\n";

    std::cout << "  2. 连接统计\n";
    std::cout << "     - 收发字节数\n";
    std::cout << "     - 连接持续时间\n";
    std::cout << "     - 当前在线连接数\n\n";

    std::cout << "  3. 超时检测\n";
    std::cout << "     - 空闲超时: 长时间无数据交互\n";
    std::cout << "     - 连接超时: 连接建立后无认证\n\n";

    std::cout << "  4. 优雅关闭\n";
    std::cout << "     - 通知对端不再发送数据\n";
    std::cout << "     - 等待对端确认\n";
    std::cout << "     - 确保数据发送完成\n\n";

    std::cout << "【连接状态机】\n\n";

    std::cout << "  CONNECTING ──→ CONNECTED ──→ CLOSING ──→ CLOSED\n";
    std::cout << "                   │                        ↑\n";
    std::cout << "                   └──── 超时/错误 ─────────┘\n\n";

    std::cout << "  CONNECTING: TCP三次握手中\n";
    std::cout << "  CONNECTED:  连接已建立, 可收发数据\n";
    std::cout << "  CLOSING:    优雅关闭中, 等待数据发送完\n";
    std::cout << "  CLOSED:     连接已关闭\n";
}

// ============================================================
// 演示2: 超时处理
// ============================================================
void demo_timeout_handling() {
    std::cout << "\n========================================\n";
    std::cout << "  演示2: 超时处理\n";
    std::cout << "========================================\n\n";

    const uint16_t port = 15008;
    const int idle_timeout = 3;  // 3秒空闲超时

    ConnectionManager conn_mgr;

    // 创建服务器
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

    std::cout << "[服务器] 超时检测服务器启动, 端口: " << port
              << ", 空闲超时: " << idle_timeout << "秒\n\n";

    std::atomic<bool> server_done{false};

    std::thread server_thread([&]() {
        while (!server_done) {
            // 非阻塞accept
            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(listen_fd, &read_fds);

            struct timeval tv{};
            tv.tv_sec = 0;
            tv.tv_usec = 500000;  // 500ms

            int ret = ::select(static_cast<int>(listen_fd + 1),
                               &read_fds, nullptr, nullptr, &tv);

            if (ret > 0 && FD_ISSET(listen_fd, &read_fds)) {
                sockaddr_in client_addr{};
                socklen_t client_len = sizeof(client_addr);
                socket_t client_fd = ::accept(listen_fd,
                    reinterpret_cast<sockaddr*>(&client_addr), &client_len);

                if (client_fd != INVALID_SOCKET_VAL) {
                    char ip[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
                    conn_mgr.add_connection(client_fd, ip, ntohs(client_addr.sin_port));
                }
            }

            // 检查超时连接
            int closed = conn_mgr.check_timeouts(idle_timeout);
            if (closed > 0) {
                std::cout << "[服务器] 关闭了 " << closed << " 个超时连接\n";
            }
        }
    });

    // 客户端1: 发送数据后保持连接 (不应超时)
    std::thread client1([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        socket_t sock = ::socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

        if (::connect(sock, reinterpret_cast<sockaddr*>(&server_addr),
                      sizeof(server_addr)) == 0) {
            std::cout << "[客户端1] 连接成功 (活跃客户端)\n";
            ::send(sock, "Hello", 5, 0);
            conn_mgr.on_data_received(sock, 5);

            // 保持连接2秒后关闭
            std::this_thread::sleep_for(std::chrono::seconds(2));
            CLOSE_SOCKET(sock);
            conn_mgr.remove_connection(sock);
        }
    });

    // 客户端2: 连接后不发送数据 (应超时)
    std::thread client2([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
        socket_t sock = ::socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

        if (::connect(sock, reinterpret_cast<sockaddr*>(&server_addr),
                      sizeof(server_addr)) == 0) {
            std::cout << "[客户端2] 连接成功 (空闲客户端, 将超时)\n";
            conn_mgr.add_connection(sock, "127.0.0.1", 54321);

            // 不发送任何数据, 等待超时
            std::this_thread::sleep_for(std::chrono::seconds(5));
            CLOSE_SOCKET(sock);
        }
    });

    client1.join();
    client2.join();

    std::cout << "\n";
    conn_mgr.print_status();

    server_done = true;
    server_thread.join();
    CLOSE_SOCKET(listen_fd);
    std::cout << "\n";
}

// ============================================================
// 演示3: 优雅关闭
// ============================================================
void demo_graceful_shutdown() {
    std::cout << "========================================\n";
    std::cout << "  演示3: 优雅关闭\n";
    std::cout << "========================================\n\n";

    std::cout << "【优雅关闭 vs 强制关闭】\n\n";

    std::cout << "  强制关闭 (closesocket):\n";
    std::cout << "    - 立即释放资源\n";
    std::cout << "    - 发送缓冲区数据可能丢失\n";
    std::cout << "    - 对端收到RST, 可能误判为错误\n\n";

    std::cout << "  优雅关闭 (shutdown + closesocket):\n";
    std::cout << "    1. 调用shutdown(fd, SHUT_WR) 通知对端不再发数据\n";
    std::cout << "    2. 继续recv()直到返回0 (对端也关闭)\n";
    std::cout << "    3. 最后调用closesocket()\n";
    std::cout << "    - 保证数据完整性\n";
    std::cout << "    - 对端收到FIN, 正常关闭\n\n";

    std::cout << "【优雅关闭流程】\n\n";

    std::cout << "  服务器端:\n";
    std::cout << "    1. 停止accept新连接\n";
    std::cout << "    2. 通知所有连接即将关闭\n";
    std::cout << "    3. 等待正在处理的请求完成\n";
    std::cout << "    4. shutdown(fd, SHUT_WR)\n";
    std::cout << "    5. 等待对端关闭 (recv返回0)\n";
    std::cout << "    6. closesocket(fd)\n\n";

    std::cout << "  客户端:\n";
    std::cout << "    1. 发送完所有数据\n";
    std::cout << "    2. shutdown(fd, SHUT_WR)\n";
    std::cout << "    3. recv()直到返回0\n";
    std::cout << "    4. closesocket(fd)\n\n";

    std::cout << "【TIME_WAIT状态】\n\n";

    std::cout << "  主动关闭方会进入TIME_WAIT状态:\n";
    std::cout << "  - 持续2MSL (通常60秒)\n";
    std::cout << "  - 占用端口号, 大量TIME_WAIT导致端口耗尽\n\n";

    std::cout << "  解决方案:\n";
    std::cout << "  1. SO_REUSEADDR: 允许重用处于TIME_WAIT的地址\n";
    std::cout << "  2. 让客户端主动关闭 (服务器不进入TIME_WAIT)\n";
    std::cout << "  3. 设置SO_LINGER为0 (RST关闭, 不推荐)\n";
    std::cout << "  4. 调整内核参数 (tcp_tw_reuse)\n\n";

    std::cout << "【连接管理的最佳实践】\n\n";

    std::cout << "  1. 限制最大连接数\n";
    std::cout << "     防止资源耗尽, 超过限制直接拒绝\n\n";

    std::cout << "  2. 心跳保活\n";
    std::cout << "     定期发送心跳包检测死连接\n";
    std::cout << "     比SO_KEEPALIVE更灵活\n\n";

    std::cout << "  3. 限流\n";
    std::cout << "     限制单连接的请求速率\n";
    std::cout << "     防止慢速攻击(Slowloris)\n\n";

    std::cout << "  4. 监控\n";
    std::cout << "     实时监控连接数、收发量、延迟\n";
    std::cout << "     异常连接告警\n";
}

// ============================================================
// 主函数
// ============================================================
int main() {
    std::cout << "============================================================\n";
    std::cout << "  第8章: 高性能网络与异步IO - 第6节 连接管理深入\n";
    std::cout << "============================================================\n";

    WinsockInit winsock_init;

    demo_connection_management();
    demo_timeout_handling();
    demo_graceful_shutdown();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
