# 什么是模糊测试Fuzzing
> 📖 相关章节：[调试技巧](../../04-工程实践/06-调试技巧.md)、[单元测试](../../04-工程实践/05-单元测试.md)

> "Fuzzing 是用随机数据暴力破解程序防御的自动化艺术。" —— Michal Zalewski

***

### 1. 精髓速览

模糊测试（Fuzzing）是一种自动化软件测试技术，通过向目标程序注入大量随机或半随机的异常输入，观察程序是否崩溃或产生未定义行为，从而发现内存安全漏洞和逻辑缺陷。

***

### 2. 模糊测试的基本原理

模糊测试的核心思想非常简单：**生成畸形输入 → 喂给目标程序 → 监控异常行为**。

```
┌─────────────┐     ┌──────────────┐     ┌──────────────┐
│  输入生成器   │────▶│   目标程序    │────▶│  异常监控器   │
│ (Fuzzer)    │     │ (Fuzz Target) │     │ (Sanitizer)  │
└─────────────┘     └──────────────┘     └──────────────┘
       ▲                                        │
       └──────────── 反馈引导 ───────────────────┘
```

基本流程：

1. **种子选择**：从初始语料库（corpus）中选取输入
2. **输入变异**：对种子进行位翻转、字节插入、边界值替换等操作
3. **执行目标**：将变异后的输入传递给被测函数
4. **监控反馈**：检测崩溃、断言失败、内存越界等异常
5. **覆盖率反馈**：记录新覆盖的代码路径，保留有效变异

```cpp
#include <cstdint>
#include <cstddef>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 4) return 0;

    uint32_t value = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);

    if (value == 0xDEADBEEF) {
        volatile int *p = nullptr;
        *p = 42;
    }

    return 0;
}
```

***

### 3. 变异型 vs 生成型 Fuzzer

| 维度 | 变异型（Mutation-based） | 生成型（Generation-based） |
|------|------------------------|--------------------------|
| **原理** | 对已有种子进行随机变异 | 从零开始按规则/语法生成输入 |
| **输入质量** | 可能产生大量无效输入 | 输入通常符合目标格式规范 |
| **适用场景** | 通用场景，无需了解协议格式 | 需要特定格式（如 XML、SQL、协议） |
| **典型工具** | AFL、libFuzzer、Honggfuzz | Peach Fuzzer、Sulley、Boofuzz |
| **覆盖率效率** | 依赖种子质量，初期较慢 | 能快速覆盖深层逻辑 |
| **实现复杂度** | 低，开箱即用 | 高，需要编写协议模型 |

变异型 Fuzzer 常见的变异策略：

```cpp
#include <cstdint>
#include <cstring>
#include <cstdlib>

void mutate_flip_bit(uint8_t *data, size_t size) {
    size_t byte_idx = rand() % size;
    size_t bit_idx = rand() % 8;
    data[byte_idx] ^= (1 << bit_idx);
}

void mutate_insert_byte(uint8_t *data, size_t *size, size_t max_size) {
    if (*size >= max_size) return;
    size_t pos = rand() % (*size + 1);
    memmove(data + pos + 1, data + pos, *size - pos);
    data[pos] = static_cast<uint8_t>(rand() % 256);
    (*size)++;
}

void mutate_replace_with_interesting(uint8_t *data, size_t size) {
    static const uint8_t interesting[] = {0, 1, 0xFF, 0x7F, 0x80, 0xDE, 0xAD};
    size_t pos = rand() % size;
    data[pos] = interesting[rand() % (sizeof(interesting) / sizeof(interesting[0]))];
}
```

生成型 Fuzzer 的典型模式（伪代码）：

```cpp
#include <string>
#include <random>

std::string generate_http_request() {
    std::mt19937 rng(42);
    std::string methods[] = {"GET", "POST", "PUT", "DELETE"};
    std::string paths[] = {"/", "/api/v1", "/login", "/upload"};

    std::string req = methods[rng() % 4] + " " + paths[rng() % 4] + " HTTP/1.1\r\n";
    req += "Content-Length: " + std::to_string(rng() % 1024) + "\r\n";
    req += "\r\n";
    return req;
}
```

***

### 4. 覆盖率引导（Coverage-Guided）Fuzzing

覆盖率引导是现代 Fuzzer 最关键的技术。它通过追踪代码覆盖率来指导变异方向，使测试资源集中在未覆盖的代码路径上。

```
种子A ──变异──▶ 输入X ──执行──▶ 覆盖路径 {1,3,5} ──已见过──▶ 丢弃
种子B ──变异──▶ 输入Y ──执行──▶ 覆盖路径 {1,3,7} ──新路径──▶ 加入语料库
```

覆盖率类型对比：

| 覆盖率类型 | 粒度 | 开销 | 效果 |
|-----------|------|------|------|
| 边覆盖（Edge） | 边转移 | 中 | 最佳，AFL 默认 |
| 基本块覆盖（Block） | 基本块 | 低 | 良好 |
| 路径覆盖（Path） | 完整路径 | 高 | 理论最优，实际不可行 |
| 分支条件覆盖 | 条件值 | 高 | 适合高安全需求 |

AFL 的边覆盖率插桩原理：

```cpp
// AFL 编译器插桩后的伪代码
// 在每个基本块入口插入
static unsigned int __afl_area_ptr[MAP_SIZE];

void afl_branch_trace(unsigned int from_id, unsigned int to_id) {
    unsigned int index = (from_id << 1) ^ to_id;
    __afl_area_ptr[index % MAP_SIZE]++;
}
```

libFuzzer 使用 SanitizerCoverage：

```cpp
// 编译时自动插桩，无需手动编写
// 编译命令：
// clang++ -fsanitize=fuzzer,address -g fuzz_target.cpp -o fuzz_target

#include <cstdint>
#include <cstddef>
#include <cstring>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 2) return 0;

    uint16_t cmd;
    memcpy(&cmd, data, 2);

    switch (cmd % 4) {
    case 0:
        if (size > 4 && data[2] == 'A' && data[3] == 'B') {
            volatile int sink = data[4];
        }
        break;
    case 1:
        if (size > 10 && memcmp(data + 2, "HELLO", 5) == 0) {
            volatile int sink = data[7];
        }
        break;
    case 2:
        break;
    case 3:
        break;
    }

    return 0;
}
```

***

### 5. libFuzzer 实战详解

libFuzzer 是 LLVM/Clang 内置的覆盖率引导 Fuzzer，与 AddressSanitizer 深度集成，是 C/C++ 项目首选的 Fuzzing 工具。

**核心 API**：

```cpp
#include <cstdint>
#include <cstddef>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    return 0;
}
```

**完整示例：Fuzz 一个 JSON 解析器**：

```cpp
#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>

class SimpleJsonParser {
public:
    bool parse(std::string_view input) {
        pos_ = 0;
        input_ = input;
        skipWhitespace();
        return parseValue() && pos_ == input_.size();
    }

private:
    size_t pos_ = 0;
    std::string_view input_;

    void skipWhitespace() {
        while (pos_ < input_.size() && (input_[pos_] == ' ' || input_[pos_] == '\n' ||
               input_[pos_] == '\r' || input_[pos_] == '\t')) {
            pos_++;
        }
    }

    bool parseValue() {
        if (pos_ >= input_.size()) return false;
        char c = input_[pos_];
        if (c == '"') return parseString();
        if (c == '{') return parseObject();
        if (c == '[') return parseArray();
        if (c == 't' || c == 'f') return parseBool();
        if (c == 'n') return parseNull();
        if (c == '-' || (c >= '0' && c <= '9')) return parseNumber();
        return false;
    }

    bool parseString() {
        if (pos_ >= input_.size() || input_[pos_] != '"') return false;
        pos_++;
        while (pos_ < input_.size() && input_[pos_] != '"') {
            if (input_[pos_] == '\\') pos_++;
            pos_++;
        }
        if (pos_ >= input_.size()) return false;
        pos_++;
        return true;
    }

    bool parseObject() {
        if (input_[pos_] != '{') return false;
        pos_++; skipWhitespace();
        if (pos_ < input_.size() && input_[pos_] == '}') { pos_++; return true; }
        while (pos_ < input_.size()) {
            skipWhitespace();
            if (!parseString()) return false;
            skipWhitespace();
            if (pos_ >= input_.size() || input_[pos_] != ':') return false;
            pos_++; skipWhitespace();
            if (!parseValue()) return false;
            skipWhitespace();
            if (pos_ < input_.size() && input_[pos_] == ',') { pos_++; continue; }
            break;
        }
        if (pos_ >= input_.size() || input_[pos_] != '}') return false;
        pos_++;
        return true;
    }

    bool parseArray() {
        if (input_[pos_] != '[') return false;
        pos_++; skipWhitespace();
        if (pos_ < input_.size() && input_[pos_] == ']') { pos_++; return true; }
        while (pos_ < input_.size()) {
            skipWhitespace();
            if (!parseValue()) return false;
            skipWhitespace();
            if (pos_ < input_.size() && input_[pos_] == ',') { pos_++; continue; }
            break;
        }
        if (pos_ >= input_.size() || input_[pos_] != ']') return false;
        pos_++;
        return true;
    }

    bool parseBool() {
        if (input_.substr(pos_).starts_with("true")) { pos_ += 4; return true; }
        if (input_.substr(pos_).starts_with("false")) { pos_ += 5; return true; }
        return false;
    }

    bool parseNull() {
        if (input_.substr(pos_).starts_with("null")) { pos_ += 4; return true; }
        return false;
    }

    bool parseNumber() {
        if (pos_ < input_.size() && input_[pos_] == '-') pos_++;
        while (pos_ < input_.size() && input_[pos_] >= '0' && input_[pos_] <= '9') pos_++;
        return true;
    }
};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    SimpleJsonParser parser;
    parser.parse(std::string_view(reinterpret_cast<const char *>(data), size));
    return 0;
}
```

**编译与运行**：

```bash
# 编译（需要 Clang）
clang++ -fsanitize=fuzzer,address -g -O1 fuzz_json.cpp -o fuzz_json

# 运行（默认无限循环）
./fuzz_json

# 指定语料库目录和运行时间
./fuzz_json corpus/ -max_total_time=3600 -jobs=4 -workers=4

# 最小化崩溃输入
./fuzz_json crash-abc -minimize_crash=1
```

| libFuzzer 常用选项 | 说明 |
|-------------------|------|
| `-max_len=N` | 限制输入最大长度 |
| `-max_total_time=N` | 总运行时间（秒） |
| `-runs=N` | 执行次数 |
| `-jobs=N` | 并行任务数 |
| `-workers=N` | 并行工作进程数 |
| `-corpus=DIR` | 语料库目录 |
| `-dict=FILE` | 字典文件 |
| `-only_ascii` | 仅生成 ASCII 输入 |
| `-minimize_crash=1` | 最小化崩溃输入 |

***

### 6. AFL 实战详解

AFL（American Fuzzy Lop）是最经典的覆盖率引导 Fuzzer，通过编译时插桩或 QEMU 模式实现覆盖率追踪。

**AFL vs libFuzzer 对比**：

| 维度 | AFL | libFuzzer |
|------|-----|-----------|
| **架构** | 独立进程，fork-server | 进程内循环 |
| **插桩方式** | afl-gcc/afl-clang | -fsanitize=fuzzer |
| **输入方式** | 文件/标准输入 | 内存回调函数 |
| **启动开销** | 每次输入 fork 一次 | 无 fork，更快 |
| **适用场景** | 文件解析、命令行工具 | 库函数、API |
| **并行支持** | 原生多实例 | -jobs/-workers |
| **稳定性** | 非常成熟稳定 | LLVM 生态集成 |

**AFL 编译与运行**：

```bash
# 编译插桩版本
afl-clang-fast++ -fsanitize=address -g -O1 target.cpp -o target_fuzz

# 创建种子语料库
mkdir -p corpus/
echo "sample input" > corpus/seed1

# 运行 AFL
afl-fuzz -i corpus/ -o findings/ -- ./target_fuzz @@

# QEMU 模式（无需源码）
afl-fuzz -i corpus/ -o findings/ -Q -- ./binary_target @@
```

**AFL Fuzz Target 示例**：

```cpp
#include <cstdio>
#include <cstdlib>
#include <cstring>

void vulnerable_parser(const char *input, size_t len) {
    if (len < 4) return;

    uint32_t magic;
    memcpy(&magic, input, 4);

    if (magic == 0x50414745) {
        if (len < 8) return;
        uint16_t count;
        memcpy(&count, input + 4, 2);
        if (count > 100) return;

        char *buffer = new char[count];
        size_t copy_len = len - 6;
        if (copy_len > count) copy_len = count;
        memcpy(buffer, input + 6, copy_len);
        delete[] buffer;
    }
}

int main(int argc, char **argv) {
    FILE *fp = fopen(argv[1], "rb");
    if (!fp) return 1;

    fseek(fp, 0, SEEK_END);
    size_t len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *data = new char[len];
    fread(data, 1, len, fp);
    fclose(fp);

    vulnerable_parser(data, len);
    delete[] data;
    return 0;
}
```

***

### 7. Fuzz Target 设计与 API 契约

Fuzz Target 是 Fuzzer 调用的入口函数，其设计质量直接影响 Fuzzing 效果。

**设计原则**：

1. **接口简洁**：只接收原始字节，内部自行解析
2. **确定性**：相同输入必须产生相同行为
3. **无副作用**：不修改全局状态，不依赖外部 I/O
4. **快速执行**：避免耗时操作，Fuzzer 每秒需执行数千次
5. **内存安全**：配合 Sanitizer 使用

```cpp
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <vector>

class PacketParser {
public:
    bool parse(const uint8_t *data, size_t size) {
        if (size < HEADER_SIZE) return false;

        uint8_t version = data[0] >> 4;
        if (version != 1 && version != 2) return false;

        uint16_t payload_len;
        memcpy(&payload_len, data + 2, 2);

        if (static_cast<size_t>(HEADER_SIZE) + payload_len > size) return false;

        return parsePayload(data + HEADER_SIZE, payload_len, version);
    }

private:
    static constexpr size_t HEADER_SIZE = 8;

    bool parsePayload(const uint8_t *payload, size_t len, uint8_t version) {
        if (version == 1) {
            return parseV1(payload, len);
        }
        return parseV2(payload, len);
    }

    bool parseV1(const uint8_t *payload, size_t len) {
        size_t pos = 0;
        while (pos + 4 <= len) {
            uint8_t type = payload[pos];
            uint16_t field_len;
            memcpy(&field_len, payload + pos + 1, 2);
            if (pos + 4 + field_len > len) return false;
            pos += 4 + field_len;
        }
        return true;
    }

    bool parseV2(const uint8_t *payload, size_t len) {
        size_t pos = 0;
        while (pos + 2 <= len) {
            uint8_t type = payload[pos];
            uint8_t field_len = payload[pos + 1];
            if (pos + 2 + field_len > len) return false;
            pos += 2 + field_len;
        }
        return true;
    }
};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    PacketParser parser;
    parser.parse(data, size);
    return 0;
}
```

**API 契约验证**：

```cpp
#include <cstdint>
#include <cstddef>
#include <cassert>
#include <cstring>

class StackBuffer {
    static constexpr size_t CAP = 256;
    char buf_[CAP];
    size_t used_ = 0;

public:
    bool push(const char *data, size_t len) {
        if (used_ + len > CAP) return false;
        memcpy(buf_ + used_, data, len);
        used_ += len;
        return true;
    }

    const char *data() const { return buf_; }
    size_t size() const { return used_; }
};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    StackBuffer buf;
    size_t pos = 0;

    while (pos < size) {
        uint8_t chunk_len = data[pos] % 64;
        pos++;
        if (pos + chunk_len > size) break;

        bool ok = buf.push(reinterpret_cast<const char *>(data + pos), chunk_len);
        assert(buf.size() <= StackBuffer::CAP);
        pos += chunk_len;
    }

    return 0;
}
```

***

### 8. 内存安全与 Sanitizer 配合

Fuzzing 的核心目标是发现内存安全漏洞。配合 Sanitizer 使用可以大幅提高漏洞检出率。

| Sanitizer | 检测能力 | 性能开销 | 适用阶段 |
|-----------|---------|---------|---------|
| AddressSanitizer (ASan) | 越界读写、UAF、双重释放 | 2x | 开发/CI |
| MemorySanitizer (MSan) | 未初始化读取 | 3x | 开发 |
| UndefinedBehaviorSanitizer (UBSan) | 未定义行为 | 1.2x | 开发/CI |
| ThreadSanitizer (TSan) | 数据竞争 | 5-10x | 开发 |

```cpp
#include <cstdint>
#include <cstddef>
#include <vector>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 8) return 0;

    size_t alloc_size = 0;
    for (size_t i = 0; i < 8 && i < size; i++) {
        alloc_size = alloc_size * 31 + data[i];
    }
    alloc_size = alloc_size % 1024 + 1;

    std::vector<uint8_t> buffer(alloc_size);

    size_t copy_len = size - 8;
    if (copy_len > alloc_size) copy_len = alloc_size;

    for (size_t i = 0; i < copy_len; i++) {
        buffer[i] = data[8 + i];
    }

    return 0;
}
```

**编译命令组合**：

```bash
# ASan + Fuzzer（最常用）
clang++ -fsanitize=fuzzer,address -g -O1 target.cpp

# MSan + Fuzzer（检测未初始化读取）
clang++ -fsanitize=fuzzer,memory -g -O1 target.cpp

# UBSan + Fuzzer（检测未定义行为）
clang++ -fsanitize=fuzzer,undefined -g -O1 target.cpp

# ASan + UBSan + Fuzzer（组合使用）
clang++ -fsanitize=fuzzer,address,undefined -g -O1 target.cpp
```

> ⚠️ **平台注意**：MSan 不能与 ASan 同时使用。Windows 上 MSan 支持有限，建议在 Linux 上运行 MSan 构建。

***

### 9. 语料库（Corpus）管理

语料库是 Fuzzing 的起点，其质量直接影响覆盖率增长速度。

**语料库策略**：

| 策略 | 说明 | 适用场景 |
|------|------|---------|
| 空语料库 | 从零开始探索 | 简单格式 |
| 最小语料库 | 几个典型输入 | 通用 |
| 格式规范语料库 | 符合协议的完整输入 | 复杂协议 |
| 回归语料库 | 历史崩溃输入 | 持续集成 |
| 合并语料库 | 多轮 Fuzzing 结果合并 | 长期项目 |

```bash
# 最小化语料库（去除冗余输入）
./fuzz_target corpus/ -merge=1 minimized_corpus/

# 生成语料库描述
./fuzz_target corpus/ -show_coverage=1

# 从真实数据创建语料库
mkdir -p corpus/
for f in real_data/*.json; do
    cp "$f" corpus/
done
```

**字典文件**（提高变异效率）：

```
# json.dict
"true"
"false"
"null"
"\"\""
"{}"
"[]"
"0"
"-1"
"1e10"
"\\u0000"
```

```bash
# 使用字典
./fuzz_target corpus/ -dict=json.dict
```

***

### 10. Fuzzing vs 单元测试

| 维度 | 单元测试 | 模糊测试 |
|------|---------|---------|
| **输入来源** | 手工编写 | 自动生成 |
| **覆盖范围** | 已知路径 | 未知路径 |
| **发现缺陷类型** | 逻辑错误 | 内存安全、边界条件 |
| **维护成本** | 高，需持续更新 | 低，自动演化 |
| **执行速度** | 快 | 慢（需长时间运行） |
| **确定性** | 100% 可复现 | 崩溃可复现，过程随机 |
| **适用阶段** | 功能验证 | 安全审计、边界探索 |

**最佳实践：两者互补**：

```cpp
#include <cstdint>
#include <cstddef>
#include <cassert>
#include <string_view>

int parse_header(std::string_view input) {
    if (input.size() < 4) return -1;
    if (input[0] != 'H' || input[1] != 'D') return -1;
    uint16_t len = static_cast<uint8_t>(input[2]) |
                   (static_cast<uint8_t>(input[3]) << 8);
    if (len > 4096) return -1;
    return len;
}

#ifdef UNIT_TEST
#include <gtest/gtest.h>

TEST(ParseHeader, ValidInput) {
    std::string input = "HD\x05\x00";
    EXPECT_EQ(parse_header(input), 5);
}

TEST(ParseHeader, TooShort) {
    EXPECT_EQ(parse_header("HD"), -1);
}

TEST(ParseHeader, BadMagic) {
    std::string input = "XX\x05\x00";
    EXPECT_EQ(parse_header(input), -1);
}

#else

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    parse_header(std::string_view(reinterpret_cast<const char *>(data), size));
    return 0;
}

#endif
```

***

### 11. OSS-Fuzz 与持续 Fuzzing

OSS-Fuzz 是 Google 提供的免费持续 Fuzzing 服务，为开源项目提供自动化的漏洞发现。

**OSS-Fuzz 工作流**：

```
项目提交配置 ──▶ Google CI 自动构建 ──▶ 集群 Fuzzing ──▶ 发现漏洞 ──▶ 通知维护者
```

**项目配置示例**（project.yaml + Dockerfile + build.sh）：

```yaml
# project.yaml
homepage: https://github.com/user/project
primary_contact: maintainer@example.com
builds_per_day: 1
sanitizers:
  - address
  - undefined
fuzzing_engines:
  - libfuzzer
```

```bash
#!/bin/bash
# build.sh
./configure --enable-fuzzing --with-sanitizers
make -j$(nproc)

for fuzzer in $(find . -name '*_fuzz_target'); do
    cp "$fuzzer" $OUT/
done
```

**本地模拟 OSS-Fuzz 环境**：

```bash
# 克隆 oss-fuzz
git clone https://github.com/google/oss-fuzz

# 构建项目
cd oss-fuzz
python infra/helper.py build_image my-project
python infra/helper.py build_fuzzers my-project

# 运行 Fuzzer
python infra/helper.py run_fuzzer my-project my_fuzz_target
```

| OSS-Fuzz 特性 | 说明 |
|--------------|------|
| **自动集成** | 支持 CMake、Makefile、Autotools |
| **多 Sanitizer** | ASan、MSan、UBSan 自动切换 |
| **集群运行** | Google 基础设施，7×24 运行 |
| **漏洞披露** | 90 天披露窗口，私有报告 |
| **覆盖率报告** | 自动生成覆盖率仪表盘 |
| **Introspector** | 静态分析辅助 Fuzz 目标发现 |

> ⚠️ **平台注意**：OSS-Fuzz 仅支持 Linux 环境。Windows 项目需确保在 Linux 下可编译运行，或使用 WSL。

***

### 12. 极简总结

| 概念 | 要点 |
|------|------|
| **Fuzzing 本质** | 自动生成异常输入，监控程序异常行为 |
| **变异型** | 基于种子变异，通用性强（AFL、libFuzzer） |
| **生成型** | 按语法生成，适合复杂协议（Peach、Boofuzz） |
| **覆盖率引导** | 追踪代码覆盖，保留触发新路径的输入 |
| **libFuzzer** | LLVM 内置，进程内循环，适合库函数测试 |
| **AFL** | fork-server 架构，适合文件/命令行工具 |
| **Fuzz Target** | 入口函数，需确定性、无副作用、快速执行 |
| **Sanitizer** | ASan 检测内存越界，MSan 检测未初始化，UBSan 检测未定义行为 |
| **语料库** | 种子输入集合，质量决定覆盖率增长速度 |
| **vs 单元测试** | 互补关系：单元测试验证已知逻辑，Fuzzing 探索未知路径 |
| **OSS-Fuzz** | Google 免费持续 Fuzzing 服务，开源项目可申请 |

**关键记忆**：
- Fuzzing 三要素：**输入生成 + 目标执行 + 异常监控**
- 覆盖率引导是现代 Fuzzer 的核心引擎
- 始终配合 Sanitizer 使用，否则大部分内存漏洞无法被检测
- 语料库质量 > Fuzzing 时长
- Fuzzing 不能替代单元测试，两者互补

***

### 相关阅读

- [什么是基准测试Benchmarking](./10-什么是基准测试Benchmarking.md)
- [什么是性能剖析Profiling](09-什么是性能剖析Profiling.md)
- [CPP工具链](03-CPP工具链.md)