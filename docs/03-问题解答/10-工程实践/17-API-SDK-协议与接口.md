# API/SDK/协议/接口 — 软件交互核心概念指南
> 📖 相关章节：[编码规范](../../04-工程实践/00-编码规范.md)、[设计模式](../../04-工程实践/03-设计模式.md)、[单元测试](../../04-工程实践/05-单元测试.md)、[代码审查](../../04-工程实践/07-代码审查.md)

> "API是菜单，SDK是厨房，协议是规矩，接口是插座——理解这四个概念，就理解了软件之间如何对话。"

***

## 1. API（应用程序编程接口）— 软件的"菜单"

### 1. 什么是API

**比喻**：API像餐厅的菜单——菜单告诉你有什么菜（功能）、每道菜需要什么原料（参数）、上菜是什么样子（返回值），但**不告诉你菜是怎么做的**（实现细节）。

```
┌──────────────────────────────────────────────────┐
│                   餐厅菜单（API）                  │
├──────────────────────────────────────────────────┤
│  宫保鸡丁 ─── 需要：口味偏好 ───→ 一盘鸡丁       │
│  红烧肉   ─── 需要：份量大小 ───→ 一盘红烧肉     │
│  蛋炒饭   ─── 需要：加什么蛋   ───→ 一碗炒饭     │
├──────────────────────────────────────────────────┤
│  你不需要知道厨房怎么炒菜，只需要按菜单点菜       │
└──────────────────────────────────────────────────┘
```

**精确定义**：API是软件组件之间交互的契约，定义了"谁提供什么功能、需要什么输入、返回什么输出"，但不暴露内部实现。

### 2. API的类型

| 类型 | 说明 | 比喻 | 示例 |
|------|------|------|------|
| **系统API** | 操作系统提供给应用程序的接口 | 政府办事窗口 | Win32 API、POSIX API、Linux系统调用 |
| **库API** | 库/框架提供给开发者的接口 | 工具箱里的工具说明书 | STL API、OpenSSL API、zlib API |
| **Web API** | 通过网络调用的远程接口 | 外卖平台下单 | GitHub REST API、微信支付API |
| **REST API** | 基于HTTP的标准化Web API | 按规矩点外卖 | GET /users、POST /orders |

### 3. C/C++中的API示例

**POSIX API**（跨平台系统接口）：

```c
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(void) {
    /* POSIX API：open/read/write/close */
    int fd = open("data.txt", O_RDONLY);
    if (fd == -1) {
        perror("open失败");
        return 1;
    }

    char buf[256];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        printf("读取内容: %s\n", buf);
    }

    close(fd);
    return 0;
}
```

**Win32 API**（Windows系统接口）：

```c
#include <windows.h>
#include <stdio.h>

int main(void) {
    /* Win32 API：CreateFile/ReadFile/CloseHandle */
    HANDLE hFile = CreateFileA(
        "data.txt",
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        printf("打开文件失败\n");
        return 1;
    }

    char buf[256];
    DWORD bytesRead;
    if (ReadFile(hFile, buf, sizeof(buf) - 1, &bytesRead, NULL)) {
        buf[bytesRead] = '\0';
        printf("读取内容: %s\n", buf);
    }

    CloseHandle(hFile);
    return 0;
}
```

**STL API**（C++标准库接口）：

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

int main() {
    /* STL API：vector/sort/for_each */
    std::vector<int> nums = {5, 3, 1, 4, 2};

    std::sort(nums.begin(), nums.end());

    for (int n : nums) {
        std::cout << n << " ";
    }
    std::cout << std::endl;
    return 0;
}
```

### 4. API设计原则

| 原则 | 说明 | 反面教材 |
|------|------|----------|
| **最小化** | 只暴露必要的功能 | 把内部函数也放进头文件 |
| **一致性** | 命名和参数风格统一 | `get_size()`和`length()`混用 |
| **稳定性** | 不随意改变已有接口 | 改了函数签名导致调用方崩溃 |
| **文档化** | 每个API都有清晰说明 | 没有注释，参数含义靠猜 |
| **版本化** | 通过版本号管理变更 | 直接修改旧接口，不提供兼容 |

```c
/* 好的API设计示例 */
/* 统一命名、清晰参数、错误处理 */

/* 创建对象，返回句柄，失败返回NULL */
typedef struct parser *parser_handle;
parser_handle parser_create(const char *config);

/* 使用对象，返回0成功，非0失败 */
int parser_parse(parser_handle h, const char *input);

/* 销毁对象，释放资源 */
void parser_destroy(parser_handle h);
```

***

## 2. SDK（软件开发工具包）— 开发者的"工具箱"

### 1. 什么是SDK

**比喻**：如果API是菜单，SDK就是**整个厨房**——菜单（API）+ 食材（库文件）+ 厨具（工具链）+ 食谱（文档和示例）。

```
┌─────────────────────────────────────────────────┐
│                   SDK = 整个厨房                  │
├─────────────────────────────────────────────────┤
│                                                   │
│   📋 菜单（API头文件）                            │
│   ┌─────────────────────┐                        │
│   │ 有哪些功能可以用     │                        │
│   └─────────────────────┘                        │
│                                                   │
│   🥩 食材（库文件 .lib/.so/.dll）                 │
│   ┌─────────────────────┐                        │
│   │ 功能的编译好的实现   │                        │
│   └─────────────────────┘                        │
│                                                   │
│   🔧 厨具（编译器/调试器/工具）                    │
│   ┌─────────────────────┐                        │
│   │ 构建和调试的工具     │                        │
│   └─────────────────────┘                        │
│                                                   │
│   📖 食谱（文档/示例/教程）                        │
│   ┌─────────────────────┐                        │
│   │ 教你怎么用这些工具   │                        │
│   └─────────────────────┘                        │
│                                                   │
└─────────────────────────────────────────────────┘
```

**精确定义**：SDK是一整套开发工具的集合，包含API（头文件）、库文件、编译工具、调试工具、文档和示例代码，帮助开发者在特定平台上构建应用。

### 2. SDK vs API的区别

| 对比项 | API | SDK |
|--------|-----|-----|
| **比喻** | 菜单 | 整个厨房 |
| **本质** | 接口规范（契约） | 工具集合（工具箱） |
| **包含关系** | SDK包含API | — |
| **内容** | 函数签名、参数、返回值 | API + 库文件 + 工具 + 文档 + 示例 |
| **使用方式** | 调用函数 | 安装、配置、链接、调用 |
| **独立性** | 不能单独使用（需要实现） | 可以独立使用 |
| **体积** | 很小（几个头文件） | 很大（几GB） |
| **示例** | OpenGL API（函数声明） | Windows SDK（头文件+库+工具+文档） |

```
包含关系：

  SDK
  ├── API（头文件 .h）        ← 接口声明
  ├── 库文件（.lib/.so/.dll） ← 编译好的实现
  ├── 工具（编译器/调试器）    ← 开发工具
  ├── 文档（参考手册）        ← 使用说明
  └── 示例（sample code）     ← 代码模板

  API只是SDK的一部分
```

### 3. 常见SDK

| SDK | 平台 | 包含内容 | 体积 |
|-----|------|----------|------|
| **Windows SDK** | Windows | Win32 API头文件、导入库、编译工具、资源编译器 | 数GB |
| **Android NDK** | Android | C/C++头文件、交叉编译工具链、JNI示例 | 数GB |
| **iOS SDK** | iOS/macOS | Objective-C/Swift头文件、Xcode工具链、模拟器 | 数GB |
| **Java JDK** | 跨平台 | Java类库、javac编译器、jdb调试器、javadoc | 数百MB |
| **CUDA SDK** | NVIDIA GPU | CUDA运行时、nvcc编译器、cuBLAS/cuDNN库 | 数GB |

### 4. C/C++ SDK的组成

```
一个典型的C/C++ SDK目录结构：

  my-sdk/
  ├── include/              ← API头文件（接口声明）
  │   ├── mylib.h
  │   └── mylib_config.h
  ├── lib/                  ← 静态库/导入库
  │   ├── libmylib.a        ← Linux静态库
  │   ├── libmylib.so       ← Linux动态库
  │   ├── mylib.lib         ← Windows静态库/导入库
  │   └── mylib.dll         ← Windows动态库
  ├── bin/                  ← 工具可执行文件
  │   └── mylib-config
  ├── docs/                 ← 文档
  │   ├── api-reference.html
  │   └── getting-started.md
  ├── examples/             ← 示例代码
  │   ├── basic/
  │   └── advanced/
  └── cmake/                ← CMake配置
      └── mylibConfig.cmake
```

```c
/* 使用SDK的典型流程 */

/* 1. 包含SDK的头文件（API） */
#include <mylib.h>

int main(void) {
    /* 2. 调用SDK的API */
    mylib_handle h = mylib_init("config.json");
    if (h == NULL) {
        fprintf(stderr, "初始化失败\n");
        return 1;
    }

    int result = mylib_process(h, "input.dat");
    printf("处理结果: %d\n", result);

    /* 3. 释放资源 */
    mylib_cleanup(h);
    return 0;
}
```

***

## 3. 协议（Protocol）— 通信的"规则"

### 1. 什么是协议

**比喻**：协议像**外交礼仪**——两国元首会面，按什么规矩来？谁先伸手、谁先讲话、用什么语言、座次怎么排，这些都是事先约定的规则。不按规矩来，沟通就失败。

```
┌─────────────────────────────────────────────────┐
│              外交礼仪（协议）                     │
├─────────────────────────────────────────────────┤
│                                                   │
│  规则1：见面先握手（建立连接）                     │
│  规则2：用英语交谈（编码格式）                     │
│  规则3：一方说完另一方再说（传输顺序）              │
│  规则4：说完挥手告别（断开连接）                   │
│                                                   │
│  不遵守规则 → 外交事故（通信失败）                  │
│                                                   │
└─────────────────────────────────────────────────┘
```

**精确定义**：协议是通信双方约定的规则集合，规定了数据格式、传输顺序、错误处理等，确保双方能正确理解对方的信息。

### 2. 网络协议分层

```
OSI七层模型 vs TCP/IP四层模型：

┌──────────────┐  ┌──────────────────────────────────────┐
│  应用层      │  │  HTTP / FTP / SMTP / DNS / WebSocket │  ← 你写的代码在这
├──────────────┤  ├──────────────────────────────────────┤
│  表示层      │  │  TLS / SSL / JSON / XML / Protobuf   │  ← 数据格式/加密
├──────────────┤  ├──────────────────────────────────────┤
│  会话层      │  │  会话管理 / Token认证                 │  ← 会话控制
├──────────────┤  ╞══════════════════════════════════════╡
│  传输层      │  │  TCP / UDP / QUIC                    │  ← 端到端传输
├──────────────┤  ├──────────────────────────────────────┤
│  网络层      │  │  IP / ICMP / ARP                     │  ← 路由寻址
├──────────────┤  ├──────────────────────────────────────┤
│  链路层      │  │  以太网 / Wi-Fi / PPP                │  ← 物理帧传输
├──────────────┤  ├──────────────────────────────────────┤
│  物理层      │  │  电缆 / 光纤 / 无线电波              │  ← 比特流
└──────────────┘  └──────────────────────────────────────┘
```

**各层常见协议**：

| 层级 | 协议 | 作用 | 比喻 |
|------|------|------|------|
| 应用层 | HTTP | 网页请求/响应 | 写信的内容和格式 |
| 应用层 | FTP | 文件传输 | 寄包裹 |
| 应用层 | SMTP | 发送邮件 | 投递信件 |
| 应用层 | DNS | 域名→IP | 查电话号码本 |
| 传输层 | TCP | 可靠传输 | 挂号信（保证送达） |
| 传输层 | UDP | 快速传输 | 广播（不保证送达） |
| 网络层 | IP | 寻址路由 | 信封上的地址 |
| 网络层 | ICMP | 网络诊断 | 查无此人的退信 |
| 链路层 | 以太网 | 局域网帧传输 | 邮递员在同一条街送信 |

### 3. 编程中的协议

**序列化协议**（数据如何编码/解码）：

| 协议 | 格式 | 特点 | 适用场景 |
|------|------|------|----------|
| JSON | 文本 | 可读性好，体积大 | Web API、配置文件 |
| XML | 文本 | 可读性好，冗余多 | SOAP、配置文件 |
| Protobuf | 二进制 | 高效紧凑，需定义schema | 高性能RPC、微服务 |
| MessagePack | 二进制 | 兼容JSON，更紧凑 | 游戏通信、IoT |
| FlatBuffers | 二进制 | 零拷贝反序列化 | 游戏、实时系统 |

**通信协议**（数据如何传输）：

```c
/* 简单的自定义二进制通信协议示例 */

/*
 * 协议格式：
 * ┌──────┬──────┬──────┬──────────┬──────┐
 * │ 魔数 │ 版本 │ 类型 │   长度   │ 数据 │
 * │ 2B   │ 1B   │ 1B   │   4B     │ NB   │
 * └──────┴──────┴──────┴──────────┴──────┘
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define MAGIC 0xCAFE

#pragma pack(push, 1)
typedef struct {
    uint16_t magic;
    uint8_t  version;
    uint8_t  type;
    uint32_t length;
} proto_header;
#pragma pack(pop)

/* 编码：构造协议包 */
int encode_packet(uint8_t type, const void *data,
                  uint32_t len, uint8_t *out) {
    proto_header hdr;
    hdr.magic   = MAGIC;
    hdr.version = 1;
    hdr.type    = type;
    hdr.length  = len;

    memcpy(out, &hdr, sizeof(hdr));
    memcpy(out + sizeof(hdr), data, len);
    return sizeof(hdr) + len;
}

/* 解码：解析协议包 */
int decode_packet(const uint8_t *buf, uint32_t buf_len,
                  proto_header *hdr, const uint8_t **data) {
    if (buf_len < sizeof(proto_header)) {
        return -1;
    }
    memcpy(hdr, buf, sizeof(proto_header));
    if (hdr->magic != MAGIC) {
        return -2;
    }
    if (buf_len < sizeof(proto_header) + hdr->length) {
        return -3;
    }
    *data = buf + sizeof(proto_header);
    return 0;
}

int main(void) {
    uint8_t send_buf[1024];
    const char *msg = "Hello, Protocol!";
    int total = encode_packet(1, msg, strlen(msg), send_buf);

    proto_header recv_hdr;
    const uint8_t *recv_data;
    int ret = decode_packet(send_buf, total, &recv_hdr, &recv_data);

    if (ret == 0) {
        printf("版本: %d, 类型: %d, 长度: %u\n",
               recv_hdr.version, recv_hdr.type, recv_hdr.length);
        printf("数据: %.*s\n", recv_hdr.length, recv_data);
    }
    return 0;
}
```

**API协议**（REST API规范）：

```
REST API协议示例：

请求：
  GET /api/v1/users/42 HTTP/1.1
  Host: api.example.com
  Authorization: Bearer token123

响应：
  HTTP/1.1 200 OK
  Content-Type: application/json

  {"id": 42, "name": "张三", "age": 25}

协议约定：
  - 用HTTP方法表示操作：GET=查询 POST=创建 PUT=更新 DELETE=删除
  - 用URL路径表示资源：/users/42 = ID为42的用户
  - 用状态码表示结果：200=成功 404=不存在 500=服务器错误
```

### 4. C/C++中的协议实现

```c
/* HTTP客户端协议交互示例 */
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int http_get(const char *host, int port, const char *path) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sock);
        return -1;
    }

    /* 按HTTP协议格式发送请求 */
    char request[512];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n"
             "\r\n",
             path, host);
    send(sock, request, strlen(request), 0);

    /* 按HTTP协议格式接收响应 */
    char response[4096];
    int n = recv(sock, response, sizeof(response) - 1, 0);
    if (n > 0) {
        response[n] = '\0';
        printf("响应:\n%s\n", response);
    }

    close(sock);
    return 0;
}
```

***

## 4. 接口（Interface）— 连接的"插座"

### 1. 什么是接口

**比喻**：接口像**插座**——不管什么电器（电视、冰箱、台灯），只要插头符合标准（两孔/三孔），就能插上用。电器不需要知道墙里的电线怎么走，插座也不关心你插的是什么电器。

```
┌─────────────────────────────────────────────────┐
│              插座（接口）                         │
├─────────────────────────────────────────────────┤
│                                                   │
│   墙内电线（实现）                                │
│   ┌─────────────────────────────────┐            │
│   │  火线 ←→ 零线 ←→ 地线          │            │
│   │  （具体实现细节，你看不到）       │            │
│   └──────────┬──────────────────────┘            │
│              │ 标准化的插孔                        │
│         ╔════╧════╗                               │
│         ║  ○  ○   ║  ← 两孔接口                   │
│         ╚═════════╝                               │
│                                                   │
│   任何符合标准的插头都能用：                        │
│   📺 电视 ✓  💡 台灯 ✓  🔌 充电器 ✓              │
│                                                   │
└─────────────────────────────────────────────────┘
```

**精确定义**：接口是两个系统之间交互的边界，定义了交互的规则（方法签名、数据格式），隐藏了各自的内部实现。

### 2. C语言的接口

**头文件 = 接口**：

```c
/* stack.h —— 接口（只声明，不实现） */
#ifndef STACK_H
#define STACK_H

typedef struct stack stack_t;

stack_t *stack_create(int capacity);
void     stack_destroy(stack_t *s);
int      stack_push(stack_t *s, int value);
int      stack_pop(stack_t *s, int *value);
int      stack_is_empty(const stack_t *s);

#endif
```

```c
/* stack.c —— 实现（用户看不到内部细节） */
#include "stack.h"
#include <stdlib.h>

struct stack {
    int *data;
    int  top;
    int  capacity;
};

stack_t *stack_create(int capacity) {
    stack_t *s = malloc(sizeof(stack_t));
    s->data = malloc(sizeof(int) * capacity);
    s->top = -1;
    s->capacity = capacity;
    return s;
}

void stack_destroy(stack_t *s) {
    free(s->data);
    free(s);
}

int stack_push(stack_t *s, int value) {
    if (s->top >= s->capacity - 1) return -1;
    s->data[++s->top] = value;
    return 0;
}

int stack_pop(stack_t *s, int *value) {
    if (s->top < 0) return -1;
    *value = s->data[s->top--];
    return 0;
}

int stack_is_empty(const stack_t *s) {
    return s->top < 0;
}
```

**函数指针 = 接口**：

```c
#include <stdio.h>
#include <stdlib.h>

/* 用函数指针定义排序策略接口 */
typedef int (*compare_func)(const void *, const void *);

void my_sort(void *base, size_t nmemb, size_t size,
             compare_func cmp) {
    /* 简单冒泡排序，使用接口比较 */
    char *arr = (char *)base;
    char *tmp = malloc(size);

    for (size_t i = 0; i < nmemb - 1; i++) {
        for (size_t j = 0; j < nmemb - 1 - i; j++) {
            void *a = arr + j * size;
            void *b = arr + (j + 1) * size;
            if (cmp(a, b) > 0) {
                memcpy(tmp, a, size);
                memcpy(a, b, size);
                memcpy(b, tmp, size);
            }
        }
    }
    free(tmp);
}

/* 不同的接口实现 */
int cmp_int(const void *a, const void *b) {
    return *(const int *)a - *(const int *)b;
}

int cmp_int_desc(const void *a, const void *b) {
    return *(const int *)b - *(const int *)a;
}

int main(void) {
    int arr[] = {5, 3, 1, 4, 2};

    my_sort(arr, 5, sizeof(int), cmp_int);
    printf("升序: ");
    for (int i = 0; i < 5; i++) printf("%d ", arr[i]);
    printf("\n");

    my_sort(arr, 5, sizeof(int), cmp_int_desc);
    printf("降序: ");
    for (int i = 0; i < 5; i++) printf("%d ", arr[i]);
    printf("\n");
    return 0;
}
```

**回调 = 接口**：

```c
#include <stdio.h>

/* 定义回调接口 */
typedef void (*event_handler)(int event_type, void *user_data);

/* 框架代码：不知道也不关心回调做什么 */
void event_loop(event_handler on_event, void *user_data) {
    int events[] = {1, 2, 3};
    for (int i = 0; i < 3; i++) {
        on_event(events[i], user_data);
    }
}

/* 用户代码：实现回调接口 */
void my_handler(int event_type, void *user_data) {
    const char *name = (const char *)user_data;
    printf("[%s] 收到事件: %d\n", name, event_type);
}

int main(void) {
    event_loop(my_handler, "测试模块");
    return 0;
}
```

### 3. C++的接口（纯虚类/抽象类）

```cpp
#include <iostream>
#include <memory>
#include <vector>

/* 接口：纯虚类 */
class IShape {
public:
    virtual ~IShape() = default;
    virtual double area() const = 0;
    virtual double perimeter() const = 0;
    virtual void   draw() const = 0;
};

/* 实现A：圆形 */
class Circle : public IShape {
    double radius_;
public:
    explicit Circle(double r) : radius_(r) {}
    double area() const override {
        return 3.14159265 * radius_ * radius_;
    }
    double perimeter() const override {
        return 2 * 3.14159265 * radius_;
    }
    void draw() const override {
        std::cout << "画一个圆，半径=" << radius_ << std::endl;
    }
};

/* 实现B：矩形 */
class Rectangle : public IShape {
    double width_, height_;
public:
    Rectangle(double w, double h) : width_(w), height_(h) {}
    double area() const override {
        return width_ * height_;
    }
    double perimeter() const override {
        return 2 * (width_ + height_);
    }
    void draw() const override {
        std::cout << "画一个矩形，宽=" << width_
                  << " 高=" << height_ << std::endl;
    }
};

/* 使用接口，不依赖具体实现 */
void print_shape_info(const IShape &shape) {
    shape.draw();
    std::cout << "面积: " << shape.area()
              << " 周长: " << shape.perimeter() << std::endl;
}

int main() {
    std::vector<std::unique_ptr<IShape>> shapes;
    shapes.push_back(std::make_unique<Circle>(5.0));
    shapes.push_back(std::make_unique<Rectangle>(3.0, 4.0));

    for (const auto &s : shapes) {
        print_shape_info(*s);
    }
    return 0;
}
```

### 4. 接口vs实现分离

```
设计原则：面向接口编程，不面向实现编程

  ┌──────────┐     接口     ┌──────────┐
  │  调用方  │ ──────────→  │  接口    │
  └──────────┘              └────┬─────┘
                                  │ 实现
                       ┌──────────┼──────────┐
                       ▼          ▼          ▼
                  ┌────────┐ ┌────────┐ ┌────────┐
                  │ 实现A  │ │ 实现B  │ │ 实现C  │
                  │ SQLite │ │ MySQL  │ │ PgSQL  │
                  └────────┘ └────────┘ └────────┘

  调用方只依赖接口，不依赖具体实现
  → 换实现不需要改调用方代码
  → 方便测试（可以用Mock实现）
  → 方便扩展（新增实现不影响旧代码）
```

```c
/* 接口与实现分离的C语言示例 */

/* db_interface.h —— 数据库接口 */
typedef struct db_ops {
    int  (*open)(void *ctx, const char *conn_str);
    int  (*query)(void *ctx, const char *sql);
    void (*close)(void *ctx);
} db_ops_t;

typedef struct database {
    const db_ops_t *ops;
    void           *ctx;
} database_t;

int  db_open(database_t *db, const char *conn_str) {
    return db->ops->open(db->ctx, conn_str);
}
int  db_query(database_t *db, const char *sql) {
    return db->ops->query(db->ctx, sql);
}
void db_close(database_t *db) {
    db->ops->close(db->ctx);
}

/* sqlite_impl.c —— SQLite实现 */
int sqlite_open(void *ctx, const char *conn_str) {
    printf("SQLite打开: %s\n", conn_str);
    return 0;
}
int sqlite_query(void *ctx, const char *sql) {
    printf("SQLite执行: %s\n", sql);
    return 0;
}
void sqlite_close(void *ctx) {
    printf("SQLite关闭\n");
}

const db_ops_t sqlite_ops = {
    .open  = sqlite_open,
    .query = sqlite_query,
    .close = sqlite_close,
};

/* 使用：只依赖接口，不依赖具体实现 */
int main(void) {
    database_t db = { .ops = &sqlite_ops, .ctx = NULL };
    db_open(&db, "test.db");
    db_query(&db, "SELECT * FROM users");
    db_close(&db);
    return 0;
}
```

***

## 5. 四者关系全景图

### 1. 比喻串联

```
开一家餐厅的完整图景：

  ┌─────────────────────────────────────────────────────────┐
  │                                                         │
  │  📋 API = 菜单                                          │
  │     告诉顾客有什么菜、需要什么配料、价格多少              │
  │     → 软件对外提供的功能声明                              │
  │                                                         │
  │  🧰 SDK = 厨房                                          │
  │     菜单(API) + 食材(库) + 厨具(工具) + 食谱(文档)       │
  │     → 开发所需的全部工具和资源                            │
  │                                                         │
  │  📏 协议 = 规矩                                         │
  │     点菜流程、上菜顺序、结账方式                          │
  │     → 通信双方约定的规则                                  │
  │                                                         │
  │  🔌 接口 = 插座/窗口                                    │
  │     顾客和厨房之间的取餐窗口                              │
  │     → 两个系统交互的边界                                  │
  │                                                         │
  └─────────────────────────────────────────────────────────┘
```

### 2. 技术视角的关系

```
┌─────────────────────────────────────────────────────────┐
│                      你的应用                            │
│                                                         │
│    调用 ──→ API（函数声明）                              │
│              │                                          │
│              │ 包含在                                    │
│              ▼                                          │
│           SDK（开发工具包）                               │
│           ├── API头文件                                  │
│           ├── 库文件（API的实现）                         │
│           ├── 工具链                                     │
│           └── 文档和示例                                 │
│                                                         │
│    通信 ──→ 协议（数据格式和传输规则）                    │
│              │                                          │
│              │ 通过                                      │
│              ▼                                          │
│           接口（交互边界）                                │
│           ├── 函数签名 = 接口                            │
│           ├── 纯虚类 = 接口                              │
│           └── 回调函数 = 接口                            │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

### 3. 实际案例：开发一个HTTP客户端

```
开发HTTP客户端时，四个概念如何配合：

  1. 协议：HTTP协议规定了请求/响应格式
     GET /api/data HTTP/1.1
     Host: example.com

  2. API：socket库提供网络通信的函数声明
     int socket(int domain, int type, int protocol);
     ssize_t send(int sockfd, const void *buf, size_t len, int flags);

  3. SDK：操作系统SDK提供socket API的实现
     Windows SDK → ws2_32.lib
     Linux SDK → libc.so

  4. 接口：HTTPClient抽象类定义了统一调用方式
     class IHttpClient {
         virtual Response get(const string &url) = 0;
     };
```

```c
/* 完整示例：四个概念的协作 */

/* 协议：HTTP请求格式 */
static const char *HTTP_REQUEST_FMT =
    "GET %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Connection: close\r\n\r\n";

/* API：socket函数（由SDK提供实现） */
#include <sys/socket.h>
#include <netdb.h>

/* 接口：定义HTTP客户端的操作 */
typedef struct http_client_ops {
    int  (*connect)(void *ctx, const char *host, int port);
    int  (*send_request)(void *ctx, const char *request);
    int  (*recv_response)(void *ctx, char *buf, int bufsize);
    void (*disconnect)(void *ctx);
} http_client_ops_t;

typedef struct http_client {
    const http_client_ops_t *ops;
    void                    *ctx;
} http_client_t;

/* 使用接口发送HTTP请求 */
int http_get(http_client_t *client, const char *host,
             int port, const char *path, char *resp, int resp_size) {
    if (client->ops->connect(client->ctx, host, port) != 0)
        return -1;

    char request[1024];
    snprintf(request, sizeof(request), HTTP_REQUEST_FMT, path, host);

    client->ops->send_request(client->ctx, request);
    int n = client->ops->recv_response(client->ctx, resp, resp_size);
    client->ops->disconnect(client->ctx);
    return n;
}
```

***

## 6. 速查对照表

| 概念 | 一句话 | 比喻 | 核心要素 | C语言体现 | C++体现 |
|------|--------|------|----------|-----------|---------|
| **API** | 软件组件间的契约 | 餐厅菜单 | 函数签名、参数、返回值 | `.h`头文件中的函数声明 | 类的public方法 |
| **SDK** | 开发所需的全部工具 | 整个厨房 | API+库+工具+文档+示例 | `.h`+`.a`/`.so`+工具+文档 | 同左+头文件+CMake |
| **协议** | 通信双方的规则 | 外交礼仪 | 格式、顺序、错误处理 | 结构体+编解码函数 | 序列化库+协议类 |
| **接口** | 系统交互的边界 | 插座 | 方法签名、数据格式 | 头文件/函数指针/回调 | 纯虚类/抽象类 |

| 对比维度 | API | SDK | 协议 | 接口 |
|----------|-----|-----|------|------|
| **关注点** | 能做什么 | 怎么开发 | 怎么通信 | 怎么连接 |
| **抽象层级** | 功能级 | 工具级 | 通信级 | 架构级 |
| **是否可独立使用** | 否（需实现） | 是 | 否（需双方遵守） | 否（需实现） |
| **变更影响** | 调用方需修改 | 重新安装 | 双方需同步修改 | 实现方可独立修改 |
| **测试方式** | 单元测试 | 集成测试 | 协议一致性测试 | Mock测试 |

```
记忆口诀：

  API是菜单 —— 告诉你有什么
  SDK是厨房 —— 给你做菜的一切
  协议是规矩 —— 按规矩才能沟通
  接口是插座 —— 标准化才能对接

  四者配合：
  用SDK里的API，按协议的规矩，通过接口来交互
```

***

## 7. 相关章节

- [FAQ-152：框架引擎中间件与架构概念指南](15-框架引擎中间件与架构.md)
- [FAQ-149：计算机专业术语比喻理解指南](../01-基础概念/00-计算机术语比喻理解.md)
- [FAQ-148：编程范式实战指南](13-编程范式概览与过程式编程.md)
- [FAQ-153：领域驱动设计DDD概念指南](16-领域驱动设计DDD.md)
- [类与面向对象——纯虚类与接口](../../02-CPP/03-类与对象.md)

***

### 相关阅读

- [框架引擎中间件与架构](15-框架引擎中间件与架构.md)
- [如何实现跨语言调用](./01-如何实现跨语言调用.md)
- [领域驱动设计DDD](16-领域驱动设计DDD.md)