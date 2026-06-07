# 什么是交叉编译 Cross-Compilation
> 📖 相关章节：[编码规范](../../04-工程实践/00-编码规范.md)、[设计模式](../../04-工程实践/03-设计模式.md)、[单元测试](../../04-工程实践/05-单元测试.md)、[代码审查](../../04-工程实践/07-代码审查.md)

> "在宿主机上编译，在目标机上运行"——嵌入式开发的基石

***

### 1. 一句话概括

交叉编译是在一个平台（宿主机）上生成另一个平台（目标机）可执行代码的编译过程，是嵌入式、IoT、移动端和 WebAssembly 开发的核心技术。

***

### 2. 交叉编译的定义与核心概念

**本地编译** vs **交叉编译**：

| 概念 | 编译平台 | 运行平台 | 示例 |
|------|----------|----------|------|
| 本地编译 | x86_64-linux | x86_64-linux | 在 Ubuntu 上编译 Ubuntu 程序 |
| 交叉编译 | x86_64-linux | arm-linux | 在 Ubuntu 上编译树莓派程序 |
| 加拿大交叉 | A 平台 | C 平台 | 在 x86 上用 ARM 工具链编译 RISC-V 程序 |

核心术语：

- **宿主机（Host）**：运行编译器的机器
- **目标机（Target）**：运行生成程序的机器
- **构建机（Build）**：实际执行构建的机器（通常与 Host 相同）
- **三元组（Triple）**：`arch-vendor-os-abi`，如 `arm-linux-gnueabihf`

常见三元组一览：

| 三元组 | 含义 |
|--------|------|
| `x86_64-linux-gnu` | 64 位 x86 Linux |
| `arm-linux-gnueabihf` | 32 位 ARM Linux 硬浮点 |
| `aarch64-linux-gnu` | 64 位 ARM Linux |
| `riscv64-unknown-linux-gnu` | 64 位 RISC-V Linux |
| `armv7a-linux-androideabi` | ARM Android |
| `wasm32-unknown-unknown` | WebAssembly |

***

### 3. 为什么需要交叉编译

交叉编译的典型场景：

**1. 目标机资源不足**

嵌入式设备通常内存小、算力弱，无法运行编译器：

```cpp
// 典型嵌入式设备资源
// STM32F407: 168MHz, 192KB RAM, 1MB Flash
// 树莓派 Zero: 1GHz, 512MB RAM
// 在这些设备上编译 LLVM/Clang 几乎不可能
```

**2. 目标机无操作系统**

裸机（Bare-metal）设备根本没有文件系统和运行环境：

```cpp
// Cortex-M 裸机启动代码
void Reset_Handler(void) {
    .bss 段清零;
    .data 段从 Flash 搬运到 RAM;
    main();
}

int main(void) {
    while (1) {
        GPIO_Toggle(LED_PIN);
        delay(500);
    }
}
```

**3. 批量构建效率**

在强大的 CI 服务器上交叉编译，比在目标设备上本地编译快数十倍：

| 场景 | 本地编译时间 | 交叉编译时间 |
|------|-------------|-------------|
| 树莓派编译 LLVM | ~72 小时 | ~2 小时（x86 服务器） |
| Android AOSP | N/A | ~4 小时（32 核服务器） |

**4. 跨平台发布**

一份源码，多个目标平台：

```
src/
  └── app.cpp          ← 同一份代码
build/
  ├── linux-x86_64/    ← 本地编译
  ├── linux-arm64/     ← ARM 交叉编译
  ├── android-arm64/   ← Android NDK
  └── wasm/            ← Emscripten
```

***

### 4. 交叉编译工具链

工具链是交叉编译的核心，包含编译器、汇编器、链接器和标准库。

**工具链组成**：

```
arm-linux-gnueabihf-         ← 前缀
├── gcc                      ← C 编译器
├── g++                      ← C++ 编译器
├── ar                       ← 归档器（创建静态库）
├── as                       ← 汇编器
├── ld                       ← 链接器
├── objdump                  ← 反汇编
├── objcopy                  ← 二进制转换
├── strip                    ← 去除符号
├── nm                       ← 符号查看
└── readelf                  ← ELF 分析
```

**安装常用工具链**：

```bash
# Debian/Ubuntu 安装 ARM 工具链
sudo apt install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf

# 安装 AArch64 工具链
sudo apt install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu

# 安装 RISC-V 工具链
sudo apt install gcc-riscv64-linux-gnu g++-riscv64-linux-gnu

# 验证工具链
arm-linux-gnueabihf-gcc --version
arm-linux-gnueabihf-gcc -dumpmachine
# 输出: arm-linux-gnueabihf
```

**基本编译命令**：

```bash
# 交叉编译单个文件
arm-linux-gnueabihf-gcc -o hello hello.c

# 交叉编译 C++ 程序
arm-linux-gnueabihf-g++ -std=c++17 -O2 -o app main.cpp utils.cpp

# 查看生成文件的架构
file hello
# 输出: hello: ELF 32-bit LSB executable, ARM, EABI5 version 1 ...
```

**Android NDK 工具链**：

```bash
# 设置 NDK 路径
export NDK=/path/to/android-ndk

# 使用 NDK 编译
$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android33-clang++ \
    -std=c++17 -O2 -o app_android main.cpp
```

**Emscripten（WebAssembly）**：

```bash
# 安装 Emscripten
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk && ./emsdk install latest && ./emsdk activate latest

# 编译为 WebAssembly
emcc -std=c++17 -O2 -o app.js main.cpp
emcc -std=c++17 -O2 -o app.html main.cpp
```

***

### 5. Sysroot 与库依赖

**Sysroot** 是目标系统的根文件系统镜像，包含头文件、库文件等。

```
sysroot/
├── usr/
│   ├── include/             ← 目标平台头文件
│   │   ├── stdio.h
│   │   ├── stdlib.h
│   │   └── c++/
│   │       └── 12/
│   │           ├── vector
│   │           └── string
│   └── lib/                 ← 目标平台库文件
│       ├── libc.so
│       ├── libm.so
│       └── libc++.so
├── lib/
│   ├── libc.so.6
│   └── ld-linux-armhf.so.3
└── etc/
```

**指定 Sysroot**：

```bash
# 使用 --sysroot 指定
arm-linux-gnueabihf-g++ --sysroot=/opt/arm-sysroot -o app main.cpp

# 使用环境变量
export SYSROOT=/opt/arm-sysroot
arm-linux-gnueabihf-g++ --sysroot=$SYSROOT -o app main.cpp
```

**从目标设备提取 Sysroot**：

```bash
# 使用 rsync 从树莓派拉取
rsync -avz pi@raspberrypi:/usr/include /opt/rpi-sysroot/usr/
rsync -avz pi@raspberrypi:/usr/lib /opt/rpi-sysroot/usr/
rsync -avz pi@raspberrypi:/lib /opt/rpi-sysroot/

# 使用 apt 下载 ARM 包（不安装）
apt-get download -o APT::Architecture=armhf libfoo-dev
dpkg-deb -x libfoo-dev_1.0_armhf.deb /opt/arm-sysroot/
```

**处理第三方库依赖**：

```cmake
# CMake 中指定第三方库路径
set(CMAKE_FIND_ROOT_PATH /opt/arm-sysroot)
set(CMAKE_PREFIX_PATH /opt/arm-sysroot/usr)

# 或直接指定
target_include_directories(app PRIVATE /opt/arm-sysroot/usr/include)
target_link_directories(app PRIVATE /opt/arm-sysroot/usr/lib)
```

***

### 6. CMake 工具链文件

CMake 工具链文件（Toolchain File）是管理交叉编译配置的标准方式。

**基础工具链文件示例**（`arm-linux-toolchain.cmake`）：

```cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)

set(CMAKE_FIND_ROOT_PATH /opt/arm-sysroot)
set(CMAKE_SYSROOT /opt/arm-sysroot)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
```

**Android NDK 工具链文件**：

```bash
cmake -B build-android \
    -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-33 \
    -DANDROID_STL=c++_shared
```

**WebAssembly 工具链文件**（`wasm-toolchain.cmake`）：

```cmake
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR wasm)

set(CMAKE_C_COMPILER emcc)
set(CMAKE_CXX_COMPILER em++)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s WASM=1 -s USE_PTHREADS=1")
```

**完整项目构建流程**：

```bash
# 配置
cmake -B build-arm \
    -DCMAKE_TOOLCHAIN_FILE=cmake/arm-linux-toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release

# 编译
cmake --build build-arm -j$(nproc)

# 验证产物
file build-arm/myapp
readelf -h build-arm/myapp | grep Machine
```

**CMake 中检测交叉编译**：

```cmake
if(CMAKE_CROSSCOMPILING)
    message(STATUS "交叉编译目标: ${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}")
    message(STATUS "编译器: ${CMAKE_CXX_COMPILER}")
else()
    message(STATUS "本地编译")
endif()
```

***

### 7. 常见目标平台

| 目标平台 | 三元组 | 工具链来源 | 典型用途 |
|----------|--------|-----------|---------|
| ARM 32-bit | `arm-linux-gnueabihf` | apt / crosstool-NG | 树莓派、旧嵌入式 |
| ARM 64-bit | `aarch64-linux-gnu` | apt / crosstool-NG | 服务器、新嵌入式 |
| RISC-V 64 | `riscv64-linux-gnu` | apt / 官方仓库 | 新兴 IoT 芯片 |
| Android ARM64 | `aarch64-linux-android33` | Android NDK | Android 应用 |
| Android x86_64 | `x86_64-linux-android33` | Android NDK | 模拟器调试 |
| WebAssembly | `wasm32-unknown-unknown` | Emscripten | Web 应用 |
| Cortex-M 裸机 | `arm-none-eabi` | apt / ARM 官方 | STM32 等微控制器 |
| iOS ARM64 | `arm64-apple-ios` | Xcode | iOS 应用 |

**crosstool-NG 自定义工具链**：

```bash
# 安装 crosstool-NG
sudo apt install crosstool-ng

# 创建配置
ct-ng arm-linux-gnueabihf
ct-ng menuconfig
ct-ng build
```

**ARM 裸机（Cortex-M）示例**：

```cpp
#include <stdint.h>

#define PERIPH_BASE  0x40000000UL
#define AHB1_OFFSET  0x00020000UL
#define RCC_BASE     (PERIPH_BASE + AHB1_OFFSET)

volatile uint32_t* RCC_AHB1ENR = (uint32_t*)(RCC_BASE + 0x30);

int main(void) {
    *RCC_AHB1ENR |= (1 << 0);

    while (1) {
    }

    return 0;
}
```

```bash
# 编译裸机程序
arm-none-eabi-g++ -mcpu=cortex-m4 -mthumb -nostdlib \
    -T stm32f407.ld -o firmware.elf main.cpp

# 生成二进制镜像
arm-none-eabi-objcopy -O binary firmware.elf firmware.bin
```

***

### 8. QEMU 模拟运行与调试

QEMU 可以在宿主机上模拟目标平台，用于运行和调试交叉编译的程序。

**用户态模拟（Linux 用户程序）**：

```bash
# 安装 QEMU 用户态
sudo apt install qemu-user qemu-user-static

# 直接运行 ARM 程序
qemu-arm ./hello_arm

# 运行 AArch64 程序
qemu-aarch64 ./hello_aarch64

# 指定 sysroot 运行
qemu-arm -L /opt/arm-sysroot ./app
```

**系统态模拟（完整操作系统）**：

```bash
# 模拟树莓派
qemu-system-arm -M versatilepb -kernel kernel.img \
    -dtb versatile-pb.dtb -drive file=rootfs.ext4 \
    -append "root=/dev/sda2" -serial stdio

# 模拟 RISC-V
qemu-system-riscv64 -M virt -kernel vmlinux \
    -drive file=rootfs.img -nographic
```

**GDB 远程调试**：

```bash
# 终端 1：启动 QEMU 并等待 GDB 连接
qemu-arm -g 1234 ./app

# 终端 2：启动交叉 GDB
arm-linux-gnueabihf-gdb ./app
(gdb) target remote :1234
(gdb) break main
(gdb) continue
```

**CMake + QEMU 集成测试**：

```cmake
if(CMAKE_CROSSCOMPILING)
    find_program(QEMU_ARM qemu-arm)
    if(QEMU_ARM)
        set(CMAKE_CROSSCOMPILING_EMULATOR
            ${QEMU_ARM} -L ${CMAKE_SYSROOT})
        message(STATUS "使用 QEMU 运行交叉编译测试")
    endif()
endif()

enable_testing()
add_executable(test_app test.cpp)
add_test(NAME test_app COMMAND test_app)
```

```bash
# 运行测试
cmake --build build-arm
cd build-arm && ctest --output-on-failure
```

***

### 9. 交叉编译工作流与最佳实践

**典型交叉编译项目结构**：

```
project/
├── CMakeLists.txt
├── cmake/
│   ├── arm-linux-toolchain.cmake
│   ├── aarch64-linux-toolchain.cmake
│   ├── riscv64-linux-toolchain.cmake
│   └── wasm-toolchain.cmake
├── src/
│   ├── main.cpp
│   └── utils.cpp
├── include/
│   └── utils.h
├── sysroot/
│   └── arm/
│       ├── usr/include/
│       └── usr/lib/
└── scripts/
    ├── build-all.sh
    └── deploy.sh
```

**多目标构建脚本**：

```bash
#!/bin/bash
set -e

TARGETS=("arm" "aarch64" "riscv64")
TOOLCHAINS=(
    "cmake/arm-linux-toolchain.cmake"
    "cmake/aarch64-linux-toolchain.cmake"
    "cmake/riscv64-linux-toolchain.cmake"
)

for i in "${!TARGETS[@]}"; do
    TARGET=${TARGETS[$i]}
    TOOLCHAIN=${TOOLCHAINS[$i]}

    echo "=== 构建 ${TARGET} ==="
    cmake -B build-${TARGET} \
        -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN} \
        -DCMAKE_BUILD_TYPE=Release
    cmake --build build-${TARGET} -j$(nproc)
    echo "=== ${TARGET} 构建完成 ==="
done
```

**最佳实践清单**：

| 实践 | 说明 |
|------|------|
| 使用工具链文件 | 不要在 CMakeLists.txt 中硬编码编译器路径 |
| 管理 Sysroot | 保持 Sysroot 与目标系统版本一致 |
| 避免本地依赖 | 不要链接宿主机的库 |
| 交叉编译测试 | 使用 QEMU 或真实设备运行测试 |
| CI 自动化 | 在 CI 中配置多目标构建矩阵 |
| 符号分离 | 发布时 strip，调试时保留符号文件 |
| 架构检测 | 代码中用宏区分平台 |

**代码中的平台检测**：

```cpp
#if defined(__arm__) || defined(__TARGET_ARCH_ARM)
    #define PLATFORM_ARM 1
#elif defined(__aarch64__)
    #define PLATFORM_ARM64 1
#elif defined(__riscv)
    #define PLATFORM_RISCV 1
#elif defined(__EMSCRIPTEN__)
    #define PLATFORM_WASM 1
#elif defined(__ANDROID__)
    #define PLATFORM_ANDROID 1
#elif defined(__x86_64__) || defined(_M_X64)
    #define PLATFORM_X86_64 1
#else
    #define PLATFORM_UNKNOWN 1
#endif

#if PLATFORM_ARM
#include <arm_neon.h>
void vector_add(float* dst, const float* a, const float* b, int n) {
    int i = 0;
    for (; i + 4 <= n; i += 4) {
        float32x4_t va = vld1q_f32(a + i);
        float32x4_t vb = vld1q_f32(b + i);
        vst1q_f32(dst + i, vaddq_f32(va, vb));
    }
    for (; i < n; ++i) dst[i] = a[i] + b[i];
}
#endif
```

***

### 10. 常见问题与排查

**问题 1：链接时找不到库**

```bash
# 错误: cannot find -lfoo
# 原因: 链接了宿主机的库路径

# 解决: 确认库是目标架构
file /opt/arm-sysroot/usr/lib/libfoo.so
# 应输出: ELF 32-bit LSB shared object, ARM, ...

# 确保 CMAKE_FIND_ROOT_PATH_MODE_LIBRARY 设为 ONLY
```

**问题 2：头文件路径错误**

```bash
# 错误: fatal error: foo.h: No such file or directory
# 原因: 使用了宿主机的头文件

# 解决: 检查 sysroot 中的头文件
ls /opt/arm-sysroot/usr/include/foo.h

# 确保编译器使用 --sysroot
arm-linux-gnueabihf-g++ --sysroot=/opt/arm-sysroot -I/opt/arm-sysroot/usr/include ...
```

**问题 3：运行时架构不匹配**

```bash
# 错误: cannot execute binary file: Exec format error
# 原因: 在 x86 机器上运行了 ARM 二进制

# 解决: 使用 QEMU 模拟
qemu-arm -L /opt/arm-sysroot ./app

# 或拷贝到目标设备运行
scp app pi@raspberrypi:~/
ssh pi@raspberrypi ./app
```

**问题 4：CMake 检测编译器失败**

```cmake
# 设置 CMAKE_TRY_COMPILE_TARGET_TYPE 避免链接测试
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
```

***

### 11. 极简总结

| 要点 | 内容 |
|------|------|
| 定义 | 在宿主机编译目标机可执行代码 |
| 核心工具 | 交叉编译工具链（gcc/g++/ld/ar） |
| 标识 | 三元组 `arch-vendor-os-abi` |
| Sysroot | 目标系统的头文件和库文件根目录 |
| CMake 配置 | 工具链文件 `toolchain.cmake` |
| 运行调试 | QEMU 用户态/系统态模拟 |
| 常见目标 | ARM、AArch64、RISC-V、Android、WASM、Cortex-M |
| 关键原则 | 不链接宿主库、管理好 Sysroot、CI 多目标构建 |

**交叉编译核心流程**：

```
源码 → 交叉工具链 → 目标二进制 → QEMU/设备 → 运行/调试
         ↑                ↑
    toolchain.cmake    sysroot/
```

***

### 相关阅读

- [什么是字节序Endianness](27-什么是字节序Endianness.md)
- [跨平台是什么意思](./00-跨平台是什么意思.md)
- [LLVM与Clang](25-LLVM与Clang.md)

***