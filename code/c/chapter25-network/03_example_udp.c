/**
 * @file 03_example_udp.c
 * @brief UDP服务器与客户端示例
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

#define UDP_PORT 9092
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

void demo_udp_echo_server(void) {
    printf("\n=== demo_udp_echo_server ===\n");

    if (socket_init() < 0) return;

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        printf("创建UDP socket失败\n");
        socket_cleanup();
        return;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(UDP_PORT);

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("bind 失败\n");
        close_socket(sock);
        socket_cleanup();
        return;
    }

    printf("UDP服务器启动, 监听端口 %d\n", UDP_PORT);
    printf("等待数据报... (5秒超时)\n");

    fd_set readfds;
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);

    int ready = select(sock + 1, &readfds, NULL, NULL, &timeout);
    if (ready <= 0) {
        printf("5秒内无数据, 演示结束\n");
        close_socket(sock);
        socket_cleanup();
        return;
    }

    char buffer[BUFFER_SIZE];
    struct sockaddr_in client_addr;
#ifdef _WIN32
    int client_len = sizeof(client_addr);
#else
    socklen_t client_len = sizeof(client_addr);
#endif

    int n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                     (struct sockaddr *)&client_addr, &client_len);
    if (n > 0) {
        buffer[n] = '\0';
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        printf("收到来自 %s:%d 的消息: \"%s\"\n",
               client_ip, ntohs(client_addr.sin_port), buffer);

        sendto(sock, buffer, n, 0, (struct sockaddr *)&client_addr, client_len);
        printf("已回送消息\n");
    }

    close_socket(sock);
    socket_cleanup();
}

void demo_udp_client_self(void) {
    printf("\n=== demo_udp_client_self ===\n");
    printf("自收自发演示: 同一进程内UDP通信\n");

    if (socket_init() < 0) return;

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        socket_cleanup();
        return;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(UDP_PORT + 1);

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("bind 失败\n");
        close_socket(sock);
        socket_cleanup();
        return;
    }

    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(UDP_PORT + 1);
    inet_pton(AF_INET, "127.0.0.1", &dest.sin_addr);

    const char *msg = "Hello UDP!";
    sendto(sock, msg, (int)strlen(msg), 0, (struct sockaddr *)&dest, sizeof(dest));
    printf("发送: \"%s\"\n", msg);

    char buffer[BUFFER_SIZE] = {0};
    struct sockaddr_in from;
#ifdef _WIN32
    int from_len = sizeof(from);
#else
    socklen_t from_len = sizeof(from);
#endif
    int n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                     (struct sockaddr *)&from, &from_len);
    if (n > 0) {
        buffer[n] = '\0';
        printf("收到: \"%s\"\n", buffer);
    }

    close_socket(sock);
    socket_cleanup();
}

void demo_udp_vs_tcp(void) {
    printf("\n=== demo_udp_vs_tcp ===\n");
    printf("UDP vs TCP 对比:\n\n");
    printf("特性          TCP              UDP\n");
    printf("连接          面向连接          无连接\n");
    printf("可靠性        可靠传输          不可靠\n");
    printf("顺序          保证顺序          不保证\n");
    printf("流量控制      有                无\n");
    printf("拥塞控制      有                无\n");
    printf("传输方式      字节流            数据报\n");
    printf("开销          较大              较小\n");
    printf("适用场景      文件传输/网页     DNS/视频/游戏\n\n");

    printf("UDP适用场景:\n");
    printf("  1. DNS查询 - 简单请求响应, 超时重发即可\n");
    printf("  2. 视频流 - 实时性优先, 丢帧可接受\n");
    printf("  3. 游戏同步 - 低延迟优先, 状态可插值\n");
    printf("  4. 局域网发现 - 广播/多播\n");

    printf("\nUDP陷阱:\n");
    printf("  1. 数据报可能丢失、重复、乱序\n");
    printf("  2. 没有流量控制, 发太快可能丢包\n");
    printf("  3. 数据报大小受限(通常<1500字节避免分片)\n");
    printf("  4. 需要应用层实现可靠性机制\n");
}

int main(void) {
    printf("UDP服务器与客户端示例\n");

    demo_udp_vs_tcp();
    demo_udp_client_self();
    demo_udp_echo_server();

    printf("\n演示完成!\n");
    return 0;
}
