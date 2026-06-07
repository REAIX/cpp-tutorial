/**
 * @file 01_example_tcp_server.c
 * @brief TCP服务器基础示例
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
#endif

#define PORT 9090
#define BUFFER_SIZE 1024

static int socket_init(void) {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup 失败: %d\n", WSAGetLastError());
        return -1;
    }
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

void demo_tcp_server_basic(void) {
    printf("\n=== demo_tcp_server_basic ===\n");

    if (socket_init() < 0) return;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        printf("创建socket失败\n");
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
        printf("bind 失败\n");
        close_socket(server_fd);
        socket_cleanup();
        return;
    }

    if (listen(server_fd, 5) < 0) {
        printf("listen 失败\n");
        close_socket(server_fd);
        socket_cleanup();
        return;
    }

    printf("TCP服务器启动, 监听端口 %d\n", PORT);
    printf("等待客户端连接... (可用另一个终端运行 02_example_tcp_client)\n");

    fd_set readfds;
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    FD_ZERO(&readfds);
    FD_SET(server_fd, &readfds);

    int ready = select(server_fd + 1, &readfds, NULL, NULL, &timeout);
    if (ready <= 0) {
        printf("5秒内无客户端连接, 演示结束\n");
        close_socket(server_fd);
        socket_cleanup();
        return;
    }

    struct sockaddr_in client_addr;
#ifdef _WIN32
    int client_len = sizeof(client_addr);
#else
    socklen_t client_len = sizeof(client_addr);
#endif
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
        printf("accept 失败\n");
        close_socket(server_fd);
        socket_cleanup();
        return;
    }

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
    printf("客户端连接: %s:%d\n", client_ip, ntohs(client_addr.sin_port));

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    int n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (n > 0) {
        buffer[n] = '\0';
        printf("收到消息: \"%s\" (%d 字节)\n", buffer, n);

        const char *response = "Hello from TCP server!";
        send(client_fd, response, (int)strlen(response), 0);
        printf("发送响应: \"%s\"\n", response);
    }

    close_socket(client_fd);
    close_socket(server_fd);
    socket_cleanup();
    printf("服务器关闭\n");
}

void demo_tcp_server_steps(void) {
    printf("\n=== demo_tcp_server_steps ===\n");
    printf("TCP服务器创建步骤:\n");
    printf("  1. socket()   - 创建套接字\n");
    printf("  2. bind()     - 绑定地址和端口\n");
    printf("  3. listen()   - 开始监听, 设置backlog队列\n");
    printf("  4. accept()   - 接受客户端连接(阻塞)\n");
    printf("  5. recv()/send() - 收发数据\n");
    printf("  6. close()    - 关闭连接\n\n");

    printf("关键概念:\n");
    printf("  - SO_REUSEADDR: 允许重用处于TIME_WAIT状态的地址\n");
    printf("  - backlog: listen()的第二个参数, 等待连接队列长度\n");
    printf("  - accept()返回新的socket描述符, 原描述符继续监听\n");
    printf("  - TCP是字节流协议, recv()可能只收到部分数据\n");
}

int main(void) {
    printf("TCP服务器基础示例\n");

    demo_tcp_server_steps();
    demo_tcp_server_basic();

    printf("\n演示完成!\n");
    return 0;
}
