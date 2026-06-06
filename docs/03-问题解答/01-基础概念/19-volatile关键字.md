# volatile 关键字详解
> 📖 相关章节：[运算符与表达式](../../01-C语言/02-运算符与表达式.md)、[控制结构](../../01-C语言/03-控制结构.md)、[函数](../../01-C语言/04-函数.md)、[基础特性](../../02-CPP/01-基础特性.md)、[命名空间](../../02-CPP/02-命名空间与编码规范.md)

### 1. 先抓核心

**volatile** = **告诉编译器：这个变量可能被外部因素改变，不要对它做优化**。每次必须从内存重新读取，不能缓存到寄存器。

***

### 2. 为什么需要 volatile

```cpp
int flag = 0;

// 线程1：等待
while (flag == 0) {
    // 编译器可能优化为死循环！
    // 因为编译器认为 flag 不可能在循环内改变
}

// 线程2：设置
flag = 1;
```

**问题**：编译器优化后，`while(flag == 0)` 可能只读一次 flag，之后永远用缓存的值。

**解决**：`volatile int flag = 0;` → 编译器每次都从内存重新读取。

### 3. volatile 的三大作用

1. **禁止寄存器缓存**：每次都从内存读/写
2. **限制编译器重排**：volatile 访问之间的相对顺序不会被编译器重排，但不保证 volatile 与非 volatile 访问之间的顺序
3. **禁止优化删除**：看似"无用"的读写不会被优化掉

#### 1. 禁止优化删除示例

```cpp
// 硬件寄存器写操作，看似"无用"但必须执行
volatile uint32_t* reset_reg = (uint32_t*)0x40002000;
*reset_reg = 0x01;  // 触发硬件复位
// 没有 volatile → 编译器可能优化掉这行赋值
```

### 4. volatile 的典型场景

| 场景 | 示例 | 说明 |
|:---|:---|:---|
| 硬件寄存器 | `volatile uint32_t* reg = (uint32_t*)0x4000;` | 寄存器值可能被硬件改变 |
| 信号处理 | `volatile sig_atomic_t flag;` | 信号处理函数可能修改 |
| 多线程标志 | `volatile bool done;`（不推荐） | 应使用 `std::atomic` |
| setjmp/longjmp | 跨跳转的局部变量 | longjmp 后变量值不确定 |

#### 1. 嵌入式硬件寄存器完整示例

```cpp
// GPIO 寄存器映射（STM32 风格）
struct GPIO_Regs {
    volatile uint32_t MODER;    // 模式寄存器
    volatile uint32_t OTYPER;   // 输出类型
    volatile uint32_t OSPEEDR;  // 速度
    volatile uint32_t PUPDR;    // 上下拉
    volatile uint32_t IDR;      // 输入数据
    volatile uint32_t ODR;      // 输出数据
};

#define GPIOA ((volatile GPIO_Regs*)0x40020000)

void toggle_led() {
    GPIOA->ODR ^= (1 << 5);  // 翻转 pin5
    // volatile 确保每次直接写硬件寄存器
}
```

### 5. volatile 不保证原子性

```cpp
volatile int counter = 0;

// 线程1
counter++;  // volatile 不保证 ++ 是原子的！
// ++ 实际是：读 → 加1 → 写，三步操作
// 线程1在读和写之间可能被线程2打断

// 线程2（同时执行）
counter++;  // 同样是非原子操作

// 期望结果：counter = 2
// 实际可能：counter = 1（读-改-写交错）

// 正确做法
std::atomic<int> counter{0};
counter++;  // 原子操作，线程安全
```

### 6. volatile 与 const 的组合

```cpp
// 只读硬件寄存器（硬件可改，程序只读）
volatile const uint32_t* status_reg = (uint32_t*)0x4000;
// volatile：每次都从硬件读
// const：程序不能写入

// 硬件输出寄存器（程序可写，硬件也可改）
volatile uint32_t* output_reg = (volatile uint32_t*)0x4004;

// ROM 映射的只读外设
volatile const char* firmware_version = (char*)0x08001000;
```

### 7. volatile 与多线程（为什么不够用）

```cpp
volatile bool ready = false;
int data = 0;

// 线程1：生产者
data = 42;          // ① 写数据
ready = true;       // ② 设标志
// volatile 不能保证 ① 先于 ② 被执行！
// CPU/编译器仍可能重排非 volatile 操作

// 线程2：消费者
while (!ready);     // ③ 等标志
int x = data;       // ④ 读数据 → 可能读到旧值！
```

**正确做法**：使用 `std::atomic` 提供内存序保证。

### 8. C++ 中 volatile 的现状

- C++20 弃用了 volatile 的部分用法（复合赋值、自增自减等）
- C++ 中多线程同步应使用 `std::atomic`
- volatile 在 C++ 中主要用于与硬件交互

### 9. 极简总结

**volatile = 禁止编译器优化 → 每次从内存读取 → 但不保证原子性 → 多线程用 atomic → 硬件交互用 volatile**

***

### 相关阅读

- [什么是未定义行为](./07-什么是未定义行为.md)
- [volatile在Cpp中的状态](./13-volatile在Cpp中的状态.md)
- [_Atomic与C11原子操作](./27-什么是-Atomic与C11原子操作.md)