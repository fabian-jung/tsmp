# Tool supported meta programming

This repo is a prove of concept how a static reflection library could be implemented with C++20 and code generation with the help of libclang.

The current implementation supports the following syntax:
```cpp
#include <tsmp/reflect.hpp>

int main(int argc, char* argv[]) {
    struct foo_t {
        int i { 42 };
    } foo;

    // Get a std::tuple with field descriptions
    const auto fields = tsmp::reflect<foo_t>::fields();

    // fields does only have one value in this case. The signature of field looks like this:
    // field_t {
    //     size_t id = <implementation defined value>;
    //     const char* name = <name of the member>;
    //     int foo_t:: *ptr = &foo_t::i; // a pointer to member to the field
    // }
    const auto first = std::get<0>(fields);
    using value_type = typename decltype(first)::value_type;
    static_assert(std::is_same_v<value_type, int>, "value type is not correct");
    assert(foo.*(first.ptr) == 42);
    assert(first.name == "i");
}
```

The code generation is automated with the help of cmake, There is a macro defined, that will take care
of the parsing of the sources

```cmake
add_executable(example)
enable_reflection(example)
```

# Dependencies

The tsmp::reflect<> trait is specialized using concepts. Therefore clang-12 or gcc-11 is required to build targets with reflection. The source parsing is done with the help of libclang. Other than that there are no more requirements. The .github/workflows.yml shows a complete workflow to build examples & tests based on a ubuntu 20.04 image.

# How does it work?

The source file `bin/introspect.cpp` is compiled into a tool that can parse source files and traverse the abstract syntax tree. The tool will search this syntax tree for all occurrences of specializations of `tsmp::reflect`. Once found the template argument will be visited and scanned for fields. With this data a header file is generated with `tsmp::reflect` specialization, that looks somewhat like this:

```cpp
// ...
template <class T>
requires requires(T) { // specialize for every type that has the following members
   T::i;
}
struct reflect<T> {
    static constexpr bool reflectable = true;
    constexpr static auto fields() {
        return std::make_tuple(
               // this is the field description that will be passed to the user
               field_description_t{ 0, "i", &T::i }
        );
    }
};

//...
```

The cmake build system will take care of generating this header, setting the include path to it and correctly tracking the dependencies.

# Discussion

It would probably be possible to build a reflection library with this approach. There will be difficulties to distinguish types with identical member names. Private data members are not dealt with in this implementation and to be able to introspect types with partially matching member names additional logic in the introspect_tool is necessary.