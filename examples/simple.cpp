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
    return "\""+std::string(value)+"\"";
}

std::string to_json(const arithmetic auto& value) {
    return std::to_string(value);
}

std::string to_json(const std::string& value) {
    return "\""+value+"\"";
}

std::string to_json(const auto& value) {
    tsmp::introspect introspecter{ value };
    std::string result;
    const auto fields = introspecter.visit_fields([&](size_t, std::string_view name, const auto& value){
        return fmt::format("\"{}\":{}", name, to_json(value));
    });
    return fmt::format("{{{}}}",  fmt::join(fields, ", "));
}

struct bar_t {
    const char* baba = "baba";
    std::string s = "asdas";
};

struct foo_t {
    int a { 42 };
    // std::string s { "test" };
    float f = 3.14;
    
    bar_t bar;
};

int main(int, const char**) {

	std::cout << to_json(foo_t{}) << std::endl;
    
    // float f;
    // std::cout<< tsmp::reflect<float>::name << std::endl;
    return 0;
}