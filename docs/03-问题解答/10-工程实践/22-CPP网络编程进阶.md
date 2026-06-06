# C++网络编程进阶
> 📖 相关章节：[编码规范](../../04-工程实践/00-编码规范.md)、[设计模式](../../04-工程实践/03-设计模式.md)、[单元测试](../../04-工程实践/06-单元测试.md)、[代码审查](../../04-工程实践/08-代码审查.md)

> "从阻塞Socket到异步I/O，从TCP字节流到gRPC微服务——网络编程的每一步进阶，都是对性能与架构的重新理解。"

---

## 1. 从基础Socket到进阶

### 1. 回顾Socket基础

```
Socket = 网络通信的端点，是应用层与传输层之间的接口

TCP通信流程：

  服务端                          客户端
  ┌──────┐                      ┌──────┐
  │socket│                      │socket│
  └──┬───┘                      └──┬───┘
     │                             │
  ┌──▼───┐                      ┌──▼───┐
  │ bind │ 绑定地址端口          │      │
  └──┬───┘                      │      │
     │                          │      │
  ┌──▼────┐                    │      │
  │listen │ 开始监听            │      │
  └──┬────┘                    │      │
     │                          │      │
  ┌──▼────┐    ┌──────────┐   ┌──▼───┐
  │accept │◄───│ connect  │───│connect│
  └──┬────┘    └──────────┘   └───────┘
     │
  ┌──▼──┐     ┌──────┐
  │recv │◄────│send  │
  └──┬──┘     └──────┘
     │
  ┌──▼──┐     ┌──────┐
  │send │────►│recv  │
  └──┬──┘     └──────┘
     │
  ┌──▼──────┐
  │close    │
  └─────────┘
```

### 2. 基础Socket的局限

```
局限1：阻塞I/O
  recv() 在没有数据时会阻塞线程
  一个线程只能处理一个连接
  1000个连接需要1000个线程 → 资源爆炸

局限2：一对一模型
  一个Socket → 一个线程
  线程创建/切换开销大
  线程间同步复杂

局限3：手动管理缓冲区
  TCP是字节流，没有消息边界
  需要自己处理粘包/拆包
  需要自己管理发送/接收缓冲区

局限4：缺乏高级功能
  没有内置的超时管理
  没有连接池
  没有重连机制
  没有负载均衡
```

| 问题 | 基础Socket | 进阶方案 |
|------|-----------|----------|
| 阻塞 | 阻塞等待 | I/O多路复用/异步I/O |
| 一对一 | 每连接一线程 | 事件驱动，一线程多连接 |
| 缓冲区 | 手动管理 | 框架自动管理 |
| 协议 | 自己实现 | HTTP/WebSocket/gRPC |

---

## 2. I/O多路复用

### 1. select / poll / epoll / kqueue / IOCP 对比

```
I/O多路复用 = 一个线程同时监控多个文件描述符的I/O事件

┌──────────┬──────────┬──────────┬──────────┬──────────┐
│          │ select   │ poll     │ epoll    │ IOCP     │
│          │          │          │ (Linux)  │(Windows) │
├──────────┼──────────┼──────────┼──────────┼──────────┤
│ 最大fd数 │ 1024     │ 无限制   │ 无限制   │ 无限制   │
│ 数据结构 │ bit数组   │ 数组     │ 红黑树   │ 完成端口  │
│ 触发方式 │ 水平触发  │ 水平触发 │ LT/ET   │ 完成通知  │
│ 每次调用 │ O(n)遍历 │ O(n)遍历 │ O(1)就绪 │ O(1)就绪  │
│ 内存拷贝 │ 每次拷贝 │ 每次拷贝 │ 共享内存 │ 共享内存  │
│ 跨平台   │ 是       │ 是(Linux)│ 仅Linux  │ 仅Windows│
│ 适用规模 │ <1000    │ <数千    │ 数万+    │ 数万+    │
└──────────┴──────────┴──────────┴──────────┴──────────┘

macOS/BSD 使用 kqueue，类似 epoll 的机制
```

### 2. epoll 的 LT 与 ET 模式

```
LT（Level Triggered，水平触发）：
  只要缓冲区有数据，epoll_wait 就会返回
  类似"水位检测"——水没排完就一直报警

ET（Edge Triggered，边缘触发）：
  缓冲区从空到有数据时，epoll_wait 只返回一次
  类似"脉冲检测"——只在变化瞬间报警
  必须一次性读完所有数据（循环read直到EAGAIN）

  LT模式（默认）：
  ┌──────────────────────────────────┐
  │ 缓冲区: [data]                   │ ← epoll_wait 返回
  │ 缓冲区: [data]                   │ ← 没读完，下次还返回
  │ 缓冲区: [data]                   │ ← 继续返回
  │ 缓冲区: []                       │ ← 读完了，不再返回
  └──────────────────────────────────┘

  ET模式：
  ┌──────────────────────────────────┐
  │ 缓冲区: [] → [data]              │ ← epoll_wait 返回（边缘！）
  │ 缓冲区: [data]                   │ ← 不再返回！必须循环读完
  │ 缓冲区: []                       │ ← 读完
  │ 缓冲区: [] → [new_data]          │ ← 新数据到达，再次返回
  └──────────────────────────────────┘
```

| 对比项 | LT | ET |
|--------|-----|-----|
| 编程难度 | 简单 | 较复杂 |
| 触发次数 | 可能多次 | 仅一次 |
| 是否需循环读 | 不必须 | 必须 |
| 效率 | 略低（重复通知） | 更高（减少系统调用） |
| 适用场景 | 通用 | 高性能服务器 |

### 3. epoll 代码示例（Linux）

```cpp
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <cstring>
#include <errno.h>

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    set_nonblocking(listen_fd);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);
    bind(listen_fd, (sockaddr*)&addr, sizeof(addr));
    listen(listen_fd, 128);

    int epfd = epoll_create1(0);

    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLET; // ET模式
    ev.data.fd = listen_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);

    epoll_event events[1024];
    while (true) {
        int nfds = epoll_wait(epfd, events, 1024, -1);
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == listen_fd) {
                // ET模式需循环accept直到EAGAIN
                while (true) {
                    int conn_fd = accept(listen_fd, nullptr, nullptr);
                    if (conn_fd == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                        break;
                    }
                    set_nonblocking(conn_fd);
                    epoll_event conn_ev{};
                    conn_ev.events = EPOLLIN | EPOLLET;
                    conn_ev.data.fd = conn_fd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, conn_fd, &conn_ev);
                }
            } else {
                int fd = events[i].data.fd;
                char buf[4096];
                // ET模式需循环读直到EAGAIN
                while (true) {
                    ssize_t n = read(fd, buf, sizeof(buf));
                    if (n > 0) {
                        write(fd, buf, n); // 回显
                    } else if (n == 0) {
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
                        close(fd);
                        break;
                    } else {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
                        close(fd);
                        break;
                    }
                }
            }
        }
    }
    close(listen_fd);
    close(epfd);
    return 0;
}
```

---

## 3. 异步I/O与事件驱动

### 1. 同步 vs 异步 I/O

```
同步I/O：发起I/O操作后，线程等待操作完成才继续执行
  ┌────────┐   read()   ┌──────────┐
  │ 线程A  │ ──────────→│ 等待数据  │ ──→ 继续执行
  └────────┘            └──────────┘
  线程被阻塞，什么也干不了

异步I/O：发起I/O操作后，线程立即返回，操作完成后内核通知
  ┌────────┐   aio_read()   ┌────────┐
  │ 线程A  │ ──────────────→│ 立即返回│ ──→ 做其他事
  └────────┘                 └────────┘
       │
       │  ...做其他工作...
       │
       ▼   完成通知
  ┌────────┐
  │ 处理结果│
  └────────┘
```

### 2. 两种异步模式

```
Reactor模式（同步非阻塞 + I/O多路复用）：
  epoll/select 检测到I/O就绪 → 回调处理读写
  代表：libevent、libev、Redis

  ┌──────────────────────────────────────────┐
  │              Reactor                     │
  │  ┌─────────┐    ┌──────────────────┐    │
  │  │epoll_wait│───→│ 分发到对应Handler │    │
  │  └─────────┘    └──────────────────┘    │
  │                       │                  │
  │          ┌────────────┼────────────┐     │
  │          ▼            ▼            ▼     │
  │     [读Handler] [写Handler] [超时Handler]│
  │          │            │            │     │
  │          ▼            ▼            ▼     │
  │       read()       write()     定时回调   │
  └──────────────────────────────────────────┘

Proactor模式（真正的异步I/O）：
  发起异步操作 → 内核完成操作 → 通知完成
  代表：Boost.Asio（Windows IOCP）、IOCP

  ┌──────────────────────────────────────────┐
  │              Proactor                    │
  │  ┌──────────┐   ┌───────────────────┐   │
  │  │异步读请求 │──→│内核完成读操作      │   │
  │  └──────────┘   └───────────────────┘   │
  │                       │                  │
  │                       ▼                  │
  │              ┌──────────────────┐        │
  │              │ 完成事件通知      │        │
  │              └──────────────────┘        │
  │                       │                  │
  │                       ▼                  │
  │              ┌──────────────────┐        │
  │              │ CompletionHandler│        │
  │              │ 处理已读取的数据  │        │
  │              └──────────────────┘        │
  └──────────────────────────────────────────┘
```

### 3. Boost.Asio 代码示例

```cpp
#include <boost/asio.hpp>
#include <iostream>
#include <memory>

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
    tcp::socket socket_;
    char buf_[4096];

public:
    explicit Session(tcp::socket socket) : socket_(std::move(socket)) {}

    void start() { do_read(); }

    void do_read() {
        auto self = shared_from_this();
        socket_.async_read_some(
            boost::asio::buffer(buf_, sizeof(buf_)),
            [this, self](boost::system::error_code ec, std::size_t len) {
                if (!ec) {
                    do_write(len);
                }
            });
    }

    void do_write(std::size_t len) {
        auto self = shared_from_this();
        boost::asio::async_write(
            socket_, boost::asio::buffer(buf_, len),
            [this, self](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    do_read();
                }
            });
    }
};

class Server {
    tcp::acceptor acceptor_;

public:
    Server(boost::asio::io_context& io_ctx, unsigned short port)
        : acceptor_(io_ctx, tcp::endpoint(tcp::v4(), port)) {
        do_accept();
    }

    void do_accept() {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::make_shared<Session>(std::move(socket))->start();
                }
                do_accept();
            });
    }
};

int main() {
    try {
        boost::asio::io_context io_ctx;
        Server server(io_ctx, 8080);
        std::cout << "服务器启动，监听端口 8080" << std::endl;
        io_ctx.run();
    } catch (std::exception& e) {
        std::cerr << "异常: " << e.what() << std::endl;
    }
    return 0;
}
```

### 4. libuv 简介

```
libuv = Node.js 的底层事件循环库

特点：
  - 跨平台（Windows IOCP / Linux epoll / macOS kqueue）
  - 提供事件循环、TCP/UDP、定时器、文件I/O、DNS等
  - C语言编写，可被C++调用

核心架构：
  ┌─────────────────────────────────────┐
  │            libuv 事件循环            │
  │                                     │
  │  ┌──────────┐  ┌──────────────┐    │
  │  │ I/O轮询  │  │ 定时器检查    │    │
  │  │(epoll/   │  │(最小堆)      │    │
  │  │ IOCP)    │  └──────────────┘    │
  │  └──────────┘                       │
  │  ┌──────────┐  ┌──────────────┐    │
  │  │ 即时回调 │  │ idle/prepare │    │
  │  │(check)   │  │ 句柄         │    │
  │  └──────────┘  └──────────────┘    │
  └─────────────────────────────────────┘
```

---

## 4. HTTP协议与实现

### 1. HTTP请求/响应格式

```
HTTP请求：
  ┌──────────────────────────────────────────┐
  │ GET /api/users?id=1 HTTP/1.1            │ ← 请求行：方法 路径 版本
  │ Host: example.com                        │ ← 请求头
  │ Accept: application/json                 │
  │ Authorization: Bearer token123           │
  │                                          │
  │                                          │ ← 空行分隔头部和正文
  │ (请求正文，GET通常没有)                    │
  └──────────────────────────────────────────┘

HTTP响应：
  ┌──────────────────────────────────────────┐
  │ HTTP/1.1 200 OK                          │ ← 状态行：版本 状态码 原因
  │ Content-Type: application/json           │ ← 响应头
  │ Content-Length: 42                        │
  │ Set-Cookie: session=abc123               │
  │                                          │
  │ {"id":1,"name":"张三","age":25}           │ ← 响应正文
  └──────────────────────────────────────────┘
```

### 2. 常见状态码

```
2xx 成功：
  200 OK              请求成功
  201 Created         资源创建成功
  204 No Content      成功但无返回内容

3xx 重定向：
  301 Moved Permanently   永久重定向
  302 Found               临时重定向
  304 Not Modified        缓存有效

4xx 客户端错误：
  400 Bad Request      请求格式错误
  401 Unauthorized     未认证
  403 Forbidden        无权限
  404 Not Found        资源不存在
  429 Too Many Requests 请求过于频繁

5xx 服务端错误：
  500 Internal Server Error  服务器内部错误
  502 Bad Gateway            网关错误
  503 Service Unavailable    服务不可用
```

### 3. 常见Content-Type

| Content-Type | 用途 |
|-------------|------|
| `application/json` | JSON数据 |
| `application/x-www-form-urlencoded` | 表单提交（默认） |
| `multipart/form-data` | 文件上传 |
| `text/html` | HTML页面 |
| `text/plain` | 纯文本 |
| `application/octet-stream` | 二进制数据 |

### 4. C++实现简单HTTP服务器

```cpp
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <cstdio>

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);
    bind(server_fd, (sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 10);

    std::cout << "HTTP服务器运行在 http://localhost:8080" << std::endl;

    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        char buf[4096] = {};
        read(client_fd, buf, sizeof(buf));

        std::string body = R"({"message":"Hello, World!"})";
        std::string header =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: " +
            std::to_string(body.size()) +
            "\r\n"
            "Connection: close\r\n"
            "\r\n";

        std::string response = header + body;
        write(client_fd, response.c_str(), response.size());
        close(client_fd);
    }

    close(server_fd);
    return 0;
}
```

### 5. HTTP客户端库

| 库 | 特点 | 语言 |
|----|------|------|
| libcurl | 最广泛使用的HTTP客户端库 | C |
| cpp-httplib | 单头文件，同时支持客户端和服务端 | C++11 |
| cpr | libcurl的C++封装，API简洁 | C++11 |

**cpp-httplib 客户端示例**：

```cpp
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include <iostream>

int main() {
    httplib::Client cli("https://jsonplaceholder.typicode.com");

    auto res = cli.Get("/posts/1");
    if (res) {
        std::cout << "状态码: " << res->status << std::endl;
        std::cout << "响应体: " << res->body << std::endl;
    } else {
        std::cout << "请求失败: " << httplib::to_string(res.error()) << std::endl;
    }

    // POST请求
    res = cli.Post("/posts",
        R"({"title":"测试","body":"内容","userId":1})",
        "application/json");

    return 0;
}
```

---

## 5. WebSocket协议

### 1. WebSocket与HTTP的区别

```
HTTP：请求-响应模式，客户端主动，服务端被动
  客户端 ──请求──→ 服务端
  客户端 ←─响应── 服务端
  客户端 ──请求──→ 服务端
  客户端 ←─响应── 服务端
  （每次通信都要客户端发起）

WebSocket：全双工，双方都可以主动发消息
  客户端 ←───── 双向通道 ─────→ 服务端
  客户端 ──消息──→ 服务端
  服务端 ──消息──→ 客户端  （服务端主动推送！）
  客户端 ──消息──→ 服务端
  服务端 ──消息──→ 客户端
```

| 对比项 | HTTP | WebSocket |
|--------|------|-----------|
| 通信模式 | 半双工（请求-响应） | 全双工 |
| 连接 | 短连接（HTTP/1.0）/ 长连接复用 | 持久连接 |
| 服务端推送 | 不支持（需轮询/SSE） | 原生支持 |
| 开销 | 每次请求带完整头部 | 握手后帧头仅2-10字节 |
| 适用场景 | 请求-响应式API | 实时聊天/推送/游戏 |

### 2. 握手过程

```
1. 客户端发起HTTP升级请求：
   GET /chat HTTP/1.1
   Host: server.example.com
   Upgrade: websocket
   Connection: Upgrade
   Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==
   Sec-WebSocket-Version: 13

2. 服务端同意升级：
   HTTP/1.1 101 Switching Protocols
   Upgrade: websocket
   Connection: Upgrade
   Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=

3. 握手完成，切换为WebSocket协议，开始双向通信
```

### 3. 帧格式

```
WebSocket帧格式（二进制）：

   0                   1                   2                   3
   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +-+-+-+-+-------+-+-------------+-------------------------------+
  |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
  |I|S|S|S|  (4)  |A|     (7)     |            (16/64)            |
  |N|V|V|V|       |S|             |   (if payload len==126/127)   |
  | |1|2|3|       |K|             |                               |
  +-+-+-+-+-------+-+-------------+-------------------------------+
  |     Extended payload length continued (if payload len == 127) |
  +-------------------------------+-------------------------------+
  |                               |Masking-key (if MASK set)      |
  +-------------------------------+-------------------------------+
  | Masking-key (continued)       |          Payload Data         |
  +-------------------------------+-------------------------------+

  FIN: 是否为最后一帧
  opcode: 帧类型（0x1文本, 0x2二进制, 0x8关闭, 0x9 Ping, 0xA Pong）
  MASK: 是否掩码（客户端发送必须掩码）
  Payload len: 数据长度
```

### 4. C++ WebSocket库

| 库 | 特点 |
|----|------|
| libwebsockets | C语言，轻量，支持TLS，最成熟 |
| uWebSockets | C++17，极高性能，μWebSockets |
| websocketpp | C++11，仅头文件，基于Boost.Asio |

**uWebSockets 示例**：

```cpp
#include "App.h"

int main() {
    uWS::App()
        .ws<PerSocketData>("/*", {
            .open = [](auto *ws) {
                std::cout << "客户端连接" << std::endl;
            },
            .message = [](auto *ws, std::string_view msg, uWS::OpCode opCode) {
                ws->send(msg, opCode); // 回显
            },
            .close = [](auto *ws, int code, std::string_view msg) {
                std::cout << "客户端断开" << std::endl;
            }
        })
        .listen(9001, [](auto *listenSocket) {
            if (listenSocket) {
                std::cout << "WebSocket服务器运行在 ws://localhost:9001" << std::endl;
            }
        })
        .run();

    return 0;
}
```

---

## 6. RPC与gRPC

### 1. RPC原理

```
RPC（Remote Procedure Call）= 远程过程调用
让调用远程函数像调用本地函数一样简单

本地调用：
  result = add(1, 2);       // 直接调用

RPC调用：
  result = client.add(1, 2); // 看起来一样，实际跨网络

  ┌──────────┐                    ┌──────────┐
  │  客户端   │                    │  服务端   │
  │          │  1. 调用add(1,2)   │          │
  │  Stub    │ ────────────────→ │  Skeleton│
  │ (客户端桩)│  2. 序列化参数     │ (服务端桩)│
  │          │  3. 网络传输       │          │
  │          │                    │  add(1,2)│ ← 4. 本地调用
  │          │  7. 返回结果3      │          │
  │  result  │ ←──────────────── │  return 3│
  │    = 3   │  6. 反序列化      │          │
  └──────────┘  5. 网络传输      └──────────┘
```

### 2. gRPC + Protobuf

```
gRPC = Google开源的高性能RPC框架
Protobuf = Protocol Buffers，Google的序列化协议

gRPC特点：
  - 基于HTTP/2（多路复用、头部压缩、服务端推送）
  - 默认使用Protobuf序列化（比JSON小3-10倍，快20-100倍）
  - 跨语言（C++/Java/Python/Go/...）
  - 支持4种通信模式：
    1. 一元RPC（Unary）：请求-响应
    2. 服务端流（Server Streaming）：一个请求，流式响应
    3. 客户端流（Client Streaming）：流式请求，一个响应
    4. 双向流（Bidirectional Streaming）：双方流式通信
```

### 3. C++ gRPC 示例

**步骤1：定义 .proto 文件**

```protobuf
// greeter.proto
syntax = "proto3";

package greeter;

service Greeter {
  rpc SayHello (HelloRequest) returns (HelloReply) {}
  rpc SayHelloStream (HelloRequest) returns (stream HelloReply) {}
}

message HelloRequest {
  string name = 1;
}

message HelloReply {
  string message = 1;
}
```

**步骤2：生成代码**

```bash
# 生成C++头文件和源文件
protoc --grpc_out=. --cpp_out=. \
  --plugin=protoc-gen-grpc=$(which grpc_cpp_plugin) \
  greeter.proto
```

**步骤3：服务端实现**

```cpp
#include "greeter.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <iostream>

class GreeterServiceImpl final : public greeter::Greeter::Service {
    grpc::Status SayHello(grpc::ServerContext* context,
                          const greeter::HelloRequest* request,
                          greeter::HelloReply* reply) override {
        std::string prefix("你好, ");
        reply->set_message(prefix + request->name());
        return grpc::Status::OK;
    }

    grpc::Status SayHelloStream(grpc::ServerContext* context,
                                const greeter::HelloRequest* request,
                                grpc::ServerWriter<greeter::HelloReply>* writer) override {
        for (int i = 0; i < 5; i++) {
            greeter::HelloReply reply;
            reply->set_message("第" + std::to_string(i + 1) + "次问候: " + request->name());
            writer->Write(reply);
        }
        return grpc::Status::OK;
    }
};

int main() {
    GreeterServiceImpl service;
    grpc::ServerBuilder builder;
    builder.AddListeningPort("0.0.0.0:50051", grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    auto server = builder.BuildAndStart();
    std::cout << "gRPC服务端运行在 0.0.0.0:50051" << std::endl;
    server->Wait();
    return 0;
}
```

**步骤4：客户端实现**

```cpp
#include "greeter.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <iostream>

int main() {
    auto channel = grpc::CreateChannel("localhost:50051",
                                       grpc::InsecureChannelCredentials());
    auto stub = greeter::Greeter::NewStub(channel);

    greeter::HelloRequest request;
    request.set_name("世界");

    greeter::HelloReply reply;
    grpc::ClientContext context;
    auto status = stub->SayHello(&context, request, &reply);

    if (status.ok()) {
        std::cout << "收到响应: " << reply.message() << std::endl;
    } else {
        std::cout << "RPC失败: " << status.error_message() << std::endl;
    }

    return 0;
}
```

---

## 7. REST API设计

### 1. RESTful原则

```
REST = Representational State Transfer（表述性状态转移）

核心原则：
  1. 资源（Resource）：URL标识资源，如 /users、/orders/123
  2. HTTP方法表示操作：
     GET    = 读取
     POST   = 创建
     PUT    = 全量更新
     PATCH  = 部分更新
     DELETE = 删除
  3. 无状态：每个请求包含所有必要信息
  4. 统一接口：一致的URL和HTTP方法使用方式
```

### 2. HTTP方法语义

```
资源：/users

┌────────┬──────────────┬──────────────────────────────────┐
│ 方法   │ 路径          │ 含义                             │
├────────┼──────────────┼──────────────────────────────────┤
│ GET    │ /users       │ 获取用户列表                      │
│ GET    │ /users/123   │ 获取ID为123的用户                 │
│ POST   │ /users       │ 创建新用户                        │
│ PUT    │ /users/123   │ 全量更新ID为123的用户              │
│ PATCH  │ /users/123   │ 部分更新ID为123的用户              │
│ DELETE │ /users/123   │ 删除ID为123的用户                  │
└────────┴──────────────┴──────────────────────────────────┘

幂等性：
  GET/PUT/DELETE 是幂等的（调用1次和N次效果相同）
  POST 不是幂等的（每次调用可能创建新资源）

安全性：
  GET 是安全的（不修改资源）
  POST/PUT/PATCH/DELETE 是不安全的
```

### 3. C++ REST框架

| 框架 | 特点 | 性能 |
|------|------|------|
| Drogon | C++17，基于epoll，高性能 | 极高 |
| Crow | C++11，类似Flask的API设计 | 中等 |
| Oat++ | 零依赖，REST API专用 | 高 |
| Pistache | C++17，异步HTTP框架 | 高 |
| userver | Yandex开源，异步框架 | 极高 |

**Crow 示例**：

```cpp
#include "crow.h"

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/api/users")([](){
        crow::json::wvalue x;
        x["users"] = std::vector<std::string>{"张三", "李四", "王五"};
        return x;
    });

    CROW_ROUTE(app, "/api/users/<int>")
    ([](int id){
        crow::json::wvalue x;
        x["id"] = id;
        x["name"] = "用户" + std::to_string(id);
        return x;
    });

    CROW_ROUTE(app, "/api/users").methods("POST"_method)
    ([](const crow::request& req){
        auto x = crow::json::load(req.body);
        if (!x) return crow::response(400);
        std::string name = x["name"].s();
        crow::json::wvalue res;
        res["message"] = "创建用户: " + name;
        return crow::response(201, res);
    });

    app.port(8080).multithreaded().run();
    return 0;
}
```

---

## 8. 网络编程性能优化

### 1. 零拷贝

```
传统数据发送（4次拷贝）：
  磁盘 → 内核缓冲区 → 用户缓冲区 → Socket缓冲区 → 网卡
         拷贝1         拷贝2         拷贝3           拷贝4

零拷贝技术：

sendfile（Linux）：
  磁盘 → 内核缓冲区 → 网卡
         拷贝1         DMA直接传输
  省去2次拷贝，适合静态文件服务

splice（Linux）：
  两个文件描述符之间零拷贝传输
  适合管道与Socket之间的数据转发

mmap：
  将文件映射到用户空间，避免read系统调用
  适合需要修改数据的场景
```

```cpp
// sendfile 示例（Linux）
#include <sys/sendfile.h>
#include <fcntl.h>
#include <unistd.h>

void send_file(int sock_fd, const char* filename) {
    int file_fd = open(filename, O_RDONLY);
    off_t offset = 0;
    struct stat st{};
    fstat(file_fd, &st);
    // 零拷贝发送文件
    sendfile(sock_fd, file_fd, &offset, st.st_size);
    close(file_fd);
}
```

### 2. 连接池

```
问题：每次请求都创建/销毁连接，开销大
  TCP三次握手 + TLS握手 → 延迟高
  频繁创建/销毁Socket → 资源浪费

连接池方案：
  ┌─────────────────────────────────────┐
  │           连接池                     │
  │  ┌────┐ ┌────┐ ┌────┐ ┌────┐      │
  │  │conn│ │conn│ │conn│ │conn│ 空闲  │
  │  │ 1  │ │ 2  │ │ 3  │ │ 4  │      │
  │  └──┬─┘ └────┘ └──┬─┘ └────┘      │
  │     │              │               │
  │  ┌──▼──────────────▼──┐            │
  │  │   使用中的连接      │            │
  │  │   conn1, conn3     │            │
  │  └────────────────────┘            │
  └─────────────────────────────────────┘

  请求来 → 从池中取空闲连接 → 使用 → 归还池中
  池空 → 创建新连接（达到上限则等待）
```

### 3. 缓冲区管理

```
常见问题：
  1. 频繁分配/释放内存 → 内存碎片
  2. 缓冲区太小 → 多次系统调用
  3. 缓冲区太大 → 内存浪费

优化方案：
  1. 缓冲区对象池：预分配，循环使用
  2. 动态扩容：小缓冲区起步，按需翻倍
  3. 分散读/集中写（scatter/gather I/O）：
     避免数据拼接的内存拷贝

  分散读（readv）：
  ┌──────┐   ┌──────┐   ┌──────┐
  │ buf1 │   │ buf2 │   │ buf3 │   ← 多个不连续缓冲区
  └──────┘   └──────┘   └──────┘
     ↑──────────┼──────────↑
          一次readv调用

  集中写（writev）：
  ┌──────┐   ┌──────┐   ┌──────┐
  │头部  │   │正文  │   │尾部  │   ← 无需拼接
  └──────┘   └──────┘   └──────┘
     └──────────┼──────────┘
          一次writev调用
```

### 4. TCP参数调优

```
Nagle算法：
  默认开启，将小包合并为大包发送
  减少网络中小包数量，提高带宽利用率
  但增加延迟（最多等200ms）

TCP_NODELAY：
  禁用Nagle算法，数据立即发送
  适用于延迟敏感的场景（游戏/实时通信）
  不适用于大量小包的场景

TCP_QUICKACK（Linux）：
  立即发送ACK，不延迟

SO_KEEPALIVE：
  开启TCP保活机制
  检测死连接（对端崩溃未发FIN）

SO_REUSEADDR：
  允许绑定处于TIME_WAIT状态的地址
  服务器重启时不用等待2MSL

SO_REUSEPORT（Linux）：
  多个进程/线程绑定同一端口
  内核负载均衡分发连接

SO_SNDBUF / SO_RCVBUF：
  调整发送/接收缓冲区大小
  高吞吐场景需增大

TCP_DEFER_ACCEPT（Linux）：
  延迟accept直到有数据到达
  避免空连接占用资源
```

```cpp
// 常用Socket选项设置
int sock = socket(AF_INET, SOCK_STREAM, 0);

int flag = 1;
setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)); // 禁用Nagle

setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &flag, sizeof(flag)); // 开启保活

setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)); // 地址复用

int bufsize = 256 * 1024; // 256KB
setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));
setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));
```

---

## 9. 网络安全基础

### 1. TLS/SSL 与 OpenSSL

```
TLS = Transport Layer Security（传输层安全协议）
SSL = TLS的前身（已废弃，但习惯上仍称SSL）

TLS握手过程：
  客户端                          服务端
    │                               │
    │─── ClientHello ──────────────→│ 协议版本+加密套件+随机数
    │                               │
    │←── ServerHello ──────────────│ 选定加密套件+随机数
    │←── Certificate ─────────────│ 服务端证书
    │←── ServerKeyExchange ───────│ 密钥交换参数
    │←── ServerHelloDone ─────────│
    │                               │
    │─── ClientKeyExchange ────────→│ 客户端密钥交换参数
    │─── ChangeCipherSpec ────────→│ 切换到加密通信
    │─── Finished(加密) ──────────→│
    │                               │
    │←── ChangeCipherSpec ─────────│ 切换到加密通信
    │←── Finished(加密) ───────────│
    │                               │
    │←══════ 加密数据传输 ════════→│
```

### 2. OpenSSL 使用示例

```cpp
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

void init_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

void cleanup_openssl() {
    EVP_cleanup();
}

SSL_CTX* create_context() {
    const SSL_METHOD* method = TLS_server_method();
    SSL_CTX* ctx = SSL_CTX_new(method);
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        exit(1);
    }
    return ctx;
}

void configure_context(SSL_CTX* ctx, const char* cert, const char* key) {
    SSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_PEM);
    SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM);
}

int main() {
    init_openssl();
    SSL_CTX* ctx = create_context();
    configure_context(ctx, "server.crt", "server.key");

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(4433);
    bind(listen_fd, (sockaddr*)&addr, sizeof(addr));
    listen(listen_fd, 10);

    while (true) {
        int client_fd = accept(listen_fd, nullptr, nullptr);
        SSL* ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client_fd);

        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
        } else {
            char buf[4096];
            int n = SSL_read(ssl, buf, sizeof(buf));
            if (n > 0) {
                const char* response = "HTTP/1.1 200 OK\r\n\r\nHello TLS!";
                SSL_write(ssl, response, strlen(response));
            }
        }

        SSL_free(ssl);
        close(client_fd);
    }

    close(listen_fd);
    SSL_CTX_free(ctx);
    cleanup_openssl();
    return 0;
}
```

### 3. 证书验证

```
证书链验证：
  根CA（自签名，内置在浏览器/系统中）
    └── 中间CA
          └── 服务端证书

  客户端验证流程：
  1. 检查证书是否在有效期内
  2. 检查证书的域名是否匹配
  3. 沿证书链向上验证签名
  4. 检查根CA是否在信任列表中
  5. 检查证书是否被吊销（CRL/OCSP）

常见问题：
  - 自签名证书：浏览器警告，开发环境可用
  - 过期证书：需要续签
  - 域名不匹配：证书的CN/SAN必须匹配访问域名
  - 证书链不完整：需包含中间CA证书
```

### 4. 防止常见攻击

```
1. SQL注入（API中的防御）：
   攻击：GET /api/users?id=1 OR 1=1
   防御：使用参数化查询，不拼接SQL

2. CSRF（跨站请求伪造）：
   攻击：恶意网站向你的API发送请求（利用浏览器自动带Cookie）
   防御：
   - 使用CSRF Token
   - 检查Referer/Origin头
   - SameSite Cookie属性
   - 关键操作使用POST/PUT而非GET

3. XSS（跨站脚本）：
   攻击：注入<script>标签执行恶意代码
   防御：
   - 对用户输入进行HTML转义
   - 设置Content-Security-Policy头
   - HttpOnly Cookie（防止JS读取）

4. 其他API安全措施：
   - 速率限制（Rate Limiting）：防止暴力破解和DDoS
   - 输入验证：验证所有请求数据的格式和范围
   - HTTPS：所有通信加密
   - 认证与授权：JWT/OAuth2
   - 日志与监控：记录异常请求
```

***

### 5. 相关章节

- [网络编程](../02-CPP/35-网络编程.md) — Socket基础与TCP/UDP编程
- [网络编程基础](../../01-C语言/20-网络编程基础.md) — C语言Socket编程
- [框架引擎中间件与架构概念指南](../10-工程实践/16-框架引擎中间件与架构.md) — 框架/引擎/中间件/架构全景
- [序列化与反序列化](../10-工程实践/20-序列化与反序列化.md) — JSON/Protobuf/MessagePack序列化
- [序列化与日志](../../02-CPP/36-序列化与日志.md) — C++序列化库与日志系统

***

### 相关阅读

- [什么是I-O多路复用](../06-并发编程/33-什么是I-O多路复用.md)
- [管道与IPC](../09-系统与安全/09-管道与IPC.md)
- [序列化与反序列化](./20-序列化与反序列化.md)