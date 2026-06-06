# 什么是看门狗Watchdog
> 📖 相关章节：[嵌入式概述](../../12-嵌入式与实时系统/00-嵌入式系统概述.md)

> **看门狗就是系统的"心跳监测器"——程序必须定期"喂狗"，如果忘了喂，看门狗就认为系统挂了，自动复位。** 它是嵌入式系统最后的防线，防止软件死锁或跑飞导致系统永久无响应。

***

### 1. 核心提炼

**看门狗定时器（Watchdog Timer, WDT）** 是一个硬件定时器，需要软件定期"喂狗"（重置计数器）。如果软件因死锁、死循环或跑飞而未能按时喂狗，看门狗定时器溢出后会触发系统复位，让系统重新启动——这是嵌入式系统自我恢复的核心机制。

***

### 2. 生活类比

| 类比 | 说明 |
|------|------|
| 看门狗 = 真正的看门犬 | 主人定时喂它，如果不喂它就叫（复位系统） |
| 看门狗 = 医院的心电监护 | 心跳正常就没事，心跳停止就报警 |
| 看门狗 = 倒计时炸弹 | 必须定期重置倒计时，否则爆炸（复位） |
| 喂狗 = 按下重置按钮 | 告诉看门狗"我还活着" |

**具体场景**：你的嵌入式设备运行了3个月后突然死机——没有看门狗，设备永远无响应，需要人工重启；有看门狗，设备在1秒内自动复位恢复。

***

### 3. 看门狗的原理

#### 3.1 工作流程

```
看门狗工作流程：

1. 初始化看门狗，设置超时时间（如1秒）
2. 启动看门狗，计数器开始递减
3. 正常运行：软件定期"喂狗"，重置计数器
4. 异常情况：软件未能喂狗
5. 计数器减到0 → 触发系统复位

时间线：
0ms      200ms    400ms    600ms    800ms   1000ms
  │        │        │        │        │        │
  ├─喂狗──→├─喂狗──→├─喂狗──→├─喂狗──→├─喂狗──→┤  正常运行
  │        │        │        │        │        │
  ├─喂狗──→├─喂狗──→├─死机────────────────────→复位!  异常情况
  │        │        │        ↑                    │
  │        │        │     未能喂狗              看门狗超时
```

#### 3.2 硬件看门狗寄存器

```cpp
// STM32 独立看门狗（IWDG）寄存器
typedef struct {
    volatile uint32_t KR;      // 键值寄存器（喂狗/使能）
    volatile uint32_t PR;      // 预分频寄存器
    volatile uint32_t RLR;     // 重装载寄存器
    volatile uint32_t SR;      // 状态寄存器
} IWDG_TypeDef;

#define IWDG ((IWDG_TypeDef*)0x40003000UL)

// 喂狗键值
#define IWDG_KEY_ENABLE   0xCCCC  // 使能看门狗
#define IWDG_KEY_WRITE    0x5555  // 允许写入寄存器
#define IWDG_KEY_RELOAD   0xAAAA  // 喂狗（重装载计数器）

// 初始化独立看门狗
void iwdg_init(uint32_t timeout_ms) {
    // 1. 使能寄存器写入
    IWDG->KR = IWDG_KEY_WRITE;

    // 2. 设置预分频器（LSI时钟约32kHz）
    // 预分频值 = 32 → 计数频率 = 32000/32 = 1000Hz
    IWDG->PR = 0x03;  // /32

    // 3. 设置重装载值
    // 超时 = RLR / 1000 秒
    // 1秒超时：RLR = 1000
    IWDG->RLR = timeout_ms;

    // 4. 等待寄存器更新完成
    while (IWDG->SR & 0x1F);

    // 5. 喂狗（重装载计数器）
    IWDG->KR = IWDG_KEY_RELOAD;

    // 6. 启动看门狗
    IWDG->KR = IWDG_KEY_ENABLE;
}

// 喂狗
void iwdg_feed(void) {
    IWDG->KR = IWDG_KEY_RELOAD;
}
```

***

### 4. 硬件看门狗 vs 软件看门狗

| 维度 | 硬件看门狗 | 软件看门狗 |
|------|-----------|-----------|
| 实现方式 | 芯片内置硬件定时器 | 软件定时器+任务监控 |
| 可靠性 | 极高（不受软件影响） | 中等（软件本身可能出问题） |
| 灵活性 | 低（固定功能） | 高（可自定义监控策略） |
| 复位方式 | 硬件复位（最彻底） | 软件复位或通知 |
| 时钟源 | 独立时钟（不受主时钟影响） | 依赖系统时钟 |
| 适用场景 | 安全关键系统 | 辅助监控 |

#### 4.1 软件看门狗实现

```cpp
// 软件看门狗：监控多个任务的存活状态

#define MAX_TASKS 8

typedef struct {
    const char* name;
    uint32_t timeout_ms;        // 超时时间
    uint32_t last_feed_ms;      // 上次喂狗时间
    bool enabled;
} SoftWatchdogEntry;

static SoftWatchdogEntry wd_entries[MAX_TASKS];
static int wd_count = 0;

// 注册任务到软件看门狗
int soft_wd_register(const char* name, uint32_t timeout_ms) {
    if (wd_count >= MAX_TASKS) return -1;

    wd_entries[wd_count].name = name;
    wd_entries[wd_count].timeout_ms = timeout_ms;
    wd_entries[wd_count].last_feed_ms = get_system_tick();
    wd_entries[wd_count].enabled = true;
    return wd_count++;
}

// 任务喂狗
void soft_wd_feed(int handle) {
    if (handle >= 0 && handle < wd_count) {
        wd_entries[handle].last_feed_ms = get_system_tick();
    }
}

// 软件看门狗检查（在低优先级定时器任务中运行）
void soft_wd_check(void) {
    uint32_t now = get_system_tick();

    for (int i = 0; i < wd_count; i++) {
        if (!wd_entries[i].enabled) continue;

        uint32_t elapsed = now - wd_entries[i].last_feed_ms;
        if (elapsed > wd_entries[i].timeout_ms) {
            // 任务超时！
            log_error("任务 '%s' 超时! 已过 %ums (阈值 %ums)",
                      wd_entries[i].name, elapsed, wd_entries[i].timeout_ms);

            // 可以选择：记录日志、复位系统、重启任务
            system_reset();
        }
    }
}
```

***

### 5. 喂狗策略

#### 5.1 喂狗位置的选择

```cpp
// 策略1：在主循环中喂狗（最简单）
int main(void) {
    hardware_init();
    iwdg_init(1000);  // 1秒超时

    for (;;) {
        task1();
        task2();
        task3();
        iwdg_feed();  // 主循环末尾喂狗
    }
}
// 问题：如果某个task死循环，永远不会到喂狗处 → 看门狗会复位
// 这正是我们想要的行为！

// 策略2：在多个位置喂狗（更精细的监控）
int main(void) {
    hardware_init();
    iwdg_init(2000);  // 2秒超时

    for (;;) {
        task1();
        iwdg_feed();  // 每个任务后都喂狗

        task2();
        iwdg_feed();

        task3();
        iwdg_feed();
    }
}
// 问题：如果task1死循环，task2和task3不执行，但喂狗在task1后面
// 看门狗会复位——正确行为

// 策略3：在RTOS任务中喂狗
void watchdog_task(void* pvParameters) {
    iwdg_init(3000);  // 3秒超时

    for (;;) {
        // 检查所有关键任务是否存活
        if (task1_is_alive() && task2_is_alive() && task3_is_alive()) {
            iwdg_feed();  // 所有任务正常才喂狗
        }
        // 如果有任务不正常，不喂狗，让看门狗复位系统
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

#### 5.2 喂狗策略对比

| 策略 | 优点 | 缺点 | 适用场景 |
|------|------|------|---------|
| 主循环末尾喂狗 | 简单 | 只监控主循环 | 简单裸机系统 |
| 多位置喂狗 | 覆盖更多代码路径 | 代码分散 | 中等复杂度 |
| 独立任务喂狗 | 监控所有任务 | 需要RTOS | RTOS系统 |
| 条件喂狗 | 精确监控 | 逻辑复杂 | 安全关键系统 |

#### 5.3 常见喂狗错误

```cpp
// 错误1：在中断中喂狗
void TIM2_IRQHandler(void) {
    iwdg_feed();  // 危险！即使主循环死锁，中断仍会触发，看门狗永远不会复位
}

// 错误2：喂狗间隔太接近超时时间
iwdg_init(1000);  // 1秒超时
// 在主循环中每990ms喂一次 → 太危险！
// 如果主循环稍有延迟就会超时

// 正确：喂狗间隔 < 超时时间 / 2
iwdg_init(2000);  // 2秒超时
// 主循环约500ms喂一次 → 安全余量充足

// 错误3：在死循环中喂狗
void some_function(void) {
    while (1) {
        do_something();
        iwdg_feed();  // 死循环中喂狗，看门狗永远不触发！
    }
}
// 这完全违背了看门狗的目的
```

***

### 6. 看门狗与系统可靠性

#### 6.1 看门狗在可靠性设计中的位置

```
系统可靠性层次（从内到外）：

1. 代码质量 → 预防bug
2. 错误检测 → 断言、校验
3. 错误恢复 → 重试、降级
4. 看门狗 → 最后的防线（自动复位）
5. 外部监控 → 外部硬件复位

看门狗是"最后手段"——它不修复问题，只是让系统重启
重启后问题可能仍然存在，但至少系统不会永久无响应
```

#### 6.2 窗口看门狗

```cpp
// 窗口看门狗（Window Watchdog）
// 与独立看门狗不同，窗口看门狗有"窗口"限制：
// 喂狗太早也会触发复位！

// 独立看门狗：0 < 喂狗时间 < 超时时间
// 窗口看门狗：窗口下限 < 喂狗时间 < 超时时间

// 窗口看门狗的优势：
// - 防止程序跑飞后恰好执行了喂狗代码
// - 确保喂狗发生在正确的时间窗口

// 时间线：
// 0ms        200ms      800ms     1000ms
// │           │          │         │
// │←─ 窗口 ──→│←── 可以喂狗 ──→│←─ 超时 ─→│
// │           │          │         │
// │  太早喂狗  │          │  正常喂狗 │  复位!  │
// │  →复位!   │          │         │         │

// STM32 窗口看门狗配置
void wwdg_init(void) {
    // 使能WWDG时钟
    RCC->APB1ENR |= RCC_APB1ENR_WWDGEN;

    // 设置预分频器
    WWDG->CFR = WWDG_PRESCALER_8;

    // 设置窗口值（喂狗的最早时间）
    WWDG->CFR |= (0x5F << 0);  // 窗口值 = 0x5F

    // 设置计数器值（超时值）
    WWDG->CR = 0x7F;  // 计数器 = 0x7F

    // 使能看门狗
    WWDG->CR |= WWDG_CR_WDGA;
}

// 喂狗：必须在窗口内
void wwdg_feed(void) {
    // 计数器值必须在窗口值和0x3F之间
    WWDG->CR = 0x7F;
}
```

***

### 7. 常见误区

| 误区 | 事实 |
|------|------|
| 看门狗能修复bug | 看门狗只是复位系统，不修复bug |
| 喂狗越频繁越好 | 喂狗太频繁会掩盖问题 |
| 只需要硬件看门狗 | 软件看门狗可以监控更细粒度 |
| 看门狗超时越短越好 | 太短容易误触发，需要留余量 |
| 中断中喂狗更安全 | 中断中喂狗会让看门狗失效 |

***

### 8. 总结

| 维度 | 核心要点 |
|------|---------|
| 本质 | 硬件定时器，不喂狗就复位 |
| 硬件 vs 软件 | 硬件可靠，软件灵活，两者配合使用 |
| 喂狗策略 | 在正确位置、正确时间喂狗 |
| 窗口看门狗 | 太早或太晚喂狗都触发复位 |
| 设计原则 | 看门狗是最后防线，不是bug修复工具 |

**核心记忆**：看门狗 = 不喂狗就复位的定时器。它是系统最后的防线——不修复问题，但保证系统不会永久无响应。喂狗位置和策略是关键，错误喂狗比没有看门狗更危险。