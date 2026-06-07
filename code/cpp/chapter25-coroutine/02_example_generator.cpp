/**
 * @file 02_example_generator.cpp
 * @brief Generator生成器实现
 * @description 对应文档: 02-CPP/26-coroutine
 */

#include <iostream>
#include <coroutine>
#include <vector>
#include <string>
#include <sstream>
#include <memory>

template<typename T>
class Generator {
public:
    struct promise_type {
        T current_value;
        std::exception_ptr exception;

        Generator get_return_object() {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        std::suspend_always yield_value(T value) {
            current_value = std::move(value);
            return {};
        }
        void return_void() {}
        void unhandled_exception() {
            exception = std::current_exception();
        }
    };

    struct Iterator {
        std::coroutine_handle<promise_type> handle;
        bool done;

        Iterator(std::coroutine_handle<promise_type> h, bool d) : handle(h), done(d) {}

        Iterator& operator++() {
            handle.resume();
            done = handle.done();
            if (!done && handle.promise().exception) {
                std::rethrow_exception(handle.promise().exception);
            }
            return *this;
        }

        T operator*() const {
            return std::move(handle.promise().current_value);
        }

        bool operator!=(const Iterator& other) const {
            return done != other.done;
        }
    };

    Generator(std::coroutine_handle<promise_type> h) : handle(h) {}
    ~Generator() {
        if (handle) handle.destroy();
    }

    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;
    Generator(Generator&& other) noexcept : handle(other.handle) {
        other.handle = nullptr;
    }

    Iterator begin() {
        handle.resume();
        if (handle.promise().exception) {
            std::rethrow_exception(handle.promise().exception);
        }
        return Iterator{handle, handle.done()};
    }

    Iterator end() {
        return Iterator{handle, true};
    }

private:
    std::coroutine_handle<promise_type> handle;
};

Generator<int> range(int start, int end, int step = 1) {
    if (step > 0) {
        for (int i = start; i < end; i += step) co_yield i;
    } else if (step < 0) {
        for (int i = start; i > end; i += step) co_yield i;
    }
}

Generator<int> fibonacci() {
    int a = 0, b = 1;
    while (true) {
        co_yield a;
        auto temp = a;
        a = b;
        b = temp + b;
    }
}

Generator<int> primes() {
    std::vector<int> found;
    for (int n = 2; ; ++n) {
        bool is_prime = true;
        for (int p : found) {
            if (p * p > n) break;
            if (n % p == 0) { is_prime = false; break; }
        }
        if (is_prime) {
            found.push_back(n);
            co_yield n;
        }
    }
}

Generator<std::string> split_string(const std::string& text, char delimiter) {
    std::string token;
    for (char c : text) {
        if (c == delimiter) {
            if (!token.empty()) co_yield token;
            token.clear();
        } else {
            token += c;
        }
    }
    if (!token.empty()) co_yield token;
}

Generator<int> collatz(int n) {
    co_yield n;
    while (n != 1) {
        if (n % 2 == 0) n /= 2;
        else n = 3 * n + 1;
        co_yield n;
    }
}

template<typename T>
Generator<T> take(Generator<T> source, int count) {
    int i = 0;
    for (auto&& val : source) {
        if (i++ >= count) break;
        co_yield std::forward<decltype(val)>(val);
    }
}

template<typename T, typename Pred>
Generator<T> filter(Generator<T> source, Pred pred) {
    for (auto&& val : source) {
        if (pred(val)) {
            co_yield std::forward<decltype(val)>(val);
        }
    }
}

template<typename T, typename Func>
auto map(Generator<T> source, Func func) -> Generator<decltype(func(std::declval<T>()))> {
    for (auto&& val : source) {
        co_yield func(std::forward<decltype(val)>(val));
    }
}

void demo_basic_generator() {
    std::cout << "\n=== 基础生成器 ===\n";

    std::cout << "range(1, 10): ";
    for (int n : range(1, 10)) std::cout << n << " ";
    std::cout << "\n";

    std::cout << "range(10, 0, -2): ";
    for (int n : range(10, 0, -2)) std::cout << n << " ";
    std::cout << "\n";
}

void demo_lazy_sequence() {
    std::cout << "\n=== 惰性序列 ===\n";

    std::cout << "斐波那契前15个: ";
    auto fib = fibonacci();
    int count = 0;
    for (int n : fib) {
        std::cout << n << " ";
        if (++count >= 15) break;
    }
    std::cout << "\n";

    std::cout << "前20个素数: ";
    auto p = primes();
    count = 0;
    for (int n : p) {
        std::cout << n << " ";
        if (++count >= 20) break;
    }
    std::cout << "\n";

    std::cout << "Collatz序列(27): ";
    for (int n : collatz(27)) std::cout << n << " ";
    std::cout << "\n";
}

void demo_string_generator() {
    std::cout << "\n=== 字符串生成器 ===\n";

    std::string text = "hello world from cpp20 coroutines";
    std::cout << "分割 \"" << text << "\":\n";
    for (const auto& word : split_string(text, ' ')) {
        std::cout << "  [" << word << "]\n";
    }
}

void demo_pipeline_pattern() {
    std::cout << "\n=== 管道模式 ===\n";

    auto result = map(
        filter(
            range(1, 50),
            [](int n) { return n % 3 == 0; }
        ),
        [](int n) { return n * n; }
    );

    std::cout << "1-50中3的倍数的平方: ";
    for (int n : result) std::cout << n << " ";
    std::cout << "\n";

    auto pipeline = filter(
        map(
            fibonacci(),
            [](int n) { return n * 10; }
        ),
        [](int n) { return n > 50 && n < 1000; }
    );

    std::cout << "斐波那契*10, 过滤(50,1000): ";
    int cnt = 0;
    for (int n : pipeline) {
        std::cout << n << " ";
        if (++cnt >= 8) break;
    }
    std::cout << "\n";
}

int main() {
    std::cout << "========== Generator生成器实现 ==========\n";
    std::cout << "注意: GCC可能需要 -fcoroutines 编译选项\n";

    demo_basic_generator();
    demo_lazy_sequence();
    demo_string_generator();
    demo_pipeline_pattern();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
