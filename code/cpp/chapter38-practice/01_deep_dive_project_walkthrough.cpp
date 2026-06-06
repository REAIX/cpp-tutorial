/**
 * @file 01_deep_dive_project_walkthrough.cpp
 * @brief 项目实战: 从需求到实现, 设计决策, 代码组织, 测试策略
 * @description 对应文档: 02-CPP/38-实战案例
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>
#include <sstream>
#include <chrono>
#include <cassert>
#include <algorithm>
#include <random>

void demo_requirements_to_design() {
    std::cout << "\n=== demo_requirements_to_design ===\n";
    std::cout << "从需求到设计: 任务管理器项目\n\n";

    std::cout << "1. 需求分析:\n";
    std::cout << "   - 创建/删除/查询任务\n";
    std::cout << "   - 任务有标题、描述、优先级、状态\n";
    std::cout << "   - 支持任务分类/标签\n";
    std::cout << "   - 支持任务搜索\n";
    std::cout << "   - 数据持久化\n\n";

    std::cout << "2. 领域模型:\n";
    std::cout << "   Task: id, title, description, priority, status, tags, timestamps\n";
    std::cout << "   TaskManager: CRUD操作, 搜索, 过滤\n";
    std::cout << "   TaskRepository: 数据持久化接口\n\n";

    std::cout << "3. 设计决策:\n";
    std::cout << "   - 使用值语义还是引用语义? -> 混合: Task用值, Manager用引用\n";
    std::cout << "   - ID生成策略? -> 递增整数 (简单可靠)\n";
    std::cout << "   - 内存管理? -> unique_ptr + 工厂方法\n";
    std::cout << "   - 错误处理? -> 异常 + optional\n";
    std::cout << "   - 线程安全? -> 按需加锁\n\n";

    enum class Priority { Low, Medium, High, Critical };
    enum class Status { Todo, InProgress, Done, Cancelled };

    auto priority_name = [](Priority p) -> const char* {
        switch (p) {
            case Priority::Low: return "低";
            case Priority::Medium: return "中";
            case Priority::High: return "高";
            case Priority::Critical: return "紧急";
            default: return "未知";
        }
    };

    auto status_name = [](Status s) -> const char* {
        switch (s) {
            case Status::Todo: return "待办";
            case Status::InProgress: return "进行中";
            case Status::Done: return "已完成";
            case Status::Cancelled: return "已取消";
            default: return "未知";
        }
    };

    struct Task {
        int id;
        std::string title;
        std::string description;
        Priority priority;
        Status status;
        std::vector<std::string> tags;
        std::chrono::system_clock::time_point created_at;
        std::chrono::system_clock::time_point updated_at;
    };

    class TaskManager {
        std::map<int, Task> tasks_;
        int next_id_ = 1;

    public:
        Task& create(const std::string& title, Priority priority = Priority::Medium) {
            int id = next_id_++;
            auto now = std::chrono::system_clock::now();
            tasks_[id] = {id, title, "", priority, Status::Todo, {}, now, now};
            return tasks_[id];
        }

        Task* find(int id) {
            auto it = tasks_.find(id);
            return it != tasks_.end() ? &it->second : nullptr;
        }

        bool remove(int id) {
            return tasks_.erase(id) > 0;
        }

        std::vector<Task*> query(std::function<bool(const Task&)> pred) {
            std::vector<Task*> result;
            for (auto& [_, task] : tasks_) {
                if (pred(task)) result.push_back(&task);
            }
            return result;
        }

        std::vector<Task*> all() {
            std::vector<Task*> result;
            for (auto& [_, task] : tasks_) result.push_back(&task);
            return result;
        }

        size_t count() const { return tasks_.size(); }
    };

    TaskManager mgr;
    auto& t1 = mgr.create("完成设计文档", Priority::High);
    t1.tags = {"文档", "设计"};
    auto& t2 = mgr.create("修复登录Bug", Priority::Critical);
    t2.status = Status::InProgress;
    t2.tags = {"Bug", "登录"};
    auto& t3 = mgr.create("优化查询性能", Priority::Medium);
    t3.tags = {"性能"};

    std::cout << "任务列表:\n";
    for (auto* task : mgr.all()) {
        std::cout << "  #" << task->id << " [" << priority_name(task->priority)
                  << "] " << task->title << " (" << status_name(task->status) << ")\n";
    }

    auto high_priority = mgr.query([](const Task& t) {
        return t.priority >= Priority::High;
    });
    std::cout << "\n高优先级任务:\n";
    for (auto* task : high_priority) {
        std::cout << "  #" << task->id << " " << task->title << "\n";
    }
}

void demo_code_organization() {
    std::cout << "\n=== demo_code_organization ===\n";
    std::cout << "代码组织最佳实践\n\n";

    std::cout << "推荐项目结构:\n";
    std::cout << "  project/\n";
    std::cout << "  ├── CMakeLists.txt          # 顶层CMake\n";
    std::cout << "  ├── README.md               # 项目说明\n";
    std::cout << "  ├── .clang-format           # 代码格式配置\n";
    std::cout << "  ├── .clang-tidy             # 静态分析配置\n";
    std::cout << "  ├── include/                # 公共头文件\n";
    std::cout << "  │   └── project/\n";
    std::cout << "  │       ├── task.hpp        # Task类声明\n";
    std::cout << "  │       └── manager.hpp     # Manager类声明\n";
    std::cout << "  ├── src/                    # 实现文件\n";
    std::cout << "  │   ├── task.cpp\n";
    std::cout << "  │   └── manager.cpp\n";
    std::cout << "  ├── tests/                  # 测试文件\n";
    std::cout << "  │   ├── CMakeLists.txt\n";
    std::cout << "  │   ├── test_task.cpp\n";
    std::cout << "  │   └── test_manager.cpp\n";
    std::cout << "  ├── examples/               # 示例代码\n";
    std::cout << "  │   └── demo.cpp\n";
    std::cout << "  └── third_party/            # 第三方依赖\n\n";

    std::cout << "头文件组织原则:\n";
    std::cout << "  1. 公共API放在include/\n";
    std::cout << "  2. 内部实现放在src/\n";
    std::cout << "  3. 使用#pragma once或include guard\n";
    std::cout << "  4. 前向声明减少头文件依赖\n";
    std::cout << "  5. Pimpl模式隐藏实现细节\n\n";

    std::cout << "CMakeLists.txt模板:\n";
    std::cout << "  cmake_minimum_required(VERSION 3.20)\n";
    std::cout << "  project(taskmanager VERSION 1.0.0 LANGUAGES CXX)\n\n";
    std::cout << "  set(CMAKE_CXX_STANDARD 20)\n";
    std::cout << "  set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n";
    std::cout << "  add_library(taskmanager\n";
    std::cout << "      src/task.cpp src/manager.cpp)\n";
    std::cout << "  target_include_directories(taskmanager PUBLIC include)\n\n";
    std::cout << "  add_executable(demo examples/demo.cpp)\n";
    std::cout << "  target_link_libraries(demo PRIVATE taskmanager)\n\n";

    std::cout << "代码规范:\n";
    std::cout << "  1. 命名: PascalCase(类), camelCase(函数), snake_case(变量)\n";
    std::cout << "  2. 格式化: clang-format统一风格\n";
    std::cout << "  3. 静态分析: clang-tidy检查\n";
    std::cout << "  4. 提交规范: Conventional Commits\n";
}

void demo_testing_strategy() {
    std::cout << "\n=== demo_testing_strategy ===\n";
    std::cout << "测试策略\n\n";

    std::cout << "测试金字塔:\n";
    std::cout << "       /\\\n";
    std::cout << "      /  \\        E2E测试 (少量)\n";
    std::cout << "     /────\\       集成测试 (适量)\n";
    std::cout << "    /──────\\      单元测试 (大量)\n";
    std::cout << "   /────────\\\n\n";

    std::cout << "1. 单元测试:\n";
    std::cout << "   测试单个函数/类的行为\n";
    std::cout << "   快速, 隔离, 可重复\n\n";

    struct Calculator {
        static int add(int a, int b) { return a + b; }
        static int subtract(int a, int b) { return a - b; }
        static int multiply(int a, int b) { return a * b; }
        static double divide(int a, int b) {
            if (b == 0) throw std::invalid_argument("Division by zero");
            return static_cast<double>(a) / b;
        }
    };

    auto run_tests = []() {
        int passed = 0, failed = 0;

        auto check = [&](bool condition, const std::string& name) {
            if (condition) { ++passed; }
            else { ++failed; std::cout << "  FAIL: " << name << "\n"; }
        };

        check(Calculator::add(2, 3) == 5, "add(2,3)==5");
        check(Calculator::add(-1, 1) == 0, "add(-1,1)==0");
        check(Calculator::subtract(5, 3) == 2, "subtract(5,3)==2");
        check(Calculator::multiply(3, 4) == 12, "multiply(3,4)==12");
        check(Calculator::divide(10, 4) == 2.5, "divide(10,4)==2.5");

        bool threw = false;
        try { Calculator::divide(1, 0); } catch (const std::invalid_argument&) { threw = true; }
        check(threw, "divide(1,0) throws");

        std::cout << "  测试结果: " << passed << "通过, " << failed << "失败\n";
    };

    std::cout << "运行单元测试:\n";
    run_tests();

    std::cout << "\n2. 集成测试:\n";
    std::cout << "   测试模块间的交互\n";
    std::cout << "   例: TaskManager + Repository的集成\n\n";

    std::cout << "3. E2E测试:\n";
    std::cout << "   测试完整用户场景\n";
    std::cout << "   例: 创建任务 -> 修改状态 -> 搜索 -> 删除\n\n";

    std::cout << "C++测试框架:\n";
    std::cout << "  Google Test: 最流行, 功能丰富\n";
    std::cout << "  Catch2: header-only, BDD风格\n";
    std::cout << "  doctest: 最快编译, 简洁\n";
    std::cout << "  Boost.Test: Boost生态\n\n";

    std::cout << "测试最佳实践:\n";
    std::cout << "  1. AAA模式: Arrange-Act-Assert\n";
    std::cout << "  2. 每个测试只验证一个行为\n";
    std::cout << "  3. 测试命名: test_what_expected\n";
    std::cout << "  4. 边界条件: 空输入, 最大值, 零值\n";
    std::cout << "  5. 异常路径: 必须测试错误处理\n";
    std::cout << "  6. CI集成: 每次提交自动运行测试\n";
}

void demo_design_decisions() {
    std::cout << "\n=== demo_design_decisions ===\n";
    std::cout << "常见设计决策\n\n";

    std::cout << "1. 值语义 vs 引用语义:\n";
    std::cout << "   值语义: 简单, 安全, 但有拷贝开销\n";
    std::cout << "   引用语义: 高效, 但需管理生命周期\n";
    std::cout << "   建议: 小对象用值, 大对象用shared_ptr\n\n";

    std::cout << "2. 异常 vs 错误码:\n";
    std::cout << "   异常: 不可忽略, 但有性能开销\n";
    std::cout << "   错误码: 零开销, 但可能被忽略\n";
    std::cout << "   建议: 预期错误用optional/expected, 意外错误用异常\n\n";

    std::cout << "3. 继承 vs 组合:\n";
    std::cout << "   继承: '是一个'关系, 多态\n";
    std::cout << "   组合: '有一个'关系, 灵活\n";
    std::cout << "   建议: 优先组合, 必要时继承\n\n";

    std::cout << "4. 静态多态 vs 动态多态:\n";
    std::cout << "   静态(CRTP/模板): 零开销, 编译期\n";
    std::cout << "   动态(virtual): 运行时灵活\n";
    std::cout << "   建议: 性能关键用静态, 灵活性需求用动态\n\n";

    std::cout << "5. 同步 vs 异步:\n";
    std::cout << "   同步: 简单, 但阻塞\n";
    std::cout << "   异步: 高效, 但复杂\n";
    std::cout << "   建议: I/O密集用异步, CPU密集用同步+线程池\n\n";

    std::cout << "6. 堆分配 vs 栈分配:\n";
    std::cout << "   栈: 快速, 自动释放, 但大小有限\n";
    std::cout << "   堆: 灵活, 但有分配开销和泄漏风险\n";
    std::cout << "   建议: 优先栈分配, 大对象/动态生命周期用堆\n";
}

int main() {
    std::cout << "项目实战演练\n";

    demo_requirements_to_design();
    demo_code_organization();
    demo_testing_strategy();
    demo_design_decisions();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
