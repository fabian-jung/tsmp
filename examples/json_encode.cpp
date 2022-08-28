#include "tsmp/json.hpp"
#include <fmt/format.h>

int main(int, char*[]) {
    struct foo_t {
        enum class state_t {
            A,
            B,
            C
        } state = state_t::A;

        int i { 42 };
        // immutable_t<4> version;
        tsmp::string_literal_t<4> sl { "asdf" };
        tsmp::immutable_t<tsmp::string_literal_t{"some_string"}> immutable_string;
        float f { 1337.0f };
        std::string s = "Hello World!";
        struct bar_t {
            int i { 0 };
        } bar;
        std::array<int, 4> numbers_array { 1, 2, 3, 4 };
        std::vector<int> numbers_vector { 1, 2, 3, 4 };
        std::optional<int> oint = std::nullopt;
        std::variant<std::string, int> variant { 5 };
    } foo;

    fmt::print("{}\n", tsmp::to_json(foo));

    foo_t foo2 = tsmp::from_json<foo_t>(tsmp::to_json(foo_t{}));
    fmt::print("{}\n", tsmp::to_json(foo2));
    return 0;
}