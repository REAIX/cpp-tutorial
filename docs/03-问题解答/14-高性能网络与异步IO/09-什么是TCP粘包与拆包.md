# 什么是TCP粘包与拆包
> 📖 相关章节：[网络编程](../../02-CPP/35-网络编程.md)、[IO多路复用](../../08-高性能网络与异步IO/01-IO多路复用深入.md)、[Reactor模式](../../08-高性能网络与异步IO/02-Reactor模式.md)

> "要点直击：TCP是字节流协议，没有消息边界——发送'Hello''World'，接收方可能收到'HelloWorld'或'Hel''loWorld'。粘包拆包就是如何在这个字节流中正确切分出每条消息。"

***

### 1. 通俗理解

- **粘包**：发送方发送的多条消息被接收方一次性收到，粘在一起
- **拆包**：发送方发送的一条消息被接收方分多次收到，拆成多段
- 根本原因：TCP是流式协议，不保证消息边界

| 概念 | 类比 | 说明 |
|------|------|------|
| TCP字节流 | 水管里的水 | 没有消息边界，水是连续的 |
| 粘包 | 两杯水倒进一个杯子 | 多条消息粘在一起 |
| 拆包 | 一杯水分两次倒 | 一条消息被拆成多段 |
| 定长帧 | 固定大小的冰块 | 每条消息固定长度 |
| 分隔符 | 冰格的隔板 | 用特殊字符分隔消息 |
| 长度字段 | 每块冰贴标签 | 用长度标记消息边界 |

**粘包拆包的四种场景**：

```
发送方连续发送两条消息: [Msg1][Msg2]

场景1: 正常（无粘包拆包）
  接收方: [Msg1] [Msg2]     ← 两次各收到一条完整消息

场景2: 粘包
  接收方: [Msg1Msg2]        ← 一次收到两条消息粘在一起

场景3: 拆包
  接收方: [Msg1前半] [Msg1后半Msg2]  ← Msg1被拆成两段

场景4: 拆包+粘包
  接收方: [Msg1前半] [Msg1后半Msg2前半] [Msg2后半]
```

***

### 2. 技术说明

#### 1. 粘包拆包的原因

**TCP层面的原因**：

| 原因 | 说明 |
|------|------|
| Nagle算法 | 小包合并发送，减少网络包数 |
| TCP发送缓冲区 | 多次write的数据可能合并发送 |
| TCP接收缓冲区 | 多次到达的数据可能合并读取 |
| MSS限制 | 超过MSS的大消息会被分片 |
| 网络延迟 | 数据包到达时间不确定 |

**Nagle算法与粘包**：

```
不开启Nagle（TCP_NODELAY）:
  write("A") → 立即发送包1
  write("B") → 立即发送包2
  write("C") → 立即发送包3
  （3个小包，不粘包但效率低）

开启Nagle（默认）:
  write("A") → 等待，合并
  write("B") → 等待，合并
  write("C") → 一起发送"ABC"
  （1个包，效率高但粘包）
```

**重要澄清**：

> "粘包"这个名字有误导性。TCP并没有"把包粘在一起"——TCP根本不知道什么是"消息"，它只保证字节流的顺序和可靠传输。所谓"粘包拆包"是**应用层**的问题，因为应用层需要从字节流中恢复消息边界。

#### 2. 解决方案：定长消息

**原理**：每条消息固定长度，不足的用填充字节补齐。

```
┌──────────────────────────────┐
│       固定 64 字节            │
│  "Hello\0\0\0...\0"          │  ← 不足部分用0填充
└──────────────────────────────┘
┌──────────────────────────────┐
│       固定 64 字节            │
│  "World\0\0\0...\0"          │
└──────────────────────────────┘
```

**优点**：实现最简单，解析O(1)
**缺点**：浪费带宽，长度不可变

**适用场景**：固定结构的控制消息、物联网协议

#### 3. 解决方案：分隔符

**原理**：用特殊字符作为消息结束标记。

```
"Hello\r\nWorld\r\n"
  ↓ 解析
消息1: "Hello"
消息2: "World"
```

**常见分隔符**：

| 协议 | 分隔符 |
|------|--------|
| HTTP/1.1 | `\r\n\r\n`（头部结束） |
| Redis | `\r\n` |
| SMTP | `\r\n` |
| MQTT | 长度前缀（不是分隔符） |

**分隔符的转义问题**：

```
如果消息体本身包含分隔符，需要转义：

原始数据: "Hello\r\nWorld"  （消息体中包含\r\n）
转义后:   "Hello\\r\\nWorld\r\n"  （消息体中的\r\n转义，末尾\r\n是分隔符）

转义规则：
  \ → \\
  分隔符 → 转义序列
```

**优点**：直观、文本友好
**缺点**：需要转义、需要逐字节扫描、二进制数据不适用

#### 4. 解决方案：长度字段（最常用）

**原理**：消息头包含消息体长度，接收方先读长度，再读对应长度的消息体。

```
┌──────────┬───────────────────────────────┐
│  Length  │          Payload              │
│  (4字节) │         (N字节)               │
│  N=5     │  "Hello"                      │
└──────────┴───────────────────────────────┘
┌──────────┬───────────────────────────────┐
│  Length  │          Payload              │
│  (4字节) │         (N字节)               │
│  N=5     │  "World"                      │
└──────────┴───────────────────────────────┘
```

**长度字段的设计选择**：

| 选择 | 说明 | 建议 |
|------|------|------|
| 字段大小 | 1/2/4/8字节 | 4字节（支持4GB） |
| 字节序 | 大端/小端 | 大端（网络序） |
| 包含自身 | 长度是否包含长度字段本身 | 明确约定 |
| 包含头部 | 长度是否包含其他头部字段 | 明确约定 |

**更完整的帧格式**：

```
┌──────────┬──────────┬──────────┬───────────────────────────┐
│  Magic   │  Type    │  Length  │       Payload             │
│  (2字节) │  (1字节)  │  (4字节) │       (N字节)             │
│  0xABCD  │  0x01    │  N       │     消息体                │
└──────────┴──────────┴──────────┴───────────────────────────┘

Magic:  快速识别协议，防止误读
Type:   消息类型（请求/响应/通知等）
Length: Payload的长度（不包含头部7字节）
Payload: 消息体
```

**解码器状态机**：

```
状态1: 读取头部（7字节）
  ↓ 头部读完整
状态2: 读取Payload（Length字节）
  ↓ Payload读完整
状态3: 分发完整消息
  ↓ 回到状态1
```

#### 5. Netty的解码器设计

Netty提供了成熟的解码器实现，是学习解码器设计的最佳参考。

**核心解码器**：

| 解码器 | 说明 |
|--------|------|
| FixedLengthFrameDecoder | 定长帧解码器 |
| DelimiterBasedFrameDecoder | 分隔符帧解码器 |
| LengthFieldBasedFrameDecoder | 长度字段帧解码器 |
| LineBasedFrameDecoder | 行分隔帧解码器 |

**LengthFieldBasedFrameDecoder的参数**：

```
┌──────────┬──────────┬──────────┬───────────────────┐
│ Header1  │  Length  │ Header2  │     Payload       │
│ (2字节)  │  (3字节)  │ (2字节)  │     (N字节)       │
└──────────┴──────────┴──────────┴───────────────────┘

参数说明：
  lengthFieldOffset = 2      ← Length字段在Header1之后
  lengthFieldLength = 3      ← Length字段占3字节
  lengthAdjustment = 2       ← Length值 + 2 = 整个帧长度（加上Header2）
  initialBytesToStrip = 0    ← 不剥离任何字节

如果Length的值 = Payload长度：
  lengthAdjustment = Header2长度 = 2
  总帧长 = Header1 + Length字段 + Header2 + Payload
         = 2 + 3 + 2 + Length值

如果Length的值 = 整个帧长度（含头部）：
  lengthAdjustment = -(Header1 + Length字段 + Header2) = -7
  总帧长 = Length值
```

**Netty解码器的工作原理**：

```
ByteBuf（累积缓冲区）:
  ┌──────────────────────────────────────────┐
  │ 已读 | 未读数据（可能包含多条不完整消息）  │
  └──────────────────────────────────────────┘

每次channelRead：
  1. 新数据追加到ByteBuf
  2. 循环尝试解码：
     a. 检查是否有足够的字节读头部
     b. 读取长度字段
     c. 检查是否有足够的字节读完整消息
     d. 如果完整，提取消息，继续循环
     e. 如果不完整，等待下次数据
  3. 丢弃已读字节，压缩缓冲区
```

***

### 3. 代码示例

#### 1. 长度字段帧解码器

```cpp
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <vector>
#include <arpa/inet.h>

// 帧格式: [Length(4字节,大端)] [Payload(N字节)]
class LengthFieldDecoder {
public:
    // 输入新收到的数据，返回完整的消息列表
    std::vector<std::vector<uint8_t>> decode(const uint8_t* data, size_t len) {
        std::vector<std::vector<uint8_t>> messages;

        // 将新数据追加到缓冲区
        buffer_.insert(buffer_.end(), data, data + len);

        // 循环尝试提取完整消息
        while (true) {
            // 步骤1：检查是否有足够的字节读长度字段
            if (buffer_.size() < HEADER_SIZE) break;

            // 步骤2：读取长度字段（不移动读指针）
            uint32_t payload_len;
            memcpy(&payload_len, buffer_.data(), 4);
            payload_len = ntohl(payload_len);

            // 步骤3：安全检查
            if (payload_len > MAX_PAYLOAD_SIZE) {
                printf("[解码器] 恶意帧: payload长度%u超过限制%u\n",
                       payload_len, MAX_PAYLOAD_SIZE);
                buffer_.clear();
                break;
            }

            // 步骤4：检查是否有完整的消息
            size_t total_frame_size = HEADER_SIZE + payload_len;
            if (buffer_.size() < total_frame_size) break;  // 数据不完整

            // 步骤5：提取完整消息
            std::vector<uint8_t> payload(
                buffer_.begin() + HEADER_SIZE,
                buffer_.begin() + total_frame_size);
            messages.push_back(std::move(payload));

            // 步骤6：从缓冲区移除已处理的帧
            buffer_.erase(buffer_.begin(), buffer_.begin() + total_frame_size);
        }

        return messages;
    }

    // 编码：将消息加上长度前缀
    static std::vector<uint8_t> encode(const uint8_t* data, size_t len) {
        std::vector<uint8_t> frame;
        uint32_t len_be = htonl(static_cast<uint32_t>(len));
        frame.reserve(4 + len);
        frame.insert(frame.end(), (uint8_t*)&len_be, (uint8_t*)&len_be + 4);
        frame.insert(frame.end(), data, data + len);
        return frame;
    }

    // 获取缓冲区大小（用于监控）
    size_t bufferSize() const { return buffer_.size(); }

private:
    static constexpr size_t HEADER_SIZE = 4;
    static constexpr size_t MAX_PAYLOAD_SIZE = 1024 * 1024;  // 1MB
    std::vector<uint8_t> buffer_;
};

// 测试
int main(void) {
    LengthFieldDecoder decoder;

    // ===== 测试1：正常情况 =====
    printf("=== 测试1：正常情况 ===\n");
    auto msg1 = LengthFieldDecoder::encode((const uint8_t*)"Hello", 5);
    auto msg2 = LengthFieldDecoder::encode((const uint8_t*)"World", 5);

    // 两条消息一起到达（粘包）
    std::vector<uint8_t> combined;
    combined.insert(combined.end(), msg1.begin(), msg1.end());
    combined.insert(combined.end(), msg2.begin(), msg2.end());

    auto results = decoder.decode(combined.data(), combined.size());
    printf("粘包: 收到%zu条消息\n", results.size());
    for (size_t i = 0; i < results.size(); i++) {
        printf("  消息%zu: %.*s\n", (unsigned)i,
               (int)results[i].size(), (char*)results[i].data());
    }

    // ===== 测试2：拆包情况 =====
    printf("\n=== 测试2：拆包情况 ===\n");
    LengthFieldDecoder decoder2;
    auto big_msg = LengthFieldDecoder::encode(
        (const uint8_t*)"This is a longer message for testing", 36);

    // 分三次到达
    size_t part1 = 3;   // 不完整的长度字段
    size_t part2 = 10;  // 长度字段剩余 + 部分payload
    size_t part3 = big_msg.size() - part1 - part2;

    auto r1 = decoder2.decode(big_msg.data(), part1);
    printf("第1片(%zu字节): %zu条消息\n", part1, r1.size());

    auto r2 = decoder2.decode(big_msg.data() + part1, part2);
    printf("第2片(%zu字节): %zu条消息\n", part2, r2.size());

    auto r3 = decoder2.decode(big_msg.data() + part1 + part2, part3);
    printf("第3片(%zu字节): %zu条消息\n", part3, r3.size());
    if (!r3.empty()) {
        printf("  消息: %.*s\n", (int)r3[0].size(), (char*)r3[0].data());
    }

    // ===== 测试3：混合情况 =====
    printf("\n=== 测试3：粘包+拆包 ===\n");
    LengthFieldDecoder decoder3;
    auto m1 = LengthFieldDecoder::encode((const uint8_t*)"ABC", 3);
    auto m2 = LengthFieldDecoder::encode((const uint8_t*)"DEFGH", 5);
    auto m3 = LengthFieldDecoder::encode((const uint8_t*)"IJKL", 4);

    std::vector<uint8_t> all;
    all.insert(all.end(), m1.begin(), m1.end());
    all.insert(all.end(), m2.begin(), m2.end());
    all.insert(all.end(), m3.begin(), m3.end());

    // 分两次到达
    size_t split = 8;  // 随机切分点
    auto ra = decoder3.decode(all.data(), split);
    auto rb = decoder3.decode(all.data() + split, all.size() - split);

    int total = ra.size() + rb.size();
    printf("混合: 共收到%d条消息\n", total);
    for (auto& msg : ra) {
        printf("  消息: %.*s\n", (int)msg.size(), (char*)msg.data());
    }
    for (auto& msg : rb) {
        printf("  消息: %.*s\n", (int)msg.size(), (char*)msg.data());
    }

    return 0;
}
```

#### 2. 分隔符帧解码器

```cpp
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

// 分隔符帧解码器
class DelimiterFrameDecoder {
public:
    DelimiterFrameDecoder(const std::string& delimiter)
        : delimiter_(delimiter) {}

    std::vector<std::string> decode(const char* data, size_t len) {
        std::vector<std::string> messages;

        // 追加到缓冲区
        buffer_.append(data, len);

        // 循环查找分隔符
        while (true) {
            size_t pos = buffer_.find(delimiter_);
            if (pos == std::string::npos) break;

            // 提取消息（不包含分隔符）
            std::string msg = buffer_.substr(0, pos);
            messages.push_back(std::move(msg));

            // 移除已处理的消息和分隔符
            buffer_.erase(0, pos + delimiter_.size());
        }

        return messages;
    }

private:
    std::string delimiter_;
    std::string buffer_;
};

// 测试
int main(void) {
    DelimiterFrameDecoder decoder("\r\n");

    // 模拟Redis协议
    const char* data = "+OK\r\n$5\r\nhello\r\n+PONG\r\n";

    auto messages = decoder.decode(data, strlen(data));
    printf("收到%zu条消息:\n", messages.size());
    for (size_t i = 0; i < messages.size(); i++) {
        printf("  [%zu] %s\n", (unsigned)i, messages[i].c_str());
    }

    return 0;
}
```

#### 3. 完整的网络读写+帧解码

```cpp
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <errno.h>
#include <arpa/inet.h>
#include <vector>
#include <unordered_map>
#include <string>

#define PORT         8080
#define MAX_EVENTS   1024
#define MAX_PAYLOAD  (1024 * 1024)

// 连接上下文：每个连接有自己的读缓冲区
struct ConnContext {
    int fd;
    std::vector<uint8_t> read_buf;  // 读缓冲区
};

// 帧解码器（每个连接一个）
class FrameDecoder {
public:
    // 输入新数据，返回完整消息列表
    std::vector<std::vector<uint8_t>> feed(const uint8_t* data, size_t len) {
        std::vector<std::vector<uint8_t>> messages;
        read_buf_.insert(read_buf_.end(), data, data + len);

        while (true) {
            if (read_buf_.size() < 4) break;  // 长度字段不完整

            uint32_t payload_len;
            memcpy(&payload_len, read_buf_.data(), 4);
            payload_len = ntohl(payload_len);

            if (payload_len > MAX_PAYLOAD) {
                read_buf_.clear();
                break;
            }

            size_t frame_size = 4 + payload_len;
            if (read_buf_.size() < frame_size) break;  // 消息不完整

            // 提取完整消息
            messages.emplace_back(read_buf_.begin() + 4,
                                 read_buf_.begin() + frame_size);
            read_buf_.erase(read_buf_.begin(),
                           read_buf_.begin() + frame_size);
        }

        return messages;
    }

    // 编码消息
    static std::vector<uint8_t> encode(const std::vector<uint8_t>& payload) {
        std::vector<uint8_t> frame;
        uint32_t len = htonl(payload.size());
        frame.resize(4 + payload.size());
        memcpy(frame.data(), &len, 4);
        memcpy(frame.data() + 4, payload.data(), payload.size());
        return frame;
    }

private:
    std::vector<uint8_t> read_buf_;
};

static int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main(void) {
    int listen_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(PORT)
    };
    bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(listen_fd, 512);

    int epfd = epoll_create1(0);

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = listen_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);

    struct epoll_event* events = new epoll_event[MAX_EVENTS];

    // 每个连接一个解码器
    std::unordered_map<int, FrameDecoder> decoders;

    printf("帧解码服务器启动，端口: %d\n", PORT);

    while (1) {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);

        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == listen_fd) {
                // 新连接
                while (1) {
                    int client_fd = accept(listen_fd, NULL, NULL);
                    if (client_fd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                        break;
                    }
                    set_nonblocking(client_fd);

                    struct epoll_event cev;
                    cev.events = EPOLLIN | EPOLLET;
                    cev.data.fd = client_fd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &cev);

                    // 创建该连接的解码器
                    decoders.emplace(client_fd, FrameDecoder());
                    printf("[连接] fd=%d\n", client_fd);
                }
            } else {
                int fd = events[i].data.fd;
                char buf[4096];

                // ET模式循环读
                while (1) {
                    int n = read(fd, buf, sizeof(buf));
                    if (n > 0) {
                        // 将数据喂给解码器
                        auto& decoder = decoders[fd];
                        auto messages = decoder.feed((uint8_t*)buf, n);

                        // 处理每条完整消息
                        for (auto& msg : messages) {
                            printf("[消息] fd=%d, 长度=%zu, 内容=%.*s\n",
                                   fd, msg.size(),
                                   (int)msg.size(), (char*)msg.data());

                            // 回显：编码后发送
                            auto frame = FrameDecoder::encode(msg);
                            write(fd, frame.data(), frame.size());
                        }
                    } else if (n == 0) {
                        close(fd);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                        decoders.erase(fd);
                        printf("[断开] fd=%d\n", fd);
                        break;
                    } else {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                        close(fd);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                        decoders.erase(fd);
                        break;
                    }
                }
            }
        }
    }

    delete[] events;
    close(epfd);
    close(listen_fd);
    return 0;
}
```

***

### 4. 常见问题

#### Q1：UDP有粘包问题吗？

没有。UDP是面向消息的协议，每个sendto对应一个recvfrom，消息边界由协议保证。但UDP有消息大小限制（通常不超过64KB），且不保证可靠传输。

#### Q2：TCP为什么不保留消息边界？

因为TCP的设计目标是"可靠的字节流"，不是"可靠的消息流"。保留消息边界会增加协议复杂度，而且很多场景（如文件传输）不需要消息边界。应用层可以自己定义边界。

#### Q3：关闭Nagle算法能解决粘包吗？

不能根本解决。关闭Nagle（TCP_NODELAY）只是减少发送端的粘包，接收端仍可能因为TCP接收缓冲区的合并读取而"粘包"。正确的做法是在应用层定义消息边界。

#### Q4：长度字段本身被拆包了怎么办？

解码器需要用状态机处理：先累积4字节读出长度，再累积N字节读出消息体。上面的代码已经处理了这种情况——缓冲区不够时等待更多数据。

#### Q5：如何防止恶意的大长度字段？

1. 设置最大消息长度（如1MB）
2. 超过限制直接断开连接
3. 记录异常日志，用于安全审计

***

### 5. 总结

| 方案 | 优点 | 缺点 | 适用场景 |
|------|------|------|---------|
| 定长 | 最简单 | 浪费带宽 | 固定结构消息 |
| 分隔符 | 文本友好 | 需转义、逐字节扫描 | 文本协议（HTTP、Redis） |
| 长度字段 | 高效通用 | 需要状态机解码 | 二进制协议（推荐） |

TCP粘包拆包的本质是**应用层需要在字节流上定义消息边界**。长度字段方案是最通用、最高效的解决方案，Netty的LengthFieldBasedFrameDecoder是最佳实践参考。记住：**永远不要假设一次read对应一条消息**——这是网络编程的基本常识。