# 什么是RTOS
> 📖 相关章节：[嵌入式概述](../../12-嵌入式与实时系统/00-嵌入式系统概述.md)

> **RTOS就是"保证按时完成"的操作系统——普通OS追求公平，RTOS追求准时。** 如果你的任务必须在5毫秒内响应，RTOS给你这个保证，普通OS只能说"尽量"。

***

### 1. 要点直击

**RTOS（Real-Time Operating System，实时操作系统）** 是一种保证任务在确定时间内完成的操作系统。与通用OS不同，RTOS的核心设计目标是**实时性（Determinism）**——不是追求最快，而是追求可预测、可保证的响应时间。

***

### 2. 生活类比

| 类比 | 通用OS | RTOS |
|------|--------|------|
| 餐厅 | 自助餐厅——先来先服务 | 急诊室——危重病人优先 |
| 交通 | 城市道路——红绿灯轮流 | 救护车通道——紧急车辆优先 |
| 银行 | 普通窗口——排队等候 | VIP窗口——优先处理 |
| 核心区别 | 追求公平和吞吐量 | 追赶截止时间 |

**具体场景**：汽车刹车系统——踩下刹车后，必须在10毫秒内响应。通用OS可能因为垃圾回收、页面换入等原因延迟100毫秒，RTOS保证绝不超过10毫秒。

***

### 3. RTOS vs 通用OS

| 维度 | 通用OS（Linux/Windows） | RTOS（FreeRTOS/VxWorks） |
|------|----------------------|------------------------|
| 设计目标 | 吞吐量、公平性 | 实时性、确定性 |
| 调度策略 | 公平调度（CFS等） | 优先级抢占调度 |
| 响应时间 | 不确定（毫秒级~秒级） | 确定（微秒级） |
| 中断延迟 | 不确定 | 有上界保证 |
| 内存管理 | 虚拟内存、分页 | 通常无MMU，物理内存 |
| 任务切换 | 较重（微秒级） | 极轻（亚微秒级） |
| 内核大小 | 数百MB | 数KB~数十KB |
| 适用场景 | 桌面、服务器 | 嵌入式、工业控制 |

#### 3.1 硬实时 vs 软实时

```
硬实时（Hard Real-Time）：
- 错过截止时间 = 灾难性后果
- 例子：汽车刹车、心脏起搏器、核电站控制
- 必须保证100%在截止时间内完成

软实时（Soft Real-Time）：
- 错过截止时间 = 质量下降，但不致命
- 例子：视频播放、网络电话、游戏
- 尽量在截止时间内完成，偶尔超时可接受

| 类型 | 超时后果 | 示例 |
|------|---------|------|
| 硬实时 | 灾难性 | 刹车、飞控、医疗设备 |
| 固实时 | 严重但非致命 | 工业控制、金融交易 |
| 软实时 | 质量下降 | 流媒体、游戏 |
```

***

### 4. FreeRTOS

#### 4.1 核心概念

```cpp
// FreeRTOS基本概念

// 1. 任务（Task）= RTOS中的执行单元
void vTask1(void* pvParameters) {
    for (;;) {
        // 任务逻辑
        gpio_toggle(LED1);
        vTaskDelay(pdMS_TO_TICKS(500));  // 延时500ms
    }
}

void vTask2(void* pvParameters) {
    for (;;) {
        // 读取传感器
        float temp = read_temperature();
        // 发送到队列
        xQueueSend(xTempQueue, &temp, 0);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// 2. 创建任务
int main(void) {
    hardware_init();

    // 创建任务：函数、名称、栈大小、参数、优先级、句柄
    xTaskCreate(vTask1, "Task1", 128, NULL, 1, NULL);
    xTaskCreate(vTask2, "Task2", 256, NULL, 2, NULL);  // 优先级更高

    // 启动调度器
    vTaskStartScheduler();

    // 正常情况不会到这里
    for (;;);
}
```

#### 4.2 任务状态与调度

```
FreeRTOS任务状态转换：

              ┌──────────┐
              │  Running  │ ← 只有一个任务在运行
              └────┬─────┘
         被抢占/   │   \阻塞
         让出CPU   │    \
              ┌───↓───┐  ┌──────────┐
              │ Ready │  │ Blocked  │
              └───┬───┘  └────┬─────┘
                  │           │
                  │ 等待事件   │ 事件发生
                  │           │
                  └───────────┘

调度规则：
1. 最高优先级的就绪任务总是先运行
2. 同优先级任务轮转调度（时间片）
3. 优先级抢占：高优先级任务就绪时立即抢占低优先级任务
```

#### 4.3 同步与通信

```cpp
// 队列（Queue）：任务间通信
QueueHandle_t xTempQueue;

void sender_task(void* pvParameters) {
    float temperature;
    for (;;) {
        temperature = read_temperature();
        // 发送数据到队列，最多等待10ms
        xQueueSend(xTempQueue, &temperature, pdMS_TO_TICKS(10));
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void receiver_task(void* pvParameters) {
    float temperature;
    for (;;) {
        // 从队列接收数据，最多等待100ms
        if (xQueueReceive(xTempQueue, &temperature, pdMS_TO_TICKS(100))) {
            display_temperature(temperature);
        }
    }
}

// 互斥量（Mutex）：保护共享资源
SemaphoreHandle_t xUartMutex;

void task_a(void* pvParameters) {
    for (;;) {
        if (xSemaphoreTake(xUartMutex, pdMS_TO_TICKS(100))) {
            uart_send_string("Task A is running\r\n");
            xSemaphoreGive(xUartMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// 信号量（Semaphore）：同步
SemaphoreHandle_t xDataReadySemaphore;

void adc_task(void* pvParameters) {
    for (;;) {
        start_adc_conversion();
        // 等待ADC完成中断释放信号量
        xSemaphoreTake(xDataReadySemaphore, portMAX_DELAY);
        process_adc_data();
    }
}

// ADC完成中断中释放信号量
void ADC_IRQHandler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xDataReadySemaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
```

***

### 5. FreeRTOS / Zephyr / RT-Thread 对比

| 维度 | FreeRTOS | Zephyr | RT-Thread |
|------|---------|--------|-----------|
| 开发者 | Amazon | Linux基金会 | 中国开源社区 |
| 许可证 | MIT | Apache 2.0 | Apache 2.0 |
| 内核大小 | ~5KB | ~10KB | ~3KB |
| 支持架构 | ARM/RISC-V/Xtensa等 | 10+架构 | ARM/RISC-V等 |
| 设备驱动 | 需要自己写 | 内置丰富驱动 | 内置驱动+软件包 |
| 网络栈 | 第三方（lwIP） | 内置 | 内置 |
| 构建系统 | 手动/IDE | CMake + Kconfig | SCons/ENV |
| 配置方式 | 头文件宏 | Kconfig菜单 | menuconfig |
| 社区 | 最大 | 快速增长 | 中国最大 |
| 学习曲线 | 低 | 中等 | 低 |
| 适用场景 | 简单嵌入式 | 复杂IoT | 国产化/IoT |

#### 5.1 Zephyr示例

```c
// Zephyr：LED闪烁
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define LED0_NODE DT_ALIAS(led0)

int main(void) {
    const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
    gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);

    while (1) {
        gpio_pin_toggle_dt(&led);
        k_msleep(500);  // Zephyr的延时API
    }
    return 0;
}
```

#### 5.2 RT-Thread示例

```c
// RT-Thread：LED闪烁
#include <rtthread.h>
#include <rtdevice.h>

#define LED_PIN    GET_PIN(A, 5)

int main(void) {
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);

    while (1) {
        rt_pin_write(LED_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }
    return 0;
}
```

***

### 6. 何时需要RTOS

#### 6.1 决策树

```
你的项目需要RTOS吗？
│
├── 只有一个循环？ → 裸机（超级循环）就够了
│   while(1) { task1(); task2(); task3(); }
│
├── 有多个并发任务？ → 需要考虑
│   ├── 任务之间需要通信？ → RTOS
│   ├── 有严格的时序要求？ → RTOS
│   └── 任务优先级不同？ → RTOS
│
├── 需要网络协议栈？ → RTOS（内置或集成lwIP）
│
├── 需要文件系统？ → RTOS
│
└── 资源极度受限（<10KB RAM）？ → 裸机或极简调度器
```

#### 6.2 裸机超级循环 vs RTOS

```cpp
// 裸机：超级循环（Super Loop）
int main(void) {
    hardware_init();
    for (;;) {
        task1();   // 读取传感器
        task2();   // 处理数据
        task3();   // 更新显示
        task4();   // 通信
    }
}
// 问题：
// - task3()如果阻塞，task4()就无法执行
// - 无法保证task1()的执行频率
// - 优先级难以实现

// RTOS：多任务调度
int main(void) {
    hardware_init();
    xTaskCreate(sensor_task,  "Sensor",  128, NULL, 3, NULL);  // 高优先级
    xTaskCreate(process_task, "Process", 256, NULL, 2, NULL);
    xTaskCreate(display_task, "Display", 128, NULL, 1, NULL);  // 低优先级
    xTaskCreate(comm_task,    "Comm",    256, NULL, 2, NULL);
    vTaskStartScheduler();
}
// 优势：
// - sensor_task保证按优先级执行
// - display_task阻塞不影响sensor_task
// - 每个任务独立，互不干扰
```

***

### 7. RTOS的实时性保证

#### 7.1 关键时序指标

| 指标 | 含义 | 典型值 |
|------|------|--------|
| 中断延迟 | 从中断发生到ISR开始执行 | <1μs |
| 上下文切换时间 | 从一个任务切换到另一个 | 1-10μs |
| 信号量获取时间 | 从调用到获取 | <1μs |
| 最大中断禁用时间 | 内核禁用中断的最长时间 | 1-10μs |

#### 7.2 优先级反转问题

```cpp
// 优先级反转：低优先级任务持有资源，高优先级任务被阻塞

// 场景：L=低优先级, M=中优先级, H=高优先级
// 1. L获取互斥量，进入临界区
// 2. H就绪，抢占L，尝试获取互斥量 → 被阻塞
// 3. M就绪，抢占L（M不需要互斥量）
// 4. L无法运行，无法释放互斥量
// 5. H被M间接阻塞 → 优先级反转！

// 解决：优先级继承协议
// 当H等待L持有的互斥量时，L的优先级临时提升到H的级别
// 这样L就不会被M抢占，能尽快释放互斥量

// FreeRTOS中创建优先级继承互斥量
SemaphoreHandle_t xMutex = xSemaphoreCreateMutex();  // 默认支持优先级继承
```

***

### 8. 常见误区

| 误区 | 事实 |
|------|------|
| RTOS = 更快的OS | RTOS追求确定性，不是追求最快 |
| RTOS只用于嵌入式 | 也有高性能RTOS（如VxWorks用于航空航天） |
| RTOS不需要考虑优先级反转 | 必须考虑，优先级继承是RTOS的基本功能 |
| FreeRTOS太简单不适合产品 | FreeRTOS运行在数百万设备上，包括亚马逊AWS |
| 用了RTOS就一定实时 | 不当使用（如过多中断禁用）仍可能破坏实时性 |

***

### 9. 总结

| 维度 | 核心要点 |
|------|---------|
| 本质 | 保证任务在确定时间内完成的OS |
| 硬实时 vs 软实时 | 超时后果决定类型 |
| 核心机制 | 优先级抢占调度 + 快速上下文切换 |
| 同步通信 | 队列/互斥量/信号量 |
| 选择依据 | 并发任务数、时序要求、资源约束 |

**核心记忆**：RTOS = 实时性保证 + 优先级抢占 + 快速响应。不是追求最快，而是追求"准时"——每次都在截止时间内完成。