# C++ 快速学习指南

[![License](https://img.shields.io/badge/license-MIT-blue)](./LICENSE)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue)](https://en.cppreference.com/w/cpp/20)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](./CONTRIBUTING.md)

> 从C语言到C++23的极速学习路径

---

## 1. 参与贡献

欢迎提交Issue和Pull Request！详见 [CONTRIBUTING.md](./CONTRIBUTING.md)

## 2. 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](./LICENSE)

---

## 目录

### 1. 第一部分：C语言基础

> 导学：[编程入门](./docs/01-C语言/导学/00-编程入门.md) | [成长之道](./docs/01-C语言/导学/01-编程工程师与成长之道.md) | [关键字速查](./docs/01-C语言/导学/02-C与Cpp关键字与词汇速查.md)

| 章节 | 文档 | 内容概要 | 示例代码 |
|-----|------|---------|---------|
| 第0章 | [00-C语言概述.md](./docs/01-C语言/00-C语言概述.md) | 编译过程、程序结构、C语言哲学 | [chapter00](./code/c/chapter00-c-overview/) |
| 第1章 | [01-数据类型与变量.md](./docs/01-C语言/01-数据类型与变量.md) | 基本类型、变量常量、类型转换、类型限制 | [chapter01](./code/c/chapter01-data-types/) |
| 第2章 | [02-运算符与表达式.md](./docs/01-C语言/02-运算符与表达式.md) | 算术/逻辑/赋值运算符、运算符优先级、位运算技巧 | [chapter02](./code/c/chapter02-operators/) |
| 第3章 | [03-控制结构.md](./docs/01-C语言/03-控制结构.md) | if/switch/循环/goto、循环模式、switch陷阱 | [chapter03](./code/c/chapter03-control-structure/) |
| 第4章 | [04-函数.md](./docs/01-C语言/04-函数.md) | 函数基础/高级、调用约定、递归、作用域与生命周期 | [chapter04](./code/c/chapter04-function/) |
| 第5章 | [05-数组.md](./docs/01-C语言/05-数组.md) | 一维/多维数组、数组与函数、数组内存布局 | [chapter05](./code/c/chapter05-array/) |
| 第6章 | [06-指针.md](./docs/01-C语言/06-指针.md) | 指针基础/算术、指针与数组、指针模式与陷阱 | [chapter06](./code/c/chapter06-pointer/) |
| 第7章 | [07-字符串处理.md](./docs/01-C语言/07-字符串处理.md) | 字符串基础/操作/高级、字符串安全 | [chapter07](./code/c/chapter07-string/) |
| 第8章 | [08-结构体与联合体.md](./docs/01-C语言/08-结构体与联合体.md) | 结构体/联合体/位域、内存对齐、内存布局 | [chapter08](./code/c/chapter08-struct-union/) |
| 第9章 | [09-内存管理.md](./docs/01-C语言/09-内存管理.md) | malloc/free、动态数组、内存泄漏、内存调试 | [chapter09](./code/c/chapter09-memory-management/) |
| 第10章 | [10-预处理器.md](./docs/01-C语言/10-预处理器.md) | 宏基础/高级、条件编译、预定义宏、宏陷阱 | [chapter10](./code/c/chapter10-preprocessor/) |
| 第11章 | [11-可变参数与命令行.md](./docs/01-C语言/11-可变参数与命令行.md) | va_list可变参数、命令行参数、getopt解析 | [chapter11](./code/c/chapter11-variadic/) |
| 第12章 | [12-位操作实战.md](./docs/01-C语言/12-位操作实战.md) | 位运算、位技巧、位域、位操作模式与应用 | [chapter12](./code/c/chapter12-bitwise/) |
| 第13章 | [13-错误处理与信号.md](./docs/01-C语言/13-错误处理与信号.md) | errno、signal、assert、错误与信号模式 | [chapter13](./code/c/chapter13-error-signal/) |
| 第14章 | [14-时间处理.md](./docs/01-C语言/14-时间处理.md) | 时间基础/格式化/测量、时间模式 | [chapter14](./code/c/chapter14-time/) |
| 第15章 | [15-文件操作.md](./docs/01-C语言/15-文件操作.md) | 文件读写、二进制文件、文件定位、临时文件 | [chapter15](./code/c/chapter15-file-io/) |
| 第16章 | [16-多文件编程.md](./docs/01-C语言/16-多文件编程.md) | 头文件组织、头文件保护、不透明指针、项目结构 | [chapter16](./code/c/chapter16-multi-file/) |
| 第17章 | [17-编译与链接.md](./docs/01-C语言/17-编译与链接.md) | 编译阶段、符号解析、链接器脚本 | [chapter17](./code/c/chapter17-compile-link/) |
| 第18章 | [18-静态库.md](./docs/01-C语言/18-静态库.md) | 静态库创建与使用、静态库内部原理 | [chapter18](./code/c/chapter18-static-library/) |
| 第19章 | [19-动态库与共享库.md](./docs/01-C语言/19-动态库与共享库.md) | 动态链接、动态加载、动态库内部原理 | [chapter19](./code/c/chapter19-dynamic-library/) |
| 第20章 | [20-GCC与G++编译器.md](./docs/01-C语言/20-GCC与G++编译器.md) | 编译器选项、优化级别、GCC高级用法 | [chapter20](./code/c/chapter20-gcc-g++/) |
| 第21章 | [21-Makefile与构建系统.md](./docs/01-C语言/21-Makefile与构建系统.md) | Makefile语法、依赖管理、Makefile模式 | [chapter21](./code/c/chapter21-makefile/) |
| 第22章 | [22-CMake构建系统.md](./docs/01-C语言/22-CMake构建系统.md) | CMake语法、CMakeLists.txt、CMake模式 | [chapter22](./code/c/chapter22-cmake/) |
| 第23章 | [23-进程与线程.md](./docs/01-C语言/23-进程与线程.md) | fork/exec/pthread/mutex、进程间通信 | [chapter23](./code/c/chapter23-process-thread/) |
| 第24章 | [24-网络编程基础.md](./docs/01-C语言/24-网络编程基础.md) | Socket编程、TCP/UDP客户端服务端、协议设计 | [chapter24](./code/c/chapter24-network/) |
| 第25章 | [25-链表与数据结构.md](./docs/01-C语言/25-链表与数据结构.md) | 单链表、双链表、栈、队列、二叉树 | [chapter25](./code/c/chapter25-data-structure/) |
| 第26章 | [26-排序与查找算法.md](./docs/01-C语言/26-排序与查找算法.md) | 冒泡/快排/归并排序、二分查找、哈希表 | [chapter26](./code/c/chapter26-algorithm/) |
| 第27章 | [27-C语言面向对象实现-基础.md](./docs/01-C语言/27-C语言面向对象实现-基础.md) | 封装、继承、多态、虚表实现 | [chapter27](./code/c/chapter27-c-oop-basic/) |

### 2. 第二部分：C++ 基础

| 章节 | 文档 | 内容概要 | 示例代码 |
|-----|------|---------|---------|
| 第0章 | [00-C++概述.md](./docs/02-CPP/00-C++概述.md) | C++哲学、C与C++对比、C++生态 | [chapter00](./code/cpp/chapter00-cpp-overview/) |
| 第1章 | [01-基础特性.md](./docs/02-CPP/01-基础特性.md) | 引用、auto、new/delete、初始化方式 | [chapter01](./code/cpp/chapter01-basics/) |
| 第2章 | [02-命名空间与编码规范.md](./docs/02-CPP/02-命名空间与编码规范.md) | 命名空间模式、API设计、编码风格 | [chapter02](./code/cpp/chapter02-namespace/) |
| 第3章 | [03-类与对象.md](./docs/02-CPP/03-类与对象.md) | 类基础、成员函数、对象生命周期、三/五法则 | [chapter03](./code/cpp/chapter03-class-object/) |
| 第4章 | [04-继承与多态.md](./docs/02-CPP/04-继承与多态.md) | 继承、多态、虚函数机制、多重继承 | [chapter04](./code/cpp/chapter04-inheritance-polymorphism/) |
| 第5章 | [05-核心机制.md](./docs/02-CPP/05-核心机制.md) | const正确性、类型转换、static、explicit | [chapter05](./code/cpp/chapter05-core-mechanism/) |
| 第6章 | [06-运算符重载与友元.md](./docs/02-CPP/06-运算符重载与友元.md) | 算术/特殊运算符重载、友元、重载陷阱 | [chapter06](./code/cpp/chapter06-operator-overload/) |
| 第7章 | [07-异常处理.md](./docs/02-CPP/07-异常处理.md) | try-catch-throw、异常安全、异常设计 | [chapter07](./code/cpp/chapter07-exception/) |
| 第8章 | [08-智能指针与内存管理.md](./docs/02-CPP/08-智能指针与内存管理.md) | unique_ptr、shared_ptr、智能指针模式与陷阱 | [chapter08](./code/cpp/chapter08-smart-pointer/) |
| 第9章 | [09-移动语义与完美转发.md](./docs/02-CPP/09-移动语义与完美转发.md) | 右值引用、移动语义、完美转发、移动模式 | [chapter09](./code/cpp/chapter09-move-semantics/) |
| 第10章 | [10-模板基础.md](./docs/02-CPP/10-模板基础.md) | 函数模板、类模板、模板推导与特化 | [chapter10](./code/cpp/chapter10-template-basic/) |
| 第11章 | [11-模板进阶.md](./docs/02-CPP/11-模板进阶.md) | 可变参数模板、SFINAE、模板元编程、CTAD | [chapter11](./code/cpp/chapter11-template-advanced/) |
| 第12章 | [12-类型推导.md](./docs/02-CPP/12-类型推导.md) | auto推导、decltype、模板参数推导、推导陷阱 | [chapter12](./code/cpp/chapter12-type-deduction/) |
| 第13章 | [13-Lambda与函数对象.md](./docs/02-CPP/13-Lambda与函数对象.md) | Lambda基础/高级、函数对象、Lambda模式 | [chapter13](./code/cpp/chapter13-lambda/) |
| 第14章 | [14-STL容器.md](./docs/02-CPP/14-STL容器.md) | 序列/关联/无序容器、容器选择与模式 | [chapter14](./code/cpp/chapter14-stl-container/) |
| 第15章 | [15-STL算法与迭代器.md](./docs/02-CPP/15-STL算法与迭代器.md) | 非修改/修改/数值算法、迭代器分类、算法模式 | [chapter15](./code/cpp/chapter15-stl-algorithm/) |
| 第16章 | [16-正则表达式.md](./docs/02-CPP/16-正则表达式.md) | regex语法、匹配搜索替换、正则引擎与模式 | [chapter16](./code/cpp/chapter16-regex/) |
| 第17章 | [17-日期时间库.md](./docs/02-CPP/17-日期时间库.md) | chrono基础/高级、时钟、计时器 | [chapter17](./code/cpp/chapter17-chrono/) |
| 第18章 | [18-文件操作与文件系统.md](./docs/02-CPP/18-文件操作与文件系统.md) | 文件流、文件系统、流操作与高级模式 | [chapter18](./code/cpp/chapter18-file-io/) |
| 第19章 | [19-C++11新特性.md](./docs/02-CPP/19-C++11新特性.md) | auto、右值引用、智能指针、Lambda、核心与库特性 | [chapter19](./code/cpp/chapter19-cpp11/) |
| 第20章 | [20-C++14新特性.md](./docs/02-CPP/20-C++14新特性.md) | 返回类型推导、泛型Lambda、make_unique | [chapter20](./code/cpp/chapter20-cpp14/) |
| 第21章 | [21-C++17新特性.md](./docs/02-CPP/21-C++17新特性.md) | 结构化绑定、optional、variant、string_view | [chapter21](./code/cpp/chapter21-cpp17/) |
| 第22章 | [22-Concepts.md](./docs/02-CPP/22-Concepts.md) | 约束定义、requires子句、自定义概念、概念与模板 | [chapter22](./code/cpp/chapter22-concepts/) |
| 第23章 | [23-Ranges.md](./docs/02-CPP/23-Ranges.md) | 视图组合、管道操作、自定义视图、范围算法 | [chapter23](./code/cpp/chapter23-ranges/) |
| 第24章 | [24-C++20与23新特性.md](./docs/02-CPP/24-C++20与23新特性.md) | Concepts、Ranges、三向比较、span、format、expected、print | [chapter24](./code/cpp/chapter24-cpp20/) |
| 第25章 | [25-协程.md](./docs/02-CPP/25-协程.md) | co_await/co_yield/co_return、Generator、Task | [chapter25](./code/cpp/chapter25-coroutine/) |
| 第26章 | [26-模块.md](./docs/02-CPP/26-模块.md) | module/export/import、模块分区、模块迁移 | [chapter26](./code/cpp/chapter26-modules/) |
| 第27章 | [27-多线程基础.md](./docs/02-CPP/27-多线程基础.md) | 线程创建/管理/参数、线程模式与陷阱 | [chapter27](./code/cpp/chapter27-thread-basic/) |
| 第28章 | [28-线程同步.md](./docs/02-CPP/28-线程同步.md) | 互斥量、条件变量、读写锁、同步模式 | [chapter28](./code/cpp/chapter28-thread-sync/) |
| 第29章 | [29-原子操作与异步编程.md](./docs/02-CPP/29-原子操作与异步编程.md) | atomic、future/promise、async、异步模式 | [chapter29](./code/cpp/chapter29-atomic-async/) |
| 第30章 | [30-线程池实现.md](./docs/02-CPP/30-线程池实现.md) | 线程池设计、任务队列、线程池模式 | [chapter30](./code/cpp/chapter30-thread-pool/) |
| 第31章 | [31-网络编程.md](./docs/02-CPP/31-网络编程.md) | Socket编程、TCP/UDP、网络模式与高级应用 | [chapter31](./code/cpp/chapter31-network/) |
| 第32章 | [32-序列化与日志.md](./docs/02-CPP/32-序列化与日志.md) | JSON/二进制序列化、日志框架设计 | [chapter32](./code/cpp/chapter32-serialization/) |
| 第33章 | [33-包管理工具.md](./docs/02-CPP/33-包管理工具.md) | vcpkg、Conan、CMake集成、依赖管理 | [chapter33](./code/cpp/chapter33-package-manager/) |
| 第34章 | [34-实战案例.md](./docs/02-CPP/34-实战案例.md) | 项目实战、代码走读、职业成长 | [chapter34](./code/cpp/chapter34-practice/) |

### 3. 第三部分：工程实践与开发环境

| 章节 | 文档 | 内容概要 |
|-----|------|---------|
| 第0章 | [00-编码规范.md](./docs/04-工程实践/00-编码规范.md) | 编码风格、命名规范 |
| 第1章 | [01-代码注释规范.md](./docs/04-工程实践/01-代码注释规范.md) | 注释规范、Doxygen文档 |
| 第2章 | [02-设计原则SOLID.md](./docs/04-工程实践/02-设计原则SOLID.md) | 单一职责、开闭原则、里氏替换 |
| 第3章 | [03-设计模式.md](./docs/04-工程实践/03-设计模式.md) | 单例、工厂、装饰器、观察者 |
| 第4章 | [04-设计模式进阶.md](./docs/04-工程实践/04-设计模式进阶.md) | 更高阶的设计模式与应用 |
| 第5章 | [05-单元测试.md](./docs/04-工程实践/05-单元测试.md) | Google Test、断言、测试夹具 |
| 第6章 | [06-调试技巧.md](./docs/04-工程实践/06-调试技巧.md) | GDB调试、内存调试、日志调试 |
| 第7章 | [07-代码审查.md](./docs/04-工程实践/07-代码审查.md) | 代码审查流程、审查要点 |
| 第8章 | [08-性能优化.md](./docs/04-工程实践/08-性能优化.md) | 编译器优化、内存优化、算法优化 |
| 第9章 | [09-算法与数据结构.md](./docs/04-工程实践/09-算法与数据结构.md) | 链表、树、哈希表、排序 |
| 第10章 | [10-陷阱与技巧.md](./docs/04-工程实践/10-陷阱与技巧.md) | 内存陷阱、并发陷阱、调试技巧 |
| 第11章 | [11-实战项目.md](./docs/04-工程实践/11-实战项目.md) | 文本编辑器、网络聊天室 |

#### 开发环境与IDE

| 章节 | 文档 | 内容概要 |
|-----|------|---------|
| 第0章 | [00-VSCode核心配置.md](./docs/04-工程实践/开发环境/00-VSCode核心配置.md) | VS Code配置文件详解、插件关联 |
| 第1章 | [01-VSCode-CMake与远程开发.md](./docs/04-工程实践/开发环境/01-VSCode-CMake与远程开发.md) | CMake集成、远程开发配置 |
| 第2章 | [02-VSCode调试与优化.md](./docs/04-工程实践/开发环境/02-VSCode调试与优化.md) | 调试配置、性能优化 |
| 第3章 | [03-CMake基础入门.md](./docs/04-工程实践/开发环境/03-CMake基础入门.md) | CMake语法、CMakeLists.txt写法 |
| 第4章 | [04-CMake进阶与外部库集成.md](./docs/04-工程实践/开发环境/04-CMake进阶与外部库集成.md) | 外部库配置、CMake进阶技巧 |
| 第5章 | [05-VisualStudio开发环境配置.md](./docs/04-工程实践/开发环境/05-VisualStudio开发环境配置.md) | 项目文件体系、属性配置、外部库 |
| 第6章 | [06-CLion开发环境配置.md](./docs/04-工程实践/开发环境/06-CLion开发环境配置.md) | 工具链配置、CMake配合、调试配置 |
| 第7章 | [07-GCC编译器基础.md](./docs/04-工程实践/开发环境/07-GCC编译器基础.md) | 编译参数、多文件编译、外部库 |
| 第8章 | [08-GCC优化与链接.md](./docs/04-工程实践/开发环境/08-GCC优化与链接.md) | 优化选项、链接管理 |
| 第9章 | [09-GDB调试器配置与使用.md](./docs/04-工程实践/开发环境/09-GDB调试器配置与使用.md) | GDB配置、断点、多线程调试 |
| 第10章 | [10-二进制分析工具.md](./docs/04-工程实践/开发环境/10-二进制分析工具.md) | 二进制分析、反汇编工具 |
| 第11章 | [11-性能调试与链接.md](./docs/04-工程实践/开发环境/11-性能调试与链接.md) | 性能分析工具、链接调试 |
| 第12章 | [12-GCC其他工具.md](./docs/04-工程实践/开发环境/12-GCC其他工具.md) | GCC工具链、构建辅助工具 |

### 4. 第四部分：问题解答

详见 [docs/03-问题解答/](./docs/03-问题解答/) 目录，包含常见问题详细解答，按主题分类：

| 分类 | 文档目录 | 问题数 |
|-----|---------|-------|
| 零基础入门 | [00-零基础入门/](./docs/03-问题解答/00-零基础入门/) | 20 |
| 基础概念 | [01-基础概念/](./docs/03-问题解答/01-基础概念/) | 36 |
| 内存与底层 | [02-内存与底层/](./docs/03-问题解答/02-内存与底层/) | 22 |
| 编译与链接 | [03-编译与链接/](./docs/03-问题解答/03-编译与链接/) | 13 |
| C++核心特性 | [04-CPP核心特性/](./docs/03-问题解答/04-CPP核心特性/) | 40 |
| 模板与泛型 | [05-模板与泛型/](./docs/03-问题解答/05-模板与泛型/) | 7 |
| 并发编程 | [06-并发编程/](./docs/03-问题解答/06-并发编程/) | 32 |
| 现代C++标准库 | [07-现代CPP标准库/](./docs/03-问题解答/07-现代CPP标准库/) | 20 |
| 调试与性能 | [08-调试与性能/](./docs/03-问题解答/08-调试与性能/) | 13 |
| 系统与安全 | [09-系统与安全/](./docs/03-问题解答/09-系统与安全/) | 11 |
| 工程实践 | [10-工程实践/](./docs/03-问题解答/10-工程实践/) | 33 |
| 常见错误与陷阱 | [11-常见错误与陷阱/](./docs/03-问题解答/11-常见错误与陷阱/) | 10 |
| 开发工具与工作流 | [12-开发工具与工作流/](./docs/03-问题解答/12-开发工具与工作流/) | 10 |

### 5. 第五部分：进阶与拓展（选读）

> 以下内容是对核心教程的进阶扩展和拓展阅读，适合有明确需求的读者选读。

#### 模板元编程

| 章节 | 文档 | 内容概要 | 示例代码 |
|-----|------|---------|---------|
| 第0章 | [00-编译期计算基础.md](./docs/05-进阶与拓展/模板元编程/00-编译期计算基础.md) | constexpr/consteval/constinit、编译期if、static_assert | [ch07](./code/advanced/ch07-template-metaprogramming/) |
| 第1章 | [01-Type-Traits与类型操作.md](./docs/05-进阶与拓展/模板元编程/01-Type-Traits与类型操作.md) | type_traits、类型判断/修改/条件traits、enable_if | [ch07](./code/advanced/ch07-template-metaprogramming/) |
| 第2章 | [02-SFINAE与替换失败.md](./docs/05-进阶与拓展/模板元编程/02-SFINAE与替换失败.md) | SFINAE原理、函数重载/偏特化/void_t模式 | [ch07](./code/advanced/ch07-template-metaprogramming/) |
| 第3章 | [03-CRTP与静态多态.md](./docs/05-进阶与拓展/模板元编程/03-CRTP与静态多态.md) | CRTP原理、静态vs动态多态 | [ch07](./code/advanced/ch07-template-metaprogramming/) |
| 第4章 | [04-模板元编程实战.md](./docs/05-进阶与拓展/模板元编程/04-模板元编程实战.md) | 编译期状态机/JSON解析/正则 | [ch07](./code/advanced/ch07-template-metaprogramming/) |

#### 高性能网络

| 章节 | 文档 | 内容概要 | 示例代码 |
|-----|------|---------|---------|
| 第0章 | [00-网络编程进阶概述.md](./docs/05-进阶与拓展/高性能网络/00-网络编程进阶概述.md) | C10K问题、5种编程模型 | [ch08](./code/advanced/ch08-high-perf-network/) |
| 第1章 | [01-IO多路复用深入.md](./docs/05-进阶与拓展/高性能网络/01-IO多路复用深入.md) | select/poll/epoll深度对比 | [ch08](./code/advanced/ch08-high-perf-network/) |
| 第2章 | [02-Reactor模式.md](./docs/05-进阶与拓展/高性能网络/02-Reactor模式.md) | 单/多Reactor架构 | [ch08](./code/advanced/ch08-high-perf-network/) |
| 第3章 | [03-Proactor模式与异步IO.md](./docs/05-进阶与拓展/高性能网络/03-Proactor模式与异步IO.md) | Reactor vs Proactor、io_uring | [ch08](./code/advanced/ch08-high-perf-network/) |
| 第4章 | [04-零拷贝与高效数据传输.md](./docs/05-进阶与拓展/高性能网络/04-零拷贝与高效数据传输.md) | sendfile/splice/mmap | [ch08](./code/advanced/ch08-high-perf-network/) |
| 第5章 | [05-高性能协议设计.md](./docs/05-进阶与拓展/高性能网络/05-高性能协议设计.md) | 二进制/文本协议、TLV | [ch08](./code/advanced/ch08-high-perf-network/) |
| 第6章 | [06-高并发服务器架构.md](./docs/05-进阶与拓展/高性能网络/06-高并发服务器架构.md) | 连接管理、M:N线程模型 | [ch08](./code/advanced/ch08-high-perf-network/) |
| 第7章 | [07-网络性能调优.md](./docs/05-进阶与拓展/高性能网络/07-网络性能调优.md) | TCP参数调优、负载均衡 | [ch08](./code/advanced/ch08-high-perf-network/) |

#### 系统级编程

| 章节 | 文档 | 内容概要 |
|-----|------|---------|
| 第0章 | [00-系统级编程概述.md](./docs/05-进阶与拓展/系统级编程/00-系统级编程概述.md) | 用户态/内核态、系统调用 |
| 第1章 | [01-高级内存管理.md](./docs/05-进阶与拓展/系统级编程/01-高级内存管理.md) | ptmalloc/jemalloc/tcmalloc |
| 第2章 | [02-CPU缓存优化.md](./docs/05-进阶与拓展/系统级编程/02-CPU缓存优化.md) | 缓存行对齐、AoS/SoA |
| 第3章 | [03-SIMD与向量化编程.md](./docs/05-进阶与拓展/系统级编程/03-SIMD与向量化编程.md) | SSE/AVX intrinsics |
| 第4章 | [04-性能剖析与调优方法论.md](./docs/05-进阶与拓展/系统级编程/04-性能剖析与调优方法论.md) | perf/VTune、火焰图 |
| 第5章 | [05-系统级编程实战.md](./docs/05-进阶与拓展/系统级编程/05-系统级编程实战.md) | 高性能日志、DPDK |

#### 安全编程

| 章节 | 文档 | 内容概要 |
|-----|------|---------|
| 第0章 | [00-安全编程概述.md](./docs/05-进阶与拓展/安全编程/00-安全编程概述.md) | 威胁模型、SDL、OWASP |
| 第1章 | [01-内存安全与漏洞防御.md](./docs/05-进阶与拓展/安全编程/01-内存安全与漏洞防御.md) | 栈/堆溢出、ASLR/DEP |
| 第2章 | [02-模糊测试与漏洞挖掘.md](./docs/05-进阶与拓展/安全编程/02-模糊测试与漏洞挖掘.md) | AFL/LibFuzzer、CVE |

#### 编程故事与警示

| 章节 | 文档 | 内容概要 |
|-----|------|---------|
| 第0章 | [00-致命Bug.md](./docs/05-进阶与拓展/编程故事/00-致命Bug.md) | 阿丽亚娜5号/Therac-25/Heartbleed/CrowdStrike等致命Bug |
| 第1章 | [01-安全风暴.md](./docs/05-进阶与拓展/编程故事/01-安全风暴.md) | Morris蠕虫/Stuxnet/SolarWinds/Log4Shell等安全事件 |
| 第2章 | [02-传奇时刻.md](./docs/05-进阶与拓展/编程故事/02-传奇时刻.md) | 第一个Bug/Unix/Linux/C语言/Git等传奇故事 |
| 第3章 | [03-惨痛教训.md](./docs/05-进阶与拓展/编程故事/03-惨痛教训.md) | Knight Capital/波音737 MAX/Cloudflare等工程失败 |
| 第4章 | [04-代码警示录.md](./docs/05-进阶与拓展/编程故事/04-代码警示录.md) | goto fail/rm -rf/整数溢出/left-pad等小错误大灾难 |
| 第5章 | [05-代码暗器.md](./docs/05-进阶与拓展/编程故事/05-代码暗器.md) | 逻辑炸弹/后门/供应链投毒/xz后门/Ken Thompson后门 |
| 第6章 | [06-计算机破坏武器库.md](./docs/05-进阶与拓展/编程故事/06-计算机破坏武器库.md) | 病毒/蠕虫/木马/勒索/间谍/Rootkit/僵尸网络/DDoS |
| 第7章 | [07-编程竞赛传奇.md](./docs/05-进阶与拓展/编程故事/07-编程竞赛传奇.md) | tourist/ICPC之神/中国信奥崛起/LeetCode文化 |
| 第8章 | [08-开源恩怨录.md](./docs/05-进阶与拓展/编程故事/08-开源恩怨录.md) | Node.js分裂/Redis许可证/Elastic vs AWS/React专利 |
| 第9章 | [09-AI伦理与失控风险.md](./docs/05-进阶与拓展/编程故事/09-AI伦理与失控风险.md) | Tay变恶/COMPAS偏见/Deepfake/自动驾驶致死 |
| 第10章 | [10-数据隐私与监控社会.md](./docs/05-进阶与拓展/编程故事/10-数据隐私与监控社会.md) | 斯诺登/剑桥分析/苹果vs FBI/人脸识别/GDPR |
| 第11章 | [11-编程未解之谜.md](./docs/05-进阶与拓展/编程故事/11-编程未解之谜.md) | 中本聪/Cicada 3301/The DAO/Duqu/Kryptos |
| 第12章 | [12-代码考古.md](./docs/05-进阶与拓展/编程故事/12-代码考古.md) | 阿波罗源码/COBOL遗产/Y2K38/Linux内核演进 |
| 第13章 | [13-技术伦理与程序员责任.md](./docs/05-进阶与拓展/编程故事/13-技术伦理与程序员责任.md) | Theranos/波音MCAS/吹哨人/武器级软件/伦理决策框架 |
| 第14章 | [14-程序员文化传奇.md](./docs/05-进阶与拓展/编程故事/14-程序员文化传奇.md) | Vim vs Emacs/Tab vs Space/北极代码库/著名注释 |

---

## 3. 学习路线图

```
┌───────────────────────────────────────────────────────────────────────┐
│                        完整学习路线                                    │
├───────────────────────────────────────────────────────────────────────┤
│                                                                       │
│  ┌─────────────┐                                                      │
│  │ C语言基础    │  00-27章：概述→类型→运算符→控制→函数→数组→指针        │
│  │             │  →字符串→结构体→内存→预处理→可变参数→位操作            │
│  │             │  →错误处理→时间→文件→多文件→编译链接→库→构建系统       │
│  │             │  →进程线程→网络→数据结构→算法→OOP                      │
│  └──────┬──────┘                                                      │
│         │                                                             │
│         ▼                                                             │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐              │
│  │  C++基础    │───▶│  类与对象    │───▶│  核心机制   │              │
│  │   00-01章   │    │   03-04章    │    │   05-06章   │              │
│  └─────────────┘    └─────────────┘    └─────────────┘              │
│         │                   │                   │                    │
│         ▼                   ▼                   ▼                    │
│  ┌─────────────────────────────────────────────────────────────┐     │
│  │              异常处理与智能指针                                 │     │
│  │              07-08章：异常安全、unique_ptr、shared_ptr         │     │
│  └─────────────────────────────────────────────────────────────┘     │
│                              │                                        │
│                              ▼                                        │
│  ┌─────────────────────────────────────────────────────────────┐     │
│  │              移动语义与模板泛型                                 │     │
│  │              09-13章：移动语义、模板、类型推导、Lambda          │     │
│  └─────────────────────────────────────────────────────────────┘     │
│                              │                                        │
│                              ▼                                        │
│  ┌─────────────────────────────────────────────────────────────┐     │
│  │              STL与标准库                                       │     │
│  │              14-18章：容器、算法、正则、chrono、文件与文件系统     │     │
│  └─────────────────────────────────────────────────────────────┘     │
│                              │                                        │
│                              ▼                                        │
│  ┌─────────────────────────────────────────────────────────────┐     │
│  │              C++11/14/17/20与23 新特性                            │     │
│  │              19-26章：各版本特性、Concepts、Ranges、协程、模块     │     │
│  └─────────────────────────────────────────────────────────────┘     │
│                              │                                        │
│                              ▼                                        │
│  ┌─────────────────────────────────────────────────────────────┐     │
│  │                    并发编程                                    │     │
│  │              27-30章：线程、同步、原子、线程池                     │     │
│  └─────────────────────────────────────────────────────────────┘     │
│                              │                                        │
│                              ▼                                        │
│  ┌─────────────────────────────────────────────────────────────┐     │
│  │                    系统编程与实战                               │     │
│  │              31-34章：网络、序列化与日志、包管理、实战案例         │     │
│  └─────────────────────────────────────────────────────────────┘     │
│                              │                                        │
│                              ▼                                        │
│  ┌─────────────────────────────────────────────────────────────┐     │
│  │                    工程实践                                    │     │
│  │              00-15章：编码规范、设计模式、调试优化               │     │
│  └─────────────────────────────────────────────────────────────┘     │
│                                                                       │
└───────────────────────────────────────────────────────────────────────┘
```

---

## 4. 知识依赖关系图

```
                        知识点依赖关系（学习前置条件）

        ┌──────────────────────────────────────────────────────┐
        │                     C语言基础 (00-27章)              │
        │  00概述 → 01类型 → 02运算符 → 03控制 → 04函数        │
        │     ↓                              ↓                 │
        │  05数组 → 06指针 → 07字符串 → 08结构体 → 09内存       │
        │     ↓                              ↓                 │
        │  10预处理 → 11可变参数 → 12位操作 → 13错误 → 14时间   │
        │     ↓                              ↓                 │
        │  15文件 → 16多文件 → 17编译链接 → 18-19库              │
        │     ↓                              ↓                 │
        │  20GCC → 21Makefile → 22CMake → 23进程线程 → 24网络  │
        │     ↓                              ↓                 │
        │  25数据结构 → 26算法 → 27OOP基础                      │
        └──────────────────────────────────────────────────────┘
                              │
                              ▼
        ┌──────────────────────────────────────────────────────┐
        │                    C++基础 (00-34章)                  │
        │                      00概述 → 01基础                  │
        │                         │                              │
        │     ┌───────────────────┼───────────────────┐        │
        │     ▼                   ▼                   ▼        │
        │  02命名空间         03类与对象          04继承与多态   │
        │                         │                              │
        │     ┌───────────────────┼───────────────────┐        │
        │     ▼                   ▼                   ▼        │
        │  05核心机制         06运算符重载         07异常处理     │
        │                         │                              │
        │                         ▼                              │
        │                    08智能指针                           │
        │                         │                              │
        │     ┌───────────────────┼───────────────────┐        │
        │     ▼                   ▼                   ▼        │
        │  09移动语义          10模板基础          11模板进阶     │
        │                                                     │
        │     ┌───────────────────┼───────────────────┐        │
        │     ▼                   ▼                   ▼        │
        │  12类型推导          13Lambda          14STL容器       │
        │                                                     │
        │     ┌───────────────────┼───────────────────┐        │
        │     ▼                   ▼                   ▼        │
        │  15STL算法          16正则              17日期时间     │
        │                                                     │
        │     ┌───────────────────┼───────────────────┐        │
        │     ▼                   ▼                   ▼        │
        │  18文件操作与文件系统                    19C++11       │
        │                                                     │
        │                         ▼                           │
        │                    20C++14 → 21C++17                │
        │                                                     │
        │     ┌───────────────────┼───────────────────┐        │
        │     ▼                   ▼                   ▼        │
        │  22Concepts         23Ranges           24C++20与23    │
        │                                                     │
        │     ┌───────────────────┼───────────────────┐        │
        │     ▼                   ▼                   ▼        │
        │  25协程              26模块                            │
        │                                                     │
        │               ┌─────────────────┐                   │
        │               │  并发编程 27-30  │                   │
        │               └────────┬────────┘                   │
        │                        │                             │
        │               ┌─────────────────┐                   │
        │               │  系统编程 31-34  │                   │
        │               └────────┬────────┘                   │
        │                        │                             │
        └────────────────────────┼─────────────────────────────┘
                                 ▼
        ┌──────────────────────────────────────────────────────┐
        │                    工程实践 (00-15章)                  │
        │  00编码规范 → 01注释 → 02SOLID → 03设计模式            │
        │     ↓                              ↓                   │
        │  04设计模式进阶 → 05关键字 → 06单元测试 → 07调试        │
        │     ↓                              ↓                   │
        │  08代码审查 → 09性能优化 → 10算法 → 11陷阱与技巧        │
        │     ↓                              ↓                   │
        │  12实战项目 → 13创新实践 → 14思维方法论 → 15能力图谱    │
        └──────────────────────────────────────────────────────┘
```

---

## 5. 快速参考卡片

### 1. C vs C++ 对照表

| C语言 | C++ | 说明 |
|------|-----|------|
| `printf/scanf` | `cout/cin` | 类型安全的I/O |
| `malloc/free` | `new/delete` | 类型安全的内存管理 |
| `#define` | `const/constexpr` | 编译期常量 |
| 函数指针 | Lambda表达式 | 匿名函数 |
| 手动实现数据结构 | STL容器 | 开箱即用 |
| 无命名空间 | `namespace` | 避免命名冲突 |

### 2. C++版本特性速查

| 版本 | 核心特性 |
|------|----------|
| **C++11** | auto、右值引用、智能指针、Lambda、constexpr |
| **C++14** | 返回类型推导、泛型Lambda、make_unique |
| **C++17** | 结构化绑定、optional、variant、string_view、并行算法 |
| **C++20** | Concepts、Ranges、三向比较、span、format、Coroutines |
| **C++23** | std::expected、std::print、flat_map、deducing this |

---

## 6. 推荐学习资源

| 类型 | 资源 | 链接 |
|-----|------|------|
| C语言教程 | C语言中文网 | https://c.biancheng.net/ |
| C语言参考 | cppreference C | https://en.cppreference.com/w/c |
| 在线教程 | LearnCpp | https://www.learncpp.com/ |
| 参考手册 | cppreference | https://en.cppreference.com/ |
| C++20特性 | C++20参考 | https://en.cppreference.com/w/cpp/20 |
| 练习平台 | LeetCode | https://leetcode.cn/ |
| 编码规范 | Google C++ Style | https://google.github.io/styleguide/cppguide.html |
| ISO标准文档 | std_docs | [docs/std_docs/](./docs/std_docs/) — ISO C (C99/C11/C17/C23) 和 ISO C++ (C++98~C++23) 标准参考文献 |

---

## 7. 编译器支持

| 编译器 | C++20支持 | 推荐版本 |
|-------|----------|---------|
| GCC | 良好 | GCC 11+ |
| Clang | 良好 | Clang 14+ |
| MSVC | 优秀 | VS 2019 16.10+ |

---

## 8. 示例代码

本教程提供完整的可运行示例代码，位于 [code](./code/) 目录。

### 1. 项目代码结构

```
code/
├── c/                                # C语言示例代码（31章目录，160个源文件）
│   ├── chapter00-c-overview/         # C语言概述
│   ├── chapter01-data-types/         # 数据类型与变量
│   ├── chapter02-operators/          # 运算符与表达式
│   ├── chapter03-control-structure/  # 控制结构
│   ├── chapter04-function/           # 函数
│   ├── chapter05-array/              # 数组
│   ├── chapter06-pointer/            # 指针
│   ├── chapter07-string/             # 字符串处理
│   ├── chapter08-struct-union/       # 结构体与联合体
│   ├── chapter09-memory-management/  # 内存管理
│   ├── chapter10-preprocessor/       # 预处理器
│   ├── chapter11-variadic/           # 可变参数与命令行
│   ├── chapter12-bitwise/            # 位操作
│   ├── chapter13-error-signal/       # 错误处理与信号
│   ├── chapter14-time/               # 时间处理
│   ├── chapter15-file-io/            # 文件操作
│   ├── chapter16-multi-file/         # 多文件编程
│   ├── chapter17-compile-link/       # 编译与链接
│   ├── chapter18-static-library/     # 静态库
│   ├── chapter19-dynamic-library/    # 动态库与共享库
│   ├── chapter20-gcc-g++/            # GCC与G++编译器
│   ├── chapter21-makefile/           # Makefile与构建系统
│   ├── chapter22-cmake/              # CMake构建系统
│   ├── chapter23-process-thread/     # 进程与线程
│   ├── chapter24-network/            # 网络编程基础
│   ├── chapter25-data-structure/     # 链表与数据结构
│   ├── chapter26-algorithm/          # 排序与查找算法
│   ├── chapter27-c-oop-basic/        # C语言面向对象-基础
│   ├── chapter28-dynamic-loading/    # 动态库加载方式
│   ├── chapter29-c-oop-advanced/     # C语言面向对象-进阶
│   ├── chapter30-c-patterns/         # C语言设计模式
│   └── CMakeLists.txt
│
├── cpp/                              # C++示例代码（39章，163个源文件）
│   ├── chapter00-cpp-overview/       # C++概述
│   ├── chapter01-basics/             # 基础特性
│   ├── chapter02-namespace/          # 命名空间与编码规范
│   ├── chapter03-class-object/       # 类与对象
│   ├── chapter04-inheritance-polymorphism/ # 继承与多态
│   ├── chapter05-core-mechanism/     # 核心机制
│   ├── chapter06-operator-overload/  # 运算符重载与友元
│   ├── chapter07-exception/          # 异常处理
│   ├── chapter08-smart-pointer/      # 智能指针与内存管理
│   ├── chapter09-move-semantics/     # 移动语义与完美转发
│   ├── chapter10-template-basic/     # 模板基础
│   ├── chapter11-template-advanced/  # 模板进阶
│   ├── chapter12-type-deduction/     # 类型推导
│   ├── chapter13-lambda/             # Lambda与函数对象
│   ├── chapter14-stl-container/      # STL容器
│   ├── chapter15-stl-algorithm/      # STL算法与迭代器
│   ├── chapter16-regex/              # 正则表达式
│   ├── chapter17-chrono/             # 日期时间库
│   ├── chapter18-file-io/            # 文件操作与文件系统
│   ├── chapter19-cpp11/              # C++11新特性
│   ├── chapter20-cpp14/              # C++14新特性
│   ├── chapter21-cpp17/              # C++17新特性
│   ├── chapter22-concepts/           # Concepts
│   ├── chapter23-ranges/             # Ranges
│   ├── chapter24-cpp20/              # C++20与23新特性
│   ├── chapter25-coroutine/          # 协程
│   ├── chapter26-modules/            # 模块
│   ├── chapter27-thread-basic/       # 多线程基础
│   ├── chapter28-thread-sync/        # 线程同步
│   ├── chapter29-atomic-async/       # 原子操作与异步编程
│   ├── chapter30-thread-pool/        # 线程池实现
│   ├── chapter31-network/            # 网络编程
│   ├── chapter32-serialization/      # 序列化与日志
│   ├── chapter33-package-manager/    # 包管理工具
│   ├── chapter34-practice/           # 实战案例
│   ├── chapter35-filesystem/         # 文件系统库
│   ├── chapter36-cpp23/              # C++23新特性
│   ├── chapter37-memory-model/       # C++内存模型
│   ├── chapter38-lock-free/          # 无锁编程
│   └── CMakeLists.txt
│
├── advanced/                         # 高级专题示例代码
│   ├── ch07-template-metaprogramming/ # 模板元编程与编译期计算（14个源文件）
│   ├── ch08-high-perf-network/       # 高性能网络与异步IO（14个源文件）
│   └── CMakeLists.txt
│
├── CMakeLists.txt                    # 顶层CMake配置
├── build.bat                         # Windows构建脚本
└── clear.bat                         # 清理脚本
```

### 2. 代码文件命名规范

每个章节内的代码文件遵循 `序号_类型_主题.扩展名` 的命名规范：

- **example**：基础示例代码，如 `01_example_pointer_basics.c`
- **deep_dive**：深入讲解代码，如 `01_deep_dive_pointer_patterns.c`

### 3. 工具库

项目提供了两套工具库，位于 [utils](./utils/) 目录：

| 工具库 | 路径 | 语言 | 标准 | 功能 |
|-------|------|------|------|------|
| cu | [utils/cu/](./utils/cu/) | C | C17 | 字符串处理、文件操作、编码、随机数、集合操作、日期时间、哈希、配置解析、日志、断言、进程工具 |
| cxxu | [utils/cxxu/](./utils/cxxu/) | C++ | C++20 | 字符串处理、文件操作、编码、随机数、集合操作、日期时间、哈希、配置解析、日志、JSON、进程工具 |

#### 工具库结构

```
utils/
├── cu/                                # C 工具库
│   ├── include/cu/                    # 头文件
│   │   ├── export.h                   # 导出宏 (CU_API)
│   │   ├── utils.h                    # 统一入口
│   │   ├── string_utils.h             # 字符串处理
│   │   ├── file_utils.h               # 文件操作
│   │   ├── encoding_utils.h           # 编码转换 (Base64/URL/Hex)
│   │   ├── collection_utils.h         # 集合与列表运算
│   │   ├── number_utils.h             # 数值转换
│   │   ├── random_utils.h             # 随机数与UUID
│   │   ├── process_utils.h            # 进程工具
│   │   ├── date_time_utils.h          # 日期时间
│   │   ├── hash_utils.h               # 哈希工具
│   │   ├── config_utils.h             # 配置文件解析
│   │   ├── log_utils.h                # 日志工具
│   │   ├── assert_utils.h             # 断言工具
│   │   └── constants.h                # 常量定义
│   ├── src/                           # 源文件
│   ├── tests/                         # 测试（含断言宏与通过率统计）
│   ├── CMakeLists.txt                 # CMake 构建
│   └── build.bat                      # 编译脚本（调用 build_common.bat）
│
└── cxxu/                              # C++ 工具库
    ├── include/cu_utils/              # 头文件
    │   ├── export.h                   # 导出宏 (CXXU_API)
    │   ├── utils.h                    # 统一入口 (Utils 类)
    │   ├── string_utils.h             # 字符串处理
    │   ├── file_utils.h               # 文件操作
    │   ├── encoding_utils.h           # 编码转换
    │   ├── collection_utils.h         # 集合与列表运算
    │   ├── number_utils.h             # 数值转换
    │   ├── random_utils.h             # 随机数与UUID
    │   ├── process_utils.h            # 进程工具
    │   ├── date_time_utils.h          # 日期时间
    │   ├── hash_utils.h               # 哈希工具
    │   ├── config_utils.h             # 配置文件解析
    │   ├── log_utils.h                # 日志工具（线程安全，含时间戳）
    │   ├── json_utils.h               # JSON解析
    │   └── constants.h                # 常量定义
    ├── src/                           # 源文件
    ├── test/                          # 测试（含断言宏与通过率统计）
    ├── CMakeLists.txt                 # CMake 构建
    └── build.bat                      # 编译脚本（调用 build_common.bat）
└── build_common.bat                   # 公共构建脚本（参数解析、CMake配置）
```

#### 编译工具库

每个工具库都有独立的 `build.bat` 脚本（内部调用公共的 `build_common.bat`），支持静态库和动态库编译。

**参数说明：**

| 参数 | 说明 |
|------|------|
| `static` | 编译静态库（默认），生成 `.a` / `.lib` 文件 |
| `shared` | 编译动态库（DLL），生成 `.dll` + `.a` / `.dll` + `.lib` 文件 |
| `debug` | Debug 模式（默认），包含调试信息，不优化 |
| `release` | Release 模式，开启优化，无调试信息 |
| `test` | 编译完成后自动运行测试程序 |
| `clean` | 删除 `build/` 目录，清理所有构建产物 |

> 参数可自由组合，顺序不限。不带参数时默认编译静态库 + Debug 模式。

**cu（C 工具库）：**

```bat
cd utils\cu

build.bat                        # 静态库 + Debug（默认）
build.bat shared                 # 动态库 + Debug
build.bat static release         # 静态库 + Release（优化编译）
build.bat shared test            # 动态库 + Debug + 运行测试
build.bat shared release test    # 动态库 + Release + 运行测试
build.bat clean                  # 清理构建目录
```

**cxxu（C++ 工具库）：**

```bat
cd utils\cxxu

build.bat                        # 静态库 + Debug（默认）
build.bat shared                 # 动态库 + Debug
build.bat static release         # 静态库 + Release（优化编译）
build.bat shared test            # 动态库 + Debug + 运行测试
build.bat shared release test    # 动态库 + Release + 运行测试
build.bat clean                  # 清理构建目录
```

> **编译器自动检测**：脚本会按顺序检测 `CMAKE_GENERATOR` 环境变量 → Visual Studio → MinGW，自动选择合适的 CMake 生成器。如需指定生成器，可设置环境变量：`set CMAKE_GENERATOR=MinGW Makefiles`

也可使用 CMake 手动编译：

```bash
# 静态库
cd utils/cu && mkdir build && cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build . --target cu_static

# 动态库
cmake --build . --target cu_shared

# 运行测试
cmake --build . --target test_cu && ./test_cu
```

#### 构建产物

| 库 | 静态库 | 动态库 | 测试 |
|---|--------|--------|------|
| cu | `build/libcu.a` | `build/cu.dll` | `build/test_cu.exe` |
| cxxu | `build/libcxxu.a` | `build/cxxu.dll` | `build/test_cxxu.exe` |

#### 测试框架

测试程序使用内置的轻量断言框架（不依赖外部测试库），提供：

- **`TEST_ASSERT(condition, message)`** — 通用条件断言
- **`TEST_ASSERT_EQ(expected, actual, message)`** — 相等断言（cu 为整数版，cxxu 为模板版）
- **`TEST_ASSERT_STR_EQ(expected, actual, message)`** — 字符串相等断言
- **通过率统计** — 测试结束时输出总数、通过数、失败数及通过率百分比
- **退出码** — 全部通过返回 0，存在失败返回 1

#### 使用工具库

**C 项目中使用 cu：**

```c
#include "cu/utils.h"

int main() {
    // 字符串操作
    char* hex = string_to_hex("Hello");
    printf("Hex: %s\n", hex);
    free(hex);

    // 文件操作
    StringArray* lines = read_lines("config.txt");
    for (size_t i = 0; i < lines->size; i++) {
        printf("%s\n", lines->elements[i]);
    }
    string_array_destroy(lines);

    // 随机数
    random_init();
    printf("Random: %d\n", random_int(1, 100));
    return 0;
}
```

编译链接：
```bash
gcc -std=c17 main.c -I utils/cu/include -L utils/cu/build -lcu -o main
# 动态库链接
gcc -std=c17 main.c -I utils/cu/include -L utils/cu/build -lcu -o main -DCU_DLL
```

**C++ 项目中使用 cxxu：**

```cpp
#include "cu_utils/utils.h"

int main() {
    // 字符串操作
    std::string hex = cu::Utils::stringToHex("Hello");
    std::cout << "Hex: " << hex << std::endl;

    // 文件操作
    auto lines = cu::Utils::readFile("config.txt", true, "UTF-8");
    for (const auto& line : lines) {
        std::cout << line << std::endl;
    }

    // 随机数
    std::cout << "UUID: " << cu::Utils::generateUuid() << std::endl;
    std::cout << "Random: " << cu::Utils::randomInt(1, 100) << std::endl;

    // 日期时间
    std::cout << "Now: " << cu::Utils::getDateTime() << std::endl;
    return 0;
}
```

编译链接：
```bash
g++ -std=c++20 main.cpp -I utils/cxxu/include -L utils/cxxu/build -lcxxu -o main
# 动态库链接
g++ -std=c++20 main.cpp -I utils/cxxu/include -L utils/cxxu/build -lcxxu -o main -DCXXU_DLL
```

### 4. 快速开始

```bash
# 进入代码目录
cd code

# 使用CMake编译（推荐）
mkdir build && cd build
cmake ..
cmake --build .

# Windows 用户可使用构建脚本
cd ..
build.bat

# 运行示例（在 build 目录下）
./c_ch00_01_example_hello_world
```

---

## 9. 学习建议

- **基础路径**：C语言基础(00-27) → C++基础(00-34) → 工程实践(00-15)
- **快速路径**：有C语言基础可直接从C++第0章开始
- **重点章节**：C++第3章（类与对象）、第8章（智能指针）、第9章（移动语义）、第27-28章（多线程）、工程实践第2-3章（设计模式）

---

## 10. 项目统计

| 模块 | 章节数 | 文档数 | 代码文件数 |
|-----|-------|-------|-----------|
| C语言（含导学） | 28+3 | 31 | 160 |
| C++ | 35 | 35 | 163 |
| 工程实践（含开发环境） | 12+13 | 25 | — |
| 问题解答 | 13分类 | 266 | — |
| 进阶与拓展 | 5子分类 | 37 | 28 |
