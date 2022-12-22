#include <algorithm>
#include <functional>
#include <stdexcept>
#include <tsmp/introspect.hpp>
#include <catch2/catch_all.hpp>
#include <variant>

int print_counter = 0;
int print2_counter = 0;

struct foo_t {
    int i;
    float f;

    void print() const {
        ++print_counter;
    }

    void print2(int) const {
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

TEST_CASE("member field&function index calculation test", "[unit]") {
    foo_t foo {0, 0.0f};
    tsmp::introspect foo_introspecter(foo);

    static_assert(tsmp::introspect<foo_t>::field_id("i") == 1, "Index of i is wrong.");
    static_assert(tsmp::introspect<foo_t>::field_id("f") == 0, "Index of f is wrong.");

    static_assert(foo_introspecter.field_id("i") == 1, "Index of i is wrong.");
    static_assert(foo_introspecter.field_id("f") == 0, "Index of f is wrong.");

    static_assert(tsmp::introspect<foo_t>::function_id("print") == 2, "Index of i is wrong.");
    static_assert(tsmp::introspect<foo_t>::function_id("print2") == 3, "Index of f is wrong.");
    static_assert(tsmp::introspect<foo_t>::function_id("add") == 0, "Index of f is wrong.");
    static_assert(tsmp::introspect<foo_t>::function_id("inc") == 1, "Index of f is wrong.");

    static_assert(foo_introspecter.function_id("print") == 2, "Index of i is wrong.");
    static_assert(foo_introspecter.function_id("print2") == 3, "Index of f is wrong.");
    static_assert(foo_introspecter.function_id("add") == 0, "Index of f is wrong.");
    static_assert(foo_introspecter.function_id("inc") == 1, "Index of f is wrong.");
}

TEST_CASE("compile-time get member attribute test", "[unit]") {
    foo_t foo {0, 0.0f};
    tsmp::introspect foo_introspecter(foo);

    REQUIRE(foo.i == 0);
    foo_introspecter.get<"i">() = 13;
    REQUIRE(foo.i == 13);
    foo_introspecter.get<"i">() = 42;
    REQUIRE(foo.i == 42);

    REQUIRE(foo.f == Catch::Approx(0.0f));
    foo_introspecter.get<0>() = 13.0f;
    REQUIRE(foo.f == Catch::Approx(13.0f));
    foo_introspecter.get<"f">() = 42.0f;
    REQUIRE(foo.f == Catch::Approx(42.0f));
}

TEST_CASE("run-time get member attribute test", "[unit]") {
    foo_t foo {42, 43.0f};
    tsmp::introspect foo_introspecter(foo);

    REQUIRE(std::holds_alternative<int>(foo_introspecter.fetch("i")));
    REQUIRE(std::get<int>(foo_introspecter.fetch("i")) == 42);
    foo_introspecter.set("i", 43);
    REQUIRE(foo.i == 43);
    REQUIRE_THROWS(foo_introspecter.set("i", "wrong type"));
    REQUIRE_THROWS(foo_introspecter.set("i", 43.0f));

    REQUIRE(std::holds_alternative<float>(foo_introspecter.fetch("f")));
    REQUIRE(std::get<float>(foo_introspecter.fetch("f")) == Catch::Approx(43.0f));

    foo_introspecter.set("f", 43.0f);
    REQUIRE(foo.f == Catch::Approx(43.0f));
    REQUIRE_THROWS(foo_introspecter.set("f", "wrong type"));
    REQUIRE_THROWS(foo_introspecter.set("f", 43));
}

TEST_CASE("run-time visit member attributes test", "[unit]") {
    foo_t foo {42, 43.0f};
    tsmp::introspect foo_introspecter(foo);

    foo_introspecter.visit_fields([](auto, auto name, auto& field){
        if(name == "i") {
            REQUIRE(field == Catch::Approx(42));
            REQUIRE(std::is_same_v<decltype(field), int&>);
            field = 4;
        } else if(name == "f") {
            REQUIRE(field == Catch::Approx(43.0f));
            REQUIRE(std::is_same_v<decltype(field), float&>);
            field = 5.0f;
        }
    });

    REQUIRE(foo.i == 4);
    REQUIRE(foo.f == Catch::Approx(5.0f));
}

TEST_CASE("member function execution test, no arguments, no result", "[unit]") {
    const foo_t foo {43, 0.0f};
    tsmp::introspect foo_introspecter(foo);
    print_counter = 0;

    foo_introspecter.call<2>();
    REQUIRE(print_counter == 1);
    foo_introspecter.call<"print">();
    REQUIRE(print_counter == 2);
    const auto invoke_result = foo_introspecter.invoke("print");
    REQUIRE(std::holds_alternative<std::monostate>(invoke_result));
    REQUIRE(print_counter == 3);
}

TEST_CASE("member function execution test, with arguments, no result", "[unit]") {
    const foo_t foo {43, 0.0f};
    tsmp::introspect foo_introspecter(foo);
    print2_counter = 0;

    foo_introspecter.call<3>(42);
    REQUIRE(print2_counter == 1);
    foo_introspecter.call<"print2">(42);
    REQUIRE(print2_counter == 2);
    const auto invoke_result = foo_introspecter.invoke("print2", 5);
    REQUIRE(std::holds_alternative<std::monostate>(invoke_result));
    REQUIRE(print2_counter == 3);
}

TEST_CASE("member function execution test, with arguments and result","[unit]") {
    const foo_t foo {43, 0.0f};
    tsmp::introspect foo_introspecter(foo);

    REQUIRE(foo_introspecter.call<0>(5, 2) == 7);
    REQUIRE(foo_introspecter.call<"add">(5, 2) == 7);
    const auto invoke_result = foo_introspecter.invoke("add", 5, 2);
    REQUIRE(std::holds_alternative<int>(invoke_result));
    REQUIRE(std::get<int>(invoke_result) == 7);
}

TEST_CASE("member function execution test with state change", "") {
    foo_t foo {43, 0.0f};
    tsmp::introspect foo_introspecter(foo);
    print_counter = 0;
    print2_counter = 0;

    REQUIRE(foo.i == 43);
    foo_introspecter.call<1>();
    REQUIRE(foo.i == 44);

    foo_introspecter.call<"inc">();
    REQUIRE(foo.i == 45);

    const auto invoke_result = foo_introspecter.invoke("inc");
    REQUIRE(std::holds_alternative<std::reference_wrapper<foo_t>>(invoke_result) == true);
    REQUIRE(&(std::get<std::reference_wrapper<foo_t>>(invoke_result).get()) == &foo);
    REQUIRE(foo.i == 46);
}

TEST_CASE("constexpr member function execution test", "[unit]") {
    constexpr auto foo_2 = foo_t{ 0, 0 }.inc();
    // static_assert(foo_2.i == 1, "i should be 1");
    // static_assert(tsmp::introspect{ foo_2 }.call<0>(5, 2) == 7, "2+5 must be equal to 7");
    static_assert(std::get<int>(tsmp::introspect{ foo_2 }.invoke("add", 5, 2)) == 7, "2+5 must be equal to 7");
    // static_assert(std::holds_alternative<int>(tsmp::introspect{ foo_2 }.get("i")), "Index i must be of type int");
}