# 什么是字节序Endianness
> 📖 相关章节：[编码规范](../../04-工程实践/00-编码规范.md)、[设计模式](../../04-工程实践/03-设计模式.md)、[单元测试](../../04-工程实践/05-单元测试.md)、[代码审查](../../04-工程实践/07-代码审查.md)

> 字节序决定了多字节数据在内存中的存放顺序——搞错字节序，网络通信和文件解析就会出现"灵异"Bug。

***

### 1. 先抓核心

大端序（Big-Endian）高位字节存低地址，小端序（Little-Endian）低位字节存低地址；网络传输用大端序，x86/ARM 默认小端序，跨平台必须显式转换。

***

### 2. 大端序与小端序

以 32 位整数 `0x01020304` 为例：

| 地址偏移 | 大端序（Big-Endian） | 小端序（Little-Endian） |
|----------|---------------------|------------------------|
| +0 | `0x01`（最高有效字节） | `0x04`（最低有效字节） |
| +1 | `0x02` | `0x03` |
| +2 | `0x03` | `0x02` |
| +3 | `0x04`（最低有效字节） | `0x01`（最高有效字节） |

**助记**：
- 大端序 = "高位在前" = 人类阅读顺序 = 网络字节序
- 小端序 = "低位在前" = 计算机计算顺序 = x86/ARM 默认

```cpp
#include <cstdio>
#include <cstdint>

void print_bytes(const void* data, size_t len) {
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    for (size_t i = 0; i < len; ++i) {
        std::printf("%02X ", bytes[i]);
    }
    std::printf("\n");
}

int main() {
    uint32_t value = 0x01020304;
    std::printf("0x01020304 在内存中的字节序: ");
    print_bytes(&value, sizeof(value));
    return 0;
}
```

**小端机器输出**：`04 03 02 01`
**大端机器输出**：`01 02 03 04`

| 架构 | 默认字节序 |
|------|-----------|
| x86 / x86_64 | 小端序 |
| ARM (大多数) | 小端序（可配置） |
| PowerPC (旧) | 大端序 |
| SPARC | 大端序 |
| MIPS | 可配置 |
| RISC-V | 小端序 |

***

### 3. 网络字节序与转换函数

TCP/IP 协议规定网络传输使用大端序（网络字节序），因此小端机器发送/接收数据时必须转换：

```cpp
#include <cstdio>
#include <cstdint>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

int main() {
    uint32_t host_val = 0x12345678;

    uint32_t net_val = htonl(host_val);
    std::printf("主机序: 0x%08X\n", host_val);
    std::printf("网络序: 0x%08X\n", net_val);
    std::printf("转回主机序: 0x%08X\n", ntohl(net_val));

    uint16_t port = 8080;
    std::printf("端口 %d 网络序: 0x%04X\n", port, htons(port));
    return 0;
}
```

| 函数 | 全称 | 作用 |
|------|------|------|
| `htonl` | Host TO Network Long | 32 位主机序 → 网络序 |
| `htons` | Host TO Network Short | 16 位主机序 → 网络序 |
| `ntohl` | Network TO Host Long | 32 位网络序 → 主机序 |
| `ntohs` | Network TO Host Short | 16 位网络序 → 主机序 |

> **关键理解**：在大端机器上，这些函数是空操作（no-op）；在小端机器上，它们执行字节反转。编写代码时始终调用这些函数，让平台自行决定是否转换。

**完整的 socket 地址设置示例**：

```cpp
#include <cstdio>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

int main() {
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    inet_pton(AF_INET, "192.168.1.1", &addr.sin_addr);

    std::printf("端口网络序: 0x%04X\n", addr.sin_port);
    return 0;
}
```

***

### 4. 运行时检测字节序

有时需要在运行时判断当前机器的字节序：

**方法一：联合体（Union）检测**：

```cpp
#include <cstdio>
#include <cstdint>

bool is_little_endian() {
    union {
        uint32_t i;
        uint8_t c[4];
    } u = {0x01020304};
    return u.c[0] == 0x04;
}

int main() {
    if (is_little_endian()) {
        std::printf("当前系统: 小端序\n");
    } else {
        std::printf("当前系统: 大端序\n");
    }
    return 0;
}
```

**方法二：指针检测**：

```cpp
#include <cstdio>
#include <cstdint>

bool is_little_endian_v2() {
    uint16_t x = 0x0001;
    return *reinterpret_cast<uint8_t*>(&x) == 0x01;
}
```

**方法三：预编译宏检测**：

```cpp
#include <cstdio>

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define IS_LITTLE_ENDIAN 1
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define IS_LITTLE_ENDIAN 0
#elif defined(_WIN32)
#define IS_LITTLE_ENDIAN 1
#else
#define IS_LITTLE_ENDIAN 0
#endif

int main() {
#if IS_LITTLE_ENDIAN
    std::printf("编译期确定: 小端序\n");
#else
    std::printf("编译期确定: 大端序\n");
#endif
    return 0;
}
```

| 检测方式 | 优点 | 缺点 |
|----------|------|------|
| Union | 直观、标准 | 严格别名规则下有争议 |
| 指针 | 简单 | 同上 |
| 预编译宏 | 零运行时开销 | 编译器/平台相关 |

***

### 5. C++20 std::endian

C++20 在 `<bit>` 头文件中提供了 `std::endian` 枚举：

```cpp
#include <bit>
#include <cstdio>

int main() {
    if constexpr (std::endian::native == std::endian::little) {
        std::printf("C++20 检测: 小端序\n");
    } else if constexpr (std::endian::native == std::endian::big) {
        std::printf("C++20 检测: 大端序\n");
    } else {
        std::printf("C++20 检测: 混合字节序（PDP-11 等）\n");
    }
    return 0;
}
```

| `std::endian` 值 | 含义 |
|-------------------|------|
| `std::endian::little` | 小端序 |
| `std::endian::big` | 大端序 |
| `std::endian::native` | 当前平台字节序（可能是 little/big/其他） |

**编译期字节序转换**（C++23 `std::byteswap`）：

```cpp
#include <bit>
#include <cstdio>
#include <cstdint>

int main() {
    uint32_t val = 0x12345678;
    uint32_t swapped = std::byteswap(val);
    std::printf("原始: 0x%08X\n", val);
    std::printf("翻转: 0x%08X\n", swapped);
    return 0;
}
```

> **注意**：`std::byteswap` 是无条件字节翻转，不判断当前字节序。如果需要"小端转大端"，在已知源字节序时使用。

***

### 6. 编写字节序无关的代码

最佳实践是：存储和传输时始终使用固定字节序（通常是大端/网络序），在边界处做转换：

```cpp
#include <cstdio>
#include <cstdint>
#include <cstring>

uint32_t host_to_big_endian(uint32_t val) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return ((val & 0xFF000000) >> 24) |
           ((val & 0x00FF0000) >> 8)  |
           ((val & 0x0000FF00) << 8)  |
           ((val & 0x000000FF) << 24);
#else
    return val;
#endif
}

uint32_t big_endian_to_host(uint32_t val) {
    return host_to_big_endian(val);
}

void write_uint32_be(uint8_t* buf, uint32_t val) {
    uint32_t be = host_to_big_endian(val);
    std::memcpy(buf, &be, sizeof(be));
}

uint32_t read_uint32_be(const uint8_t* buf) {
    uint32_t be;
    std::memcpy(&be, buf, sizeof(be));
    return big_endian_to_host(be);
}

int main() {
    uint8_t buffer[4];
    write_uint32_be(buffer, 0x12345678);

    std::printf("缓冲区: ");
    for (int i = 0; i < 4; ++i) {
        std::printf("%02X ", buffer[i]);
    }
    std::printf("\n");

    uint32_t restored = read_uint32_be(buffer);
    std::printf("还原值: 0x%08X\n", restored);
    return 0;
}
```

**核心原则**：

| 原则 | 说明 |
|------|------|
| 外部格式固定 | 文件、网络协议始终用大端序 |
| 内部自由选择 | 内存中用本机序，性能最优 |
| 边界做转换 | 序列化/反序列化时转换 |
| 用 memcpy 不用指针强转 | 避免对齐和严格别名问题 |

***

### 7. 常见协议的字节序规定

| 协议/格式 | 字节序 | 说明 |
|-----------|--------|------|
| TCP/IP | 大端序 | 网络字节序标准 |
| IPv4 头部 | 大端序 | 所有字段 |
| IPv6 头部 | 大端序 | 所有字段 |
| UDP/TCP 端口 | 大端序 | 端口号字段 |
| DNS | 大端序 | 所有 16/32 位字段 |
| HTTP | 文本协议 | 不涉及字节序 |
| WebSocket | 大端序 | 帧长度字段 |
| PNG | 大端序 | 所有整数 |
| JPEG | 大端序 | 标记和长度 |
| BMP | 小端序 | Windows 格式 |
| ELF | 可变 | 头部 `e_ident[EI_DATA]` 标识 |
| PE (Windows EXE) | 小端序 | 微软格式 |
| MQTT | 大端序 | 长度字段 |
| Modbus TCP | 大端序 | 寄存器值 |
| Protobuf | 小端序（varint） | 变长编码 |

**解析网络协议头部示例**：

```cpp
#include <cstdio>
#include <cstdint>
#include <cstring>

struct IPHeader {
    uint8_t  version_ihl;
    uint8_t  tos;
    uint16_t total_length;
    uint16_t identification;
    uint16_t flags_fragment;
    uint8_t  ttl;
    uint8_t  protocol;
    uint16_t checksum;
    uint32_t src_addr;
    uint32_t dst_addr;
};

void parse_ip_header(const uint8_t* raw) {
    IPHeader hdr;
    std::memcpy(&hdr, raw, sizeof(hdr));

    uint16_t total_length = (raw[2] << 8) | raw[3];
    uint32_t src = (raw[12] << 24) | (raw[13] << 16) | (raw[14] << 8) | raw[15];
    uint32_t dst = (raw[16] << 24) | (raw[17] << 16) | (raw[18] << 8) | raw[19];

    std::printf("总长度: %d\n", total_length);
    std::printf("源地址: %u.%u.%u.%u\n", raw[12], raw[13], raw[14], raw[15]);
    std::printf("目的地址: %u.%u.%u.%u\n", raw[16], raw[17], raw[18], raw[19]);
}
```

> **最佳实践**：解析网络协议时，逐字节读取并手动拼接，而非直接 `memcpy` 到结构体——这样避免了对齐问题和字节序陷阱。

***

### 8. BOM（Byte Order Mark）

BOM 是 Unicode 文本开头的字节序标记，用于标识文本的字节序和编码：

| 编码 | BOM 字节 | 说明 |
|------|----------|------|
| UTF-8 | `EF BB BF` | 可选，推荐不加 |
| UTF-16 BE | `FE FF` | 大端序 |
| UTF-16 LE | `FF FE` | 小端序 |
| UTF-32 BE | `00 00 FE FF` | 大端序 |
| UTF-32 LE | `FF FE 00 00` | 小端序 |

```cpp
#include <cstdio>
#include <cstdint>

enum class Encoding { UTF8, UTF16_BE, UTF16_LE, UTF32_BE, UTF32_LE, Unknown };

Encoding detect_bom(const uint8_t* data, size_t len) {
    if (len >= 3 && data[0] == 0xEF && data[1] == 0xBB && data[2] == 0xBF) {
        return Encoding::UTF8;
    }
    if (len >= 4 && data[0] == 0x00 && data[1] == 0x00 &&
        data[2] == 0xFE && data[3] == 0xFF) {
        return Encoding::UTF32_BE;
    }
    if (len >= 4 && data[0] == 0xFF && data[1] == 0xFE &&
        data[2] == 0x00 && data[3] == 0x00) {
        return Encoding::UTF32_LE;
    }
    if (len >= 2 && data[0] == 0xFE && data[1] == 0xFF) {
        return Encoding::UTF16_BE;
    }
    if (len >= 2 && data[0] == 0xFF && data[1] == 0xFE) {
        return Encoding::UTF16_LE;
    }
    return Encoding::Unknown;
}

int main() {
    uint8_t utf16le_bom[] = {0xFF, 0xFE, 0x41, 0x00};
    Encoding enc = detect_bom(utf16le_bom, sizeof(utf16le_bom));

    switch (enc) {
        case Encoding::UTF16_LE: std::printf("UTF-16 小端序\n"); break;
        case Encoding::UTF16_BE: std::printf("UTF-16 大端序\n"); break;
        case Encoding::UTF8:     std::printf("UTF-8\n"); break;
        default:                 std::printf("未知编码\n"); break;
    }
    return 0;
}
```

> **UTF-8 的 BOM 争议**：UTF-8 是字节序无关的，BOM 对 UTF-8 无实际意义。但某些 Windows 程序（如记事本）会添加 UTF-8 BOM，这可能导致 Linux 工具（如 shell 脚本）解析出错——`#!/usr/bin/env python` 前有 BOM 会导致 `bad interpreter` 错误。

***

### 9. 真实世界的字节序 Bug

**Bug 1：直接发送结构体**

```cpp
struct Packet {
    uint32_t magic;
    uint16_t version;
    uint16_t length;
};

void send_packet(int sockfd, const Packet& pkt) {
    send(sockfd, reinterpret_cast<const char*>(&pkt), sizeof(pkt), 0);
}
```

**问题**：小端机器发送的结构体，大端机器接收后所有字段值错误。

**修复**：

```cpp
void send_packet_fixed(int sockfd, uint32_t magic, uint16_t version, uint16_t length) {
    uint8_t buf[8];
    buf[0] = (magic >> 24) & 0xFF;
    buf[1] = (magic >> 16) & 0xFF;
    buf[2] = (magic >> 8)  & 0xFF;
    buf[3] = magic & 0xFF;
    buf[4] = (version >> 8) & 0xFF;
    buf[5] = version & 0xFF;
    buf[6] = (length >> 8) & 0xFF;
    buf[7] = length & 0xFF;
    send(sockfd, reinterpret_cast<const char*>(buf), sizeof(buf), 0);
}
```

**Bug 2：文件格式解析**

```cpp
uint32_t read_u32_from_file(FILE* f) {
    uint32_t val;
    fread(&val, sizeof(val), 1, f);
    return val;
}
```

**问题**：如果文件格式规定大端序，小端机器上读出的值完全错误。

**修复**：

```cpp
uint32_t read_u32_be(FILE* f) {
    uint8_t buf[4];
    fread(buf, 1, 4, f);
    return (static_cast<uint32_t>(buf[0]) << 24) |
           (static_cast<uint32_t>(buf[1]) << 16) |
           (static_cast<uint32_t>(buf[2]) << 8)  |
           static_cast<uint32_t>(buf[3]);
}
```

**Bug 3：序列化浮点数**

```cpp
void write_float(FILE* f, float val) {
    fwrite(&val, sizeof(val), 1, f);
}
```

**问题**：浮点数的字节序与整数一致，但 IEEE 754 浮点数不能简单字节翻转后 reinterpret——不同字节序机器间传递浮点数需要统一处理。

**修复**：

```cpp
#include <cstring>
#include <cstdint>

void write_float_be(FILE* f, float val) {
    uint32_t bits;
    std::memcpy(&bits, &val, sizeof(bits));
    uint8_t buf[4];
    buf[0] = (bits >> 24) & 0xFF;
    buf[1] = (bits >> 16) & 0xFF;
    buf[2] = (bits >> 8)  & 0xFF;
    buf[3] = bits & 0xFF;
    fwrite(buf, 1, 4, f);
}

float read_float_be(FILE* f) {
    uint8_t buf[4];
    fread(buf, 1, 4, f);
    uint32_t bits = (static_cast<uint32_t>(buf[0]) << 24) |
                    (static_cast<uint32_t>(buf[1]) << 16) |
                    (static_cast<uint32_t>(buf[2]) << 8)  |
                    static_cast<uint32_t>(buf[3]);
    float val;
    std::memcpy(&val, &bits, sizeof(val));
    return val;
}
```

***

### 10. 字节序相关的面试题与速查

**Q1：为什么 x86 用小端序？**

小端序的优势：强制类型转换时不需要调整地址。例如 `int32_t` 转 `int16_t`，小端序只需截断低地址的 2 字节，大端序需要跳过 2 字节。

**Q2：为什么网络用大端序？**

历史原因：早期网络设备（IBM、Sun）是大端架构。大端序的"高位在前"与人类阅读习惯一致，便于调试抓包。

**Q3：如何判断一段数据是大端还是小端？**

看最低有效字节（LSB）的位置：LSB 在低地址 = 小端，LSB 在高地址 = 大端。

**速查表**：

| 场景 | 字节序 | 转换函数 |
|------|--------|----------|
| x86 内存 | 小端 | 无需转换 |
| 网络传输 | 大端 | `htonl/htons/ntohl/ntohs` |
| PNG/JPEG 文件 | 大端 | 手动逐字节读取 |
| BMP 文件 | 小端 | 手动逐字节读取 |
| ELF 文件 | 看头部标识 | 按标识处理 |
| UTF-16 文本 | 看 BOM | 按 BOM 处理 |

***

### 11. 极简总结

| 概念 | 要点 |
|------|------|
| 大端序 | 高位字节存低地址，人类阅读顺序，网络字节序 |
| 小端序 | 低位字节存低地址，x86/ARM 默认，类型转换方便 |
| 网络字节序 | 大端序，TCP/IP 标准，必须用 `htonl/htons` 转换 |
| 运行时检测 | Union 法、指针法、预编译宏 |
| C++20 | `std::endian` 编译期判断，`std::byteswap` 无条件翻转 |
| 字节序无关代码 | 外部固定大端，内部用本机序，边界做转换 |
| BOM | Unicode 文本字节序标记，UTF-8 BOM 有争议 |
| 常见 Bug | 直接发送结构体、文件解析忽略字节序、浮点数序列化 |
| 核心原则 | 永远不要假设字节序，序列化时逐字节读写 |

***

### 相关阅读

- [跨平台是什么意思](00-跨平台与可移植性.md)
- [C与CPP的跨平台可移植性](00-跨平台与可移植性.md)
- [序列化与反序列化](19-序列化与反序列化.md)

***