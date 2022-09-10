#include <string>
#include <catch2/catch_all.hpp>

std::string function_with_reflection();

TEST_CASE("reflection with linking test", "[unit]") {
    REQUIRE(function_with_reflection() == "foo_t");
}