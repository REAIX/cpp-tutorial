# 什么是内存池 Memory Pool
> 📖 相关章节：[内存管理](../../01-C语言/09-内存管理.md)、[性能优化](../../04-工程实践/08-性能优化.md)

> "与其每次找系统要内存，不如一次要一大块自己管。"

***

### 1. 要义概览

内存池是预先向系统申请一大块内存，然后按自定义策略从中分配和回收小块内存的技术，避免频繁调用 malloc/free，显著提升分配性能并减少内存碎片。

***

### 2. 为什么需要内存池

直接使用 `malloc`/`new` 存在多个问题：

```cpp
#include <iostream>
#include <chrono>
#include <vector>

void benchmarkMalloc() {
    constexpr int N = 1000000;

    auto start = std::chrono::high_resolution_clock::now();
    {
        std::vector<int*> ptrs;
        ptrs.reserve(N);
        for (int i = 0; i < N; ++i) {
            ptrs.push_back(new int(i));
        }
        for (auto p : ptrs) delete p;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "malloc/free: " << ns / N << " ns/次" << std::endl;
}

int main() {
    benchmarkMalloc();
}
```

`malloc`/`free` 的问题：

| 问题 | 说明 | 影响 |
|-----|------|------|
| 系统调用开销 | 每次分配可能触发内核态切换 | 慢 100~1000 ns |
| 内存碎片 | 频繁分配释放导致碎片化 | 内存利用率低 |
| 锁竞争 | 多线程共享堆，需要加锁 | 并发瓶颈 |
| 元数据开销 | 每块内存需要额外管理信息 | 8~16 字节/块 |
| 缓存不友好 | 分散在堆各处 | 缓存命中率低 |

内存池的优势：

| 优势 | 说明 |
|-----|------|
| O(1) 分配 | 从预分配池中直接取 |
| O(1) 释放 | 归还到池中，不调用 free |
| 零碎片 | 固定大小块无外部碎片 |
| 缓存友好 | 连续内存布局 |
| 无锁可能 | 线程本地池无需加锁 |

### 3. 固定大小内存池（Free List）

固定大小内存池是最简单高效的实现，用空闲链表管理预分配的内存块：

```cpp
#include <iostream>
#include <cstdint>
#include <cassert>

class FixedSizePool {
    struct FreeNode {
        FreeNode* next;
    };

    void* pool_;
    FreeNode* free_list_;
    std::size_t block_size_;
    std::size_t capacity_;
    std::size_t allocated_;

public:
    FixedSizePool(std::size_t block_size, std::size_t capacity)
        : block_size_(block_size), capacity_(capacity), allocated_(0) {
        std::size_t actual_size = block_size_;
        if (actual_size < sizeof(FreeNode)) {
            actual_size = sizeof(FreeNode);
        }

        pool_ = operator new(actual_size * capacity);

        free_list_ = nullptr;
        char* current = static_cast<char*>(pool_);
        for (std::size_t i = 0; i < capacity; ++i) {
            FreeNode* node = reinterpret_cast<FreeNode*>(current);
            node->next = free_list_;
            free_list_ = node;
            current += actual_size;
        }
    }

    ~FixedSizePool() {
        operator delete(pool_);
    }

    FixedSizePool(const FixedSizePool&) = delete;
    FixedSizePool& operator=(const FixedSizePool&) = delete;

    void* allocate() {
        if (!free_list_) return nullptr;
        FreeNode* node = free_list_;
        free_list_ = node->next;
        ++allocated_;
        return node;
    }

    void deallocate(void* ptr) {
        if (!ptr) return;
        FreeNode* node = static_cast<FreeNode*>(ptr);
        node->next = free_list_;
        free_list_ = node;
        --allocated_;
    }

    std::size_t allocated() const { return allocated_; }
    std::size_t capacity() const { return capacity_; }
    std::size_t available() const { return capacity_ - allocated_; }
};

struct Particle {
    float x, y, z;
    float vx, vy, vz;
    int life;
};

int main() {
    FixedSizePool pool(sizeof(Particle), 10000);

    std::cout << "块大小: " << sizeof(Particle) << " 字节" << std::endl;
    std::cout << "池容量: " << pool.capacity() << std::endl;

    Particle* p1 = static_cast<Particle*>(pool.allocate());
    p1->x = 1.0f; p1->y = 2.0f; p1->z = 3.0f;
    p1->vx = 0.1f; p1->vy = 0.2f; p1->vz = 0.3f;
    p1->life = 100;

    std::cout << "分配后: " << pool.allocated() << "/" << pool.capacity() << std::endl;

    pool.deallocate(p1);
    std::cout << "释放后: " << pool.allocated() << "/" << pool.capacity() << std::endl;
}
```

Free List 内存布局：

```
预分配的大块内存:
+--------+--------+--------+--------+--------+
| Block0 | Block1 | Block2 | Block3 | Block4 | ...
+--------+--------+--------+--------+--------+

Free List 链表（初始状态）:
Block0 -> Block1 -> Block2 -> Block3 -> Block4 -> nullptr

分配 2 块后:
Block2 -> Block3 -> Block4 -> nullptr
( Block0, Block1 已被使用 )

释放 Block1 后:
Block1 -> Block2 -> Block3 -> Block4 -> nullptr
( Block0 仍在使用, Block1 回到链表头部 )
```

### 4. 可变大小内存池

可变大小内存池支持不同大小的分配请求，通常使用块式管理：

```cpp
#include <iostream>
#include <vector>
#include <cstdint>
#include <cstring>

class VariableSizePool {
    struct BlockHeader {
        std::size_t size;
        bool is_free;
        BlockHeader* next;
    };

    std::vector<void*> chunks_;
    BlockHeader* first_block_;

    static constexpr std::size_t ALIGNMENT = sizeof(void*);
    static constexpr std::size_t HEADER_SIZE = sizeof(BlockHeader);
    static constexpr std::size_t CHUNK_SIZE = 4096;

    std::size_t alignUp(std::size_t n) {
        return (n + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    }

    void allocateChunk(std::size_t min_size) {
        std::size_t chunk_size = CHUNK_SIZE;
        if (min_size + HEADER_SIZE > chunk_size) {
            chunk_size = min_size + HEADER_SIZE;
        }

        void* chunk = operator new(chunk_size);
        chunks_.push_back(chunk);

        BlockHeader* header = static_cast<BlockHeader*>(chunk);
        header->size = chunk_size - HEADER_SIZE;
        header->is_free = true;
        header->next = first_block_;
        first_block_ = header;
    }

public:
    VariableSizePool() : first_block_(nullptr) {}

    ~VariableSizePool() {
        for (void* chunk : chunks_) {
            operator delete(chunk);
        }
    }

    VariableSizePool(const VariableSizePool&) = delete;
    VariableSizePool& operator=(const VariableSizePool&) = delete;

    void* allocate(std::size_t size) {
        std::size_t aligned_size = alignUp(size);

        BlockHeader* best = nullptr;
        BlockHeader* best_prev = nullptr;
        BlockHeader* prev = nullptr;
        BlockHeader* current = first_block_;

        while (current) {
            if (current->is_free && current->size >= aligned_size) {
                if (!best || current->size < best->size) {
                    best = current;
                    best_prev = prev;
                }
            }
            prev = current;
            current = current->next;
        }

        if (!best) {
            allocateChunk(aligned_size);
            best = first_block_;
            while (best) {
                if (best->is_free && best->size >= aligned_size) {
                    break;
                }
                best = best->next;
            }
            if (!best) return nullptr;
        }

        if (best->size >= aligned_size + HEADER_SIZE + ALIGNMENT) {
            BlockHeader* new_block = reinterpret_cast<BlockHeader*>(
                reinterpret_cast<char*>(best) + HEADER_SIZE + aligned_size
            );
            new_block->size = best->size - aligned_size - HEADER_SIZE;
            new_block->is_free = true;
            new_block->next = best->next;

            best->size = aligned_size;
            best->next = new_block;
        }

        best->is_free = false;
        return reinterpret_cast<char*>(best) + HEADER_SIZE;
    }

    void deallocate(void* ptr) {
        if (!ptr) return;
        BlockHeader* header = reinterpret_cast<BlockHeader*>(
            static_cast<char*>(ptr) - HEADER_SIZE
        );
        header->is_free = true;

        BlockHeader* current = first_block_;
        while (current) {
            if (current->is_free && current->next && current->next->is_free) {
                current->size += HEADER_SIZE + current->next->size;
                current->next = current->next->next;
            } else {
                current = current->next;
            }
        }
    }
};

int main() {
    VariableSizePool pool;

    int* a = static_cast<int*>(pool.allocate(sizeof(int)));
    *a = 42;
    double* b = static_cast<double*>(pool.allocate(sizeof(double)));
    *b = 3.14;
    char* c = static_cast<char*>(pool.allocate(128));
    std::memcpy(c, "Hello, Variable Pool!", 21);

    std::cout << "a = " << *a << std::endl;
    std::cout << "b = " << *b << std::endl;
    std::cout << "c = " << c << std::endl;

    pool.deallocate(a);
    pool.deallocate(b);
    pool.deallocate(c);
}
```

### 5. Arena/Region 分配器

Arena 分配器是最简单的策略：只分配不释放，一次性全部回收。适合生命周期相同的对象群：

```cpp
#include <iostream>
#include <cstdint>
#include <cstring>
#include <vector>

class ArenaAllocator {
    static constexpr std::size_t BLOCK_SIZE = 4096;

    struct Block {
        char* memory;
        std::size_t used;
        std::size_t capacity;
    };

    std::vector<Block> blocks_;
    std::size_t current_block_;
    std::size_t total_allocated_;

public:
    ArenaAllocator() : current_block_(0), total_allocated_(0) {
        addBlock(BLOCK_SIZE);
    }

    ~ArenaAllocator() {
        for (auto& block : blocks_) {
            delete[] block.memory;
        }
    }

    ArenaAllocator(const ArenaAllocator&) = delete;
    ArenaAllocator& operator=(const ArenaAllocator&) = delete;

    void* allocate(std::size_t size, std::size_t alignment = alignof(std::max_align_t)) {
        std::size_t aligned_size = (size + alignment - 1) & ~(alignment - 1);

        if (current_block_ >= blocks_.size()) {
            addBlock(std::max(aligned_size, BLOCK_SIZE));
        }

        Block& block = blocks_[current_block_];
        std::size_t aligned_offset = (block.used + alignment - 1) & ~(alignment - 1);

        if (aligned_offset + size <= block.capacity) {
            void* ptr = block.memory + aligned_offset;
            block.used = aligned_offset + size;
            total_allocated_ += size;
            return ptr;
        }

        addBlock(std::max(aligned_size, BLOCK_SIZE));
        Block& new_block = blocks_.back();
        current_block_ = blocks_.size() - 1;
        std::size_t aligned_offset_new = (new_block.used + alignment - 1) & ~(alignment - 1);
        void* ptr = new_block.memory + aligned_offset_new;
        new_block.used = aligned_offset_new + size;
        total_allocated_ += size;
        return ptr;
    }

    void reset() {
        for (auto& block : blocks_) {
            block.used = 0;
        }
        current_block_ = 0;
        total_allocated_ = 0;
    }

    std::size_t totalAllocated() const { return total_allocated_; }

private:
    void addBlock(std::size_t size) {
        Block block;
        block.memory = new char[size];
        block.used = 0;
        block.capacity = size;
        blocks_.push_back(block);
    }
};

struct Enemy {
    float x, y;
    int hp;
    int type;
};

struct Bullet {
    float x, y, dx, dy;
    int damage;
};

void gameFrame(ArenaAllocator& arena) {
    arena.reset();

    Enemy* enemies[100];
    for (int i = 0; i < 100; ++i) {
        enemies[i] = static_cast<Enemy*>(arena.allocate(sizeof(Enemy)));
        enemies[i]->x = static_cast<float>(i);
        enemies[i]->y = 0.0f;
        enemies[i]->hp = 100;
        enemies[i]->type = i % 3;
    }

    Bullet* bullets[500];
    for (int i = 0; i < 500; ++i) {
        bullets[i] = static_cast<Bullet*>(arena.allocate(sizeof(Bullet)));
        bullets[i]->x = 0.0f;
        bullets[i]->y = static_cast<float>(i);
        bullets[i]->dx = 1.0f;
        bullets[i]->dy = 0.0f;
        bullets[i]->damage = 10;
    }

    std::cout << "帧内分配: " << arena.totalAllocated() << " 字节" << std::endl;
}

int main() {
    ArenaAllocator arena;

    for (int frame = 0; frame < 3; ++frame) {
        std::cout << "=== 第 " << frame << " 帧 ===" << std::endl;
        gameFrame(arena);
    }
}
```

Arena 的特点：

| 特性 | 说明 |
|-----|------|
| 分配速度 | O(1)，仅移动指针 |
| 释放方式 | 整体 reset，不支持单个释放 |
| 碎片 | 零碎片（顺序分配） |
| 缓存友好 | ✅ 极佳，数据连续 |
| 适用场景 | 游戏帧、请求处理、编译过程 |

### 6. 对象池

对象池在内存池基础上增加了构造/析构管理，适合反复创建销毁的同类型对象：

```cpp
#include <iostream>
#include <vector>
#include <cassert>

template <typename T>
class ObjectPool {
    union Slot {
        T object;
        Slot* next;
        Slot() {}
        ~Slot() {}
    };

    std::vector<Slot*> chunks_;
    Slot* free_list_;
    std::size_t chunk_size_;
    std::size_t allocated_;

    void allocateChunk() {
        Slot* chunk = new Slot[chunk_size_];
        chunks_.push_back(chunk);

        for (std::size_t i = 0; i < chunk_size_ - 1; ++i) {
            chunk[i].next = &chunk[i + 1];
        }
        chunk[chunk_size_ - 1].next = free_list_;
        free_list_ = chunk;
    }

public:
    explicit ObjectPool(std::size_t chunk_size = 64)
        : free_list_(nullptr), chunk_size_(chunk_size), allocated_(0) {
        allocateChunk();
    }

    ~ObjectPool() {
        assert(allocated_ == 0 && "对象池中仍有未释放的对象");
        for (auto chunk : chunks_) {
            delete[] chunk;
        }
    }

    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;

    template <typename... Args>
    T* acquire(Args&&... args) {
        if (!free_list_) {
            allocateChunk();
        }
        Slot* slot = free_list_;
        free_list_ = slot->next;
        ++allocated_;
        return new (&slot->object) T(std::forward<Args>(args)...);
    }

    void release(T* obj) {
        obj->~T();
        Slot* slot = reinterpret_cast<Slot*>(obj);
        slot->next = free_list_;
        free_list_ = slot;
        --allocated_;
    }

    std::size_t allocated() const { return allocated_; }
};

class Connection {
    int id_;
    static int counter_;
public:
    Connection() : id_(counter_++) {
        std::cout << "Connection " << id_ << " 创建" << std::endl;
    }
    ~Connection() {
        std::cout << "Connection " << id_ << " 销毁" << std::endl;
    }
    int id() const { return id_; }
};

int Connection::counter_ = 0;

int main() {
    ObjectPool<Connection> pool(4);

    Connection* c1 = pool.acquire();
    Connection* c2 = pool.acquire();
    Connection* c3 = pool.acquire();

    std::cout << "活跃连接: " << pool.allocated() << std::endl;

    pool.release(c2);
    std::cout << "释放 c2 后活跃连接: " << pool.allocated() << std::endl;

    Connection* c4 = pool.acquire();
    std::cout << "重新获取连接: " << c4->id() << std::endl;

    pool.release(c1);
    pool.release(c3);
    pool.release(c4);
}
```

### 7. 内存池 vs malloc 性能对比

```cpp
#include <iostream>
#include <chrono>
#include <vector>

class SimplePool {
    struct Node { Node* next; };
    Node* free_list_ = nullptr;
    std::vector<void*> chunks_;

public:
    SimplePool(std::size_t block_size, std::size_t count) {
        if (block_size < sizeof(Node)) block_size = sizeof(Node);
        void* chunk = operator new(block_size * count);
        chunks_.push_back(chunk);
        char* p = static_cast<char*>(chunk);
        for (std::size_t i = 0; i < count; ++i) {
            Node* node = reinterpret_cast<Node*>(p);
            node->next = free_list_;
            free_list_ = node;
            p += block_size;
        }
    }

    ~SimplePool() { for (auto c : chunks_) operator delete(c); }

    void* alloc() {
        if (!free_list_) return nullptr;
        Node* n = free_list_;
        free_list_ = n->next;
        return n;
    }

    void free(void* p) {
        Node* n = static_cast<Node*>(p);
        n->next = free_list_;
        free_list_ = n;
    }
};

void benchmark() {
    constexpr int N = 1000000;
    constexpr int BLOCK = 32;

    auto start = std::chrono::high_resolution_clock::now();
    {
        std::vector<void*> ptrs;
        ptrs.reserve(N);
        for (int i = 0; i < N; ++i) ptrs.push_back(operator new(BLOCK));
        for (auto p : ptrs) operator delete(p);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto malloc_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    SimplePool pool(BLOCK, N);
    start = std::chrono::high_resolution_clock::now();
    {
        std::vector<void*> ptrs;
        ptrs.reserve(N);
        for (int i = 0; i < N; ++i) ptrs.push_back(pool.alloc());
        for (auto p : ptrs) pool.free(p);
    }
    end = std::chrono::high_resolution_clock::now();
    auto pool_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    std::cout << "malloc/new: " << malloc_ns / N << " ns/次" << std::endl;
    std::cout << "内存池:     " << pool_ns / N << " ns/次" << std::endl;
    std::cout << "加速比:     " << static_cast<double>(malloc_ns) / pool_ns << "x" << std::endl;
}

int main() {
    benchmark();
}
```

典型结果：

| 分配器 | 每次操作耗时 | 碎片 | 缓存友好 |
|-------|------------|------|---------|
| `malloc`/`new` | 50~200 ns | 高 | 差 |
| 固定大小池 | 2~5 ns | 零 | 优 |
| Arena 分配器 | 1~3 ns | 零 | 极优 |
| `std::pmr` | 5~15 ns | 低 | 良 |

### 8. std::pmr::memory_resource

C++17 引入了 `std::pmr` 多态内存资源，是标准化的内存池接口：

```cpp
#include <iostream>
#include <memory_resource>
#include <vector>
#include <string>

void demoPmr() {
    char buffer[1024];
    std::pmr::monotonic_buffer_resource mbr(buffer, sizeof(buffer));

    std::pmr::vector<int> vec(&mbr);
    for (int i = 0; i < 100; ++i) {
        vec.push_back(i);
    }

    std::pmr::string str(&mbr);
    str = "Hello, PMR!";

    std::cout << "vec[0]=" << vec[0] << " vec[99]=" << vec[99] << std::endl;
    std::cout << "str=" << str << std::endl;
    std::cout << "buffer 大小: " << sizeof(buffer) << " 字节" << std::endl;
}

void demoPmrPool() {
    std::pmr::unsynchronized_pool_resource pool;

    {
        std::pmr::vector<int> v1(&pool);
        v1.reserve(1000);
        for (int i = 0; i < 1000; ++i) v1.push_back(i);

        std::pmr::vector<double> v2(&pool);
        v2.reserve(500);
        for (int i = 0; i < 500; ++i) v2.push_back(i * 0.5);

        std::cout << "v1 size=" << v1.size() << " v2 size=" << v2.size() << std::endl;
    }
}

void demoSynchronizedPool() {
    std::pmr::synchronized_pool_resource sync_pool;

    std::pmr::vector<std::pmr::string> strings(&sync_pool);
    strings.push_back("hello");
    strings.push_back("world");
    strings.push_back("pmr");

    for (const auto& s : strings) {
        std::cout << s << " ";
    }
    std::cout << std::endl;
}

int main() {
    std::cout << "=== monotonic_buffer_resource ===" << std::endl;
    demoPmr();

    std::cout << "\n=== unsynchronized_pool_resource ===" << std::endl;
    demoPmrPool();

    std::cout << "\n=== synchronized_pool_resource ===" << std::endl;
    demoSynchronizedPool();
}
```

`std::pmr` 内存资源层次：

| 内存资源 | 线程安全 | 释放方式 | 适用场景 |
|---------|---------|---------|---------|
| `new_delete_resource` | ✅ | 单个释放 | 默认后备 |
| `null_memory_resource` | - | - | 分配即失败（测试用） |
| `monotonic_buffer_resource` | ❌ | 整体释放 | Arena 模式 |
| `unsynchronized_pool_resource` | ❌ | 单个释放 | 单线程池 |
| `synchronized_pool_resource` | ✅ | 单个释放 | 多线程池 |

### 9. 游戏与嵌入式场景

内存池在游戏和嵌入式系统中至关重要：

```cpp
#include <iostream>
#include <cstdint>
#include <cstring>

class GameMemorySystem {
    static constexpr std::size_t TOTAL_MEMORY = 64 * 1024 * 1024;

    char* base_memory_;

    struct PoolInfo {
        std::size_t block_size;
        std::size_t count;
        void* free_list;
        std::size_t allocated;
    };

    PoolInfo pools_[4];

    void initPool(int idx, std::size_t block_size, std::size_t count) {
        pools_[idx].block_size = block_size;
        pools_[idx].count = count;
        pools_[idx].allocated = 0;

        std::size_t actual = block_size;
        if (actual < sizeof(void*)) actual = sizeof(void*);

        char* start = base_memory_ + idx * 16 * 1024 * 1024;
        void** current = reinterpret_cast<void**>(start);

        pools_[idx].free_list = start;
        for (std::size_t i = 0; i < count - 1; ++i) {
            *current = start + (i + 1) * actual;
            current = reinterpret_cast<void**>(start + (i + 1) * actual);
        }
        *current = nullptr;
    }

public:
    GameMemorySystem() {
        base_memory_ = new char[TOTAL_MEMORY];
        initPool(0, 16,   1024 * 64);
        initPool(1, 64,   1024 * 16);
        initPool(2, 256,  1024 * 4);
        initPool(3, 1024, 1024);
    }

    ~GameMemorySystem() { delete[] base_memory_; }

    void* allocate(std::size_t size) {
        for (int i = 0; i < 4; ++i) {
            if (size <= pools_[i].block_size && pools_[i].free_list) {
                void** node = static_cast<void**>(pools_[i].free_list);
                pools_[i].free_list = *node;
                pools_[i].allocated++;
                return node;
            }
        }
        return nullptr;
    }

    void deallocate(void* ptr, std::size_t size) {
        for (int i = 0; i < 4; ++i) {
            if (size <= pools_[i].block_size) {
                void** node = static_cast<void**>(ptr);
                *node = pools_[i].free_list;
                pools_[i].free_list = node;
                pools_[i].allocated--;
                return;
            }
        }
    }

    void printStatus() const {
        const char* names[] = {"16B", "64B", "256B", "1024B"};
        for (int i = 0; i < 4; ++i) {
            std::cout << "池[" << names[i] << "]: "
                      << pools_[i].allocated << "/" << pools_[i].count
                      << " 已分配" << std::endl;
        }
    }
};

int main() {
    GameMemorySystem mem;

    void* a = mem.allocate(8);
    void* b = mem.allocate(50);
    void* c = mem.allocate(200);
    void* d = mem.allocate(800);

    std::cout << "分配后状态:" << std::endl;
    mem.printStatus();

    mem.deallocate(b, 50);
    std::cout << "\n释放 64B 块后:" << std::endl;
    mem.printStatus();
}
```

嵌入式系统注意事项：

| 注意点 | 说明 |
|-------|------|
| 确定性分配 | 内存池保证 O(1) 分配，无不可预测延迟 |
| 无碎片 | 固定大小池消除外部碎片 |
| 静态分配 | 可在编译期确定内存总量 |
| 无异常 | 分配失败返回 nullptr，不抛异常 |
| 对齐 | 需注意硬件对齐要求 |

### 10. 极简总结

| 要点 | 说明 |
|-----|------|
| **定义** | 预分配大块内存，自定义策略管理小块分配回收 |
| **固定大小池** | Free List 实现，O(1) 分配/释放，零碎片 |
| **可变大小池** | 块式管理，支持不同大小，有碎片风险 |
| **Arena 分配器** | 只分配不释放，整体 reset，极快 |
| **对象池** | 内存池 + 构造/析构管理 |
| **std::pmr** | C++17 标准化内存资源接口 |
| **vs malloc** | 快 10~50 倍，零碎片，缓存友好 |
| **适用场景** | 游戏、嵌入式、高频交易、服务器 |
| **核心权衡** | 预分配内存 vs 按需分配，灵活性 vs 性能 |

**口诀**：预分配大块，小块自己管；固定大小零碎片，Arena 最简单。

***

### 相关阅读

- [new/delete与malloc/free](./07-new-delete与malloc-free.md)
- [什么是内存映射mmap](20-什么是内存映射mmap.md)