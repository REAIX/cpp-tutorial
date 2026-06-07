# CI/CD 与 DevOps 实践
> 📖 相关章节：[编码规范](../../04-工程实践/00-编码规范.md)、[设计模式](../../04-工程实践/03-设计模式.md)、[单元测试](../../04-工程实践/05-单元测试.md)、[代码审查](../../04-工程实践/07-代码审查.md)

## 1. 什么是 DevOps

### 1. 定义

DevOps = **Dev**elopment + **Op**s，即开发与运维的深度融合。它不是一种工具或技术，而是一种强调开发团队和运维团队协作的文化理念与实践方法。

### 2. 比喻理解

```
传统模式：
┌──────────┐     扔过去     ┌──────────┐
│  开发团队  │ ──────────→  │  运维团队  │
│  "写完了"  │              │  "又挂了"  │
└──────────┘               └──────────┘
   甩锅分离，互相推诿

DevOps 模式：
┌────────────────────────────────┐
│         开发 + 运维一体化        │
│  共同负责：开发 → 测试 → 部署    │
│  共同目标：快速、稳定地交付价值   │
└────────────────────────────────┘
   一家人，共同进退
```

### 3. DevOps 文化核心

| 原则       | 说明                                   |
| ---------- | -------------------------------------- |
| 协作       | 打破部门墙，开发与运维共享目标和责任   |
| 自动化     | 能自动的不手动，减少人为错误           |
| 持续反馈   | 快速获取用户和生产环境的反馈           |
| 快速交付   | 小步快跑，频繁发布，降低每次发布风险   |

---

## 2. 什么是 CI（持续集成）

### 1. 定义

持续集成（Continuous Integration）要求开发者**频繁地**（通常每天多次）将代码合并到主分支，每次合并都会触发**自动构建**和**自动测试**，尽早发现集成问题。

### 2. 比喻理解

```
没有 CI：
┌──────────────────────────────────────┐
│  一年体检一次 → 发现问题已经是晚期    │
│  三个月集成一次 → 集成地狱            │
└──────────────────────────────────────┘

有 CI：
┌──────────────────────────────────────┐
│  每天检查身体 → 小病早治              │
│  每次提交都构建测试 → 问题即时暴露    │
└──────────────────────────────────────┘
```

### 3. CI 流程图

```
  开发者                CI 服务器                   通知
┌────────┐         ┌──────────────┐           ┌──────────┐
│  git   │  push   │  自动构建     │  成功/失败 │  邮件     │
│ commit │───────→ │  自动测试     │─────────→│  Slack   │
│  push  │         │  代码检查     │           │  飞书     │
└────────┘         └──────────────┘           └──────────┘
                        │
                   ┌────┴────┐
                   │ 通过/失败 │
                   └────┬────┘
                        │
              ┌─────────┴─────────┐
              ↓                   ↓
         构建产物可部署       阻止合并/告警
```

### 4. 常见 CI 工具

| 工具             | 特点                       | 适用场景               |
| ---------------- | -------------------------- | ---------------------- |
| GitHub Actions   | 与 GitHub 深度集成，免费额度大 | 开源项目、GitHub 托管  |
| GitLab CI        | 与 GitLab 一体化，功能强大  | 自建 GitLab、企业项目  |
| Jenkins          | 插件生态丰富，高度可定制    | 复杂流水线、企业级     |
| CircleCI         | 速度快，配置简单            | 中小团队、快速上手     |

---

## 3. 什么是 CD（持续交付/持续部署）

### 1. 持续交付 vs 持续部署

```
持续交付（Continuous Delivery）：
代码提交 → 自动构建 → 自动测试 → 自动到"可部署状态" → 【手动点击】→ 生产环境
                                                    ↑
                                              需要人工审批

持续部署（Continuous Deployment）：
代码提交 → 自动构建 → 自动测试 → 自动预发布 → 自动部署 → 生产环境
                                                    ↑
                                              全自动，无需人工
```

### 2. CD 流程图

```
┌────────┐   ┌──────┐   ┌──────┐   ┌────────┐   ┌────────┐   ┌──────────┐
│ 代码    │   │ 构建  │   │ 测试  │   │ 预发布  │   │ 审批    │   │ 生产环境  │
│ 提交    │──→│ 阶段  │──→│ 阶段  │──→│ 环境    │──→│ (可选)  │──→│ 部署     │
└────────┘   └──────┘   └──────┘   └────────┘   └────────┘   └──────────┘
   自动        自动        自动        自动      持续交付=手动   持续部署=自动
```

### 3. 对比

| 维度         | 持续交付             | 持续部署               |
| ------------ | -------------------- | ---------------------- |
| 部署触发     | 手动审批后触发       | 全自动                 |
| 风险控制     | 有人工把关           | 依赖自动化测试覆盖     |
| 适用场景     | 对稳定性要求极高     | 测试覆盖充分的项目     |
| 发布频率     | 较高（按需）         | 极高（每次提交）       |

---

## 4. GitHub Actions 实战

### 1. 目录结构

```
项目根目录/
├── .github/
│   └── workflows/
│       ├── ci.yml          # 持续集成
│       ├── release.yml     # 发布流程
│       └── coverage.yml    # 覆盖率报告
├── src/
├── tests/
├── CMakeLists.txt
└── README.md
```

### 2. YAML 配置文件详解

```yaml
name: CI                    # workflow 名称
on:                         # 触发条件
  push:
    branches: [main]
  pull_request:
    branches: [main]

jobs:                       # 任务列表
  build:                    # 任务名称
    runs-on: ubuntu-latest  # 运行环境
    steps:                  # 步骤列表
      - uses: actions/checkout@v4    # 检出代码
      - name: 安装依赖
        run: sudo apt-get install -y cmake
      - name: 构建
        run: |
          cmake -B build
          cmake --build build
```

### 3. C/C++ 项目的 CI 配置示例

```yaml
name: C++ CI

on:
  push:
    branches: [main, develop]
  pull_request:
    branches: [main]

jobs:
  build-and-test:
    strategy:
      matrix:
        os: [ubuntu-latest, windows-latest, macos-latest]
        compiler: [gcc, clang, msvc]
        exclude:
          - os: windows-latest
            compiler: clang
          - os: ubuntu-latest
            compiler: msvc
        include:
          - compiler: gcc
            cc: gcc
            cxx: g++
          - compiler: clang
            cc: clang
            cxx: clang++
          - compiler: msvc
            cc: cl
            cxx: cl

    runs-on: ${{ matrix.os }}

    steps:
      - uses: actions/checkout@v4

      - name: 安装依赖 (Linux)
        if: runner.os == 'Linux'
        run: |
          sudo apt-get update
          sudo apt-get install -y cmake lcov

      - name: 配置 CMake
        run: |
          cmake -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTING=ON
        env:
          CC: ${{ matrix.cc }}
          CXX: ${{ matrix.cxx }}

      - name: 构建
        run: cmake --build build --config Debug --parallel

      - name: 运行测试
        working-directory: build
        run: ctest --output-on-failure --config Debug

      - name: 生成覆盖率报告
        if: matrix.os == 'ubuntu-latest' && matrix.compiler == 'gcc'
        run: |
          lcov --capture --directory build --output-file coverage.info
          lcov --remove coverage.info '/usr/*' --output-file coverage.info
          lcov --list coverage.info

      - name: 上传覆盖率到 Codecov
        if: matrix.os == 'ubuntu-latest' && matrix.compiler == 'gcc'
        uses: codecov/codecov-action@v4
        with:
          files: coverage.info
          token: ${{ secrets.CODECOV_TOKEN }}
```

---

## 5. CMake 项目的 CI/CD

### 1. CMakePresets.json 与 CI 集成

```json
{
  "version": 6,
  "configurePresets": [
    {
      "name": "ci-linux",
      "binaryDir": "${sourceDir}/build",
      "generator": "Ninja",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "CMAKE_C_COMPILER": "gcc",
        "CMAKE_CXX_COMPILER": "g++",
        "ENABLE_TESTING": "ON",
        "ENABLE_COVERAGE": "ON"
      }
    },
    {
      "name": "ci-windows",
      "binaryDir": "${sourceDir}/build",
      "generator": "Visual Studio 17 2022",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "ENABLE_TESTING": "ON"
      }
    },
    {
      "name": "ci-sanitizer",
      "binaryDir": "${sourceDir}/build",
      "generator": "Ninja",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "CMAKE_CXX_FLAGS": "-fsanitize=address,undefined -fno-omit-frame-pointer",
        "ENABLE_TESTING": "ON"
      }
    }
  ],
  "buildPresets": [
    {
      "name": "ci-linux",
      "configurePreset": "ci-linux"
    },
    {
      "name": "ci-windows",
      "configurePreset": "ci-windows"
    }
  ],
  "testPresets": [
    {
      "name": "ci-linux",
      "configurePreset": "ci-linux",
      "output": {
        "outputOnFailure": true
      }
    }
  ]
}
```

CI 中使用预设：

```yaml
- name: 配置
  run: cmake --preset ci-linux
- name: 构建
  run: cmake --build --preset ci-linux
- name: 测试
  run: ctest --preset ci-linux
```

### 2. 多编译器测试

| 编译器 | 平台        | CI 配置要点                      |
| ------ | ----------- | -------------------------------- |
| GCC    | Linux       | `apt install gcc g++`            |
| Clang  | Linux/macOS | `apt install clang` / Xcode 自带 |
| MSVC   | Windows     | 使用 `Visual Studio` 生成器      |

### 3. Sanitizer 在 CI 中的使用

```yaml
  sanitizer:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: 安装依赖
        run: sudo apt-get install -y cmake ninja-build clang

      - name: 配置 (Address Sanitizer)
        run: |
          cmake -B build -G Ninja \
            -DCMAKE_C_COMPILER=clang \
            -DCMAKE_CXX_COMPILER=clang++ \
            -DCMAKE_BUILD_TYPE=Debug \
            -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" \
            -DENABLE_TESTING=ON

      - name: 构建
        run: cmake --build build

      - name: 测试
        working-directory: build
        env:
          LSAN_OPTIONS: "detect_leaks=1"
        run: ctest --output-on-failure
```

常用 Sanitizer：

| Sanitizer            | 检测内容           | 编译选项                    |
| -------------------- | ------------------ | --------------------------- |
| AddressSanitizer     | 内存越界、UAF      | `-fsanitize=address`        |
| UndefinedBehaviorSan | 未定义行为         | `-fsanitize=undefined`      |
| ThreadSanitizer      | 数据竞争           | `-fsanitize=thread`         |
| MemorySanitizer      | 未初始化内存读取   | `-fsanitize=memory`         |

### 4. 代码覆盖率

```yaml
  coverage:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: 安装依赖
        run: sudo apt-get install -y cmake lcov

      - name: 配置
        run: cmake -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON

      - name: 构建
        run: cmake --build build

      - name: 运行测试
        working-directory: build
        run: ctest --output-on-failure

      - name: 生成覆盖率
        run: |
          lcov --capture --directory build --output-file coverage.info
          lcov --remove coverage.info '/usr/*' '*/test/*' --output-file coverage.info

      - name: 上传到 Codecov
        uses: codecov/codecov-action@v4
        with:
          files: coverage.info
```

---

## 6. Docker 与 C/C++ 项目

### 1. 为什么 C/C++ 需要 Docker

```
没有 Docker：
┌─────────────────────────────────────────────┐
│  开发者A: Ubuntu 22.04 + GCC 12 → 编译通过  │
│  开发者B: CentOS 7 + GCC 7   → 编译失败     │
│  CI 服务器: Ubuntu 20.04     → 链接错误      │
│  生产环境: Debian 11         → 运行时缺库    │
└─────────────────────────────────────────────┘
  "在我机器上能跑" = 环境不一致的经典问题

有 Docker：
┌─────────────────────────────────────────────┐
│  所有人使用同一个 Docker 镜像构建             │
│  构建环境完全一致 → 消除"我机器上能跑"问题   │
└─────────────────────────────────────────────┘
```

### 2. Dockerfile 示例（多阶段构建）

```dockerfile
# 阶段1：构建
FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    cmake \
    g++ \
    ninja-build \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --parallel

# 阶段2：运行
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/build/myapp /app/myapp

ENTRYPOINT ["/app/myapp"]
```

### 3. Docker Compose 多服务编排

```yaml
version: "3.8"
services:
  app:
    build:
      context: .
      dockerfile: Dockerfile
    depends_on:
      - redis
    environment:
      - REDIS_HOST=redis

  redis:
    image: redis:7-alpine
    ports:
      - "6379:6379"

  test:
    build:
      context: .
      dockerfile: Dockerfile.test
    depends_on:
      - app
    command: ["./run_tests.sh"]
```

---

## 7. 发布与版本管理

### 1. 语义化版本（SemVer）

```
版本号格式：MAJOR.MINOR.PATCH

  2   .   1   .   3
  │       │       │
  │       │       └── PATCH：修复 bug，不改变 API
  │       └────────── MINOR：新增功能，向后兼容
  └────────────────── MAJOR：破坏性变更，不兼容旧版

示例：
1.0.0 → 1.0.1  修复了一个崩溃 bug（PATCH）
1.0.1 → 1.1.0  新增了导出 CSV 功能（MINOR）
1.1.0 → 2.0.0  重构了 API，旧接口不再可用（MAJOR）
```

### 2. Git 标签

```bash
# 创建带注释的标签
git tag -a v1.2.0 -m "发布 v1.2.0：新增 CSV 导出功能"

# 推送标签到远程
git push origin v1.2.0

# 推送所有标签
git push origin --tags

# 查看标签列表
git tag -l

# 删除远程标签
git push origin --delete v1.2.0
```

### 3. 自动生成 Changelog

```yaml
name: 生成 Changelog

on:
  push:
    tags:
      - 'v*'

jobs:
  changelog:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
        with:
          fetch-depth: 0

      - name: 生成 Changelog
        id: changelog
        uses: mikepenz/release-changelog-builder-action@v4
        with:
          configuration: ".github/changelog-config.json"
        env:
          GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}

      - name: 创建 Release
        uses: softprops/action-gh-release@v2
        with:
          body: ${{ steps.changelog.outputs.changelog }}
```

### 4. 发布流程

```
1. 开发完成 → 合并到 main 分支
2. 更新版本号（CMakeLists.txt 中的 PROJECT_VERSION）
3. 更新 CHANGELOG.md
4. 提交并打标签：git tag -a v1.2.0 -m "Release v1.2.0"
5. 推送标签：git push origin v1.2.0
6. CI 自动构建多平台二进制
7. CI 自动创建 GitHub Release
8. CI 自动上传构建产物到 Release
```

---

## 8. CI/CD 最佳实践

### 1. 核心原则

| 原则                 | 说明                                         |
| -------------------- | -------------------------------------------- |
| 提交前本地验证       | 推送前先本地构建和测试，避免浪费 CI 资源     |
| 小步提交             | 每次提交只做一件事，出问题容易定位           |
| 快速反馈             | CI 流水线尽量在 10 分钟内完成                |
| 失败必须优先修复     | 主分支 CI 失败时，其他工作暂停，优先修复     |
| 安全扫描集成         | 在 CI 中加入依赖漏洞扫描和代码安全检查       |

### 2. 提交前本地验证

```bash
# 本地快速验证脚本
cmake -B build && cmake --build build && cd build && ctest --output-on-failure
```

也可以使用 Git pre-commit 钩子：

```bash
#!/bin/bash
# .git/hooks/pre-commit
cmake -B build -DCMAKE_BUILD_TYPE=Debug && \
cmake --build build --parallel && \
(cd build && ctest --output-on-failure)
if [ $? -ne 0 ]; then
    echo "构建或测试失败，禁止提交"
    exit 1
fi
```

### 3. 安全扫描集成

```yaml
  security:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Trivy 漏洞扫描
        uses: aquasecurity/trivy-action@master
        with:
          scan-type: 'fs'
          scan-ref: '.'

      - name: Cppcheck 静态分析
        run: |
          sudo apt-get install -y cppcheck
          cppcheck --enable=all --error-exitcode=1 --suppress=missingInclude src/
```

### 4. CI 流水线优化

```
优化前（串行）：
构建(8min) → 测试(5min) → 覆盖率(3min) → 部署(2min)  总计：18min

优化后（并行）：
┌─ 构建 Linux(5min)   ─┐
├─ 构建 Windows(6min)  ─┤
├─ 构建 macOS(5min)    ─┼→ 部署(2min)  总计：8min
├─ 测试(5min)          ─┤
└─ 安全扫描(4min)      ─┘
```

---

## 9. 相关章节

- [Git与GitHub实战指南](14-Git与GitHub.md)
- [CMake与构建系统实战配置](../05-开发环境与IDE/03-CMake基础入门.md)
- [框架引擎中间件与架构概念指南](15-框架引擎中间件与架构.md)
- [编码规范](../../04-工程实践/00-编码规范.md)
- [项目理解分析与工程化编码指南](12-项目理解分析与工程化编码.md)

***

### 相关阅读

- [Git与GitHub](14-Git与GitHub.md)
- [开源许可协议](23-开源许可协议.md)
- [CPP编码规范与命名约定](./04-CPP编码规范与命名约定.md)