#include <fmt/ranges.h>
#include <string_view>
#include <tuple>
#include <fmt/core.h>
#include <concepts>

#include <tsmp/reflect.hpp>
#include <iostream>

struct todo_ {

};

struct foo_t {
    const char* baba = "baba";
    std::string s = "asdas";

    constexpr void call() const& {
        fmt::print("foo::call() const& const was called;\n");
    }

    constexpr void call() && {
        fmt::print("foo::call() && const was called;\n");
    }

    constexpr void call2() const {
        fmt::print("foo::call2() const const was called;\n");
    }

    constexpr void call2() {
        fmt::print("foo::call() && const was called;\n");
    }

    constexpr void call3(int&& i) const {
        fmt::print("foo::call3({}) was called;\n", i);
    }


    constexpr void call4(const int& i) const {
        fmt::print("foo::call4({}) was called;\n", i);
    }


    constexpr void call5(int& i) const {
        fmt::print("foo::call5({}) was called;\n", i);
    }


    constexpr void call6(int const& i) const {
        fmt::print("foo::call6({}) was called;\n", i);
    }

    constexpr void call7(const int i) const {
        fmt::print("foo::call7({}) was called;\n", i);
    }

    constexpr void call8(int* const i) const {
        fmt::print("foo::call8({}) was called;\n", *i);
    }

    constexpr void call9(const int* i) const {
        fmt::print("foo::call9({}) was called;\n", *i);
    }
};

int main(int, const char**) {
    auto functions = tsmp::reflect<foo_t>::functions();
    std::invoke(std::get<0>(functions).ptr, foo_t{});
    std::invoke(std::get<1>(functions).ptr, foo_t{});
    std::invoke(std::get<2>(functions).ptr, foo_t{});
    return 0;
}