/**
 * @file 04_example_zero_copy_concepts.cpp
 * @brief 零拷贝与高效数据传输: 零拷贝概念, scatter/gather IO
 * @description 对应文档: 高性能网络与异步IO / 第4节 零拷贝与高效数据传输
 *
 * 本文件演示:
 *   - 传统数据拷贝的问题
 *   - 零拷贝技术原理
 *   - scatter/gather IO (WSARecv/WSASend多缓冲区)
 *   - sendfile/splice概念
 */

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <chrono>
#include <numeric>

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
    #include <sys/uio.h>
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
// 演示1: 传统数据拷贝的问题
// ============================================================
void demo_copy_problem() {
    std::cout << "\n========================================\n";
    std::cout << "  演示1: 传统数据拷贝的问题\n";
    std::cout << "========================================\n\n";

    std::cout << "【传统send的数据流 (4次拷贝)】\n\n";

    std::cout << "  应用缓冲区 ──→ 内核缓冲区 ──→ Socket缓冲区 ──→ 网卡\n";
    std::cout << "     (1)           (2)            (3)            (4)\n\n";

    std::cout << "  详细过程:\n";
    std::cout << "  1. 应用调用send(data)\n";
    std::cout << "  2. 数据从应用缓冲区拷贝到内核缓冲区 (CPU拷贝)\n";
    std::cout << "  3. 数据从内核缓冲区拷贝到Socket发送缓冲区 (CPU拷贝)\n";
    std::cout << "  4. DMA从Socket缓冲区拷贝到网卡 (DMA拷贝)\n\n";

    std::cout << "  上下文切换: 2次 (用户态→内核态→用户态)\n";
    std::cout << "  CPU拷贝:    2次\n";
    std::cout << "  DMA拷贝:    1次\n\n";

    std::cout << "【传统recv的数据流 (4次拷贝)】\n\n";

    std::cout << "  网卡 ──→ Socket缓冲区 ──→ 内核缓冲区 ──→ 应用缓冲区\n";
    std::cout << "   (1)          (2)             (3)            (4)\n\n";

    std::cout << "  详细过程:\n";
    std::cout << "  1. DMA从网卡拷贝到Socket接收缓冲区 (DMA拷贝)\n";
    std::cout << "  2. 数据从Socket缓冲区拷贝到内核缓冲区 (CPU拷贝)\n";
    std::cout << "  3. 应用调用recv(buf)\n";
    std::cout << "  4. 数据从内核缓冲区拷贝到应用缓冲区 (CPU拷贝)\n\n";

    std::cout << "【问题】\n";
    std::cout << "  - 数据在内核和用户空间之间多次拷贝\n";
    std::cout << "  - 每次拷贝都消耗CPU和内存带宽\n";
    std::cout << "  - 高吞吐场景下成为瓶颈\n";
    std::cout << "  - 例如: 10Gbps网络, 拷贝开销可达数GB/s\n";

    // 实际测量: 模拟拷贝开销
    const size_t data_size = 100 * 1024 * 1024;  // 100MB
    std::vector<char> src(data_size, 'A');
    std::vector<char> dst(data_size);

    auto start = std::chrono::high_resolution_clock::now();
    std::memcpy(dst.data(), src.data(), data_size);
    auto end = std::chrono::high_resolution_clock::now();

    auto ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    double throughput = static_cast<double>(data_size) / ms * 1000;  // MB/s

    std::cout << "\n【实测: memcpy 100MB耗时】\n";
    std::cout << "  耗时: " << ms << " 微秒\n";
    std::cout << "  吞吐: " << throughput << " MB/s\n";
    std::cout << "  (这只是单次拷贝的开销, 实际网络IO还有更多拷贝)\n";
}

// ============================================================
// 演示2: 零拷贝技术
// ============================================================
void demo_zero_copy_techniques() {
    std::cout << "\n========================================\n";
    std::cout << "  演示2: 零拷贝技术\n";
    std::cout << "========================================\n\n";

    std::cout << "【零拷贝技术一览】\n\n";

    std::cout << "  1. sendfile() (Linux)\n";
    std::cout << "     直接在内核中将文件数据发送到socket\n";
    std::cout << "     无需将数据拷贝到用户空间\n\n";

    std::cout << "     传统: read(fd, buf) → send(sock, buf)  [4次拷贝]\n";
    std::cout << "     sendfile: sendfile(sock, fd, ...)       [2次拷贝]\n";
    std::cout << "     (DMA: 磁盘→内核→网卡, 无CPU参与)\n\n";

    std::cout << "  2. mmap() (跨平台)\n";
    std::cout << "     将文件映射到用户空间, 避免read拷贝\n";
    std::cout << "     修改映射区域直接反映到文件\n\n";

    std::cout << "     传统: read(fd, buf) → 处理buf → send(sock, buf)\n";
    std::cout << "     mmap: ptr = mmap(fd) → 处理ptr → send(sock, ptr)\n";
    std::cout << "     (省去read的内核→用户拷贝)\n\n";

    std::cout << "  3. splice() (Linux 2.6.17+)\n";
    std::cout << "     在两个文件描述符之间移动数据\n";
    std::cout << "     数据完全不经过用户空间\n\n";

    std::cout << "     splice(pipe_in, ..., sock, ..., ...)\n";
    std::cout << "     [0次CPU拷贝, 仅DMA]\n\n";

    std::cout << "  4. TransmitFile() (Windows)\n";
    std::cout << "     Windows版sendfile\n";
    std::cout << "     直接将文件内容发送到socket\n\n";

    std::cout << "     TransmitFile(sock, file_handle, ...)\n";
    std::cout << "     [内核直接传输, 无用户态拷贝]\n\n";

    std::cout << "  5. io_uring (Linux 5.1+)\n";
    std::cout << "     最新的零拷贝方案\n";
    std::cout << "     支持注册固定缓冲区, 避免拷贝\n";
    std::cout << "     IOSQE_FIXED_BUF标志\n\n";

    std::cout << "【零拷贝对比表】\n\n";

    std::cout << "  技术            平台       CPU拷贝   适用场景\n";
    std::cout << "  ─────────────────────────────────────────────────\n";
    std::cout << "  sendfile        Linux      0~1       文件→Socket\n";
    std::cout << "  splice          Linux      0         管道→Socket\n";
    std::cout << "  mmap            跨平台     1         文件→处理→Socket\n";
    std::cout << "  TransmitFile    Windows    0~1       文件→Socket\n";
    std::cout << "  io_uring        Linux 5.1+ 0         通用零拷贝\n";
}

// ============================================================
// 演示3: Scatter/Gather IO
// ============================================================
void demo_scatter_gather() {
    std::cout << "\n========================================\n";
    std::cout << "  演示3: Scatter/Gather IO (分散/聚集IO)\n";
    std::cout << "========================================\n\n";

    std::cout << "【Scatter IO (分散读)】\n";
    std::cout << "  一次recv操作, 将数据分散到多个缓冲区\n\n";

    std::cout << "  网络数据流: [Header][Body1][Body2]\n";
    std::cout << "                ↓       ↓       ↓\n";
    std::cout << "             buf[0]  buf[1]  buf[2]\n\n";

    std::cout << "【Gather IO (聚集写)】\n";
    std::cout << "  一次send操作, 将多个缓冲区的数据聚集发送\n\n";

    std::cout << "  buf[0]  buf[1]  buf[2]\n";
    std::cout << "    ↓       ↓       ↓\n";
    std::cout << "  网络数据流: [Header][Body1][Body2]\n\n";

    std::cout << "【优点】\n";
    std::cout << "  1. 减少系统调用次数 (一次send/recv代替多次)\n";
    std::cout << "  2. 避免拼接缓冲区的内存拷贝\n";
    std::cout << "  3. 自然匹配协议的Header+Body结构\n\n";

    // 实际演示: 使用scatter/gather构建协议消息
    std::cout << "【示例: 使用scatter/gather构建协议消息】\n\n";

    // 模拟协议头和消息体
    struct MessageHeader {
        uint32_t type;
        uint32_t length;
        uint32_t sequence;
    };

    MessageHeader header{};
    header.type = 1;
    header.length = 12;
    header.sequence = 42;

    std::string body = "Hello World";

#ifdef _WIN32
    // Windows: 使用WSABUF
    WSABUF buffers[2];
    buffers[0].buf = reinterpret_cast<char*>(&header);
    buffers[0].len = sizeof(header);
    buffers[1].buf = body.data();
    buffers[1].len = static_cast<ULONG>(body.size());

    std::cout << "  WSABUF buffers[2]:\n";
    std::cout << "    buffers[0]: Header (" << buffers[0].len << " 字节)\n";
    std::cout << "    buffers[1]: Body   (" << buffers[1].len << " 字节)\n\n";

    std::cout << "  WSASend(sock, buffers, 2, &bytes_sent, 0, nullptr, nullptr);\n";
    std::cout << "  → 一次系统调用发送完整消息!\n\n";
#else
    // POSIX: 使用iovec + writev/readv
    struct iovec iov[2];
    iov[0].iov_base = &header;
    iov[0].iov_len = sizeof(header);
    iov[1].iov_base = body.data();
    iov[1].iov_len = body.size();

    std::cout << "  struct iovec iov[2]:\n";
    std::cout << "    iov[0]: Header (" << iov[0].iov_len << " 字节)\n";
    std::cout << "    iov[1]: Body   (" << iov[1].iov_len << " 字节)\n\n";

    std::cout << "  writev(sock, iov, 2);\n";
    std::cout << "  → 一次系统调用发送完整消息!\n\n";
#endif

    std::cout << "  对比传统方式:\n";
    std::cout << "    // 需要拼接缓冲区\n";
    std::cout << "    std::vector<char> full_msg;\n";
    std::cout << "    full_msg.insert(end(), (char*)&header, (char*)&header + sizeof(header));\n";
    std::cout << "    full_msg.insert(end(), body.begin(), body.end());\n";
    std::cout << "    send(sock, full_msg.data(), full_msg.size(), 0);\n";
    std::cout << "    // 多了一次内存拷贝!\n\n";

    // 性能对比模拟
    const int iterations = 100000;

    // 方式1: 拼接后发送 (模拟)
    auto start1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        std::vector<char> combined;
        combined.reserve(sizeof(header) + body.size());
        combined.insert(combined.end(),
                       reinterpret_cast<const char*>(&header),
                       reinterpret_cast<const char*>(&header) + sizeof(header));
        combined.insert(combined.end(), body.begin(), body.end());
    }
    auto end1 = std::chrono::high_resolution_clock::now();

    // 方式2: scatter/gather (模拟, 无需拼接)
    auto start2 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
#ifdef _WIN32
        WSABUF bufs[2];
        bufs[0].buf = reinterpret_cast<char*>(&header);
        bufs[0].len = sizeof(header);
        bufs[1].buf = const_cast<char*>(body.data());
        bufs[1].len = static_cast<ULONG>(body.size());
#else
        struct iovec iov2[2];
        iov2[0].iov_base = &header;
        iov2[0].iov_len = sizeof(header);
        iov2[1].iov_base = const_cast<char*>(body.data());
        iov2[1].iov_len = body.size();
#endif
    }
    auto end2 = std::chrono::high_resolution_clock::now();

    auto us1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();
    auto us2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();

    std::cout << "【性能对比 (10万次迭代)】\n";
    std::cout << "  拼接缓冲区:   " << us1 << " 微秒\n";
    std::cout << "  Scatter/Gather: " << us2 << " 微秒\n";
    std::cout << "  Scatter/Gather避免了内存拷贝, 性能更优\n";
}

// ============================================================
// 主函数
// ============================================================
int main() {
    std::cout << "============================================================\n";
    std::cout << "  第8章: 高性能网络与异步IO - 第4节 零拷贝与高效数据传输\n";
    std::cout << "============================================================\n";

    WinsockInit winsock_init;

    demo_copy_problem();
    demo_zero_copy_techniques();
    demo_scatter_gather();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
