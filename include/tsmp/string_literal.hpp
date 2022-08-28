#pragma once

#include <algorithm>
#include <string_view>
#include <array>

namespace tsmp {
template <size_t N>
struct string_literal_t : public std::array<char, N> {

    constexpr string_literal_t(const char* c_str) noexcept
    {
        const auto end = std::find(c_str, c_str+N, '\0');
        std::copy(c_str, end, std::array<char, N>::begin());
        std::fill(std::array<char, N>::begin()+std::distance(c_str, end), std::array<char, N>::end(), '\0');
    }

    template <class... Args>
    string_literal_t(Args&&... args) :
        std::array<char, N>{std::forward<Args>(args)...}
    {}

    constexpr operator std::string_view() const noexcept{
        return std::string_view(std::array<char, N>::data(), N);
    }
};

template <size_t N>
string_literal_t(const char (&s)[N]) -> string_literal_t<N-1>;

}