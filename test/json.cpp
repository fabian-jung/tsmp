#include "tsmp/json.hpp"
#include "tsmp/string_literal.hpp"
#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <optional>
#include <deque>
#include <forward_list>

TEST_CASE("arithmetic json test", "[core][unit]") {
    REQUIRE(tsmp::to_json(static_cast<std::int32_t>(42)) == "42");
    REQUIRE(tsmp::to_json(static_cast<std::uint32_t>(42)) == "42");
    REQUIRE(tsmp::to_json(static_cast<std::int8_t>(42)) == "42");
    REQUIRE(tsmp::to_json(static_cast<std::uint8_t>(42)) == "42");
    REQUIRE(tsmp::to_json(static_cast<float>(42.1337)) == "42.1337");
    REQUIRE(tsmp::to_json(static_cast<double>(42)) == "42");

    REQUIRE(tsmp::from_json<std::uint32_t>("42.0") == 42);
    REQUIRE(tsmp::try_from_json<std::uint32_t>("42").value() == 42);
    REQUIRE_THROWS(tsmp::from_json<std::uint32_t>("\"abc\""));
    REQUIRE(tsmp::try_from_json<std::uint32_t>("\"abc'\"") == std::nullopt);
}

TEST_CASE("enum json test", "[core][unit]") {
    enum class enum_t {
        value1,
        value2,
        value3
    };

    REQUIRE(tsmp::to_json(enum_t::value1) == "\"value1\"");
    REQUIRE(tsmp::to_json(enum_t::value2) == "\"value2\"");
    REQUIRE(tsmp::to_json(enum_t::value3) == "\"value3\"");

    REQUIRE(tsmp::from_json<enum_t>("\"value1\"") == enum_t::value1);
    REQUIRE(tsmp::from_json<enum_t>("\"value2\"") == enum_t::value2);
    REQUIRE(tsmp::from_json<enum_t>("\"value3\"") == enum_t::value3);
}

TEST_CASE("string json test", "[core][unit]") {
    REQUIRE(tsmp::to_json("test string") == "\"test string\"");
    
    REQUIRE(tsmp::from_json<std::string>("\"test string\"") == "test string");
}

TEST_CASE("array json test", "[core][unit]") {
    REQUIRE(tsmp::to_json(std::array<int, 4>{{1, 2, 3, 4}}) == "[1,2,3,4]");

    REQUIRE(tsmp::from_json<std::array<int, 4>>("[1,2,3,4]") == std::array<int, 4>{{1, 2, 3, 4}});
    REQUIRE_THROWS(tsmp::from_json<std::array<int, 4>>("[1,2,3,4,5]"));
    REQUIRE_THROWS(tsmp::from_json<std::array<int, 4>>("[1,2,3]"));
    REQUIRE_THROWS(tsmp::from_json<std::array<int, 0>>("{}"));
    REQUIRE_THROWS(tsmp::from_json<std::array<int, 4>>("\"abcd\""));
    REQUIRE_THROWS(tsmp::from_json<std::array<int, 1>>("4"));
    REQUIRE(tsmp::from_json<std::array<int, 0>>("[]") == std::array<int, 0>{});
}

TEST_CASE("string_literal_t json test", "[core][unit]") {
    REQUIRE(tsmp::to_json(tsmp::string_literal_t("test string")) == "\"test string\"");
    REQUIRE(tsmp::from_json<tsmp::string_literal_t<5>>("\"12345\"") == tsmp::string_literal_t("12345"));
    REQUIRE(tsmp::from_json<tsmp::string_literal_t<128>>("\"12345\"") == tsmp::string_literal_t<128>("12345"));
    REQUIRE_THROWS(tsmp::from_json<tsmp::string_literal_t<3>>("\"12345\""));
}

TEST_CASE("immutable json test", "[core][unit]") {
    REQUIRE(tsmp::to_json(tsmp::immutable_t<5>()) == "5");
    REQUIRE(tsmp::to_json(tsmp::immutable_t<tsmp::string_literal_t("test")>()) == "\"test\"");

    REQUIRE(tsmp::from_json<tsmp::immutable_t<5>>("5") == 5);
    REQUIRE(tsmp::from_json<tsmp::immutable_t<tsmp::string_literal_t("test")>>("\"test\"").get() == "test");
    REQUIRE_THROWS(tsmp::from_json<tsmp::immutable_t<5>>("6"));
    REQUIRE_THROWS(tsmp::from_json<tsmp::immutable_t<tsmp::string_literal_t("test")>>("\"test2\""));
    REQUIRE_THROWS(tsmp::from_json<tsmp::immutable_t<tsmp::string_literal_t("test")>>("\"tes\""));
}

TEST_CASE("range json test", "[core][unit]") {
    REQUIRE(tsmp::to_json(std::vector{ 1, 2, 3}) == "[1,2,3]");
    REQUIRE(tsmp::to_json(std::deque{ 1, 2, 3}) == "[1,2,3]");
    REQUIRE(tsmp::to_json(std::forward_list{ 1, 2, 3}) == "[1,2,3]");

    REQUIRE(tsmp::from_json<std::vector<int>>("[1,2,3]") == std::vector{1,2,3});
    REQUIRE(tsmp::from_json<std::deque<int>>("[1,2,3]") == std::deque{1,2,3});
    REQUIRE(tsmp::from_json<std::forward_list<int>>("[1,2,3]") == std::forward_list{1,2,3});

    REQUIRE_THROWS(tsmp::from_json<std::vector<int>>("[\"1\",\"2\",\"3\"]"));
    REQUIRE_THROWS(tsmp::from_json<std::vector<int>>("{\"1\",\"2\",\"3\"}"));
    REQUIRE_THROWS(tsmp::from_json<std::vector<int>>("{\"1\":\"2\"}"));
    REQUIRE_THROWS(tsmp::from_json<std::vector<int>>("1"));
    REQUIRE_THROWS(tsmp::from_json<std::vector<int>>("\"1\""));
}

TEST_CASE("variant json test", "[core][unit]") {
    using variant = std::variant<int, float, std::string>;
    REQUIRE(tsmp::to_json(variant("test")) == "\"test\"");
    REQUIRE(tsmp::to_json(variant(5)) == "5");
    REQUIRE(tsmp::to_json(variant(5.0f)) == "5");

    REQUIRE(tsmp::from_json<variant>("\"test\"") == variant("test"));
    REQUIRE(tsmp::from_json<variant>("5") == variant(5));

    REQUIRE(tsmp::try_from_json<variant>("\"test\"") == variant("test"));
    REQUIRE(tsmp::try_from_json<variant>("5") == variant(5));

    // JSON does not distinguish between int and float, therfore the first compatible type will be choosen
    REQUIRE(tsmp::from_json<variant>("5.000000") == variant(5));

    REQUIRE_THROWS(tsmp::from_json<variant>("{}"));
    REQUIRE_THROWS(tsmp::from_json<variant>("[5]"));
}

TEST_CASE("struct json test", "[core][unit]") {
    struct foo_t {
        int i;
        std::string str;
        auto operator<=>(const foo_t&) const noexcept = default;
    };
    const foo_t foo { 42, "test" };
    
    REQUIRE(tsmp::to_json(foo) == "{\"i\":42,\"str\":\"test\"}");
    REQUIRE(tsmp::from_json<foo_t>("{\"i\":42,\"str\":\"test\"}") == foo);

    REQUIRE_THROWS(tsmp::from_json<foo_t>("[]"));
    REQUIRE_THROWS(tsmp::from_json<foo_t>("{\"i\":42}"));
    REQUIRE(tsmp::try_from_json<foo_t>("[]") == std::nullopt);
    REQUIRE(tsmp::try_from_json<foo_t>("{\"i\":42}") == std::nullopt);
}

TEST_CASE("struct with optional json test", "[core][unit]") {
    struct foo_t {
        int i;
        std::optional<std::string> str;
        auto operator<=>(const foo_t&) const noexcept = default;
    };
    const foo_t foo { 42, "test" };
    const foo_t foo2 { 42, std::nullopt };
    
    REQUIRE(tsmp::to_json(foo) == "{\"i\":42,\"str\":\"test\"}");
    REQUIRE(tsmp::from_json<foo_t>("{\"i\":42,\"str\":\"test\"}") == foo);
    REQUIRE(tsmp::from_json<foo_t>("{\"i\":42 }") == foo2);

    REQUIRE(tsmp::try_from_json<foo_t>("{\"i\":42,\"str\":\"test\"}") == foo);
    REQUIRE(tsmp::try_from_json<foo_t>("{\"i\":42 }") == foo2);
}

TEST_CASE("validator json test", "[core][unit]") {
    constexpr const auto is_fourtytwo = [](auto number){ return number == 42; };
    constexpr const auto not_fourtytwo = [](auto number){ return number != 42; };

    REQUIRE(tsmp::from_json<std::uint32_t>("42.0", is_fourtytwo) == 42);
    REQUIRE(tsmp::try_from_json<std::uint32_t>("42", not_fourtytwo) == std::nullopt);
}