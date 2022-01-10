#include <tsmp/proxy.hpp>

#include <iostream>

struct foo_t {
    int i;

    auto add(int a, int b) {
        return a+b;
    }
};

struct interface_t {
    interface_t() = default;
    virtual ~interface_t() = default;
    interface_t(const interface_t&) = default;

    virtual void print() const = 0;
};


struct foo_impl : interface_t{
    virtual void print() const override {
        std::cout << "I am a foo_impl." << std::endl;
    };
};

struct bar_impl : interface_t{
    virtual void print() const override {
        std::cout << "I am a bar_impl." << std::endl;
    };
};


int main(int, char*[]) {

    tsmp::unique_proxy pfoo{ foo_t{}, [](auto base, std::string_view name, auto... args){
        std::cout << "call " << name << " with args ";
        ((std::cout << args << ", "), ...);
        return base(std::forward<decltype(args)>(args)...);
    }};

    tsmp::unique_proxy
    pfoo2 { std::move(pfoo) };
    
    auto result = pfoo.add(5, 7);
    pfoo.i = 12;
    std::cout << "result is " << result << std::endl;

    tsmp::polymorphic_value<interface_t> pvalue1 { foo_impl{} };
    pvalue1.print();

    auto cpy_foo = pvalue1;
    cpy_foo.print();

    tsmp::polymorphic_value<interface_t> pvalue2 { bar_impl{} };
    pvalue2.print();
}