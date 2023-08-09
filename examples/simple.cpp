#include <concepts>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <string_view>
#include <tuple>

#include <iostream>
#include <tsmp/reflect.hpp>

struct todo_
{};

struct foo_t
{
    const char* baba = "baba";
    std::string s = "asdas";

    void call() const& { fmt::print("foo::call() const& const was called;\n"); }

    void call() && { fmt::print("foo::call() && const was called;\n"); }

    void call2() const { fmt::print("foo::call2() const const was called;\n"); }

    void call2() { fmt::print("foo::call() && const was called;\n"); }

    void call3(int&& i) const { fmt::print("foo::call3({}) was called;\n", i); }

    void call4(const int& i) const { fmt::print("foo::call4({}) was called;\n", i); }

    void call5(int& i) const { fmt::print("foo::call5({}) was called;\n", i); }

    void call6(int const& i) const { fmt::print("foo::call6({}) was called;\n", i); }

    void call7(const int i) const { fmt::print("foo::call7({}) was called;\n", i); }

    void call8(int* const i) const { fmt::print("foo::call8({}) was called;\n", *i); }

    void call9(const int* i) const { fmt::print("foo::call9({}) was called;\n", *i); }

    void call9(const int* i) { fmt::print("foo::call9({}) was called;\n", *i); }
};

int main(int, const char**)
{
    auto functions = tsmp::reflect<foo_t>::functions();
    std::invoke(std::get<0>(functions).ptr, foo_t{});
    std::invoke(std::get<1>(functions).ptr, foo_t{});
    std::invoke(std::get<2>(functions).ptr, foo_t{});

    return 0;
}