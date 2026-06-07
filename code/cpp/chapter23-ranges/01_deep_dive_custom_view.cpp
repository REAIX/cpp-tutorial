/**
 * @file 01_deep_dive_custom_view.cpp
 * @brief 自定义View深入探讨
 * @description 对应文档: 02-CPP/24-ranges
 */

#include <iostream>
#include <vector>
#include <ranges>
#include <algorithm>
#include <concepts>
#include <iterator>
#include <sstream>

namespace detail {

template<std::ranges::input_range V>
requires std::ranges::view<V>
class take_every_n_view : public std::ranges::view_interface<take_every_n_view<V>> {
private:
    V base_;
    std::size_t step_;

    template<bool IsConst>
    class iterator {
        using Base = std::conditional_t<IsConst, const V, V>;
        using base_iterator = std::ranges::iterator_t<Base>;
        using base_sentinel = std::ranges::sentinel_t<Base>;

        base_iterator current_;
        base_sentinel end_;
        std::size_t step_;
        std::size_t pos_;

    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = std::ranges::range_value_t<Base>;
        using difference_type = std::ranges::range_difference_t<Base>;

        iterator() = default;

        iterator(base_iterator begin, base_sentinel end, std::size_t step)
            : current_(std::move(begin)), end_(std::move(end)), step_(step), pos_(0) {}

        auto operator*() const -> decltype(*current_) {
            return *current_;
        }

        iterator& operator++() {
            for (std::size_t i = 0; i < step_ && current_ != end_; ++i) {
                ++current_;
                ++pos_;
            }
            return *this;
        }

        void operator++(int) {
            ++(*this);
        }

        bool operator==(const base_sentinel& s) const {
            return current_ == s;
        }

        friend bool operator==(const iterator& it, const base_sentinel& s) {
            return it.current_ == s;
        }

        friend bool operator==(const iterator& a, const iterator& b) {
            return a.current_ == b.current_;
        }

        friend bool operator!=(const iterator& a, const iterator& b) {
            return a.current_ != b.current_;
        }
    };

public:
    take_every_n_view() = default;

    take_every_n_view(V base, std::size_t step)
        : base_(std::move(base)), step_(step) {}

    auto begin() {
        return iterator<false>(std::ranges::begin(base_), std::ranges::end(base_), step_);
    }

    auto end() {
        return iterator<false>(std::ranges::end(base_), std::ranges::end(base_), step_);
    }
};

}

namespace views {

struct take_every_n_fn {
    std::size_t step_;

    explicit take_every_n_fn(std::size_t step) : step_(step) {}

    template<std::ranges::input_range R>
    requires std::ranges::viewable_range<R>
    auto operator()(R&& r) const {
        return detail::take_every_n_view<std::views::all_t<R>>(
            std::views::all(std::forward<R>(r)), step_);
    }
};

struct take_every_n_adapter {
    template<std::ranges::input_range R>
    requires std::ranges::viewable_range<R>
    auto operator()(R&& r, std::size_t step) const {
        return detail::take_every_n_view<std::views::all_t<R>>(
            std::views::all(std::forward<R>(r)), step);
    }
};

inline constexpr take_every_n_adapter take_every_n;

}

void demo_custom_view() {
    std::cout << "\n=== 自定义View: take_every_n ===\n";

    std::vector<int> nums = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto every2 = views::take_every_n(nums, 2);
    std::cout << "每隔2个取一个: ";
    for (int n : every2) std::cout << n << " ";
    std::cout << "\n";

    auto every3 = views::take_every_n(nums, 3);
    std::cout << "每隔3个取一个: ";
    for (int n : every3) std::cout << n << " ";
    std::cout << "\n";
}

void demo_view_concepts() {
    std::cout << "\n=== View概念检查 ===\n";

    std::vector<int> vec = {1, 2, 3, 4, 5};

    std::cout << "vector是view: " << std::boolalpha
              << std::ranges::view<std::vector<int>> << "\n";
    std::cout << "vector是viewable_range: " << std::boolalpha
              << std::ranges::viewable_range<std::vector<int>&> << "\n";
    std::cout << "filter_view是view: " << std::boolalpha
              << std::ranges::view<std::ranges::filter_view<std::views::all_t<std::vector<int>&>, bool(*)(int)>> << "\n";
    std::cout << "vector是input_range: " << std::boolalpha
              << std::ranges::input_range<std::vector<int>> << "\n";
    std::cout << "vector是forward_range: " << std::boolalpha
              << std::ranges::forward_range<std::vector<int>> << "\n";
    std::cout << "vector是random_access_range: " << std::boolalpha
              << std::ranges::random_access_range<std::vector<int>> << "\n";
    std::cout << "vector是sized_range: " << std::boolalpha
              << std::ranges::sized_range<std::vector<int>> << "\n";

    auto v = vec | std::views::filter([](int n) { return n > 2; });
    std::cout << "filter_view是view: " << std::boolalpha
              << std::ranges::view<decltype(v)> << "\n";
    std::cout << "filter_view是input_range: " << std::boolalpha
              << std::ranges::input_range<decltype(v)> << "\n";
}

void demo_sentinel_pattern() {
    std::cout << "\n=== Sentinel模式 ===\n";

    std::vector<int> nums = {1, 2, 3, 0, 4, 5, 0, 6};

    auto until_zero = nums | std::views::take_while([](int n) { return n != 0; });
    std::cout << "take_while(非0): ";
    for (int n : until_zero) std::cout << n << " ";
    std::cout << "\n";

    auto after_zero = nums | std::views::drop_while([](int n) { return n != 0; }) | std::views::drop(1);
    std::cout << "第一个0之后: ";
    for (int n : after_zero) std::cout << n << " ";
    std::cout << "\n";

    std::string text = "hello world cpp20 ranges";
    auto words = std::views::split(text, ' ');
    std::cout << "按空格分割: ";
    for (auto word : words) {
        std::string s;
        for (char c : word) s += c;
        std::cout << "[" << s << "] ";
    }
    std::cout << "\n";
}

void demo_range_adapter_closure() {
    std::cout << "\n=== Range适配器闭包 ===\n";

    auto square_and_filter = std::views::transform([](int n) { return n * n; })
        | std::views::filter([](int n) { return n > 10; });

    std::vector<int> a = {1, 2, 3, 4, 5};
    auto result = a | square_and_filter;
    std::cout << "组合适配器(平方->过滤>10): ";
    for (int n : result) std::cout << n << " ";
    std::cout << "\n";

    std::vector<int> b = {3, 4, 5, 6};
    auto result2 = b | square_and_filter;
    std::cout << "另一组数据: ";
    for (int n : result2) std::cout << n << " ";
    std::cout << "\n";

    auto process = std::views::filter([](int n) { return n % 2 == 0; })
        | std::views::transform([](int n) { return n / 2; })
        | std::views::take(3);
    std::vector<int> c = {2, 4, 6, 8, 10, 12, 14};
    auto result3 = c | process;
    std::cout << "偶数/2取前3: ";
    for (int n : result3) std::cout << n << " ";
    std::cout << "\n";
}

void demo_pitfalls() {
    std::cout << "\n=== 常见陷阱 ===\n";

    auto temp_view = std::vector<int>{1, 2, 3} | std::views::take(2);
    std::cout << "注意: 临时vector的视图 - 仅因组合表达式延长了生命周期\n";
    for (int n : temp_view) std::cout << n << " ";
    std::cout << "\n";

    std::cout << "\n陷阱: 对临时容器直接取视图是危险的\n";
    std::cout << "正确做法: 先保存容器, 再取视图\n";
    std::vector<int> saved = {1, 2, 3};
    auto safe_view = saved | std::views::take(2);
    for (int n : safe_view) std::cout << n << " ";
    std::cout << "\n";

    std::cout << "\n陷阱: filter_view不满足random_access_range\n";
    auto fv = saved | std::views::filter([](int) { return true; });
    std::cout << "filter_view是random_access: " << std::boolalpha
              << std::ranges::random_access_range<decltype(fv)> << "\n";

    std::cout << "\n陷阱: 视图是惰性的, 每次遍历都会重新计算\n";
    int call_count = 0;
    auto counting = saved | std::views::transform([&call_count](int n) {
        ++call_count;
        return n * 2;
    });
    for (int n : counting) { (void)n; }
    std::cout << "第一次遍历调用次数: " << call_count << "\n";
    call_count = 0;
    for (int n : counting) { (void)n; }
    std::cout << "第二次遍历调用次数: " << call_count << "\n";
}

int main() {
    std::cout << "========== 自定义View深入探讨 ==========\n";

    demo_custom_view();
    demo_view_concepts();
    demo_sentinel_pattern();
    demo_range_adapter_closure();
    demo_pitfalls();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
