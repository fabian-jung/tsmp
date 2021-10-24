#include <catch2/catch.hpp>

#include <tsmp/reflect.hpp>
#include <cmath>

TEST_CASE("image attribute check for vec3", "[core][unit]") {
    struct foo_t {
        int i { 42 };
    } foo;
    const auto fields = tsmp::reflect<foo_t>::fields();
    const auto first = std::get<0>(fields);
    using value_type = typename decltype(first)::value_type;
    REQUIRE(std::is_same_v<value_type, int>);
    REQUIRE(foo.*(first.ptr) == 42);
    REQUIRE(first.name == "i");
}
