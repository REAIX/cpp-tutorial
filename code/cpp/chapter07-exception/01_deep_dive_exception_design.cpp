/** @file 01_deep_dive_exception_design.cpp
 *  @brief 异常类层次设计、自定义异常、noexcept、异常规格历史
 *  @description 对应文档: 02-CPP/07-exception
 */

#include <iostream>
#include <string>
#include <stdexcept>
#include <system_error>

// ===== 1. 自定义异常类层次 =====

// 基类: 应用异常
class AppException : public std::runtime_error {
public:
    explicit AppException(const std::string& msg)
        : std::runtime_error(msg) {}

    virtual std::string type() const {
        return "AppException";
    }
};

// 网络异常
class NetworkException : public AppException {
public:
    NetworkException(const std::string& msg, int error_code)
        : AppException(msg), error_code_(error_code) {}

    int error_code() const { return error_code_; }

    std::string type() const override {
        return "NetworkException";
    }

private:
    int error_code_;
};

// 数据库异常
class DatabaseException : public AppException {
public:
    DatabaseException(const std::string& msg, const std::string& query)
        : AppException(msg), query_(query) {}

    const std::string& query() const { return query_; }

    std::string type() const override {
        return "DatabaseException";
    }

private:
    std::string query_;
};

// 验证异常
class ValidationException : public AppException {
public:
    ValidationException(const std::string& msg, const std::string& field)
        : AppException(msg), field_(field) {}

    const std::string& field() const { return field_; }

    std::string type() const override {
        return "ValidationException";
    }

private:
    std::string field_;
};

void demo_custom_exception_hierarchy() {
    std::cout << "===== 自定义异常类层次 =====" << std::endl;

    auto test = [](const AppException& e) {
        std::cout << "  类型: " << e.type() << std::endl;
        std::cout << "  消息: " << e.what() << std::endl;
    };

    try {
        throw NetworkException("连接超时", 10060);
    } catch (const NetworkException& e) {
        test(e);
        std::cout << "  错误码: " << e.error_code() << std::endl;
    }

    try {
        throw DatabaseException("查询失败", "SELECT * FROM users");
    } catch (const DatabaseException& e) {
        test(e);
        std::cout << "  查询: " << e.query() << std::endl;
    }

    try {
        throw ValidationException("邮箱格式无效", "email");
    } catch (const ValidationException& e) {
        test(e);
        std::cout << "  字段: " << e.field() << std::endl;
    }

    // 用基类捕获
    try {
        throw DatabaseException("连接失败", "CONNECT");
    } catch (const AppException& e) {
        std::cout << "  基类捕获: " << e.type() << " - " << e.what() << std::endl;
    }

    std::cout << "\n自定义异常设计原则:" << std::endl;
    std::cout << "  1. 继承 std::exception 或其派生类" << std::endl;
    std::cout << "  2. 按错误领域组织异常层次" << std::endl;
    std::cout << "  3. 携带足够的上下文信息" << std::endl;
    std::cout << "  4. 提供 what() 方法返回描述信息" << std::endl;
    std::cout << "  5. 异常类应该是简单的值类型" << std::endl;
}

// ===== 2. 使用 std::system_error =====
void demo_system_error() {
    std::cout << "\n===== std::system_error =====" << std::endl;

    // std::system_error: 携带错误码的异常
    try {
        throw std::system_error(
            std::make_error_code(std::errc::permission_denied),
            "文件访问被拒绝"
        );
    } catch (const std::system_error& e) {
        std::cout << "  what(): " << e.what() << std::endl;
        std::cout << "  code(): " << e.code() << std::endl;
        std::cout << "  code().value(): " << e.code().value() << std::endl;
        std::cout << "  code().category().name(): " << e.code().category().name() << std::endl;
    }

    std::cout << "\nsystem_error 优势:" << std::endl;
    std::cout << "  - 标准化的错误码" << std::endl;
    std::cout << "  - 错误类别 (system_category, generic_category)" << std::endl;
    std::cout << "  - 适合系统调用和 OS 错误" << std::endl;
}

// 条件 noexcept 示例 (需在使用前定义)
template<typename T>
void template_func() noexcept(std::is_nothrow_move_constructible_v<T>) {
    std::cout << "  条件 noexcept: is_nothrow_move_constructible<T> = "
              << std::is_nothrow_move_constructible_v<T> << std::endl;
}

// ===== 3. noexcept =====
void demo_noexcept() {
    std::cout << "\n===== noexcept =====" << std::endl;

    // noexcept: 承诺函数不抛出异常
    auto safe_func = []() noexcept {
        std::cout << "  noexcept 函数: 保证不抛出异常" << std::endl;
    };

    // 条件 noexcept
    template_func<int>();

    safe_func();

    // noexcept 与违反: 如果 noexcept 函数抛出异常, std::terminate 被调用
    auto broken_noexcept = []() noexcept {
        // throw std::runtime_error("违反 noexcept!");  // std::terminate!
        std::cout << "  noexcept 函数不能抛出异常" << std::endl;
    };
    broken_noexcept();

    std::cout << "\nnoexcept 的用途:" << std::endl;
    std::cout << "  1. 析构函数: 隐式 noexcept (C++11)" << std::endl;
    std::cout << "  2. 移动构造/移动赋值: 建议标记 noexcept" << std::endl;
    std::cout << "  3. swap: 应该 noexcept" << std::endl;
    std::cout << "  4. 内存释放: operator delete noexcept" << std::endl;

    std::cout << "\nnoexcept 的性能影响:" << std::endl;
    std::cout << "  - 编译器不需要生成异常展开代码" << std::endl;
    std::cout << "  - vector 扩容时优先使用 noexcept 移动构造" << std::endl;
    std::cout << "  - noexcept 是函数接口的一部分" << std::endl;

    std::cout << "\nnoexcept 运算符:" << std::endl;
    std::cout << "  noexcept(safe_func) = " << noexcept(safe_func) << std::endl;
}

// ===== 4. 异常规格历史 =====
void demo_exception_spec_history() {
    std::cout << "\n===== 异常规格历史 =====" << std::endl;

    std::cout << "C++98: 动态异常规格 (已弃用)" << std::endl;
    std::cout << "  void func() throw(std::bad_alloc);" << std::endl;
    std::cout << "  void func() throw();  // 不抛出" << std::endl;
    std::cout << "  问题:" << std::endl;
    std::cout << "    - 运行时检查, 违反时调用 std::unexpected" << std::endl;
    std::cout << "    - 编译器无法优化" << std::endl;
    std::cout << "    - 模板难以使用" << std::endl;

    std::cout << "\nC++11: noexcept 替代 throw()" << std::endl;
    std::cout << "  void func() noexcept;           // 不抛出" << std::endl;
    std::cout << "  void func() noexcept(true);     // 不抛出" << std::endl;
    std::cout << "  void func() noexcept(expr);     // 条件" << std::endl;
    std::cout << "  void func();                    // 可能抛出" << std::endl;

    std::cout << "\nC++17: throw() 移除" << std::endl;
    std::cout << "  throw() 等价于 noexcept(true)" << std::endl;
    std::cout << "  动态异常规格 throw(type_list) 被移除" << std::endl;

    std::cout << "\nC++20: noexcept 是函数类型的一部分" << std::endl;
    std::cout << "  void (*p1)() noexcept = func;  // OK" << std::endl;
    std::cout << "  void (*p2)() = noexcept_func;  // 错误!" << std::endl;
}

int main() {
    std::cout << "========== 异常设计深入 ==========\n" << std::endl;

    demo_custom_exception_hierarchy();
    demo_system_error();
    demo_noexcept();
    demo_exception_spec_history();

    return 0;
}
