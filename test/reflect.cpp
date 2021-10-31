#include <catch2/catch.hpp>

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

TEST_CASE("shared attributes reflection test", "[core][unit]") {
    struct foo_t {
        int i { 42 };
    } foo;

    struct bar_t {
        int i { 42 };
        float f { 3.1415 };
    } bar;

    // const auto foo_i = std::get<0>(tsmp::reflect<foo_t>::fields());
    // using foo_i_t = typename decltype(foo_i)::value_type;
    // static_assert(std::is_same_v<foo_i_t, int>, "value type is not correct");
    // REQUIRE(foo.*(foo_i.ptr) == 42);
    // REQUIRE(foo_i.name == "i");

    // const auto bar_i = std::get<0>(tsmp::reflect<bar_t>::fields());
    // using bar_i_t = typename decltype(bar_i)::value_type;
    // static_assert(std::is_same_v<bar_i_t, int>, "value type is not correct");
    // REQUIRE(foo.*(bar_i.ptr) == 42);
    // REQUIRE(bar_i.name == "i");

    // const auto bar_f = std::get<1>(tsmp::reflect<bar_t>::fields());
    // using bar_f_t = typename decltype(bar_f)::value_type;
    // static_assert(std::is_same_v<bar_f_t, float>, "value type is not correct");
    // REQUIRE(foo.*(bar_f.ptr) == 42);
    // REQUIRE(bar_f.name == "i");
}