# 什么是模糊测试Fuzzing
> 📖 相关章节：[安全编程概述](../../13-安全编程与逆向工程/00-安全编程概述.md)、[内存安全](../../13-安全编程与逆向工程/01-内存安全与漏洞防御.md)

> **模糊测试就是"疯狂猴子测试"——给程序喂大量随机垃圾数据，看它会不会崩溃。** 人类想不到的边界情况，模糊测试能帮你找到。

***

### 1. 本质速解

**模糊测试（Fuzzing）** 是一种自动化软件测试技术，通过向目标程序输入大量随机或半随机的数据，观察程序是否出现崩溃、内存错误等异常行为。覆盖率引导的模糊测试能自动发现人类难以想到的边界情况，是目前发现安全漏洞最高效的方法之一。

***

### 2. 生活类比

| 类比 | 说明 |
|------|------|
| 模糊测试 = 疯狂按键 | 随机按键盘上的键，看程序会不会崩溃 |
| 模糊测试 = 压力测试 | 不断加大压力，找到断裂点 |
| 覆盖率引导 = 智能探索 | 哪里没去过就去哪里，提高发现率 |
| 种子输入 = 起点地图 | 从已知的正常输入出发，逐步变异 |

**具体场景**：一个PDF阅读器——手动测试你能想出多少种异常PDF？模糊测试可以自动生成数百万种变异PDF，找出导致崩溃的那个。

***

### 3. 模糊测试的原理

#### 3.1 基本流程

```
模糊测试流程：

┌──────────┐     ┌──────────┐     ┌──────────┐
│ 生成输入  │────→│ 执行程序  │────→│ 监控异常  │
│ (变异)   │     │          │     │ (崩溃等)  │
└──────────┘     └──────────┘     └──────────┘
       ↑                                 │
       │         ┌──────────┐           │
       └─────────│ 覆盖率反馈│←──────────┘
                 │ (新路径?) │  保留产生新路径的输入
                 └──────────┘

循环过程：
1. 从种子语料库选择一个输入
2. 对输入进行变异（翻转位、插入字节等）
3. 用变异后的输入运行目标程序
4. 监控是否崩溃或出现异常
5. 如果发现了新的代码路径，保留这个输入
6. 回到步骤1
```

#### 3.2 变异策略

```
常见变异策略：

1. 位翻转（Bit Flip）
   原始: 0x41 → 变异: 0x01（翻转bit 6）

2. 字节替换（Byte Replace）
   原始: "Hello" → 变异: "H\x00llo"

3. 插入/删除（Insert/Delete）
   原始: "ABC" → 变异: "AB123C" 或 "AC"

4. 算术操作（Arithmetic）
   原始: 0x64 → 变异: 0x65 (+1) 或 0x00 (-100)

5. 基于字典的替换
   用已知的高价值字符串替换（如"SELECT"、"<script>"）

6. 块操作
   复制/移动/删除输入中的大块数据

7. 基于结构的变异
   根据输入格式（如PNG/PE）进行智能变异
```

***

### 4. AFL（American Fuzzy Lop）

#### 4.1 AFL工作原理

```
AFL的核心创新：覆盖率引导（Coverage-guided）

传统模糊测试：随机生成输入 → 大部分输入被程序早期拒绝
AFL：保留产生新路径的输入 → 逐步深入程序逻辑

覆盖率追踪：
- 编译时插桩：在每个基本块插入记录代码
- 运行时记录：哪些分支被执行了
- 边覆盖率：不仅记录是否执行，还记录执行次数的区间

分支对（edge）覆盖率：
  A→B 执行1次   → bucket 1
  A→B 执行2次   → bucket 2
  A→B 执行3-4次 → bucket 3
  A→B 执行5-8次 → bucket 4
  ...

如果变异后的输入产生了新的bucket，说明发现了新路径
→ 保留这个输入，继续变异
```

#### 4.2 AFL使用

```bash
# 1. 安装AFL
sudo apt-get install afl++

# 2. 编译目标程序（使用afl-gcc插桩）
afl-gcc -o target target.c

# 或使用afl-clang-fast（更快的插桩）
afl-clang-fast -o target target.c

# 3. 准备种子语料库
mkdir seeds
echo "sample input" > seeds/input1

# 4. 运行AFL
afl-fuzz -i seeds -o output -m none -- ./target @@

# 参数说明：
# -i seeds    种子输入目录
# -o output   输出目录（存放崩溃和语料）
# -m none     不限制内存
# @@          表示输入文件的位置

# 5. 查看结果
ls output/default/crashes/  # 崩溃输入
ls output/default/queue/    # 有趣的输入
```

#### 4.3 AFL界面解读

```
AFL状态界面：

┌──────────────────────────────────────────────┐
│ american fuzzy lop ++3.x                     │
├──────────────────────────────────────────────┤
│ strategy         : exploration               │
│ cycle progress   : 0.1%                      │
│ cycles done      : 0                         │
│ unique crashes   : 3        ← 发现的崩溃数   │
│ unique hangs     : 1        ← 发现的挂起数   │
│ exec speed       : 12500/sec ← 执行速度      │
│ total execs      : 1234567  ← 总执行次数     │
│ edges covered    : 2345/5000 ← 覆盖率        │
│ corpus count     : 567      ← 语料库大小     │
└──────────────────────────────────────────────┘

关键指标：
- unique crashes：不同原因导致的崩溃数
- edges covered：已覆盖的代码边数
- exec speed：每秒执行次数（越高越好）
```

***

### 5. LibFuzzer

#### 5.1 LibFuzzer特点

```
LibFuzzer vs AFL：

| 维度       | AFL              | LibFuzzer        |
|-----------|------------------|------------------|
| 架构      | 进程级           | 函数级（in-process） |
| 输入方式  | 文件             | 内存缓冲区        |
| 启动开销  | 每次fork新进程    | 无需fork          |
| 速度      | 较慢（~1万次/秒） | 极快（~10万次/秒） |
| 集成      | 无需修改代码      | 需要写harness     |
| 适合      | 任意程序          | 库函数            |
| 依赖      | afl-gcc编译      | Clang编译器       |
```

#### 5.2 LibFuzzer使用

```cpp
// LibFuzzer harness示例
#include <stdint.h>
#include <stddef.h>

// 被测试的函数
int parse_header(const uint8_t* data, size_t size) {
    if (size < 4) return -1;
    if (data[0] != 0x89) return -1;  // PNG文件头
    if (data[1] != 'P') return -1;
    if (data[2] != 'N') return -1;
    if (data[3] != 'G') return -1;

    // 解析PNG头部...
    int width = (data[16] << 24) | (data[17] << 16) |
                (data[18] << 8) | data[19];
    if (width > 10000) return -1;  // 宽度检查

    // 更多解析逻辑...
    return 0;
}

// LibFuzzer入口函数
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    parse_header(data, size);
    return 0;
}

// 编译：
// clang++ -fsanitize=fuzzer,address -o fuzzer fuzzer.cpp

// 运行：
// ./fuzzer -max_len=4096 -timeout=10 corpus/
//   -max_len=4096  最大输入长度
//   -timeout=10    单次执行超时（秒）
//   corpus/        种子语料目录
```

#### 5.3 结构感知模糊测试

```cpp
// 问题：纯随机输入很难通过格式检查
// 解决：使用结构感知的模糊测试

// 方法1：自定义变异器（LibFuzzer）
extern "C" size_t LLVMFuzzerCustomMutator(
    uint8_t* data, size_t size,
    size_t max_size, unsigned int seed
) {
    // 自定义变异逻辑，保持输入的结构有效性
    // 例如：只变异PNG的数据块，不破坏文件头
    if (size < 8) return 0;  // 保持最小有效大小

    // 随机选择一个数据块进行变异
    size_t offset = 8 + (seed % (size - 8));
    data[offset] ^= 0xFF;  // 翻转一个字节

    return size;
}

// 方法2：使用protobuf定义输入结构
// 定义输入格式 → 自动生成结构化变异

// 方法3：使用FuzzedDataProvider
#include <fuzzer/FuzzedDataProvider.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    FuzzedDataProvider provider(data, size);

    // 从模糊数据中提取结构化输入
    int command = provider.ConsumeIntegralInRange<int>(0, 5);
    std::string name = provider.ConsumeRandomLengthString(32);
    bool flag = provider.ConsumeBool();

    // 用提取的数据调用目标函数
    process_command(command, name, flag);
    return 0;
}
```

***

### 6. 覆盖率引导

```
覆盖率引导的核心思想：

程序代码（控制流图）：
┌─────────┐
│ 入口     │
└────┬────┘
     │
┌────↓────┐     ┌─────────┐
│ 检查A    │────→│ 路径1    │ ← 大部分随机输入走这里
└────┬────┘     └─────────┘
     │
┌────↓────┐     ┌─────────┐
│ 检查B    │────→│ 路径2    │ ← 需要特定输入才能到这里
└────┬────┘     └─────────┘
     │
┌────↓────┐     ┌─────────┐
│ 检查C    │────→│ 路径3    │ ← 漏洞可能在这里！
└─────────┘     └─────────┘

覆盖率引导的过程：
1. 初始输入走路径1 → 记录覆盖率
2. 变异输入，发现走路径2的输入 → 保留
3. 继续变异，发现走路径3的输入 → 保留
4. 在路径3中发现崩溃 → 漏洞！

没有覆盖率引导：
- 随机输入几乎不可能通过检查A、B、C
- 路径3永远不会被探索
```

***

### 7. 模糊测试的局限

| 局限 | 说明 | 缓解方法 |
|------|------|---------|
| 格式敏感 | 随机输入难以通过格式检查 | 结构感知变异、自定义变异器 |
| 状态依赖 | 需要多步操作才能触发的漏洞 | 状态机模糊测试 |
| 速度瓶颈 | I/O密集型程序速度慢 | in-process模糊测试 |
| 覆盖率盲区 | 某些路径难以到达 | 种子优化、字典辅助 |
| 误报 | 崩溃不一定是安全漏洞 | 人工分析崩溃原因 |
| 不适合逻辑漏洞 | 只能发现内存错误，不能发现逻辑错误 | 结合符号执行 |

***

### 8. 常见误区

| 误区 | 事实 |
|------|------|
| 模糊测试能发现所有bug | 主要发现内存错误，逻辑漏洞很难发现 |
| 模糊测试不需要人工 | 需要编写harness、分析崩溃、优化种子 |
| 模糊测试只适合C/C++ | 任何语言都可以，但C/C++的内存错误最多 |
| 跑得越久越好 | 收益递减，可能需要调整策略 |
| 发现崩溃就是发现漏洞 | 需要分析崩溃是否可利用 |

***

### 9. 总结

| 维度 | 核心要点 |
|------|---------|
| 本质 | 自动化输入随机数据，观察程序异常 |
| 覆盖率引导 | 保留产生新路径的输入，逐步深入 |
| AFL | 进程级模糊测试，适用范围广 |
| LibFuzzer | 函数级模糊测试，速度极快 |
| 局限 | 格式敏感、状态依赖、不适合逻辑漏洞 |

**核心记忆**：模糊测试 = 随机输入 + 覆盖率引导 + 崩溃检测。AFL适合完整程序，LibFuzzer适合库函数。覆盖率引导是核心创新——保留产生新路径的输入，让模糊测试"越来越聪明"。