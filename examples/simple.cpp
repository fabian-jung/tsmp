#include <fmt/ranges.h>
#include <string_view>
#include <tuple>
#include <fmt/core.h>
#include <concepts>

#include <tsmp/introspect.hpp>
#include <iostream>

template <typename T> 
concept arithmetic = std::floating_point<T> || std::integral<T>;

std::string to_json(const char* value) {
    return fmt::format("\"{}\"", value);
}

std::string to_json(const arithmetic auto& value) {
    return fmt::format("{}", value);
}

std::string to_json(const std::string& value) {
    return fmt::format("\"{}\"", value);
}

std::string to_json(const auto& value) {
    tsmp::introspect introspecter{ value };
    std::string result;
    if constexpr(introspecter.has_fields()) {
        const auto fields = introspecter.visit_fields([&](size_t, std::string_view name, const auto& value) {
            return fmt::format("\"{}\":{}", name, to_json(value));
        });
        return fmt::format("{{{}}}",  fmt::join(fields, ", "));
    } else {
        return "{}";
    }
}

struct bar_t {
    const char* baba = "baba";
    std::string s = "asdas";
};

namespace some::special::ns {
struct foo_t {
    int a { 42 };
    // std::string s { "test" };
    float f = 3.14;

    
    bar_t bar;
};
}

struct outer_t {
    struct inner_t {
        int some_inner_i = 0;
    };

    inner_t some_outer_i {};
};

int main(int, const char**) {
    tsmp::reflect<some::special::ns::foo_t>::functions();

	std::cout << to_json(some::special::ns::foo_t{}) << std::endl;
    std::cout << to_json(outer_t::inner_t{}) << std::endl;
    std::cout << to_json(outer_t{}) << std::endl;

    return 0;
}