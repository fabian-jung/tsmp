#include <catch2/catch_all.hpp>

#include <tsmp/reflect.hpp>

TEST_CASE("basic reflection test", "[core][unit]") {
    struct foo_t {
        int i { 42 };
    } foo;
    const auto fields = tsmp::reflect<foo_t>::fields();
    const auto functions = tsmp::reflect<foo_t>::functions();
    static_assert(tsmp::reflect<foo_t>::reflectable == false, "Types must not be reflectable if tsmp has not been activated in cmake");
    static_assert(std::tuple_size_v<decltype(fields)> == 0, "fields must have size 0");
    static_assert(std::tuple_size_v<decltype(functions)> == 0, "functions must have size 0");
}