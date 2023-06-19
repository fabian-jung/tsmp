#include <fmt/core.h>
#include <fmt/ranges.h>
#include <tsmp/introspect.hpp>

#include <concepts>
#include <range/v3/view/transform.hpp>
#include <type_traits>

template <class T>
concept Arithmetic = std::floating_point<T> || std::integral<T>;

std::string to_json(const auto& value);

std::string to_json(const char* const& cstr) {
    return fmt::format("\"{}\"", cstr);
}

std::string to_json(const std::string& str) {
    return fmt::format("\"{}\"", str);
}

std::string to_json(const Arithmetic auto& number) {
    return std::to_string(number);
}

template <std::ranges::input_range Range>
std::string to_json(const Range& range) {
    using value_type = typename Range::value_type;
    using signature_t = std::string(*)(const value_type&);
    const auto elements = ranges::transform_view(range, static_cast<signature_t>(to_json));
    return fmt::format("[{}]", fmt::join(elements, ","));
}

std::string to_json(const auto& value) {
    tsmp::introspect introspect { value };
    if constexpr(introspect.has_fields()) {
        const auto fields = introspect.visit_fields([](size_t, std::string_view name, const auto& field) {
            return fmt::format("\"{}\":{}", name, to_json(field));
        });
        return fmt::format("{{{}}}", fmt::join(fields, ","));
    } else {
        return "{}";
    }
}

struct foo_t {
    int i { 42 };
    float f { 1337.0f };
    const char* s = "Hello World!";
    struct bar_t {
        int i { 0 };
    } bar;
    #warning todo
    std::vector<int> numbers { 1, 2, 3, 4 };
    // std::array<int, 4> numbers { 1, 2, 3, 4 };
};

int main(int, char*[]) {
    foo_t foo;
    fmt::print("{}\n", to_json(foo));

    return 0;
}