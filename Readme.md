# Tool supported meta programming

This repo is a prove of concept how a static reflection library could be implemented with C++20 and code generation with the help of libclang and cmake.

The current implementation supports the following example application:
```cpp
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <tsmp/introspect.hpp>

#include <concepts>
#include <ranges>
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
    const auto elements = std::ranges::transform_view(range, static_cast<signature_t>(to_json));
    return fmt::format("[{}]", fmt::join(elements, ","));
}

std::string to_json(const auto& value) {
    tsmp::introspect introspect { value };
    const auto fields = introspect.visit_fields([](size_t id, std::string_view name, const auto& field) {
        return fmt::format("\"{}\":{}", name, to_json(field));
    });
    return fmt::format("{{{}}}", fmt::join(fields, ",")); 
}

int main(int, char*[]) {

    struct foo_t {
        int i { 42 };
        float f { 1337.0f };
        const char* s = "Hello World!";
        struct bar_t {
            int i { 0 };
        } bar;
        std::array<int, 4> numbers { 1, 2, 3, 4 };
    } foo;

    fmt::print("{}\n", to_json(foo));
    // Prints: {"i":42,"f":1337.000000,"s":"Hello World!","bar":{"i":0},"numbers":[1,2,3,4]}
    return 0;
}
```

Because as of now there is no support for static reflection in C++ the meta-data from the type system needs to be made accessible to your application. This is done via a helper tool, that will parse your source code and generates a special type trait, that is used by the library. 

This step does not need any user intervention or addition of macros to your source code. The code generator needs to be integrated into the build system, but can be done pretty easy for cmake with the help of the function ```enable_reflection()```.

```cmake
add_executable(example)
add_target_library(example PRIVATE ...)
enable_reflection(example) # This line will add code generation for this target
```

# Dependencies

The ```tsmp::reflect<>``` trait is specialized using concepts. Therefore a c++20 compliant compiler is required. gcc-11 is used in the CI-Pipeline. Additionaly libclang and the llvm runtime needs to be installed. The code generator is implemented with the help of the fmt lib.
The [ci pipeline](.github/workflows/cmake.yml) shows a complete workflow including setup, build and test execution based on a ubuntu 20.04 image.

# API

Currently there are two ways to interact with tsmp. You can fetch the raw field and function descriptions via ```tsmp::reflect``` or get a more user friendly interface via ```tsmp::introspect```

## Get raw reflection data

The access to reflection is done via the ```tsmp::reflect<T>``` type trait. This trait exports a ```name()```, ```fields()``` and ```functions()``` member functions. The name will does return const char* of the unmangled typename T, if the type can be identified via its members. Otherwise it will retunr the string ```"<unknown>"```. The ```fields()``` and ```functions()``` returns a tuple of speciallized ```field_description_t```. This type holds fields with a id, name of the member and pointer to member. Its not guranteed that the id of attributes will relate to the position in the binary layout of the type.

The definition of the field_description_t can be found here [reflect.hpp](include/tsmp/reflect.hpp).

```cpp
// Pseudo code of the interface

namespace tsmp {

template <class T, class V>
struct field_description_t {
    using value_type = V;
    std::size_t id;
    std::string_view name;
    V T::* ptr;
};

template <class T>
struct reflect {
    constexpr static const char* name();
    constexpr static std::tuple<field_description_t...> fields();
    constexpr static std::tuple<field_description_t...> functions();
};

}
```

Here is a short example of the basic usage:
```cpp

int main() {
    struct foo_t {
        int a, b;
    }
    constexpr auto fields = tsmp::reflect<foo_t>::fields();
    constexpr expr a_decl = std::get<0>(fields);
    constexpr expr b_decl = std::get<1>(fields);
    std::cout << "name of first field in foo_t: " << a_decl.name  << std::endl; // prints "a";
    std::cout << "name of second field in foo_t: " << b_decl.name  << std::endl; // prints "b";
}

```

## Introspect by reference

Working with tuples and pointer-to-member(functions) can be quite cumbersome. For that reason there is a helper class that can be used to directly access members of lvalues. This utility class is called ```tsmp::introspect``` and the implementation can be found here [introspect.hpp](include/tsmp/introspect.hpp). The interface looks more or less like this:

```cpp

namespace tsmp {

template <class T>
struct introspect {
    T& internal;

    constexpr introspect(T& lvalue) noexcept;

    static constexpr auto field_id(const std::string_view name);
    static constexpr auto function_id(const std::string_view name);

    template <size_t id, class... Args>
    constexpr decltype(auto) call(Args... args) const ;

    template <string_literal_t name, class... Args>
    constexpr decltype(auto) call(Args... args) const;

    template <size_t id>
    constexpr auto& get() const;

    template <string_literal_t name>
    constexpr auto& get() const;

    constexpr auto fetch(const std::string_view name) const;

    template <class Arg>
    constexpr auto set(const std::string_view name, Arg arg) const;

    template <class Visitor>
    constexpr std::array<auto> visit_fields(Visitor&& visitor) const;
};

}

```

The functions do pretty much, what the name and signature suggest. ```field_id()``` and ```function()``` return the ids of either a field of function. A member with the requested name does not exist, the function will throw in runtime contexts and do not compile in ```constexpr``` contexts.

The ```call()``` lets you call a function on the passed reference. The ```get()``` will return a reference to a member variable. The ```fetch()``` and The ```set()``` can be used to retreive or set a member variable in runtime-contexts. The ```fetch()``` function will return a ```std::variant``` containing all member types.

The The ```visit_fields()``` function lets you iterate over all member variables. It'll take a visitor with the signature ```auto(size_t id, std::string_view name, auto& value)```. The first parameter is a unique id for ever field. The second parameter contains the name of the member. The third parameter must be overloaded for every member type can be a (const-)reference or pass-by-value and will return the value of the corresponding field. The variant can return ```void``` for all overloads, then the ```visit_fields()``` will also return void or it can return an arbitraty, but same type T for every overload. In that case the result will be an ```std::array<T>```.

The usage of  ```tsmp::introspect``` could look like this:

```cpp
int main() {
    foo_t {
        int a;
        void bar() {};    
    } foo;

    tsmp::introspect intro { foo };

    intro.get<"a"> = 42;
    assert(std::get<int>(intro.fetch("a")> == 42);

    intro.call<"bar">();

    std::cout << "foo fields:" << std::endl;
    intro.visit_fields([](size_t id, std::string_view name, const auto& field){
        std::cout << "name:" << field << std::endl;
    });
}
```

# How does it work?

The source file `bin/introspect.cpp` is compiled into a tool that can parse source files and traverse the abstract syntax tree of your build. The tool will search the syntax tree for all occurrences of specializations of `tsmp::reflect`. Once found the template argument will be visited and scanned for fields and functions. With this data a header file is generated with `tsmp::reflect` specialization for the type, that looks somewhat like this:

```cpp
// ...
template <class T>
requires requires(T) { // specialize for every type that has the following members
    T::i;
    &T::bar;
}
struct reflect<T> {
    constexpr static auto name() {
        return "foo_t";
    }
    constexpr static auto fields() {
        return std::make_tuple(
               // this is the field description that will be passed to the user
               field_description_t{ 0, "i", &T::i }
        );
    }
    constexpr static auto functions() {
        return std::make_tuple(
               // this is the field description that will be passed to the user
               field_description_t{ 0, "bar", &T::bar }
        );
    }
};
//...
```

The cmake build system will take care of generating this header, setting the include path to it and correctly tracking the dependencies.


# Discussion

This library is not production ready and needs to handle a lot more of the corner cases. The basic idea seems viable, but all the corner cases needs to be addressed. If you want a further read into the code base I recommend starting with the [examples](examples), [reflect](test/reflect.cpp) and [introspect](test/introspect.cpp) tests and work your way up from there. If you are interested in the source tree parsing take a look at the [bin/introspect.cpp](tooling/bin/introspect.cpp)
