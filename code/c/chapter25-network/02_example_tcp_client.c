/**
 * @file 02_example_tcp_client.c
 * @brief TCP客户端基础示例
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

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9090
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

void demo_tcp_client_connect(void) {
    printf("\n=== demo_tcp_client_connect ===\n");

    if (socket_init() < 0) return;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("创建socket失败\n");
        socket_cleanup();
        return;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        printf("无效地址: %s\n", SERVER_IP);
        close_socket(sock);
        socket_cleanup();
        return;
    }

    printf("尝试连接服务器 %s:%d...\n", SERVER_IP, SERVER_PORT);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("连接失败 (请先启动 01_example_tcp_server)\n");
        close_socket(sock);
        socket_cleanup();
        return;
    }

    printf("连接成功!\n");

    const char *message = "Hello from TCP client!";
    send(sock, message, (int)strlen(message), 0);
    printf("发送消息: \"%s\"\n", message);

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    int n = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (n > 0) {
        buffer[n] = '\0';
        printf("收到响应: \"%s\" (%d 字节)\n", buffer, n);
    } else if (n == 0) {
        printf("服务器关闭了连接\n");
    } else {
        printf("接收失败\n");
    }

    close_socket(sock);
    socket_cleanup();
    printf("客户端关闭\n");
}

void demo_tcp_client_steps(void) {
    printf("\n=== demo_tcp_client_steps ===\n");
    printf("TCP客户端创建步骤:\n");
    printf("  1. socket()   - 创建套接字\n");
    printf("  2. connect()  - 连接服务器(三次握手)\n");
    printf("  3. send()     - 发送数据\n");
    printf("  4. recv()     - 接收数据\n");
    printf("  5. close()    - 关闭连接\n\n");

    printf("TCP三次握手:\n");
    printf("  客户端 -> SYN -> 服务器\n");
    printf("  客户端 <- SYN+ACK <- 服务器\n");
    printf("  客户端 -> ACK -> 服务器\n\n");

    printf("TCP四次挥手:\n");
    printf("  主动方 -> FIN -> 被动方\n");
    printf("  主动方 <- ACK <- 被动方\n");
    printf("  主动方 <- FIN <- 被动方\n");
    printf("  主动方 -> ACK -> 被动方\n");
}

void demo_tcp_client_loopback(void) {
    printf("\n=== demo_tcp_client_loopback ===\n");
    printf("自连接演示: 客户端连接本机回环地址\n");

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
    addr.sin_port = htons(9091);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("bind 失败\n");
        close_socket(server_fd);
        socket_cleanup();
        return;
    }

    listen(server_fd, 1);

    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_port = htons(9091);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(client_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("连接失败\n");
        close_socket(client_fd);
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
    int conn_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

    const char *msg = "Loopback test message";
    send(client_fd, msg, (int)strlen(msg), 0);
    printf("客户端发送: \"%s\"\n", msg);

    char buffer[BUFFER_SIZE] = {0};
    int n = recv(conn_fd, buffer, sizeof(buffer) - 1, 0);
    if (n > 0) {
        buffer[n] = '\0';
        printf("服务器收到: \"%s\"\n", buffer);
    }

    const char *resp = "Loopback response";
    send(conn_fd, resp, (int)strlen(resp), 0);
    printf("服务器发送: \"%s\"\n", resp);

    memset(buffer, 0, sizeof(buffer));
    n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (n > 0) {
        buffer[n] = '\0';
        printf("客户端收到: \"%s\"\n", buffer);
    }

    close_socket(conn_fd);
    close_socket(client_fd);
    close_socket(server_fd);
    socket_cleanup();
}

int main(void) {
    printf("TCP客户端基础示例\n");

    demo_tcp_client_steps();
    demo_tcp_client_loopback();
    demo_tcp_client_connect();

    printf("\n演示完成!\n");
    return 0;
}
