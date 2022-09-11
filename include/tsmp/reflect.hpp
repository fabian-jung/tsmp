#pragma once
#include <cstdint>
#include <string_view>
#include <tuple>
#include <range/v3/algorithm/transform.hpp>
#include <range/v3/algorithm/find.hpp>

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

template <class T>
concept Enum = std::is_enum_v<T>;

template <Enum E>
struct enum_entry_description_t {
    std::string_view name;
    E value;

    static constexpr std::string_view get_name(enum_entry_description_t<E> entry) noexcept {
        return entry.name;
    }

    static constexpr E get_value(enum_entry_description_t<E> entry) noexcept {
        return entry.value;
    }
};

#ifndef TSMP_INTROSPECT_PASS
template <class T>
struct reflect;

template<Enum E>
struct enum_value_adapter;

#else
template <class T>
struct reflect {
    constexpr static bool reflectable = false;
    constexpr static auto name()  {
        return "";
    }
    constexpr static auto fields() {
        return std::tuple<>();
    }
    constexpr static auto functions() {
        return std::tuple<>();
    }
};

template<Enum E>
struct enum_value_adapter {
    constexpr static std::array<enum_entry_description_t<E>, 0> values {};
};

#endif

template <Enum E>
constexpr auto enum_values = [](){
    constexpr auto enum_fields = enum_value_adapter<E>::values;
    std::array<E, enum_fields.size()> result;
    ranges::transform(enum_fields, result.begin(), enum_entry_description_t<E>::get_value);
    return result;
}();

template <Enum E>
constexpr auto enum_names = [](){
    constexpr auto enum_fields = enum_value_adapter<E>::values;
    std::array<std::string_view, enum_fields.size()> result;
    ranges::transform(enum_fields, result.begin(), enum_entry_description_t<E>::get_name);
    return result;
}();

template <Enum E>
constexpr std::string_view enum_to_string(E value) {
    const auto it = ranges::find(enum_value_adapter<E>::values, value, enum_entry_description_t<E>::get_value);
    if(it == enum_value_adapter<E>::values.end()) {
        throw std::runtime_error("Value is not part of enumeration.");
    }
    return it->name;
}

template <Enum E>
constexpr E enum_from_string(std::string_view name) {
    const auto it = ranges::find(enum_value_adapter<E>::values, name, enum_entry_description_t<E>::get_name);
    if(it == enum_value_adapter<E>::values.end()) {
        throw std::runtime_error("Name is not part of enumeration.");
    }
    return it->value;
}

}

#if !defined(TSMP_INTROSPECT_PASS) && defined(TSMP_REFLECTION_ENABLED)
#include "reflection.hpp"
#endif