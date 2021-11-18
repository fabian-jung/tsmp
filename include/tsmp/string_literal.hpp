#pragma once

#include <algorithm>

namespace tsmp {
template <size_t N>
struct string_literal_t {
   
    constexpr string_literal_t(const char (&s)[N])
    {
        std::copy_n(s, N-1, string);
    }

    constexpr operator std::string_view() const {
        return std::string_view(string, N-1);
    }

    char string[N-1];
};
}