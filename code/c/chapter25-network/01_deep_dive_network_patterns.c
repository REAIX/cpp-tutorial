/**
 * @file 01_deep_dive_network_patterns.c
 * @brief 网络模式深入: select多路复用、非阻塞IO、超时处理、优雅关闭
 * @description 对应文档: 25-网络编程基础
 *  @note C++ 中可使用 RAII 封装和异步 I/O 模式, 参见 C++ 章节 35-网络编程
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#endif

#define PORT 9093
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

static int socket_init(void) {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return -1;
#endif
    return 0;
}

static void socket_cleanup(void) {
#ifdef _WIN32
    WSACleanup();
#endif
}

static void close_socket(int sock) {
#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif
}

static void set_nonblocking(int sock) {
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);
#else
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
#endif
}

void demo_select_multiplexing(void) {
    printf("\n=== demo_select_multiplexing ===\n");
    printf("select多路复用: 同时监控多个文件描述符的读写事件\n\n");

    if (socket_init() < 0) return;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        socket_cleanup();
        return;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close_socket(server_fd);
        socket_cleanup();
        return;
    }

    listen(server_fd, 5);
    printf("select服务器启动, 端口 %d\n", PORT);

    int client_fds[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++) client_fds[i] = -1;

    int max_fd = server_fd;
    int running = 1;
    int loop_count = 0;

    while (running && loop_count < 50) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_fds[i] >= 0) {
                FD_SET(client_fds[i], &readfds);
                if (client_fds[i] > max_fd) max_fd = client_fds[i];
            }
        }

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        int ready = select(max_fd + 1, &readfds, NULL, NULL, &timeout);
        if (ready < 0) {
            printf("select 错误\n");
            break;
        }

        if (ready == 0) {
            loop_count++;
            continue;
        }

        if (FD_ISSET(server_fd, &readfds)) {
            struct sockaddr_in client_addr;
#ifdef _WIN32
            int len = sizeof(client_addr);
#else
            socklen_t len = sizeof(client_addr);
#endif
            int new_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len);
            if (new_fd >= 0) {
                int slot = -1;
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (client_fds[i] < 0) { slot = i; break; }
                }
                if (slot >= 0) {
                    client_fds[slot] = new_fd;
                    printf("新客户端连接, fd=%d\n", new_fd);
                } else {
                    printf("连接数已满, 拒绝连接\n");
                    close_socket(new_fd);
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_fds[i] >= 0 && FD_ISSET(client_fds[i], &readfds)) {
                char buffer[BUFFER_SIZE];
                int n = recv(client_fds[i], buffer, sizeof(buffer) - 1, 0);
                if (n <= 0) {
                    printf("客户端 fd=%d 断开\n", client_fds[i]);
                    close_socket(client_fds[i]);
                    client_fds[i] = -1;
                } else {
                    buffer[n] = '\0';
                    printf("收到(fd=%d): \"%s\"\n", client_fds[i], buffer);
                    send(client_fds[i], buffer, n, 0);
                }
            }
        }

        loop_count++;
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_fds[i] >= 0) close_socket(client_fds[i]);
    }
    close_socket(server_fd);
    socket_cleanup();

    printf("\nselect陷阱:\n");
    printf("  1. FD_SETSIZE限制(通常1024), 不适合高并发\n");
    printf("  2. 每次调用需要重新设置fd_set\n");
    printf("  3. 内核->用户空间拷贝开销大\n");
    printf("  4. 替代方案: poll(无FD_SETSIZE限制), epoll(Linux), kqueue(BSD)\n");
}

void demo_nonblocking_io(void) {
    printf("\n=== demo_nonblocking_io ===\n");
    printf("非阻塞IO: 操作不阻塞, 返回EAGAIN/EWOULDBLOCK时稍后重试\n\n");

    if (socket_init() < 0) return;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    set_nonblocking(sock);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT + 1);

    bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    listen(sock, 1);

    struct sockaddr_in client_addr;
#ifdef _WIN32
    int len = sizeof(client_addr);
#else
    socklen_t len = sizeof(client_addr);
#endif

    int conn_fd = accept(sock, (struct sockaddr *)&client_addr, &len);
    if (conn_fd < 0) {
#ifdef _WIN32
        int err = WSAGetLastError();
        printf("非阻塞accept返回错误: %d (WSAEWOULDBLOCK=%d 表示暂无连接)\n",
               err, WSAEWOULDBLOCK);
#else
        printf("非阻塞accept返回: %s (EAGAIN表示暂无连接)\n", strerror(errno));
#endif
    }

    char buffer[BUFFER_SIZE];
    int n = recv(sock, buffer, sizeof(buffer), 0);
    if (n < 0) {
#ifdef _WIN32
        int err = WSAGetLastError();
        printf("非阻塞recv返回错误: %d (WSAEWOULDBLOCK=%d 表示暂无数据)\n",
               err, WSAEWOULDBLOCK);
#else
        printf("非阻塞recv返回: %s (EAGAIN表示暂无数据)\n", strerror(errno));
#endif
    }

    close_socket(sock);
    socket_cleanup();

    printf("\n非阻塞IO模式:\n");
    printf("  1. 轮询模式: 循环检查, CPU占用高, 不推荐\n");
    printf("  2. select/poll: 等待就绪后再操作, 推荐\n");
    printf("  3. 非阻塞+多路复用: 最佳实践\n");
}

void demo_timeout_handling(void) {
    printf("\n=== demo_timeout_handling ===\n");
    printf("超时处理: 防止操作无限阻塞\n\n");

    if (socket_init() < 0) return;

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct timeval send_timeout = {3, 0};
    struct timeval recv_timeout = {5, 0};
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char *)&send_timeout, sizeof(send_timeout));
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&recv_timeout, sizeof(recv_timeout));

    printf("设置发送超时: 3秒\n");
    printf("设置接收超时: 5秒\n");

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT + 2);
    bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    listen(sock, 1);

    fd_set readfds;
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);

    int ready = select(sock + 1, &readfds, NULL, NULL, &timeout);
    if (ready == 0) {
        printf("select超时(2秒), 无连接到达\n");
    }

    close_socket(sock);
    socket_cleanup();

    printf("\n超时处理方式:\n");
    printf("  1. SO_SNDTIMEO/SO_RCVTIMEO: socket级别超时\n");
    printf("  2. select/poll超时: 等待就绪时超时\n");
    printf("  3. 非阻塞IO + 自定义计时器\n");
    printf("  4. alarm信号(SIGALRM): 不推荐, 全局影响\n");
}

void demo_graceful_shutdown(void) {
    printf("\n=== demo_graceful_shutdown ===\n");
    printf("优雅关闭: 确保数据发送完毕再关闭连接\n\n");

    if (socket_init() < 0) return;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT + 3);

    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 1);

    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    addr.sin_port = htons(PORT + 3);
    connect(client_fd, (struct sockaddr *)&addr, sizeof(addr));

    struct sockaddr_in client_addr;
#ifdef _WIN32
    int len = sizeof(client_addr);
#else
    socklen_t len = sizeof(client_addr);
#endif
    int conn_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len);

    const char *msg = "Important data to send";
    send(conn_fd, msg, (int)strlen(msg), 0);
    printf("发送数据: \"%s\"\n", msg);

    printf("\n优雅关闭步骤:\n");
    printf("  1. shutdown(fd, SHUT_WR) - 关闭写端, 发送FIN\n");
    printf("  2. 继续recv()读取对方数据\n");
    printf("  3. recv()返回0表示对方也关闭\n");
    printf("  4. close()释放资源\n");

#ifndef _WIN32
    shutdown(conn_fd, SHUT_WR);
    printf("已调用 shutdown(conn_fd, SHUT_WR)\n");

    char buffer[BUFFER_SIZE];
    int n = recv(conn_fd, buffer, sizeof(buffer) - 1, 0);
    if (n == 0) {
        printf("对方已关闭连接\n");
    }
#else
    shutdown(conn_fd, SD_SEND);
    printf("已调用 shutdown(conn_fd, SD_SEND)\n");
#endif

    close_socket(conn_fd);
    close_socket(client_fd);
    close_socket(server_fd);
    socket_cleanup();

    printf("\nshutdown vs close:\n");
    printf("  shutdown(SHUT_WR): 半关闭, 只关闭写端, 引用计数不减\n");
    printf("  close(): 完全关闭, 引用计数减1, 减到0才真正关闭\n");
    printf("  优雅关闭 = shutdown写端 + 读完对方数据 + close\n");
}

int main(void) {
    printf("网络模式深入: select多路复用、非阻塞IO、超时处理、优雅关闭\n");

    demo_nonblocking_io();
    demo_timeout_handling();
    demo_graceful_shutdown();
    demo_select_multiplexing();

    printf("\n所有演示完成!\n");
    return 0;
}
