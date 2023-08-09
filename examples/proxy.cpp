#include <concepts>
#include <functional>
#include <memory>
#include <stdexcept>
#include <tsmp/proxy.hpp>

#include <iostream>
#include <type_traits>
#include <utility>

struct foo_t
{
    int i;

    auto add(int a, int b) { return a + b; }
};

struct interface_t
{
    virtual ~interface_t() = default;

    virtual void print() const = 0;
};

struct foo_impl : interface_t
{
    void print() const override { std::cout << "I am a foo_impl." << std::endl; };
};

struct bar_impl : interface_t
{
    virtual void print() const override { std::cout << "I am a bar_impl." << std::endl; };
};

struct late_binding_example
{

    late_binding_example() = default;

    virtual ~late_binding_example() = default;

    virtual void foo()
    {
        std::cout << "late_binding_example::foo() called\n";
        bar();
    }

    virtual void bar() { std::cout << "late_binding_example::bar() called\n"; }
};

struct printer
{
    decltype(auto) operator()(auto& base, std::string_view name, auto&&... args) const
    {
        std::cout << "printer called " << name << " with args ";
        ((std::cout << args << ", "), ...);
        std::cout << "\n";
        return std::invoke(base, std::forward<decltype(args)>(args)...);
    }
};

int main(int, char*[])
{

    tsmp::unique_proxy pfoo{foo_t{}, printer{}};
    pfoo.add(5, 42);

    tsmp::polymorphic_value<interface_t> pvalue1{foo_impl{}};
    pvalue1.print();

    auto cpy_foo = pvalue1;
    cpy_foo.print();

    tsmp::polymorphic_value<interface_t> pvalue2{bar_impl{}};
    pvalue2.print();

    tsmp::polymorphic_value<interface_t> pvalue3{tsmp::virtual_proxy<foo_impl, printer>{}};
    pvalue3.print();

    tsmp::virtual_proxy<late_binding_example, printer> vproxy;
    vproxy.foo();
}