# Makefile与构建系统

> 掌握Makefile编写与项目构建

---

> 💡 **通俗理解 - 什么是Makefile？**

想象你要做一桌年夜饭：
- **手动一个个做**：先做凉菜、再做热菜...很麻烦
- **用Makefile**：写好菜单和步骤，让"大厨"自动安排

**Makefile就是"做菜的配方"！**
- 告诉电脑先做什么、后做什么
- 哪个文件变了，就重新做哪个菜
- 自动完成整个编译过程

---

> 🔬 **抽象理解 - 构建系统的本质**：
> - **构建系统**：是"自动化编译"的工具，管理项目的编译、测试、部署
> - **Makefile**：是Make工具的配置文件，描述文件依赖关系和构建规则
> - **Make**：是根据Makefile自动执行构建的工具
> - **构建目标**：是"要生成的文件"，如可执行文件、库文件

---

> **🎯 大事不糊涂，小事不纠缠。Makefile让编译井井有条。**
> 
> （处理大事不糊涂，处理小事不斤斤计较；Makefile帮你把复杂的编译管理得井井有条。）

> **"Automation applied to an inefficient operation will magnify the inefficiency."** — Bill Gates
> （将自动化应用于低效操作会放大低效。—— Makefile让高效的构建流程自动化。）

## 前置知识
- [GCC/G++编译器详解](21-GCC与G++编译器详解.md)

## 后续内容
- [CMake构建系统](22-CMake构建系统.md)
---

## 目录

- [1. Makefile基础](#1-makefile基础)
- [2. 变量与函数](#2-变量与函数)
- [3. 模式匹配](#3-模式匹配)
- [4. 条件与循环](#4-条件与循环)
- [5. 实战Makefile](#5-实战makefile)

---

## 1. Makefile基础

### 1. 基本结构

```makefile
# 这是一个注释

# 目标: 依赖
# [Tab]命令
target: dependencies
    command
```

### 2. 最简单的Makefile

```makefile
# 编译单个文件
hello: hello.c
    gcc hello.c -o hello

# 清理
clean:
    rm -f hello
```

### 3. Makefile核心语法

Makefile的基本结构非常简单：

```makefile
目标: 依赖
	命令
```

- **目标**：要生成的文件（如可执行文件）或要执行的操作（如clean）
- **依赖**：生成目标所需要的文件
- **命令**：生成目标的具体指令（**必须以Tab开头**）

### 4. 最重要的规则：Tab键！

**Makefile里，命令行前面必须是Tab键，不能用空格！**

如果用空格代替Tab，会报错：
```
Makefile:6: *** missing separator.  Stop.
```

这是99%的初学者都会踩的坑！

### 5. clean命令详解

`clean` 目标用于清理编译生成的文件：

```makefile
clean:
    rm -f hello
```

- `rm`：删除命令（Linux/Mac）
- `-f`：force（强制删除），文件不存在也不报错
- `hello`：要删除的可执行文件

**Windows + MinGW注意**：
```makefile
clean:
    rm -f hello.exe    # 或 del hello.exe
```

### 6. 使用Makefile

```bash
# 执行默认目标（第一个目标）
make

# 执行指定目标
make clean

# 指定Makefile
make -f mymakefile
```

### 7. 升级版：超通用、最标准的Makefile

```makefile
CC = gcc
CFLAGS = -Wall -g

all: hello

hello: hello.c
	$(CC) hello.c -o hello $(CFLAGS)

clean:
	rm -f hello
```

**升级好处**：
- `-Wall`：开启所有警告，帮助发现潜在问题
- `-g`：生成调试信息，方便调试
- `$(CC)` 和 `$(CFLAGS)`：可轻松改编译器和编译参数

**Windows + MinGW版本**：
```makefile
CC = gcc
CFLAGS = -Wall -g

all: hello

hello: hello.c
	$(CC) hello.c -o hello $(CFLAGS)

clean:
	del hello.exe
```

### 8. 目标（Target）

```makefile
# 最常用的目标
all: program1 program2    # all是默认目标

# 可执行文件目标
myapp: main.o utils.o
    g++ main.o utils.o -o myapp

# 伪目标（不代表文件）
clean:
    rm -f *.o myapp

.PHONY: clean all       # 声明伪目标
```

### 9. 依赖（Dependencies）

```makefile
# 目标文件依赖于源文件和头文件
main.o: main.cpp utils.h
    g++ -c main.cpp

# 依赖可以是另一个目标
all: myapp

myapp: main.o
    g++ main.o -o myapp
```

---

## 2. 变量与函数

### 1. 变量

```makefile
# 定义变量
CC = g++
CFLAGS = -Wall -g
TARGET = myapp

# 使用变量
$(TARGET): main.o
    $(CC) $(CFLAGS) main.o -o $(TARGET)
```

### 2. 自动变量

```makefile
# 常用的自动变量
%.o: %.c
    # $@ - 目标名
    # $< - 第一个依赖名
    # $^ - 所有依赖名
    # $? - 比目标更新的依赖
    gcc -c $< -o $@

# 例子
myapp: main.o utils.o
    g++ $^ -o $@    # 编译所有.o文件
```

### 3. 函数

```makefile
# 字符串替换
SRCS = main.c utils.c test.c
OBJS = $(SRCS:.c=.o)    # main.o utils.o test.o

# 通配符
SRCS = $(wildcard *.c)   # 当前目录所有.c文件

# 模式替换
OBJS = $(patsubst %.c, %.o, $(SRCS))

# 目录
DIRS = src include lib
ALL_DIRS = $(DIRS) build dist
```

---

## 3. 模式匹配

### 1. 模式规则

```makefile
# 模式规则：把所有.c编译为.o
%.o: %.c
    gcc -c $< -o $@

# 完整例子
SRCS = main.c utils.c helper.c
OBJECTS = $(SRCS:.c=.o)

myapp: $(OBJECTS)
    g++ $^ -o $@

%.o: %.c
    gcc -c $< -o $@

clean:
    rm -f $(OBJECTS) myapp
```

### 2. 链式规则

```makefile
# Make自动推导中间步骤
# main -> main.o -> main.c
# 只需要写
main: main.o
    gcc main.o -o main

main.o: main.c
    gcc -c main.c

# Make会自动找到 main.c
```

---

## 4. 条件与循环

### 1. 条件判断

```makefile
# 条件判断
DEBUG = 1

ifeq ($(DEBUG),1)
    CFLAGS = -g -DDEBUG
else
    CFLAGS = -O2
endif

# 判断变量是否定义
ifdef RELEASE
    CFLAGS += -DNDEBUG
endif
```

### 2. 循环

```makefile
# 循环（使用函数）
SUBDIRS = src test docs

all:
    @for dir in $(SUBDIRS); do \
        echo "Building $$dir"; \
    done
```

---

## 5. 实战Makefile

### 1. 完整项目Makefile

```makefile
# ==========================================
# 变量定义
# ==========================================
CC = g++
CXX = g++
CFLAGS = -Wall -Wextra -std=c++17
CXXFLAGS = -Wall -Wextra -std=c++17

# 目录
SRC_DIR = src
INC_DIR = include
LIB_DIR = lib
BUILD_DIR = build
BIN_DIR = bin

# 文件
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES))
TARGET = $(BIN_DIR)/myapp

# 库
LIBS = -lpthread -lcurl
LIBPATH = -L$(LIB_DIR)

# ==========================================
# 目标
# ==========================================
all: $(TARGET)

$(TARGET): $(OBJECTS)
    @mkdir -p $(BIN_DIR)
    $(CXX) $^ -o $@ $(LIBPATH) $(LIBS)
    @echo "Build complete: $@"

# 模式规则：编译.cpp到.o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
    @mkdir -p $(dir $@)
    $(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

# ==========================================
# 清理
# ==========================================
clean:
    rm -rf $(BUILD_DIR) $(BIN_DIR)

# ==========================================
# 安装
# ==========================================
install:
    cp $(TARGET) /usr/local/bin/

uninstall:
    rm -f /usr/local/bin/myapp

# ==========================================
# 伪目标
# ==========================================
.PHONY: all clean install uninstall
```

### 2. 多目标Makefile

```makefile
# 编译多个可执行文件
CC = g++
CFLAGS = -Wall -g

TARGETS = server client tool

all: $(TARGETS)

server: server.cpp
    $(CC) $(CFLAGS) $< -o $@

client: client.cpp
    $(CC) $(CFLAGS) $< -o $@

tool: tool.cpp
    $(CC) $(CFLAGS) $< -o $@

clean:
    rm -f $(TARGETS)

.PHONY: all clean
```

---

## 6. 常见问题

**Q：Makefile中Tab问题？**
A：Makefile要求命令前必须用Tab，不是空格。如果使用空格会报错。

**Q：如何调试Makefile？**
A：
```bash
# 显示执行的命令
make -n           # 只显示不执行
make DEBUG=1      # 传递变量
```

**Q：如何处理头文件依赖？**
A：使用-MM生成依赖
```makefile
CFLAGS = -MM -MT $@
%.o: %.cpp
    $(CXX) $(CXXFLAGS) -I$(INC_DIR) $< $@
```

---

## 7. 自动依赖生成

### 1. 为什么需要自动依赖？

Makefile的一个经典陷阱：**头文件修改后，`.o`文件不会自动重新编译**。

```makefile
# 看似正确的规则
main.o: main.c
    gcc -c main.c -o main.o
```

问题在于：`main.c`中`#include "utils.h"`，但Makefile没有写`main.o: utils.h`。
当你修改了`utils.h`，`make`认为`main.o`不需要更新——因为Makefile里没写这个依赖！

**手动维护依赖太痛苦**：每个`.c`文件`#include`了哪些头文件，都要手写一遍，改了还要同步更新。

**解决方案**：让编译器自动生成依赖关系！

### 2. GCC依赖生成选项

GCC提供了一系列`-M`选项来自动生成依赖：

| 选项 | 作用 |
|------|------|
| `-M` | 输出所有依赖（包括系统头文件），输出到stdout |
| `-MM` | 只输出用户头文件（忽略系统头文件），输出到stdout |
| `-MD` | 和`-MM`一样，但将依赖写入`.d`文件，同时继续编译 |
| `-MMD` | 和`-MD`一样，但忽略系统头文件 |
| `-MF file` | 指定依赖文件的输出路径 |
| `-MT target` | 指定依赖规则中的目标名 |
| `-MP` | 为每个头文件生成一个空目标（防止删除头文件后报错） |

### 3. -MP选项：防止删除头文件后报错

假设生成了这样的依赖：

```
main.o: main.c utils.h
```

如果你删除了`utils.h`，`make`会报错：`No rule to make target 'utils.h'`。

`-MP`会额外生成空目标：

```
main.o: main.c utils.h
utils.h:
```

这样即使`utils.h`被删除，`make`也不会报错——它会把`utils.h`当作不需要任何依赖的目标。

### 4. 完整的自动依赖Makefile示例

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c89
DEPFLAGS = -MMD -MP

SRC_DIR = src
BUILD_DIR = build
INC_DIR = include

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
DEPS = $(OBJS:.o=.d)

TARGET = myapp

all: $(TARGET)

$(TARGET): $(OBJS)
    $(CC) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
    @mkdir -p $(dir $@)
    $(CC) $(CFLAGS) $(DEPFLAGS) -I$(INC_DIR) -c $< -o $@

clean:
    rm -rf $(BUILD_DIR) $(TARGET)

# 关键：引入所有.d依赖文件
-include $(DEPS)

.PHONY: all clean
```

**核心原理**：
1. `-MMD -MP`在编译`.c`时，自动在`build/`下生成同名的`.d`文件
2. `-include $(DEPS)`将所有`.d`文件包含进来
3. 前面的`-`表示文件不存在也不报错（首次编译时没有`.d`文件）

生成的`.d`文件内容示例：

```
build/main.o: src/main.c include/utils.h include/config.h
include/utils.h:
include/config.h:
```

### 5. -MMD -MP的推荐用法

`-MMD -MP`是最推荐的组合：

```makefile
# 最简写法：直接加到CFLAGS里
CFLAGS = -Wall -Wextra -MMD -MP

# 编译规则
%.o: %.c
    $(CC) $(CFLAGS) -c $< -o $@

# 包含依赖
-include $(OBJS:.o=.d)
```

**为什么推荐`-MMD -MP`？**
- `-MMD`：只跟踪你自己的头文件，不跟踪系统头文件（`/usr/include/stdio.h`等）
- `-MP`：生成空目标，删除头文件后不会报错
- 两者结合，既精确又健壮

### 6. 与模式规则结合的写法

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -MMD -MP

SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c, build/%.o, $(SRCS))

TARGET = myapp

all: $(TARGET)

$(TARGET): $(OBJS)
    $(CC) $^ -o $@

build/%.o: src/%.c
    @mkdir -p build
    $(CC) $(CFLAGS) -Iinclude -c $< -o $@

clean:
    rm -rf build $(TARGET)

-include $(OBJS:.o=.d)

.PHONY: all clean
```

**流程说明**：
1. 首次`make`时，没有`.d`文件，`-include`不报错
2. 编译每个`.o`时，`-MMD -MP`自动生成对应的`.d`文件
3. 第二次`make`时，`-include`读取`.d`文件，获得完整的头文件依赖
4. 修改任何头文件，`make`都能正确地重新编译受影响的`.o`

---

## 8. 并行编译与高级技巧

### 1. -j参数：并行编译

`make`默认单线程编译。`-j`参数可以并行执行多个编译任务，大幅缩短编译时间：

```bash
# 使用所有CPU核心并行编译
make -j$(nproc)

# 指定并行数
make -j4

# 无限制并行（可能导致系统卡顿）
make -j
```

**推荐用法**：

```bash
# Linux/Mac
make -j$(nproc)

# Windows (PowerShell)
make -j$env:NUMBER_OF_PROCESSORS
```

**注意事项**：
- 并行数不宜超过CPU核心数的2倍
- 链接步骤通常是串行的，并行主要加速编译阶段
- 如果Makefile有隐式依赖关系，并行编译可能暴露bug

### 2. -B参数：强制重新编译

```bash
# 强制重新编译所有目标，不管是否过期
make -B

# 等效于先clean再build，但不会删除中间文件
make -B -j$(nproc)
```

**使用场景**：
- 切换了编译器或编译选项后
- 怀疑增量编译结果不正确时
- CI/CD中确保完整构建

### 3. -C参数：切换目录执行

```bash
# 在src目录下执行make
make -C src

# 在build目录下执行make，并传递变量
make -C build DEBUG=1
```

**常见用法**：在项目根目录的Makefile中调用子目录的Makefile：

```makefile
SUBDIRS = lib src tests

all:
    @for dir in $(SUBDIRS); do \
        make -C $$dir; \
    done
```

### 4. -n参数：Dry Run（只显示不执行）

```bash
# 只显示将要执行的命令，不实际执行
make -n

# 常用于检查构建流程
make -n clean all
```

**使用场景**：
- 检查Makefile逻辑是否正确
- 确认哪些文件会被重新编译
- CI脚本调试

### 5. PHONY的完整用法

`.PHONY`声明"伪目标"——这些目标不对应实际文件：

```makefile
# 基本用法
.PHONY: all clean install

all: myapp

clean:
    rm -f *.o myapp

install:
    cp myapp /usr/local/bin/
```

**为什么需要`.PHONY`？**

如果目录下恰好有个叫`clean`的文件，`make clean`会认为`clean`已经是最新的，不执行！

```bash
# 假设当前目录有个clean文件
touch clean
make clean
# make: 'clean' is up to date.  ← 不会执行清理！
```

声明`.PHONY: clean`后，`make`不再检查文件是否存在，总是执行。

**完整的伪目标声明**：

```makefile
.PHONY: all clean install uninstall test help

all: 构建所有目标
clean: 清理构建产物
install: 安装到系统
uninstall: 卸载
test: 运行测试
help: 显示帮助信息
```

### 6. 嵌套Make（递归make -C）

大型项目常用嵌套Make：每个子目录有自己的Makefile，根目录Makefile递归调用。

**项目结构**：

```
project/
├── Makefile           # 根Makefile
├── lib/
│   └── Makefile       # 库的Makefile
├── src/
│   └── Makefile       # 源码的Makefile
└── test/
    └── Makefile       # 测试的Makefile
```

**根Makefile**：

```makefile
SUBDIRS = lib src test

all: $(SUBDIRS)

$(SUBDIRS):
    make -C $@

clean:
    @for dir in $(SUBDIRS); do \
        make -C $$dir clean; \
    done

.PHONY: all clean $(SUBDIRS)
```

**注意事项**：
- 递归make中，变量不会自动传递，需要用`export`或命令行传递
- `$(MAKE)`比直接写`make`更好——它会传递`-j`等参数
- 递归make有性能开销，现代项目更推荐非递归Makefile

**使用$(MAKE)的写法**：

```makefile
SUBDIRS = lib src test

all:
    @for dir in $(SUBDIRS); do \
        $(MAKE) -C $$dir; \
    done

clean:
    @for dir in $(SUBDIRS); do \
        $(MAKE) -C $$dir clean; \
    done

.PHONY: all clean
```

### 7. VPATH/vpath：源文件搜索

当源文件分散在多个目录时，`VPATH`和`vpath`可以让`make`自动搜索源文件。

**VPATH方式**（搜索所有类型的文件）：

```makefile
# 在src和lib目录中搜索依赖文件
VPATH = src:lib

# make会自动在src/和lib/中查找main.c和utils.c
main.o: main.c utils.c
    gcc -c $< -o $@
```

**vpath方式**（按模式搜索，更精确）：

```makefile
# .c文件在src目录搜索
vpath %.c src

# .h文件在include目录搜索
vpath %.h include

# .cpp文件在多个目录搜索（按顺序）
vpath %.cpp src lib
```

**完整示例**：

```makefile
CC = gcc
CFLAGS = -Wall -MMD -MP

# 源文件搜索路径
vpath %.c src
vpath %.h include

SRCS = main.c utils.c helper.c
OBJS = $(SRCS:.c=.o)

TARGET = myapp

all: $(TARGET)

$(TARGET): $(OBJS)
    $(CC) $^ -o $@

%.o: %.c
    $(CC) $(CFLAGS) -Iinclude -c $< -o $@

clean:
    rm -f $(OBJS) $(TARGET) $(OBJS:.o=.d)

-include $(OBJS:.o=.d)

.PHONY: all clean
```

---

## 9. 相关链接

**上一章：** [第21章：GCC/G++编译器详解](20-GCC与G++编译器.md)\
**下一章：** [第23章：CMake构建系统](22-CMake构建系统.md)

***

### 1. 相关章节

- [CMake构建系统](../04-工程实践/开发环境/03-CMake基础入门.md) — CMake vs Make对比、CMakeLists.txt写法
- [VSCode核心配置](../04-工程实践/开发环境/00-VSCode核心配置.md) — tasks.json中调用make构建
