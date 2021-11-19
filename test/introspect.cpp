#include <algorithm>
#include <stdexcept>
#include <tsmp/introspect.hpp>
#include <catch2/catch.hpp>

int print_counter = 0;
int print2_counter = 0;

TEST_CASE("member function reflection test", "[unit]") {

    struct foo_t {
        int i;
        float f;

        void print() {
            ++print_counter;
        }

        void print2(int i) {
            ++print2_counter;
        }

        constexpr int add(int lhs, int rhs) const {
            return lhs + rhs;
        }

        constexpr auto& inc() {
            ++i;
            return *this;
        }
    };

    foo_t foo {0, 0.0f};
    
    #warning this line should not be needed
    using reflect = tsmp::reflect<foo_t>;


    tsmp::introspect foo_introspecter(foo);

    static_assert(tsmp::introspect<foo_t>::field_id("i") == 0, "Index of i is wrong.");
    static_assert(tsmp::introspect<foo_t>::field_id("f") == 1, "Index of f is wrong.");

    static_assert(foo_introspecter.field_id("i") == 0, "Index of i is wrong.");
    static_assert(foo_introspecter.field_id("f") == 1, "Index of f is wrong.");

    static_assert(tsmp::introspect<foo_t>::function_id("print") == 0, "Index of i is wrong.");
    static_assert(tsmp::introspect<foo_t>::function_id("print2") == 1, "Index of f is wrong.");
    static_assert(tsmp::introspect<foo_t>::function_id("add") == 2, "Index of f is wrong.");
    static_assert(tsmp::introspect<foo_t>::function_id("inc") == 3, "Index of f is wrong.");

    static_assert(foo_introspecter.function_id("print") == 0, "Index of i is wrong.");
    static_assert(foo_introspecter.function_id("print2") == 1, "Index of f is wrong.");
    static_assert(foo_introspecter.function_id("add") == 2, "Index of f is wrong.");
    static_assert(foo_introspecter.function_id("inc") == 3, "Index of f is wrong.");

    REQUIRE(foo.i == 0);
    foo_introspecter.get<"i">() = 13;
    REQUIRE(foo.i == 13);
    foo_introspecter.get<"i">() = 42;
    REQUIRE(foo.i == 42);

    REQUIRE(foo.f == 0);
    foo_introspecter.get<1>() = 13.0f;
    REQUIRE(foo.f == 13.0f);
    foo_introspecter.get<"f">() = 42.0f;
    REQUIRE(foo.f == 42.0f);

    REQUIRE(std::holds_alternative<int>(foo_introspecter.get("i")));
    REQUIRE(std::get<int>(foo_introspecter.get("i")) == 42);
    
    REQUIRE(std::holds_alternative<float>(foo_introspecter.get("f")));
    REQUIRE(std::get<float>(foo_introspecter.get("f")) == 42.0f);

    foo_introspecter.call<0>();
    REQUIRE(print_counter == 1);
    foo_introspecter.call<"print">();
    REQUIRE(print_counter == 2);
    foo_introspecter.call<1>(42);
    REQUIRE(print2_counter == 1);
    foo_introspecter.call<"print2">(42);
    REQUIRE(print2_counter == 2);
    REQUIRE(foo_introspecter.call<2>(5, 2) == 7);
    REQUIRE(foo_introspecter.call<"add">(5, 2) == 7);

    REQUIRE(foo.i == 42);
    foo_introspecter.call<3>();
    REQUIRE(foo.i == 43);


    constexpr auto foo_2 = foo_t{ 0, 0 }.inc();
    static_assert(foo_2.i == 1, "i should be 1");
    static_assert(tsmp::introspect{ foo_2 }.call<2>(5, 2) == 7, "2+5 must be equal to 7");
}