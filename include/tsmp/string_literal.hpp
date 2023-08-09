#pragma once

#include <algorithm>
#include <array>
#include <cstring>
#include <string_view>

namespace tsmp {
template<size_t N>
struct string_literal_t : public std::array<char, N>
{

    constexpr string_literal_t(const char* cstr) noexcept
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
        const auto end_of_zero_delimited_str = std::distance(cstr, std::find(cstr, cstr + N, '\0'));
#pragma GCC diagnostic pop

        std::copy_n(cstr, end_of_zero_delimited_str, std::array<char, N>::begin());
        std::fill(std::array<char, N>::begin() + end_of_zero_delimited_str, std::array<char, N>::end(), '\0');
    }

    constexpr operator std::string_view() const noexcept { return std::string_view(std::array<char, N>::data(), N); }

    constexpr auto operator<=>(const string_literal_t&) const noexcept = default;
};

template<size_t N>
string_literal_t(const char (&s)[N]) -> string_literal_t<N - 1>;

}