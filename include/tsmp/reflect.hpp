#pragma once
#include <cstdint>
#include <string_view>
#include <tuple>

namespace tsmp {

template <class T, class V>
struct field_description_t {
    using value_type = V;
    std::size_t id;
    std::string_view name;
    V T::* ptr;
};
template <class T, class V>
field_description_t(std::size_t, std::string_view,  V T::*) -> field_description_t<T, V>;

#ifdef TSMP_REFLECTION_ENABLED
template <class T>
struct reflect;

#else
template <class T>
struct reflect {
    constexpr static bool reflectable = false;
    constexpr static auto fields() {
        return std::make_tuple();
    }
    constexpr static auto functions() {
        return std::make_tuple();
    }
};
#endif

}

#if !defined(INTROSPECT_PASS) && defined(TSMP_REFLECTION_ENABLED)
#include "reflection.hpp"
#endif