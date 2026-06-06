/** @file 07_example_compile_time_state_machine.cpp
 *  @brief 编译期状态机：使用模板实现类型安全的状态机
 *  @description 对应文档: 07-模板元编程与编译期计算 / 模板元编程实战
 */

#include <iostream>
#include <string>
#include <vector>
#include <type_traits>
#include <functional>

// ============================================================
// 1. 状态机基本概念
// ============================================================

// 状态机 = 有限状态自动机 (FSM)
// 组成: 状态集合 + 事件集合 + 转移规则 + 动作
// 编译期状态机的优势:
//   - 类型安全: 非法状态转移在编译期被拒绝
//   - 零开销: 无虚函数、无运行期查找
//   - 可验证: 编译期检查状态机的完整性

void demo_fsm_concept() {
    std::cout << "=== 状态机基本概念 ===\n";
    std::cout << "FSM = 有限状态自动机\n";
    std::cout << "  状态(State) + 事件(Event) + 转移(Transition) + 动作(Action)\n\n";
    std::cout << "编译期状态机优势:\n";
    std::cout << "  1. 类型安全: 非法转移编译期拒绝\n";
    std::cout << "  2. 零开销: 无虚函数/运行期查找\n";
    std::cout << "  3. 可验证: 编译期检查完整性\n\n";
}

// ============================================================
// 2. 基础编译期状态机
// ============================================================

// 状态定义（使用标签类型）
struct Idle {};
struct Running {};
struct Paused {};
struct Stopped {};

// 事件定义
struct Start {};
struct Pause {};
struct Resume {};
struct Stop {};

// 转移规则：使用模板特化定义合法的状态转移
// 默认情况：不允许转移
template<typename State, typename Event>
struct Transition {
    // 无效转移
    using next_state = void;
    static constexpr bool valid = false;
};

// 合法转移的特化
template<>
struct Transition<Idle, Start> {
    using next_state = Running;
    static constexpr bool valid = true;
};

template<>
struct Transition<Running, Pause> {
    using next_state = Paused;
    static constexpr bool valid = true;
};

template<>
struct Transition<Running, Stop> {
    using next_state = Stopped;
    static constexpr bool valid = true;
};

template<>
struct Transition<Paused, Resume> {
    using next_state = Running;
    static constexpr bool valid = true;
};

template<>
struct Transition<Paused, Stop> {
    using next_state = Stopped;
    static constexpr bool valid = true;
};

template<>
struct Transition<Stopped, Start> {
    using next_state = Running;
    static constexpr bool valid = true;
};

// 状态名称辅助
template<typename State>
constexpr const char* state_name() {
    if constexpr (std::is_same_v<State, Idle>)    return "空闲(Idle)";
    else if constexpr (std::is_same_v<State, Running>) return "运行(Running)";
    else if constexpr (std::is_same_v<State, Paused>)  return "暂停(Paused)";
    else if constexpr (std::is_same_v<State, Stopped>) return "停止(Stopped)";
    else return "未知";
}

template<typename Event>
constexpr const char* event_name() {
    if constexpr (std::is_same_v<Event, Start>)  return "启动(Start)";
    else if constexpr (std::is_same_v<Event, Pause>)  return "暂停(Pause)";
    else if constexpr (std::is_same_v<Event, Resume>) return "恢复(Resume)";
    else if constexpr (std::is_same_v<Event, Stop>)   return "停止(Stop)";
    else return "未知";
}

// 编译期状态机
template<typename InitialState = Idle>
class StateMachine {
    using CurrentState = InitialState;

    // 检查转移是否合法
    template<typename Event>
    static constexpr bool is_valid_transition() {
        return Transition<InitialState, Event>::valid;
    }

public:
    // 处理事件，返回新状态的状态机
    template<typename Event>
    auto process(Event) const {
        static_assert(is_valid_transition<Event>(),
            "非法的状态转移！检查当前状态和事件是否匹配。");
        return StateMachine<typename Transition<InitialState, Event>::next_state>{};
    }

    // 获取当前状态名
    static constexpr const char* current_state() {
        return state_name<InitialState>();
    }

    // 检查是否可以处理某事件
    template<typename Event>
    static constexpr bool can_process() {
        return is_valid_transition<Event>();
    }
};

void demo_basic_state_machine() {
    std::cout << "=== 基础编译期状态机 ===\n";

    // 初始状态: Idle
    StateMachine<Idle> sm;
    std::cout << "初始状态: " << sm.current_state() << "\n";

    // Idle -> Start -> Running
    auto sm_running = sm.process(Start{});
    std::cout << "Start 后: " << sm_running.current_state() << "\n";

    // Running -> Pause -> Paused
    auto sm_paused = sm_running.process(Pause{});
    std::cout << "Pause 后: " << sm_paused.current_state() << "\n";

    // Paused -> Resume -> Running
    auto sm_resumed = sm_paused.process(Resume{});
    std::cout << "Resume 后: " << sm_resumed.current_state() << "\n";

    // Running -> Stop -> Stopped
    auto sm_stopped = sm_resumed.process(Stop{});
    std::cout << "Stop 后: " << sm_stopped.current_state() << "\n";

    // 检查可处理的事件
    std::cout << "\nIdle 状态下:\n";
    std::cout << "  可 Start: " << StateMachine<Idle>::can_process<Start>() << "\n";
    std::cout << "  可 Pause: " << StateMachine<Idle>::can_process<Pause>() << "\n";

    // 非法转移会在编译期被拒绝:
    // auto bad = sm.process(Pause{});  // 编译错误! Idle + Pause 无效

    std::cout << "\n";
}

// ============================================================
// 3. 带动作的状态机
// ============================================================

// 转移动作
template<typename State, typename Event>
struct Action {
    static void execute() {
        std::cout << "  [动作] " << state_name<State>()
                  << " + " << event_name<Event>()
                  << " -> " << state_name<typename Transition<State, Event>::next_state>() << "\n";
    }
};

// 带动作的状态机
template<typename InitialState = Idle>
class ActionStateMachine {
public:
    template<typename Event>
    auto process(Event e) const {
        static_assert(Transition<InitialState, Event>::valid,
            "非法的状态转移！");

        // 执行动作
        Action<InitialState, Event>::execute();

        return ActionStateMachine<typename Transition<InitialState, Event>::next_state>{};
    }

    static constexpr const char* current_state() {
        return state_name<InitialState>();
    }
};

// 自定义动作特化
template<>
struct Action<Running, Stop> {
    static void execute() {
        std::cout << "  [动作] 运行中停止 → 释放资源，保存状态\n";
    }
};

template<>
struct Action<Idle, Start> {
    static void execute() {
        std::cout << "  [动作] 启动 → 初始化资源，开始运行\n";
    }
};

void demo_action_state_machine() {
    std::cout << "=== 带动作的状态机 ===\n";

    ActionStateMachine<Idle> asm1;
    std::cout << "初始: " << asm1.current_state() << "\n";

    auto asm2 = asm1.process(Start{});
    auto asm3 = asm2.process(Stop{});

    std::cout << "\n";
}

// ============================================================
// 4. 运行期状态机（类型擦除）
// ============================================================

// 编译期状态机类型安全但不够灵活
// 运行期状态机更实用，但可以借鉴模板元编程的设计

class RuntimeStateMachine {
    int current_state_ = 0;
    std::vector<std::string> state_names_;
    std::vector<std::vector<int>> transitions_;  // [state][event] -> next_state
    std::vector<std::string> event_names_;
    std::vector<std::function<void(int, int)>> actions_;  // (from, to) -> action

public:
    int add_state(const std::string& name) {
        int id = static_cast<int>(state_names_.size());
        state_names_.push_back(name);
        transitions_.emplace_back();
        return id;
    }

    int add_event(const std::string& name) {
        int id = static_cast<int>(event_names_.size());
        event_names_.push_back(name);
        // 扩展所有状态的转移表
        for (auto& row : transitions_) {
            row.resize(event_names_.size(), -1);
        }
        return id;
    }

    void add_transition(int from, int event, int to) {
        transitions_[from][event] = to;
    }

    void set_action(int from, int event, std::function<void(int, int)> action) {
        // 简化：直接在处理时调用
        (void)from; (void)event;
        actions_.push_back(std::move(action));
    }

    bool process(int event) {
        int next = transitions_[current_state_][event];
        if (next < 0) {
            std::cout << "  非法转移: " << state_names_[current_state_]
                      << " + " << event_names_[event] << "\n";
            return false;
        }
        std::cout << "  " << state_names_[current_state_]
                  << " -> " << state_names_[next] << "\n";
        current_state_ = next;
        return true;
    }

    const std::string& current_state_name() const {
        return state_names_[current_state_];
    }
};

void demo_runtime_state_machine() {
    std::cout << "=== 运行期状态机 ===\n";

    RuntimeStateMachine rsm;

    // 定义状态
    int idle = rsm.add_state("空闲");
    int running = rsm.add_state("运行");
    int paused = rsm.add_state("暂停");
    int stopped = rsm.add_state("停止");

    // 定义事件
    int start = rsm.add_event("启动");
    int pause = rsm.add_event("暂停");
    int resume = rsm.add_event("恢复");
    int stop = rsm.add_event("停止");

    // 定义转移
    rsm.add_transition(idle, start, running);
    rsm.add_transition(running, pause, paused);
    rsm.add_transition(running, stop, stopped);
    rsm.add_transition(paused, resume, running);
    rsm.add_transition(paused, stop, stopped);
    rsm.add_transition(stopped, start, running);

    // 处理事件序列
    std::cout << "当前: " << rsm.current_state_name() << "\n";
    rsm.process(start);
    rsm.process(pause);
    rsm.process(resume);
    rsm.process(stop);
    rsm.process(pause);  // 非法转移

    std::cout << "\n";
}

// ============================================================
// 5. TCP 连接状态机示例
// ============================================================

// TCP 连接状态
struct TCP_Closed {};
struct TCP_Listen {};
struct TCP_SynSent {};
struct TCP_Established {};
struct TCP_CloseWait {};
struct TCP_FinWait1 {};
struct TCP_FinWait2 {};
struct TCP_TimeWait {};

// TCP 事件
struct TCP_Syn {};
struct TCP_SynAck {};
struct TCP_Ack {};
struct TCP_Fin {};
struct TCP_Close {};
struct TCP_Timeout {};

// TCP 状态名称
template<typename S>
constexpr const char* tcp_state_name() {
    if constexpr (std::is_same_v<S, TCP_Closed>)      return "CLOSED";
    else if constexpr (std::is_same_v<S, TCP_Listen>)     return "LISTEN";
    else if constexpr (std::is_same_v<S, TCP_SynSent>)    return "SYN_SENT";
    else if constexpr (std::is_same_v<S, TCP_Established>) return "ESTABLISHED";
    else if constexpr (std::is_same_v<S, TCP_CloseWait>)  return "CLOSE_WAIT";
    else if constexpr (std::is_same_v<S, TCP_FinWait1>)   return "FIN_WAIT1";
    else if constexpr (std::is_same_v<S, TCP_FinWait2>)   return "FIN_WAIT2";
    else if constexpr (std::is_same_v<S, TCP_TimeWait>)   return "TIME_WAIT";
    else return "UNKNOWN";
}

// TCP 转移规则（简化版）
template<typename State, typename Event>
struct TCP_Transition {
    using next_state = void;
    static constexpr bool valid = false;
};

// CLOSED + Syn -> LISTEN (服务器端)
template<>
struct TCP_Transition<TCP_Closed, TCP_Syn> {
    using next_state = TCP_Listen;
    static constexpr bool valid = true;
};

// LISTEN + Syn -> ESTABLISHED (简化: 三次握手合并)
template<>
struct TCP_Transition<TCP_Listen, TCP_Syn> {
    using next_state = TCP_Established;
    static constexpr bool valid = true;
};

// ESTABLISHED + Fin -> CLOSE_WAIT
template<>
struct TCP_Transition<TCP_Established, TCP_Fin> {
    using next_state = TCP_CloseWait;
    static constexpr bool valid = true;
};

// ESTABLISHED + Close -> FIN_WAIT1
template<>
struct TCP_Transition<TCP_Established, TCP_Close> {
    using next_state = TCP_FinWait1;
    static constexpr bool valid = true;
};

// CLOSE_WAIT + Close -> LAST_ACK (简化为 CLOSED)
template<>
struct TCP_Transition<TCP_CloseWait, TCP_Close> {
    using next_state = TCP_Closed;
    static constexpr bool valid = true;
};

// FIN_WAIT1 + Ack -> FIN_WAIT2
template<>
struct TCP_Transition<TCP_FinWait1, TCP_Ack> {
    using next_state = TCP_FinWait2;
    static constexpr bool valid = true;
};

// FIN_WAIT2 + Fin -> TIME_WAIT
template<>
struct TCP_Transition<TCP_FinWait2, TCP_Fin> {
    using next_state = TCP_TimeWait;
    static constexpr bool valid = true;
};

// TIME_WAIT + Timeout -> CLOSED
template<>
struct TCP_Transition<TCP_TimeWait, TCP_Timeout> {
    using next_state = TCP_Closed;
    static constexpr bool valid = true;
};

// TCP 状态机
template<typename State = TCP_Closed>
class TCPStateMachine {
public:
    template<typename Event>
    auto process(Event) const {
        static_assert(TCP_Transition<State, Event>::valid,
            "非法的 TCP 状态转移!");
        std::cout << "  " << tcp_state_name<State>()
                  << " -> " << tcp_state_name<typename TCP_Transition<State, Event>::next_state>() << "\n";
        return TCPStateMachine<typename TCP_Transition<State, Event>::next_state>{};
    }

    static constexpr const char* current_state() {
        return tcp_state_name<State>();
    }
};

void demo_tcp_state_machine() {
    std::cout << "=== TCP 连接状态机 ===\n";

    // 服务器端: 被动关闭场景
    std::cout << "服务器端(被动关闭):\n";
    TCPStateMachine<TCP_Closed> tcp;
    auto t1 = tcp.process(TCP_Syn{});       // CLOSED -> LISTEN
    auto t2 = t1.process(TCP_Syn{});        // LISTEN -> ESTABLISHED
    auto t3 = t2.process(TCP_Fin{});        // ESTABLISHED -> CLOSE_WAIT
    auto t4 = t3.process(TCP_Close{});      // CLOSE_WAIT -> CLOSED

    // 客户端: 主动关闭场景
    std::cout << "\n客户端(主动关闭):\n";
    TCPStateMachine<TCP_Established> tcp_client;
    auto c1 = tcp_client.process(TCP_Close{});  // ESTABLISHED -> FIN_WAIT1
    auto c2 = c1.process(TCP_Ack{});            // FIN_WAIT1 -> FIN_WAIT2
    auto c3 = c2.process(TCP_Fin{});            // FIN_WAIT2 -> TIME_WAIT
    auto c4 = c3.process(TCP_Timeout{});        // TIME_WAIT -> CLOSED

    std::cout << "\n";
}

// ============================================================
// 6. 编译期验证状态机完整性
// ============================================================

// 检查每个状态是否至少有一个出转移
template<typename State, typename... Events>
struct HasOutTransitions;

template<typename State>
struct HasOutTransitions<State> : std::false_type {};

template<typename State, typename First, typename... Rest>
struct HasOutTransitions<State, First, Rest...>
    : std::conditional_t<
        Transition<State, First>::valid,
        std::true_type,
        HasOutTransitions<State, Rest...>
      > {};

// 检查所有状态是否可达
template<typename... States>
struct AllStatesReachable;  // 简化，实际需要图遍历

void demo_state_machine_validation() {
    std::cout << "=== 编译期验证状态机 ===\n";

    using AllEvents = Start;  // 简化

    // 检查 Idle 状态是否有出转移
    std::cout << "Idle 有出转移(Start): "
              << HasOutTransitions<Idle, Start>::value << "\n";
    std::cout << "Idle 有出转移(Pause): "
              << HasOutTransitions<Idle, Pause>::value << "\n";
    std::cout << "Idle 有出转移(Stop): "
              << HasOutTransitions<Idle, Stop>::value << "\n";

    std::cout << "\n编译期验证的优势:\n";
    std::cout << "  1. 非法转移: 编译期 static_assert 拒绝\n";
    std::cout << "  2. 死状态: 编译期检查每个状态是否可达\n";
    std::cout << "  3. 完整性: 编译期检查所有事件是否被处理\n";

    std::cout << "\n";
}

// ============================================================
// main
// ============================================================

int main() {
    std::cout << "============================================\n";
    std::cout << "  编译期状态机\n";
    std::cout << "============================================\n\n";

    demo_fsm_concept();
    demo_basic_state_machine();
    demo_action_state_machine();
    demo_runtime_state_machine();
    demo_tcp_state_machine();
    demo_state_machine_validation();

    std::cout << "============================================\n";
    std::cout << "  核心要点总结:\n";
    std::cout << "  1. 编译期状态机: 类型安全、零开销\n";
    std::cout << "  2. 转移规则: 模板特化定义\n";
    std::cout << "  3. 动作: 可通过特化自定义\n";
    std::cout << "  4. 运行期: 类型擦除更灵活\n";
    std::cout << "  5. 实际应用: 协议、UI、工作流\n";
    std::cout << "============================================\n";

    return 0;
}
