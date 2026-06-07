# C++ GUI编程入门
> 📖 相关章节：[C++概述](../../02-CPP/00-C++概述.md)

> "命令行是你告诉程序做什么，GUI是程序告诉你它能做什么——然后你随时可能点任何按钮。"——从命令行思维转向事件驱动思维，是GUI编程的核心挑战。

***

## 1. 什么是GUI编程

### 1. 定义

**GUI**（Graphical User Interface，图形用户界面）是通过图形元素（窗口、按钮、文本框等）与用户交互的程序界面。与之相对的是CLI（Command Line Interface，命令行界面）。

```
CLI程序：
  用户输入命令 → 程序执行 → 输出结果 → 等待下一条命令
  线性流程，一步一步来

GUI程序：
  程序显示界面 → 用户随时点击任何按钮 → 程序响应 → 继续等待
  事件驱动，随时响应
```

### 2. 命令行 vs 图形界面

| 维度 | CLI | GUI |
|------|-----|-----|
| 交互方式 | 键盘输入文本 | 鼠标点击/拖拽/键盘 |
| 流程控制 | 线性——程序决定下一步 | 非线性——用户决定下一步 |
| 编程模型 | 顺序执行 | 事件循环 |
| 学习曲线 | 低（简单程序） | 高（需要理解事件驱动） |
| 开发效率 | 快速原型 | 界面开发耗时 |
| 用户体验 | 需要记忆命令 | 直观易上手 |
| 适用场景 | 服务器/脚本/工具 | 桌面应用/游戏/工具软件 |

### 3. GUI程序的本质

**GUI程序 = 事件循环 + 消息驱动**

```
GUI程序的核心循环：

  while (程序运行中) {
      event = 获取下一个事件();     // 等待用户操作
      switch (event.type) {
          case 鼠标点击:  处理点击(event);   break;
          case 键盘输入:  处理按键(event);   break;
          case 窗口重绘:  重绘界面(event);   break;
          case 定时器:    处理定时(event);    break;
          ...
      }
  }

  你不主动调用代码，是事件在调用你的代码——这就是"事件驱动"
```

***

## 2. GUI程序的基本结构

### 1. 事件循环

事件循环是GUI程序的心脏，它不断从事件队列中取出事件并分发给对应的处理函数。

```
事件循环流程：

  ┌─────────────────────────────────────────┐
  │              事件循环                    │
  │                                         │
  │   事件队列 ──→ 取出事件 ──→ 分发事件     │
  │   ┌──────┐     │            │           │
  │   │鼠标  │     │            ▼           │
  │   │键盘  │─────┘     调用处理函数        │
  │   │定时器│           (回调/槽函数)       │
  │   │重绘  │               │              │
  │   │关闭  │               ▼              │
  │   └──────┘         更新界面状态          │
  │                         │              │
  │                         ▼              │
  │                    回到事件循环 ←────────┘
  └─────────────────────────────────────────┘
```

### 2. 核心概念

| 概念 | 说明 | 比喻 |
|------|------|------|
| **窗口（Window）** | 屏幕上的矩形区域，是控件的容器 | 房间 |
| **控件（Widget）** | 按钮/文本框/列表等交互元素 | 房间里的家具 |
| **布局（Layout）** | 控件在窗口中的排列方式 | 家具的摆放方式 |
| **事件（Event）** | 用户的操作（点击/按键/拖拽） | 有人按门铃 |
| **信号（Signal）** | 控件发出的通知（被点击/值改变） | 门铃响了 |
| **槽（Slot）** | 响应信号的函数 | 你去开门 |

### 3. GUI程序的main函数模式

```cpp
// 通用GUI程序的main函数模式
int main(int argc, char *argv[]) {
    Application app(argc, argv);

    Window window;
    window.setTitle("我的第一个GUI");
    window.resize(800, 600);

    window.show();

    return app.exec();
}
```

```
main函数做了什么：

  1. 创建Application对象 ─── 初始化GUI框架
  2. 创建Window ─── 构建界面
  3. window.show() ─── 显示窗口
  4. app.exec() ─── 进入事件循环（阻塞在这里）
       │
       └── 循环处理事件，直到窗口关闭
           └── 返回退出码
```

***

## 3. Qt框架入门

### 1. Qt的核心机制

| 机制 | 说明 |
|------|------|
| **信号与槽** | 对象间通信机制，替代回调函数 |
| **MOC** | 元对象编译器，为C++增加信号槽等元信息 |
| **父子对象树** | 父对象自动销毁子对象，简化内存管理 |

**信号与槽**：

```
信号与槽 = 观察者模式的类型安全实现

  [按钮] ─── 发出信号 clicked() ───→ [处理函数] ─── 槽函数 onButtonClicked()

  连接方式：
  QObject::connect(sender, &Sender::signal, receiver, &Receiver::slot);

  特点：
  ├── 一个信号可以连接多个槽（一对多）
  ├── 多个信号可以连接同一个槽（多对一）
  ├── 信号可以连接信号（信号转发）
  └── 连接可以断开
```

**MOC（Meta-Object Compiler）**：

```
MOC是Qt的代码预处理器：

  你的头文件(.h) ──→ MOC ──→ moc_*.cpp ──→ 编译 ──→ 链接

  MOC为包含Q_OBJECT宏的类生成：
  ├── 信号函数的实现
  ├── 元对象信息（类名/属性/方法）
  └── 动态属性系统

  这就是为什么Qt扩展了C++——MOC在编译前预处理
```

**父子对象树**：

```
Qt的内存管理——父子对象树：

  QApplication
      │
      ├── QMainWindow
      │       │
      │       ├── QWidget (中央控件)
      │       │       │
      │       │       ├── QPushButton
      │       │       ├── QLabel
      │       │       └── QLineEdit
      │       │
      │       └── QStatusBar
      │
      └── QDialog

  规则：父对象销毁时，自动销毁所有子对象
  → 你只需要关心顶层对象的销毁
  → 子控件会随父控件自动销毁
```

### 2. 创建第一个Qt窗口

```cpp
#include <QApplication>
#include <QMainWindow>
#include <QLabel>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle("第一个Qt窗口");
    window.resize(400, 300);

    QLabel *label = new QLabel("你好，Qt！", &window);
    label->setAlignment(Qt::AlignCenter);
    window.setCentralWidget(label);

    window.show();
    return app.exec();
}
```

### 3. 信号与槽示例

```cpp
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        auto *central = new QWidget(this);
        auto *layout = new QVBoxLayout(central);

        label = new QLabel("点击按钮试试", central);
        label->setAlignment(Qt::AlignCenter);

        auto *button = new QPushButton("点我", central);

        layout->addWidget(label);
        layout->addWidget(button);
        setCentralWidget(central);

        connect(button, &QPushButton::clicked, this, &MainWindow::onClicked);
    }

private slots:
    void onClicked() {
        count++;
        label->setText(QString("你点击了 %1 次").arg(count));
    }

private:
    QLabel *label;
    int count = 0;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}
```

### 4. 布局管理

```
Qt布局管理器：

  QHBoxLayout（水平布局）：
  ┌────────┬────────┬────────┐
  │ 控件A  │ 控件B  │ 控件C  │
  └────────┴────────┴────────┘

  QVBoxLayout（垂直布局）：
  ┌────────────────────┐
  │     控件A          │
  ├────────────────────┤
  │     控件B          │
  ├────────────────────┤
  │     控件C          │
  └────────────────────┘

  QGridLayout（网格布局）：
  ┌────────┬────────┐
  │ (0,0)  │ (0,1)  │
  ├────────┼────────┤
  │ (1,0)  │ (1,1)  │
  └────────┴────────┘

  布局可以嵌套：
  QVBoxLayout
    ├── QHBoxLayout
    │     ├── QPushButton
    │     └── QPushButton
    ├── QTextEdit
    └── QHBoxLayout
          ├── QPushButton
          └── QPushButton
```

```cpp
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QWidget window;
    auto *mainLayout = new QVBoxLayout(&window);

    auto *topLayout = new QHBoxLayout;
    topLayout->addWidget(new QPushButton("新建"));
    topLayout->addWidget(new QPushButton("打开"));
    topLayout->addWidget(new QPushButton("保存"));

    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(new QTextEdit);

    auto *bottomLayout = new QHBoxLayout;
    bottomLayout->addStretch();
    bottomLayout->addWidget(new QPushButton("确定"));
    bottomLayout->addWidget(new QPushButton("取消"));

    mainLayout->addLayout(bottomLayout);

    window.setWindowTitle("布局示例");
    window.resize(500, 400);
    window.show();

    return app.exec();
}
```

### 5. 常用控件

| 控件 | 类名 | 用途 |
|------|------|------|
| 标签 | `QLabel` | 显示文本/图片 |
| 按钮 | `QPushButton` | 点击触发操作 |
| 单行输入 | `QLineEdit` | 输入一行文本 |
| 多行输入 | `QTextEdit` | 输入多行文本（支持富文本） |
| 列表 | `QListView` | 显示列表数据 |
| 下拉框 | `QComboBox` | 下拉选择 |
| 复选框 | `QCheckBox` | 多选 |
| 单选框 | `QRadioButton` | 单选 |
| 滑块 | `QSlider` | 拖动选择数值 |
| 进度条 | `QProgressBar` | 显示进度 |
| 表格 | `QTableView` | 表格数据 |
| 树形 | `QTreeView` | 层级数据 |

### 6. Qt Creator使用

```
Qt Creator 工作流：

  1. 新建项目 ─── Qt Widgets Application
  2. 设计界面 ─── 拖拽控件到窗体（.ui文件）
  3. 编辑代码 ─── 在.h/.cpp中编写信号槽逻辑
  4. 构建运行 ─── Ctrl+R 编译运行
  5. 调试 ─── F5 启动调试

  .ui文件 ─── XML格式描述界面 ─── uic编译为C++代码
  .h文件 ─── 类声明（含Q_OBJECT宏）
  .cpp文件 ─── 类实现
```

***

## 4. ImGui入门

### 1. 即时模式GUI

**即时模式GUI（Immediate Mode GUI）**：每帧重新绘制整个界面，不保留控件状态。

```
即时模式 vs 保留模式：

  保留模式（Qt/MFC）：
    创建按钮 → 设置属性 → 注册回调 → 框架管理控件生命周期
    控件是"持久对象"，框架帮你管理

  即时模式（ImGui）：
    每帧调用 ImGui::Button("点击") → 返回是否被点击
    控件是"瞬时"的，每帧重新创建
    你完全控制状态

  代码对比：

  保留模式（Qt）：
    button = new QPushButton("点击");
    connect(button, &QPushButton::clicked, []{ count++; });

  即时模式（ImGui）：
    if (ImGui::Button("点击")) count++;
```

### 2. ImGui适用场景

| 适合 | 不适合 |
|------|--------|
| 游戏调试工具 | 正式产品界面 |
| 数据可视化工具 | 需要精美UI的应用 |
| 实时编辑器 | 需要无障碍访问的应用 |
| 引擎内嵌工具 | 需要原生外观的应用 |
| 快速原型 | 复杂的文档编辑器 |

### 3. 集成到现有项目

```
ImGui集成步骤：

  1. 添加ImGui源码到项目
     ├── imgui.cpp
     ├── imgui.h
     ├── imgui_draw.cpp
     ├── imgui_tables.cpp
     ├── imgui_widgets.cpp
     └── 后端文件（如 imgui_impl_glfw.cpp + imgui_impl_opengl3.cpp）

  2. 初始化
     ImGui::CreateContext()
     ImGui_ImplGlfw_InitForOpenGL(window, true)
     ImGui_ImplOpenGL3_Init()

  3. 主循环中
     ImGui_ImplOpenGL3_NewFrame()
     ImGui_ImplGlfw_NewFrame()
     ImGui::NewFrame()
     // 绘制你的ImGui控件
     ImGui::Render()
     ImGui_ImplOpenGL3_RenderDrawData()

  4. 清理
     ImGui_ImplOpenGL3_Shutdown()
     ImGui_ImplGlfw_Shutdown()
     ImGui::DestroyContext()
```

### 4. 代码示例

```cpp
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

int main(void) {
    glfwInit();
    GLFWwindow *window = glfwCreateWindow(800, 600, "ImGui示例", NULL, NULL);
    glfwMakeContextCurrent(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    int count = 0;
    float value = 0.5f;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("控制面板");
        if (ImGui::Button("点击计数")) count++;
        ImGui::SameLine();
        ImGui::Text("次数: %d", count);
        ImGui::SliderFloat("数值", &value, 0.0f, 1.0f);
        ImGui::End();

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
```

***

## 5. 其他GUI方案

### 1. wxWidgets

```
wxWidgets 特点：
  ├── 使用各平台原生控件（Windows上像Windows，macOS上像macOS）
  ├── 外观与平台一致
  ├── 开源免费（LGPL）
  ├── API风格类似MFC
  └── 适合需要原生外观的跨平台应用
```

### 2. GTK

```
GTK 特点：
  ├── Linux桌面主流GUI框架（GNOME桌面基于GTK）
  ├── C语言编写，有C++绑定（gtkmm）
  ├── 跨平台但Windows/macOS体验不如Linux
  ├── 开源免费（LGPL）
  └── 适合Linux桌面应用开发
```

### 3. MFC

```
MFC 特点：
  ├── Windows老牌GUI框架
  ├── 对Win32 API的C++封装
  ├── 只支持Windows
  ├── 大量遗留项目仍在使用
  ├── 学习曲线陡峭
  └── 适合维护Windows老项目
```

### 4. Qt Quick/QML

```
Qt Quick/QML 特点：
  ├── 声明式UI（类似CSS/JSON的语法描述界面）
  ├── QML写界面 + C++写逻辑
  ├── 适合触摸/移动端界面
  ├── 动画和过渡效果丰富
  └── 适合现代风格UI、嵌入式设备
```

### 5. Electron + Web前端

```
Electron 特点：
  ├── 用HTML/CSS/JavaScript写界面
  ├── C++通过Node.js addon或子进程调用
  ├── 跨平台（Chromium内核）
  ├── 体积大（包含整个Chromium）
  ├── 内存占用高
  └── 代表：VS Code、Discord
```

### 6. Tauri

```
Tauri 特点：
  ├── 类似Electron但后端用Rust
  ├── 前端用Web技术，后端用Rust
  ├── C++可通过FFI与Rust交互
  ├── 体积小（使用系统WebView）
  └── 适合轻量级跨平台应用
```

### 7. FLTK

```
FLTK 特点：
  ├── 轻量级GUI工具包
  ├── 体积小，静态链接后仅几MB
  ├── C++编写，API简洁
  ├── 跨平台但外观较朴素
  └── 适合嵌入式/小型工具
```

***

## 6. GUI方案对比选型

### 1. 对比表

| 方案 | 语言 | 跨平台 | 许可证 | 体积 | 性能 | 学习曲线 | 适用场景 |
|------|------|--------|--------|------|------|---------|---------|
| Qt | C++ | Win/Mac/Linux | LGPL/商业 | 大(50MB+) | 高 | 中高 | 专业桌面应用 |
| ImGui | C++ | 任意(需渲染后端) | MIT | 小 | 高 | 低 | 工具/调试器/游戏UI |
| wxWidgets | C++ | Win/Mac/Linux | LGPL | 中 | 中 | 中 | 需要原生外观的应用 |
| GTK | C/C++ | Win/Mac/Linux | LGPL | 中 | 中 | 中高 | Linux桌面应用 |
| MFC | C++ | 仅Windows | 商业 | 小 | 高 | 高 | Windows遗留项目 |
| Qt Quick | QML/C++ | Win/Mac/Linux | LGPL/商业 | 大 | 中 | 中 | 触摸界面/嵌入式 |
| Electron | JS/C++ | Win/Mac/Linux | MIT | 很大(100MB+) | 低 | 低 | Web开发者做桌面 |
| Tauri | Rust/JS | Win/Mac/Linux | MIT/Apache | 小 | 中 | 中 | 轻量跨平台应用 |
| FLTK | C++ | Win/Mac/Linux | LGPL | 小 | 高 | 低 | 小型工具/嵌入式 |

### 2. 选型决策树

```
如何选择GUI方案？

你需要什么类型的界面？
│
├── 游戏/3D工具内嵌界面
│   └── ImGui ─── 轻量、即时模式、集成简单
│
├── 专业桌面应用（功能复杂）
│   ├── 需要原生外观？
│   │   ├── 是 → wxWidgets
│   │   └── 否 → Qt（功能最全、生态最好）
│   └── 只在Windows？
│       └── 维护老项目 → MFC / 新项目 → Qt
│
├── Linux桌面应用
│   └── GTK ─── Linux原生体验
│
├── 触摸/移动风格界面
│   └── Qt Quick/QML ─── 声明式UI、动画流畅
│
├── Web开发者做桌面
│   ├── 不在意体积 → Electron
│   └── 在意体积 → Tauri
│
├── 小型工具/嵌入式
│   └── FLTK 或 ImGui ─── 轻量
│
└── 快速原型/调试工具
    └── ImGui ─── 最快上手
```

***

## 7. C++ GUI编程的挑战

### 1. 线程与UI线程

```
铁律：UI操作必须在主线程（UI线程）执行

  主线程（UI线程）：
  ├── 创建窗口和控件
  ├── 处理事件循环
  ├── 更新界面
  └── 绘制界面

  工作线程：
  ├── 执行耗时计算
  ├── 网络请求
  ├── 文件读写
  └── 不能直接操作UI控件！

  线程间通信方式：
  ├── Qt：信号与槽（跨线程自动排队）
  ├── Win32：PostMessage/SendMessage
  └── 通用：事件队列 + 条件变量
```

```cpp
// Qt中安全地从工作线程更新UI
class Worker : public QObject {
    Q_OBJECT
signals:
    void resultReady(const QString &result);
public slots:
    void doWork() {
        QString result = heavyComputation();
        emit resultReady(result);
    }
};

// 在主线程中
Worker *worker = new Worker;
QThread *thread = new QThread;
worker->moveToThread(thread);

connect(thread, &QThread::started, worker, &Worker::doWork);
connect(worker, &Worker::resultReady, this, &MainWindow::updateUI);
connect(thread, &QThread::finished, worker, &QObject::deleteLater);

thread->start();
```

### 2. 事件循环与阻塞操作

```
问题：在事件循环中执行阻塞操作

  错误做法：
  void onButtonClick() {
      auto result = httpGet("http://slow-api.com");  // 阻塞5秒
      label->setText(result);                         // 界面冻结5秒！
  }

  正确做法1：异步回调
  void onButtonClick() {
      httpGetAsync("http://slow-api.com", [](auto result) {
          label->setText(result);
      });
  }

  正确做法2：使用工作线程
  void onButtonClick() {
      QtConcurrent::run([this]() {
          auto result = httpGet("http://slow-api.com");
          emit resultReady(result);
      });
  }

  正确做法3：使用QTimer分片处理
  void processChunk() {
      for (int i = 0; i < 100 && pos < total; i++, pos++) {
          processItem(pos);
      }
      if (pos < total)
          QTimer::singleShot(0, this, &MyClass::processChunk);
  }
```

### 3. 内存管理

```
Qt的父子对象树简化内存管理：

  规则1：给控件指定parent → parent销毁时自动销毁child
  规则2：new控件时传入parent指针
  规则3：布局会自动接管控件的ownership

  不需要手动delete的情况：
  ├── 有parent的QObject
  ├── 添加到布局中的控件
  └── 使用QScopedPointer/QSharedPointer

  需要手动delete的情况：
  ├── 没有parent的QObject
  ├── 使用new分配但未加入对象树的
  └── 工作线程中的临时对象
```

### 4. DPI适配

```
高DPI问题：

  普通屏幕：96 DPI（100%缩放）
  高分屏：144 DPI（150%）、192 DPI（200%）

  不适配的后果：
  ├── 界面元素太小看不清
  ├── 文字模糊
  ├── 布局错乱

  Qt的DPI适配：
  ├── QApplication::setAttribute(Qt::AA_EnableHighDpiScaling)
  ├── 使用矢量图标（SVG）而非位图
  ├── 用布局而非固定像素尺寸
  └── CMake中添加：set(CMAKE_AUTORCC ON)
```

### 5. 主题与样式

```
Qt样式系统：

  1. QSS（Qt Style Sheets）—— 类似CSS
     widget->setStyleSheet("QPushButton { color: red; font-size: 14px; }");

  2. QPalette —— 调色板
     QPalette pal = widget->palette();
     pal.setColor(QPalette::Button, Qt::blue);
     widget->setPalette(pal);

  3. 自定义绘制 —— 重写paintEvent
     void MyWidget::paintEvent(QPaintEvent *) {
         QPainter p(this);
         p.drawEllipse(0, 0, width(), height());
     }

  4. QStyle —— 平台风格
     QApplication::setStyle("Fusion");
```

***

## 8. 从命令行到GUI的思维转变

### 1. 核心区别

```
命令行思维：
  我做一步，你跟一步
  程序流程由代码控制
  用户按顺序输入

  main() {
      输入名字();
      输入年龄();
      计算结果();
      输出结果();
  }

GUI思维：
  你随时可能点任何按钮
  程序流程由用户控制
  必须随时响应任何操作

  main() {
      创建界面();
      注册所有事件处理();
      进入事件循环();  // 不知道用户下一步做什么
  }
```

### 2. 状态管理

```
命令行：局部变量就够了
  void process() {
      string name = input();
      int age = input();
      print(name + to_string(age));
  }

GUI：需要持久化状态
  class MainWindow {
      string name;     // 必须保存，因为用户可能随时切换操作
      int age;
      bool modified;

      void onNameChanged(string n) { name = n; modified = true; }
      void onAgeChanged(int a) { age = a; modified = true; }
      void onSave() { save(name, age); modified = false; }
      void onClose() {
          if (modified) askSaveOrDiscard();
      }
  }
```

### 3. MVC/MVP/MVVM模式

```
GUI架构模式——分离界面与逻辑：

MVC（Model-View-Controller）：
  ┌──────────┐     更新     ┌──────────┐
  │   View   │ ←────────── │   Model  │
  │  (视图)   │             │  (模型)   │
  └────┬─────┘             └────▲─────┘
       │ 用户操作                │ 修改数据
       ▼                        │
  ┌──────────┐                 │
  │Controller│ ────────────────┘
  │ (控制器)  │
  └──────────┘

MVP（Model-View-Presenter）：
  ┌──────────┐             ┌──────────┐
  │   View   │ ←── 更新 ── │   Model  │
  │  (视图)   │             │  (模型)   │
  └────┬─────┘             └────▲─────┘
       │                        │
       ▼                        │
  ┌──────────┐                 │
  │Presenter │ ────────────────┘
  │(呈现器)   │
  └──────────┘
  View不直接访问Model，Presenter完全控制

MVVM（Model-View-ViewModel）：
  ┌──────────┐             ┌──────────┐
  │   View   │ ←── 绑定 ── │ViewModel │ ←── 更新 ── │  Model  │
  │  (视图)   │  (数据绑定)  │(视图模型) │             │  (模型)   │
  └──────────┘             └──────────┘             └──────────┘
  View和ViewModel双向绑定，不需要手动同步

Qt中的对应：
  Model → QAbstractItemModel
  View → QListView/QTableView
  Controller → 你的槽函数
  ViewModel → Qt的属性系统+信号槽
```

***

### 4. 相关章节

- [框架引擎中间件与架构概念指南](15-框架引擎中间件与架构.md) — 框架/引擎/中间件/架构全景
- [API-SDK-协议与接口概念指南](17-API-SDK-协议与接口.md) — API/SDK/协议/接口核心概念
- [编程范式实战指南](13-编程范式概览与过程式编程.md) — 过程式/OOP/函数式/泛型4大范式
- [类与面向对象](../../02-CPP/03-类与对象.md) — 信号与槽的OOP基础
- [Visual Studio开发环境配置指南](../05-开发环境与IDE/05-VisualStudio开发环境配置.md) — VS配置与使用

***

### 相关阅读

- [框架引擎中间件与架构](15-框架引擎中间件与架构.md)
- [跨平台是什么意思](./00-跨平台是什么意思.md)
- [C与CPP的跨平台可移植性](./11-C与CPP的跨平台可移植性.md)