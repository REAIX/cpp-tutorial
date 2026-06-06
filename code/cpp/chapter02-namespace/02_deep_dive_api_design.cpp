/** @file 02_deep_dive_api_design.cpp
 *  @brief API 设计原则、PIMPL 惯用法、前向声明、头文件卫生
 *  @description 对应文档: 02-CPP/02-namespace
 */

#include <iostream>
#include <string>
#include <memory>
#include <vector>

// ===== 1. API 设计原则 =====

// 原则1: 最小接口原则
class GoodStack {
public:
    void push(int value) { data_.push_back(value); }
    void pop() { data_.pop_back(); }
    int top() const { return data_.back(); }
    bool empty() const { return data_.empty(); }
    size_t size() const { return data_.size(); }
private:
    std::vector<int> data_;
};

// 反例: 暴露过多实现细节
class BadStack {
public:
    std::vector<int>& data() { return data_; }  // 暴露内部实现!
    // 用户可以直接修改 vector, 绕过栈的语义约束
private:
    std::vector<int> data_;
};

// 原则2: 难以误用
class SafeFile {
public:
    explicit SafeFile(const std::string& path) : path_(path), open_(true) {
        std::cout << "  打开文件: " << path_ << std::endl;
    }

    ~SafeFile() {
        if (open_) {
            std::cout << "  关闭文件: " << path_ << std::endl;
        }
    }

    SafeFile(const SafeFile&) = delete;
    SafeFile& operator=(const SafeFile&) = delete;

    void close() {
        if (open_) {
            std::cout << "  关闭文件: " << path_ << std::endl;
            open_ = false;
        }
    }

private:
    std::string path_;
    bool open_;
};

void demo_api_principles() {
    std::cout << "===== API 设计原则 =====" << std::endl;

    GoodStack stack;
    stack.push(1);
    stack.push(2);
    std::cout << "栈顶: " << stack.top() << std::endl;
    stack.pop();
    std::cout << "弹出后栈顶: " << stack.top() << std::endl;

    {
        SafeFile f("test.txt");
        // f = SafeFile("other.txt");  // 编译错误: 禁止赋值
    }  // 自动关闭

    std::cout << "\nAPI 设计原则:" << std::endl;
    std::cout << "  1. 最小接口: 只暴露必要的操作" << std::endl;
    std::cout << "  2. 难以误用: RAII, explicit, delete" << std::endl;
    std::cout << "  3. 容易正确使用: 合理默认值, 清晰命名" << std::endl;
    std::cout << "  4. 隐藏实现细节: PIMPL, 前向声明" << std::endl;
}

// ===== 2. PIMPL 惯用法 (Pointer to Implementation) =====

// 头文件中: 只暴露接口, 不暴露实现
class NetworkClient {
public:
    NetworkClient(const std::string& host, int port);
    ~NetworkClient();

    NetworkClient(const NetworkClient&) = delete;
    NetworkClient& operator=(const NetworkClient&) = delete;

    void connect();
    void send(const std::string& message);
    void disconnect();
    bool is_connected() const;

private:
    struct Impl;  // 前向声明实现类
    std::unique_ptr<Impl> pImpl;  // 指向实现的指针
};

// 源文件中: 完整定义实现类
struct NetworkClient::Impl {
    std::string host;
    int port;
    bool connected = false;

    void do_connect() {
        std::cout << "  连接到 " << host << ":" << port << std::endl;
        connected = true;
    }

    void do_send(const std::string& msg) {
        std::cout << "  发送: " << msg << " -> " << host << std::endl;
    }

    void do_disconnect() {
        std::cout << "  断开 " << host << std::endl;
        connected = false;
    }
};

NetworkClient::NetworkClient(const std::string& host, int port)
    : pImpl(std::make_unique<Impl>()) {
    pImpl->host = host;
    pImpl->port = port;
}

NetworkClient::~NetworkClient() = default;

void NetworkClient::connect() { pImpl->do_connect(); }
void NetworkClient::send(const std::string& message) { pImpl->do_send(message); }
void NetworkClient::disconnect() { pImpl->do_disconnect(); }
bool NetworkClient::is_connected() const { return pImpl->connected; }

void demo_pimpl() {
    std::cout << "\n===== PIMPL 惯用法 =====" << std::endl;

    NetworkClient client("example.com", 8080);
    client.connect();
    client.send("Hello, Server!");
    client.disconnect();

    std::cout << "\nPIMPL 的优势:" << std::endl;
    std::cout << "  1. 编译防火墙: 修改实现无需重新编译使用者" << std::endl;
    std::cout << "  2. 隐藏私有成员: 头文件不暴露实现细节" << std::endl;
    std::cout << "  3. 减少头文件依赖: 私有依赖移到源文件" << std::endl;
    std::cout << "  4. ABI 稳定: 实现变化不影响二进制兼容" << std::endl;

    std::cout << "\nPIMPL 的代价:" << std::endl;
    std::cout << "  1. 额外的堆分配 (可用自定义分配器优化)" << std::endl;
    std::cout << "  2. 间接访问的性能开销" << std::endl;
    std::cout << "  3. 代码复杂度增加" << std::endl;
}

// ===== 3. 前向声明 =====
class Renderer;   // 前向声明: 只声明类名, 不定义
class Texture;    // 前向声明

class GameObject {
public:
    void render(Renderer& renderer);  // 可以使用引用/指针参数
    void set_texture(Texture* texture);  // 可以使用指针参数

    // void draw(Renderer renderer);  // 错误: 值传递需要完整定义
    // Renderer get_renderer();       // 错误: 返回值需要完整定义

private:
    Texture* texture_;  // 指针: 只需前向声明
    // Texture texture_;  // 错误: 值成员需要完整定义
};

// 在源文件中包含完整头文件后实现
class Renderer {
public:
    void draw(const std::string& name) {
        std::cout << "  渲染: " << name << std::endl;
    }
};

class Texture {
public:
    std::string name;
};

void GameObject::render(Renderer& renderer) {
    renderer.draw("GameObject");
}

void GameObject::set_texture(Texture* texture) {
    texture_ = texture;
}

void demo_forward_declarations() {
    std::cout << "\n===== 前向声明 =====" << std::endl;

    Renderer renderer;
    GameObject obj;
    obj.render(renderer);

    Texture tex;
    tex.name = "wood";
    obj.set_texture(&tex);

    std::cout << "\n前向声明的规则:" << std::endl;
    std::cout << "  可以使用前向声明的场景:" << std::endl;
    std::cout << "    - 声明指针或引用" << std::endl;
    std::cout << "    - 声明函数参数/返回值为指针或引用" << std::endl;
    std::cout << "    - 声明指针类型的成员变量" << std::endl;
    std::cout << "  不能使用前向声明的场景:" << std::endl;
    std::cout << "    - 值类型的成员变量" << std::endl;
    std::cout << "    - 值传递的函数参数" << std::endl;
    std::cout << "    - 调用成员函数" << std::endl;
    std::cout << "    - sizeof / typeid" << std::endl;

    std::cout << "\n前向声明的好处:" << std::endl;
    std::cout << "  - 减少头文件包含, 加快编译速度" << std::endl;
    std::cout << "  - 打破循环依赖" << std::endl;
    std::cout << "  - 减少不必要的重编译" << std::endl;
}

// ===== 4. 头文件卫生 =====
void demo_header_hygiene() {
    std::cout << "\n===== 头文件卫生 =====" << std::endl;

    std::cout << "头文件卫生规则:" << std::endl;
    std::cout << "  1. 头文件必须自包含" << std::endl;
    std::cout << "     - 包含自身需要的所有头文件" << std::endl;
    std::cout << "     - 不依赖使用者的间接包含" << std::endl;

    std::cout << "\n  2. 头文件中禁止 using namespace" << std::endl;
    std::cout << "     - 污染包含者的命名空间" << std::endl;
    std::cout << "     - 可能导致名称冲突" << std::endl;
    std::cout << "     - using 声明可以谨慎使用" << std::endl;

    std::cout << "\n  3. 最小化头文件依赖" << std::endl;
    std::cout << "     - 优先使用前向声明代替 #include" << std::endl;
    std::cout << "     - 私有依赖移到源文件 (PIMPL)" << std::endl;

    std::cout << "\n  4. 避免在头文件中定义非内联函数" << std::endl;
    std::cout << "     - 多个翻译单元包含时导致重复定义" << std::endl;
    std::cout << "     - 内联函数和模板定义例外" << std::endl;

    std::cout << "\n  5. include-what-you-use (IWYU) 工具" << std::endl;
    std::cout << "     - 自动检查头文件依赖是否正确" << std::endl;
    std::cout << "     - 移除不必要的 #include" << std::endl;
    std::cout << "     - 添加缺失的 #include" << std::endl;
}

int main() {
    std::cout << "========== API 设计与头文件卫生 ==========\n" << std::endl;

    demo_api_principles();
    demo_pimpl();
    demo_forward_declarations();
    demo_header_hygiene();

    return 0;
}
