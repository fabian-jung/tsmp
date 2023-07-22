#include <catch2/catch_all.hpp>

#include <tsmp/reflect.hpp>
#include <cmath>

struct foo_t {
    int i { 42 };
};

TEST_CASE("basic reflection test", "[core][unit]") {
    foo_t foo;
    const auto fields = tsmp::reflect<foo_t>::fields();
    const auto first = std::get<0>(fields);
    using value_type = typename decltype(first)::value_type;
    static_assert(std::is_same_v<value_type, int>, "value type is not correct");
    REQUIRE(foo.*(first.ptr) == 42);
    REQUIRE(std::string(first.name) == "i");
}

struct foo_empty_t {
};

TEST_CASE("empty class reflection test", "[core][unit]") {
    
    const auto fields = tsmp::reflect<foo_empty_t>::fields();
    static_assert(std::tuple_size_v<decltype(fields)> == 0, "Empty types has no fields.");
}


struct bar_t {
    int i { 42 };
    float f { 3.1415 };
};

TEST_CASE("shared attributes reflection test", "[core][unit]") {
    foo_t foo;
    bar_t bar;

    constexpr auto foo_fields = tsmp::reflect<foo_t>::fields();
    constexpr auto bar_fields = tsmp::reflect<bar_t>::fields();

    const auto foo_i = std::get<0>(foo_fields);
    using foo_i_t = typename decltype(foo_i)::value_type;
    static_assert(std::is_same_v<foo_i_t, int>, "value type is not correct");
    REQUIRE(foo.*(foo_i.ptr) == 42);
    REQUIRE(std::string(foo_i.name) == "i");

    const auto bar_i = std::get<0>(bar_fields);
    using bar_i_t = typename decltype(bar_i)::value_type;
    static_assert(std::is_same_v<bar_i_t, int>, "value type is not correct");
    REQUIRE(bar.*(bar_i.ptr) == 42);
    REQUIRE(std::string(bar_i.name) == "i");

    const auto bar_f = std::get<1>(bar_fields);
    using bar_f_t = typename decltype(bar_f)::value_type;
    static_assert(std::is_same_v<bar_f_t, float>, "value type is not correct");
    REQUIRE(bar.*(bar_f.ptr) == Catch::Approx(3.1415f));
    REQUIRE(std::string(bar_f.name) == "f");
}

struct member_function_t {
    void print() {}
    int add(int a, int b) {
        return a + b;
    }
};

TEST_CASE("member function reflection test", "[core][unit]") {
    member_function_t memfn;
    constexpr auto fields = tsmp::reflect<member_function_t>::fields();
    static_assert(std::tuple_size_v<decltype(fields)> == 0, "member_function_t should not have any fields");

    constexpr auto functions = tsmp::reflect<member_function_t>::functions();
    static_assert(std::tuple_size_v<decltype(functions)> >= 2, "member_function_t should have functions");

    const auto print = std::get<0>(functions);
    REQUIRE(std::string(print.name) == "print");

    const auto add = std::get<1>(functions);
    REQUIRE(std::string(add.name) == "add");
    REQUIRE((memfn.*(add.ptr))(2, 2) == 4);
}

enum class enum_t : std::uint32_t {
    a = 0,
    b = 1337,
    c = 42
};

TEST_CASE("enum test", "[core][unit]") {


    constexpr auto values = tsmp::enum_values<enum_t>;
    constexpr auto names = tsmp::enum_names<enum_t>;
    
    static_assert(values[0] == enum_t::a);
    static_assert(values[1] == enum_t::b);
    static_assert(values[2] == enum_t::c);

    static_assert(names[0] == "a");
    static_assert(names[1] == "b");
    static_assert(names[2] == "c");

    static_assert(tsmp::enum_from_string<enum_t>("a") == enum_t::a);
    static_assert(tsmp::enum_from_string<enum_t>("b") == enum_t::b);
    static_assert(tsmp::enum_from_string<enum_t>("c") == enum_t::c);

    static_assert(tsmp::enum_to_string(enum_t::a) == "a");
    static_assert(tsmp::enum_to_string(enum_t::b) == "b");
    static_assert(tsmp::enum_to_string(enum_t::c) == "c");
}