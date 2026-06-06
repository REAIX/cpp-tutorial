/**
 * @file 02_deep_dive_protocol_design.c
 * @brief 协议设计深入: 消息帧定界、长度前缀协议、心跳机制
 * @description 对应文档: 25-网络编程基础
 *  @note C++ 中可使用 RAII 封装和异步 I/O 模式, 参见 C++ 章节 35-网络编程
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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

#define PORT 9094
#define BUFFER_SIZE 4096

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

typedef struct {
    uint32_t length;
    uint16_t type;
    uint16_t flags;
    char payload[];
} __attribute__((packed)) ProtocolMessage;

#define MSG_TYPE_DATA    1
#define MSG_TYPE_ACK     2
#define MSG_TYPE_HEARTBEAT 3

void demo_message_framing(void) {
    printf("\n=== demo_message_framing ===\n");
    printf("消息帧定界: TCP是字节流, 需要应用层定义消息边界\n\n");

    printf("常见帧定界方式:\n");
    printf("  1. 定长消息: 每条消息固定长度, 简单但浪费带宽\n");
    printf("  2. 分隔符: 用特殊字符分隔, 如HTTP的\\r\\n\\r\\n\n");
    printf("  3. 长度前缀: 消息头包含长度, 最灵活\n");
    printf("  4. TLV格式: Type-Length-Value, 可扩展性好\n\n");

    printf("TCP粘包问题:\n");
    printf("  发送: [消息A][消息B] -> 可能收到: [消息A部分][消息A剩余+消息B]\n");
    printf("  原因: TCP是字节流, 不保证一次recv对应一次send\n");
    printf("  解决: 应用层定义消息边界(帧定界)\n");
}

void demo_length_prefix_protocol(void) {
    printf("\n=== demo_length_prefix_protocol ===\n");
    printf("长度前缀协议: [4字节长度][消息体]\n\n");

    const char *messages[] = {
        "Hello",
        "This is a longer message for testing",
        "Hi"
    };
    int msg_count = 3;

    char send_buffer[BUFFER_SIZE];
    int total_sent = 0;

    for (int i = 0; i < msg_count; i++) {
        uint32_t len = (uint32_t)strlen(messages[i]);
        memcpy(send_buffer + total_sent, &len, sizeof(len));
        memcpy(send_buffer + total_sent + sizeof(len), messages[i], len);
        total_sent += sizeof(len) + len;
        printf("打包消息%d: 长度=%u, 内容=\"%s\"\n", i + 1, len, messages[i]);
    }

    printf("\n模拟TCP粘包: 3条消息合并为 %d 字节\n\n", total_sent);

    int offset = 0;
    int msg_index = 0;
    while (offset < total_sent) {
        uint32_t len;
        memcpy(&len, send_buffer + offset, sizeof(len));
        offset += sizeof(len);

        char payload[256] = {0};
        memcpy(payload, send_buffer + offset, len);
        offset += len;

        printf("解包消息%d: 长度=%u, 内容=\"%s\"\n", ++msg_index, len, payload);
    }

    printf("\n举一反三 - 更完善的协议头:\n");
    printf("  [魔数4B][版本2B][类型2B][长度4B][序列号4B][载荷NB][校验4B]\n");
    printf("  魔数: 快速识别非法数据\n");
    printf("  版本: 兼容性处理\n");
    printf("  类型: 区分不同消息\n");
    printf("  序列号: 请求响应匹配\n");
    printf("  校验: 数据完整性\n");
}

static int recv_exact(int sock, char *buffer, int size) {
    int received = 0;
    while (received < size) {
        int n = recv(sock, buffer + received, size - received, 0);
        if (n <= 0) return n;
        received += n;
    }
    return received;
}

static int send_all(int sock, const char *buffer, int size) {
    int sent = 0;
    while (sent < size) {
        int n = send(sock, buffer + sent, size - sent, 0);
        if (n <= 0) return n;
        sent += n;
    }
    return sent;
}

void demo_protocol_roundtrip(void) {
    printf("\n=== demo_protocol_roundtrip ===\n");
    printf("协议收发实战: 长度前缀协议的完整收发\n\n");

    if (socket_init() < 0) return;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
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
    listen(server_fd, 1);

    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    connect(client_fd, (struct sockaddr *)&addr, sizeof(addr));

    struct sockaddr_in client_addr;
#ifdef _WIN32
    int len = sizeof(client_addr);
#else
    socklen_t len = sizeof(client_addr);
#endif
    int conn_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len);

    const char *payload = "Protocol design test message";
    uint32_t payload_len = (uint32_t)strlen(payload);
    uint16_t msg_type = MSG_TYPE_DATA;
    uint16_t flags = 0;

    char send_buf[BUFFER_SIZE];
    int header_size = sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint16_t);
    memcpy(send_buf, &payload_len, sizeof(payload_len));
    memcpy(send_buf + sizeof(uint32_t), &msg_type, sizeof(msg_type));
    memcpy(send_buf + sizeof(uint32_t) + sizeof(uint16_t), &flags, sizeof(flags));
    memcpy(send_buf + header_size, payload, payload_len);

    send_all(client_fd, send_buf, header_size + payload_len);
    printf("客户端发送: 类型=%d, 长度=%u, 内容=\"%s\"\n", msg_type, payload_len, payload);

    char recv_header[8];
    recv_exact(conn_fd, recv_header, header_size);

    uint32_t recv_len;
    uint16_t recv_type, recv_flags;
    memcpy(&recv_len, recv_header, sizeof(recv_len));
    memcpy(&recv_type, recv_header + sizeof(uint32_t), sizeof(recv_type));
    memcpy(&recv_flags, recv_header + sizeof(uint32_t) + sizeof(uint16_t), sizeof(recv_flags));

    char recv_payload[256] = {0};
    if (recv_len > 0 && recv_len < sizeof(recv_payload)) {
        recv_exact(conn_fd, recv_payload, recv_len);
    }

    printf("服务器收到: 类型=%d, 长度=%u, 内容=\"%s\"\n", recv_type, recv_len, recv_payload);

    uint16_t ack_type = MSG_TYPE_ACK;
    uint32_t ack_len = 0;
    char ack_buf[header_size];
    memcpy(ack_buf, &ack_len, sizeof(ack_len));
    memcpy(ack_buf + sizeof(uint32_t), &ack_type, sizeof(ack_type));
    memcpy(ack_buf + sizeof(uint32_t) + sizeof(uint16_t), &flags, sizeof(flags));
    send_all(conn_fd, ack_buf, header_size);
    printf("服务器发送ACK\n");

    char ack_recv[8];
    recv_exact(client_fd, ack_recv, header_size);
    uint16_t recv_ack_type;
    memcpy(&recv_ack_type, ack_recv + sizeof(uint32_t), sizeof(recv_ack_type));
    printf("客户端收到ACK, 类型=%d\n", recv_ack_type);

    close_socket(conn_fd);
    close_socket(client_fd);
    close_socket(server_fd);
    socket_cleanup();
}

void demo_heartbeat_mechanism(void) {
    printf("\n=== demo_heartbeat_mechanism ===\n");
    printf("心跳机制: 定期发送探测包, 检测连接是否存活\n\n");

    printf("心跳设计要点:\n");
    printf("  1. 间隔: 通常5-30秒, 根据场景调整\n");
    printf("  2. 超时: 连续N次未收到响应则断开\n");
    printf("  3. 内容: 可以携带时间戳、序列号\n");
    printf("  4. 双向: 客户端和服务端都可以发起\n\n");

    printf("心跳实现方式:\n");
    printf("  1. 应用层心跳: 自定义心跳消息, 最灵活\n");
    printf("  2. TCP KeepAlive: SO_KEEPALIVE选项, 系统级\n");
    printf("  3. 两者结合: TCP KeepAlive兜底, 应用层心跳更及时\n\n");

    if (socket_init() < 0) return;

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    int keepalive = 1;
    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (const char *)&keepalive, sizeof(keepalive));

#ifndef _WIN32
    int keepidle = 10;
    int keepintvl = 3;
    int keepcnt = 3;
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle));
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(keepintvl));
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, sizeof(keepcnt));
    printf("TCP KeepAlive: idle=%ds, interval=%ds, count=%d\n", keepidle, keepintvl, keepcnt);
#else
    printf("TCP KeepAlive: 已启用 (Windows使用系统默认参数)\n");
#endif

    close_socket(sock);
    socket_cleanup();

    printf("\n心跳协议示例:\n");
    printf("  [类型=HEARTBEAT][长度=8][时间戳8字节]\n");
    printf("  响应: [类型=HEARTBEAT_ACK][长度=8][原时间戳8字节]\n");
    printf("  超时3次未响应 -> 断开连接\n");

    printf("\n心跳陷阱:\n");
    printf("  1. 心跳间隔太短: 浪费带宽和CPU\n");
    printf("  2. 心跳间隔太长: 检测不及时\n");
    printf("  3. 只发不收: 需要双向确认\n");
    printf("  4. 忽略半开连接: 一方崩溃, 另一方不知道\n");
}

int main(void) {
    printf("协议设计深入: 消息帧定界、长度前缀协议、心跳机制\n");

    demo_message_framing();
    demo_length_prefix_protocol();
    demo_heartbeat_mechanism();
    demo_protocol_roundtrip();

    printf("\n所有演示完成!\n");
    return 0;
}
