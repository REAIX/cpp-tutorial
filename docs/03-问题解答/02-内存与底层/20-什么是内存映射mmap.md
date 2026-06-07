# 什么是内存映射 mmap
> 📖 相关章节：[文件操作](../../01-C语言/15-文件操作.md)、[内存管理](../../01-C语言/09-内存管理.md)

> "把文件当内存用，把内存当文件用"——mmap 的双面哲学

***

### 1. 核心提炼

mmap 将文件或设备映射到进程的虚拟地址空间，使程序可以通过指针直接访问文件内容，由操作系统的缺页中断机制按需完成 I/O。

***

### 2. mmap 与 munmap 基础

**函数原型**：

```cpp
// POSIX (Linux/macOS)
#include <sys/mman.h>

void* mmap(void* addr, size_t length, int prot, int flags,
           int fd, off_t offset);

int munmap(void* addr, size_t length);
```

**参数详解**：

| 参数 | 说明 |
|------|------|
| `addr` | 建议映射地址，通常传 `nullptr` 让内核选择 |
| `length` | 映射长度（字节），自动向上取整到页大小 |
| `prot` | 保护标志：`PROT_READ`/`PROT_WRITE`/`PROT_EXEC`/`PROT_NONE` |
| `flags` | 映射类型标志（见下表） |
| `fd` | 文件描述符，匿名映射传 `-1` |
| `offset` | 文件偏移量，必须是页大小的整数倍 |

**flags 标志**：

| 标志 | 说明 |
|------|------|
| `MAP_SHARED` | 共享映射，修改写回文件/对其他进程可见 |
| `MAP_PRIVATE` | 私有映射，修改触发 COW，不写回文件 |
| `MAP_ANONYMOUS` | 匿名映射，不关联文件，fd 传 `-1` |
| `MAP_FIXED` | 强制映射到指定地址（危险，会覆盖已有映射） |
| `MAP_HUGETLB` | 使用大页（2MB/1GB） |
| `MAP_LOCKED` | 锁定映射区域，防止被换出 |

**基本使用示例**：

```cpp
// POSIX (Linux/macOS)
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>

int main() {
    int fd = open("data.txt", O_RDWR);
    if (fd < 0) { perror("open"); return 1; }

    struct stat st;
    fstat(fd, &st);
    size_t size = st.st_size;

    void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE,
                     MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) { perror("mmap"); return 1; }

    close(fd);

    char* data = static_cast<char*>(ptr);
    printf("文件内容: %.*s\n", (int)size, data);

    data[0] = 'H';

    msync(ptr, size, MS_SYNC);
    munmap(ptr, size);
    return 0;
}
```

***

### 3. 文件映射 vs 匿名映射

| 维度 | 文件映射（File-backed） | 匿名映射（Anonymous） |
|------|------------------------|----------------------|
| 数据来源 | 文件内容 | 初始化为零 |
| 后端存储 | 磁盘文件 | 交换空间（swap） |
| 典型用途 | 文件 I/O、共享库 | 内存分配、进程间共享 |
| 持久化 | 修改可写回文件 | 不持久化 |
| flags | `MAP_SHARED` 或 `MAP_PRIVATE` | `MAP_ANONYMOUS \| MAP_SHARED/PRIVATE` |

**文件映射——读取文件**：

```cpp
// POSIX (Linux/macOS)
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>

int main() {
    int fd = open("large_file.bin", O_RDONLY);
    struct stat st;
    fstat(fd, &st);

    void* ptr = mmap(nullptr, st.st_size, PROT_READ,
                     MAP_PRIVATE, fd, 0);
    close(fd);

    if (ptr == MAP_FAILED) return 1;

    const unsigned char* data = static_cast<const unsigned char*>(ptr);
    unsigned long sum = 0;
    for (off_t i = 0; i < st.st_size; ++i) {
        sum += data[i];
    }
    printf("校验和: %lu\n", sum);

    munmap(ptr, st.st_size);
    return 0;
}
```

**匿名映射——大块内存分配**：

```cpp
#include <sys/mman.h>
#include <cstdio>
#include <cstring>

int main() {
    size_t size = 1024 * 1024 * 1024;

    void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) { perror("mmap"); return 1; }

    int* arr = static_cast<int*>(ptr);
    size_t count = size / sizeof(int);
    for (size_t i = 0; i < count; ++i) {
        arr[i] = static_cast<int>(i);
    }

    printf("分配并初始化 %zu MB\n", size / (1024 * 1024));

    munmap(ptr, size);
    return 0;
}
```

**匿名映射 vs malloc**：

| 对比 | malloc | mmap 匿名映射 |
|------|--------|---------------|
| 小块分配 | ✅ 高效（ptmalloc/tcmalloc） | ❌ 页对齐浪费 |
| 大块分配 | 内部可能调用 mmap | 直接调用 mmap |
| 对齐 | 默认 16 字节 | 页对齐（4KB） |
| 释放 | free() | munmap() |
| 零初始化 | 不保证 | 保证 |

***

### 4. 共享映射 vs 私有映射

**MAP_SHARED——修改对其他进程可见**：

```cpp
// 进程 A：创建共享映射
int fd = open("shared.dat", O_RDWR | O_CREAT, 0666);
ftruncate(fd, 4096);

void* ptr = mmap(nullptr, 4096, PROT_READ | PROT_WRITE,
                 MAP_SHARED, fd, 0);
close(fd);

int* counter = static_cast<int*>(ptr);
*counter = 0;

// 进程 A 递增
for (int i = 0; i < 100000; ++i) {
    __sync_fetch_and_add(counter, 1);
}
```

```cpp
// 进程 B：映射同一文件
int fd = open("shared.dat", O_RDWR);

void* ptr = mmap(nullptr, 4096, PROT_READ | PROT_WRITE,
                 MAP_SHARED, fd, 0);
close(fd);

int* counter = static_cast<int*>(ptr);

// 进程 B 递增
for (int i = 0; i < 100000; ++i) {
    __sync_fetch_and_add(counter, 1);
}
```

**MAP_PRIVATE——写时复制（Copy-on-Write）**：

```cpp
int fd = open("data.txt", O_RDONLY);
void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE,
                 MAP_PRIVATE, fd, 0);
close(fd);

char* data = static_cast<char*>(ptr);
data[0] = 'X';

// 修改不会写回文件
// 内核创建了私有副本
```

**COW 机制图解**：

```
MAP_SHARED:
  进程A页表 ──→ 物理页 ←── 进程B页表
                    │
                    ↓
                 磁盘文件

MAP_PRIVATE（修改前）:
  进程A页表 ──→ 物理页 ←── 进程B页表
                    │
                    ↓
                 磁盘文件

MAP_PRIVATE（进程A修改后）:
  进程A页表 ──→ 私有副本（新物理页）  ✗ 不写回
  进程B页表 ──→ 原始物理页 ──→ 磁盘文件
```

***

### 5. mmap 用于文件 I/O

**mmap 文件 I/O vs read/write**：

```cpp
// 方式 1：传统 read
#include <unistd.h>
#include <fcntl.h>

void read_file(const char* path) {
    int fd = open(path, O_RDONLY);
    char buf[4096];
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        process(buf, n);
    }
    close(fd);
}

// 方式 2：mmap
#include <sys/mman.h>

void mmap_file(const char* path) {
    int fd = open(path, O_RDONLY);
    struct stat st;
    fstat(fd, &st);

    void* ptr = mmap(nullptr, st.st_size, PROT_READ,
                     MAP_PRIVATE, fd, 0);
    close(fd);

    process(static_cast<const char*>(ptr), st.st_size);
    munmap(ptr, st.st_size);
}
```

**性能对比**：

| 维度 | read/write | mmap |
|------|-----------|------|
| 系统调用 | 每次 read/write 一次 | 仅 mmap/munmap 各一次 |
| 数据拷贝 | 内核缓冲区 → 用户缓冲区（2次拷贝） | 直接访问页缓存（0次额外拷贝） |
| 随机访问 | 需 seek + read | 直接指针偏移 |
| 顺序访问 | 高效（预读） | 可能产生大量缺页中断 |
| 内存占用 | 用户缓冲区 + 内核缓冲区 | 仅页缓存 |
| 适用场景 | 大文件顺序读写、网络 I/O | 随机访问、多进程共享、小文件 |

**mmap 适合的场景**：

```cpp
// 场景 1：随机访问大文件中的记录
struct Record {
    int id;
    char name[64];
    double value;
};

void random_access(const char* path, int target_id) {
    int fd = open(path, O_RDONLY);
    struct stat st;
    fstat(fd, &st);

    auto* records = static_cast<Record*>(
        mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0));
    close(fd);

    size_t count = st.st_size / sizeof(Record);
    size_t idx = target_id % count;

    printf("Record %d: name=%s, value=%.2f\n",
           records[idx].id, records[idx].name, records[idx].value);

    munmap(records, st.st_size);
}

// 场景 2：只读配置文件
// 场景 3：共享库加载（ld.so 使用 mmap）
// 场景 4：数据库文件访问
```

**mmap 不适合的场景**：

```cpp
// 场景 1：大文件顺序写入（mmap 修改需要逐页写回）
// 场景 2：文件大小超过虚拟地址空间（32 位系统）
// 场景 3：频繁修改小区域（COW 开销）
// 场景 4：需要确保数据落盘的日志系统
```

***

### 6. mmap 用于 IPC（进程间通信）

共享内存是最快的 IPC 方式，mmap 是其实现基础。

**方式 1：文件共享映射**：

```cpp
// 写入进程
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

struct SharedData {
    int flag;
    char message[256];
};

int main() {
    int fd = open("/tmp/shm.dat", O_RDWR | O_CREAT | O_TRUNC, 0666);
    ftruncate(fd, sizeof(SharedData));

    auto* shared = static_cast<SharedData*>(
        mmap(nullptr, sizeof(SharedData), PROT_READ | PROT_WRITE,
             MAP_SHARED, fd, 0));
    close(fd);

    shared->flag = 0;
    strcpy(shared->message, "Hello from writer!");

    __sync_synchronize();
    shared->flag = 1;

    munmap(shared, sizeof(SharedData));
    return 0;
}
```

```cpp
// 读取进程
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>

struct SharedData {
    int flag;
    char message[256];
};

int main() {
    int fd = open("/tmp/shm.dat", O_RDWR);

    auto* shared = static_cast<SharedData*>(
        mmap(nullptr, sizeof(SharedData), PROT_READ | PROT_WRITE,
             MAP_SHARED, fd, 0));
    close(fd);

    while (shared->flag == 0) {
        __sync_synchronize();
        usleep(1000);
    }

    printf("收到消息: %s\n", shared->message);

    munmap(shared, sizeof(SharedData));
    return 0;
}
```

**方式 2：POSIX 共享内存（shm_open）**：

```cpp
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>

int main() {
    int fd = shm_open("/my_shm", O_RDWR | O_CREAT, 0666);
    ftruncate(fd, 4096);

    void* ptr = mmap(nullptr, 4096, PROT_READ | PROT_WRITE,
                     MAP_SHARED, fd, 0);
    close(fd);

    char* msg = static_cast<char*>(ptr);
    strcpy(msg, "通过 POSIX 共享内存通信");
    printf("写入: %s\n", msg);

    munmap(ptr, 4096);
    shm_unlink("/my_shm");
    return 0;
}
```

**方式 3：System V 共享内存**：

```cpp
#include <sys/ipc.h>
#include <sys/shm.h>
#include <cstdio>
#include <cstring>

int main() {
    key_t key = ftok("/tmp", 'A');
    int shmid = shmget(key, 4096, IPC_CREAT | 0666);

    void* ptr = shmat(shmid, nullptr, 0);
    strcpy(static_cast<char*>(ptr), "System V 共享内存");

    shmdt(ptr);
    shmctl(shmid, IPC_RMID, nullptr);
    return 0;
}
```

**共享内存 IPC 对比**：

| 方式 | API | 生命周期 | 路径 | 跨平台 |
|------|-----|---------|------|--------|
| 文件 mmap | open + mmap | 文件存在即有效 | 文件系统路径 | ✅ |
| POSIX shm | shm_open + mmap | 显式 shm_unlink | `/dev/shm/` 下 | Linux/macOS |
| System V shm | shmget + shmat | 显式 IPC_RMID | key_t 标识 | Linux/Unix |

***

### 7. mmap 用于内存分配

glibc 的 malloc 在分配大块内存时内部使用 mmap：

```cpp
// malloc 内部策略（简化）
void* malloc(size_t size) {
    if (size < MMAP_THRESHOLD) {
        // 小块：从堆上分配（brk/sbrk）
        return heap_alloc(size);
    } else {
        // 大块：使用 mmap 匿名映射
        void* ptr = mmap(nullptr, size + overhead,
                         PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        return ptr + overhead;
    }
}
```

**自定义内存分配器基于 mmap**：

```cpp
#include <sys/mman.h>
#include <cstddef>

class MmapAllocator {
public:
    static void* allocate(size_t size) {
        size_t page_size = 4096;
        size_t aligned = (size + page_size - 1) & ~(page_size - 1);

        void* ptr = mmap(nullptr, aligned, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (ptr == MAP_FAILED) return nullptr;
        return ptr;
    }

    static void deallocate(void* ptr, size_t size) {
        size_t page_size = 4096;
        size_t aligned = (size + page_size - 1) & ~(page_size - 1);
        munmap(ptr, aligned);
    }
};

int main() {
    constexpr size_t N = 1024 * 1024;
    int* arr = static_cast<int*>(MmapAllocator::allocate(N * sizeof(int)));

    for (size_t i = 0; i < N; ++i) {
        arr[i] = static_cast<int>(i % 256);
    }

    MmapAllocator::deallocate(arr, N * sizeof(int));
    return 0;
}
```

**大页（Huge Pages）分配**：

```cpp
#include <sys/mman.h>
#include <cstdio>

int main() {
    size_t huge_page_size = 2 * 1024 * 1024;
    size_t size = 10 * huge_page_size;

    void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
                     -1, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap huge page");
        return 1;
    }

    printf("分配 %zu MB 大页内存成功\n", size / (1024 * 1024));

    munmap(ptr, size);
    return 0;
}
```

| 页大小 | 常规页 | 大页（x86） | 巨页（x86） |
|--------|--------|------------|------------|
| 大小 | 4 KB | 2 MB | 1 GB |
| TLB 覆盖 | 小 | 大 | 极大 |
| 适用 | 通用 | 数据库、大数组 | 超大内存 |

***

### 8. 缺页中断与按需分页

mmap 建立映射时并不立即加载数据，而是在首次访问时触发缺页中断。

**缺页中断流程**：

```
1. 进程调用 mmap() → 仅建立 VMA（虚拟内存区域），不分配物理页
2. 进程首次访问映射地址 → CPU 触发缺页中断（Page Fault）
3. 内核处理缺页中断：
   a. 文件映射 → 从磁盘读取对应页到页缓存
   b. 匿名映射 → 分配零填充的物理页
   c. 私有映射写入 → COW，分配新物理页
4. 更新页表，映射虚拟地址到物理页
5. 恢复进程执行
```

**观察缺页中断**：

```bash
# 使用 perf 追踪缺页
perf stat -e faults ./myapp

# 使用 /proc 查看缺页
cat /proc/$(pidof myapp)/stat | awk '{print "minor faults:", $10, "major faults:", $12}'
```

**预读建议（madvise）**：

```cpp
#include <sys/mman.h>

void* ptr = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);

// 告知内核将使用顺序访问，触发预读
madvise(ptr, size, MADV_SEQUENTIAL);

// 告知内核将使用随机访问，禁用预读
madvise(ptr, size, MADV_RANDOM);

// 告知内核将很快访问，提前加载
madvise(ptr, size, MADV_WILLNEED);

// 告知内核不再使用，可释放
madvise(ptr, size, MADV_DONTNEED);
```

***

### 9. Windows 上的内存映射

Windows 使用 `CreateFileMapping` + `MapViewOfFile` 实现类似功能。

**Windows 文件映射**：

```cpp
#include <windows.h>
#include <cstdio>

int main() {
    HANDLE hFile = CreateFileA("data.txt",
        GENERIC_READ | GENERIC_WRITE,
        0, nullptr, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, nullptr);

    LARGE_INTEGER file_size;
    GetFileSizeEx(hFile, &file_size);

    HANDLE hMap = CreateFileMappingA(hFile, nullptr,
        PAGE_READWRITE, 0, 0, nullptr);

    void* ptr = MapViewOfFile(hMap,
        FILE_MAP_READ | FILE_MAP_WRITE,
        0, 0, 0);

    char* data = static_cast<char*>(ptr);
    printf("文件内容: %.*s\n", (int)file_size.QuadPart, data);

    data[0] = 'H';
    FlushViewOfFile(ptr, 0);

    UnmapViewOfFile(ptr);
    CloseHandle(hMap);
    CloseHandle(hFile);
    return 0;
}
```

**Linux vs Windows API 对照**：

| 功能 | Linux | Windows |
|------|-------|---------|
| 创建映射 | `mmap()` | `CreateFileMapping()` |
| 映射视图 | `mmap()`（同一调用） | `MapViewOfFile()` |
| 解除映射 | `munmap()` | `UnmapViewOfFile()` |
| 同步写回 | `msync()` | `FlushViewOfFile()` |
| 匿名映射 | `MAP_ANONYMOUS` | `CreateFileMapping(INVALID_HANDLE_VALUE)` |
| 共享内存 | `shm_open()` | `CreateFileMapping(name)` |
| 建议策略 | `madvise()` | `PrefetchVirtualMemory()` |
| 大页支持 | `MAP_HUGETLB` | `SEC_LARGE_PAGES` 标志 |

**跨平台封装**：

```cpp
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

class MappedFile {
public:
    bool open(const char* path, bool writable) {
#ifdef _WIN32
        DWORD access = writable ? GENERIC_READ | GENERIC_WRITE : GENERIC_READ;
        DWORD protect = writable ? PAGE_READWRITE : PAGE_READONLY;
        DWORD map_access = writable ? FILE_MAP_READ | FILE_MAP_WRITE : FILE_MAP_READ;

        hFile_ = CreateFileA(path, access, 0, nullptr,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile_ == INVALID_HANDLE_VALUE) return false;

        LARGE_INTEGER size;
        GetFileSizeEx(hFile_, &size);
        size_ = size.QuadPart;

        hMap_ = CreateFileMappingA(hFile_, nullptr, protect, 0, 0, nullptr);
        if (!hMap_) { CloseHandle(hFile_); return false; }

        data_ = MapViewOfFile(hMap_, map_access, 0, 0, 0);
        return data_ != nullptr;
#else
        int flags = writable ? O_RDWR : O_RDONLY;
        fd_ = ::open(path, flags);
        if (fd_ < 0) return false;

        struct stat st;
        fstat(fd_, &st);
        size_ = st.st_size;

        int prot = writable ? PROT_READ | PROT_WRITE : PROT_READ;
        data_ = mmap(nullptr, size_, prot, MAP_SHARED, fd_, 0);
        if (data_ == MAP_FAILED) { data_ = nullptr; ::close(fd_); return false; }
        return true;
#endif
    }

    void close() {
#ifdef _WIN32
        if (data_) UnmapViewOfFile(data_);
        if (hMap_) CloseHandle(hMap_);
        if (hFile_ != INVALID_HANDLE_VALUE) CloseHandle(hFile_);
#else
        if (data_) munmap(data_, size_);
        if (fd_ >= 0) ::close(fd_);
#endif
        data_ = nullptr;
        size_ = 0;
    }

    void* data() const { return data_; }
    size_t size() const { return size_; }

private:
    void* data_ = nullptr;
    size_t size_ = 0;
#ifdef _WIN32
    HANDLE hFile_ = INVALID_HANDLE_VALUE;
    HANDLE hMap_ = nullptr;
#else
    int fd_ = -1;
#endif
};
```

***

### 10. mmap 常见陷阱

**陷阱 1：映射大小不是文件大小**：

```cpp
// 错误：映射大小超过文件大小
int fd = open("small.txt", O_RDONLY);
void* ptr = mmap(nullptr, 4096, PROT_READ, MAP_PRIVATE, fd, 0);
// 如果文件只有 100 字节，访问 100~4095 会触发 SIGBUS

// 正确：先获取文件大小
struct stat st;
fstat(fd, &st);
void* ptr = mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
```

**陷阱 2：文件被截断**：

```cpp
// 进程 A 映射了文件
void* ptr = mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0);

// 进程 B 截断了文件
truncate("data.txt", 0);

// 进程 A 访问映射区域 → SIGBUS
```

**陷阱 3：不调用 msync 可能丢数据**：

```cpp
// MAP_SHARED 映射修改后，数据不保证立即写回
// 必须调用 msync 确保持久化
msync(ptr, size, MS_SYNC);

// MS_SYNC: 同步等待写回完成
// MS_ASYNC: 异步发起写回
// MS_INVALIDATE: 使缓存失效
```

**陷阱 4：munmap 失败导致内存泄漏**：

```cpp
// munmap 的地址和大小必须与 mmap 完全匹配
void* ptr = mmap(nullptr, 8192, PROT_READ | PROT_WRITE,
                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

munmap(ptr, 4096);         // 解除前半部分
munmap((char*)ptr + 4096, 4096);  // 解除后半部分

// 错误：部分解除后再用原地址 munmap
// munmap(ptr, 8192);  // 前半已解除，行为未定义
```

**陷阱 5：mmap 不是万能替代 read/write**：

```cpp
// 顺序写入大文件：write 比 mmap 更高效
// 原因：mmap 每页修改都需要页缓存和写回机制

// 推荐：顺序 I/O 用 read/write，随机 I/O 用 mmap
```

***

### 11. 极简总结

| 要点 | 内容 |
|------|------|
| 核心机制 | 将文件/设备映射到虚拟地址空间，按需缺页加载 |
| 两大类型 | 文件映射（有后端文件）、匿名映射（零初始化） |
| 两种模式 | MAP_SHARED（共享写回）、MAP_PRIVATE（COW 私有） |
| 文件 I/O | 零拷贝、随机访问高效；顺序写入不如 read/write |
| IPC | 共享内存是最快 IPC，基于 mmap MAP_SHARED |
| 内存分配 | 大块分配用匿名 mmap，小块用 malloc |
| 缺页中断 | 首次访问触发，madvise 可给出预读建议 |
| Windows | CreateFileMapping + MapViewOfFile |
| 常见陷阱 | 文件截断→SIGBUS、不 msync→丢数据、大小不匹配 |

***

### 相关阅读

- [什么是虚拟内存](../01-基础概念/31-什么是虚拟内存.md)
- [栈与堆](./00-栈与堆.md)