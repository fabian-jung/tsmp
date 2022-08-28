#include <catch2/catch_all.hpp>

#include <tsmp/reflect.hpp>
#include <cmath>

TEST_CASE("basic reflection test", "[core][unit]") {
    struct foo_t {
        int i { 42 };
    } foo;
    const auto fields = tsmp::reflect<foo_t>::fields();
    const auto first = std::get<0>(fields);
    using value_type = typename decltype(first)::value_type;
    static_assert(std::is_same_v<value_type, int>, "value type is not correct");
    REQUIRE(foo.*(first.ptr) == 42);
    REQUIRE(first.name == "i");
}

TEST_CASE("empty class reflection test", "[core][unit]") {
    struct foo_empty_t {
    };
    const auto fields = tsmp::reflect<foo_empty_t>::fields();
    static_assert(std::tuple_size_v<decltype(fields)> == 0, "Empty types has no fields.");
}

TEST_CASE("shared attributes reflection test", "[core][unit]") {
    struct foo_t {
        int i { 42 };
    } foo;

    struct bar_t {
        int i { 42 };
        float f { 3.1415 };
    } bar;

    constexpr auto foo_fields = tsmp::reflect<foo_t>::fields();
    constexpr auto bar_fields = tsmp::reflect<bar_t>::fields();

    const auto foo_i = std::get<0>(foo_fields);
    using foo_i_t = typename decltype(foo_i)::value_type;
    static_assert(std::is_same_v<foo_i_t, int>, "value type is not correct");
    REQUIRE(foo.*(foo_i.ptr) == 42);
    REQUIRE(foo_i.name == "i");

    const auto bar_i = std::get<0>(bar_fields);
    using bar_i_t = typename decltype(bar_i)::value_type;
    static_assert(std::is_same_v<bar_i_t, int>, "value type is not correct");
    REQUIRE(bar.*(bar_i.ptr) == 42);
    REQUIRE(bar_i.name == "i");

    const auto bar_f = std::get<1>(bar_fields);
    using bar_f_t = typename decltype(bar_f)::value_type;
    static_assert(std::is_same_v<bar_f_t, float>, "value type is not correct");
    REQUIRE(bar.*(bar_f.ptr) == Catch::Approx(3.1415f));
    REQUIRE(bar_f.name == "f");
}

TEST_CASE("member function reflection test", "[core][unit]") {
    struct foo_t {
        void print() {}
        int add(int a, int b) {
            return a + b;
        }
    };

    foo_t foo;
    constexpr auto fields = tsmp::reflect<foo_t>::fields();
    static_assert(std::tuple_size_v<decltype(fields)> == 0, "foo_t should not have any fields");

    constexpr auto functions = tsmp::reflect<foo_t>::functions();
    static_assert(std::tuple_size_v<decltype(functions)> >= 2, "foo_t should have functions");

    const auto print = std::get<0>(functions);
    REQUIRE(print.name == "print");

    const auto add = std::get<1>(functions);
    REQUIRE(add.name == "add");
    REQUIRE((foo.*(add.ptr))(2, 2) == 4);
}

TEST_CASE("enum test", "[core][unit]") {
    enum class foo_t : std::uint32_t {
        a = 0,
        b = 1337,
        c = 42
    };

    constexpr auto values = tsmp::enum_values<foo_t>;
    constexpr auto names = tsmp::enum_names<foo_t>;
    
    static_assert(values[0] == foo_t::a);
    static_assert(values[1] == foo_t::b);
    static_assert(values[2] == foo_t::c);

    static_assert(names[0] == "a");
    static_assert(names[1] == "b");
    static_assert(names[2] == "c");

    static_assert(tsmp::enum_from_string<foo_t>("a") == foo_t::a);
    static_assert(tsmp::enum_from_string<foo_t>("b") == foo_t::b);
    static_assert(tsmp::enum_from_string<foo_t>("c") == foo_t::c);

    static_assert(tsmp::enum_to_string(foo_t::a) == "a");
    static_assert(tsmp::enum_to_string(foo_t::b) == "b");
    static_assert(tsmp::enum_to_string(foo_t::c) == "c");
}