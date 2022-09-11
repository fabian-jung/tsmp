#include <tsmp/string_literal.hpp>
#include <catch2/catch_all.hpp>

#include <string_view>

template <auto string_literal>
constexpr auto template_value = string_literal;

TEST_CASE("string literal test", "[core][unit]") {
    static_assert(tsmp::string_literal_t("test") == "test");
    static_assert(tsmp::string_literal_t("test").size() == 4);
    static_assert(tsmp::string_literal_t("test")[0] == 't');
    static_assert(tsmp::string_literal_t("test")[1] == 'e');
    static_assert(tsmp::string_literal_t("test")[2] == 's');
    static_assert(tsmp::string_literal_t("test")[3] == 't');

    static_assert(template_value<tsmp::string_literal_t("test")> == "test");
}