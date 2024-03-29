#pragma once
#include <cstdint>
#include <range/v3/algorithm/find.hpp>
#include <range/v3/algorithm/transform.hpp>
#include <string_view>
#include <tuple>

namespace tsmp {

template<class T, class V>
struct field_description_t
{
    using value_type = V;
    std::size_t id;
    std::string_view name;
    V T::*ptr;
};
template<class T, class V>
field_description_t(std::size_t, std::string_view, V T::*) -> field_description_t<T, V>;

template<class Fn>
struct function_description_t
{
    size_t id;
    std::string_view name;
    Fn ptr;
};

template<class T>
function_description_t(size_t, std::string_view, T) -> function_description_t<T>;

template<class T>
concept Enum = std::is_enum_v<T>;

template<Enum E>
struct enum_entry_description_t
{
    std::string_view name;
    E value;

    static constexpr std::string_view get_name(enum_entry_description_t<E> entry) noexcept { return entry.name; }

    static constexpr E get_value(enum_entry_description_t<E> entry) noexcept { return entry.value; }
};

template<Enum E>
enum_entry_description_t(std::string_view, E) -> enum_entry_description_t<E>;

#ifdef TSMP_INTROSPECT_PASS
template<class GlobalNamespaceHelper, class T>
struct reflect_impl
{
    constexpr static bool reflectable = false;
    constexpr static auto name() { return ""; }
    constexpr static auto fields() { return std::tuple<>(); }
    constexpr static auto functions() { return std::tuple<>(); }
};

template<class GlobalNamespaceHelper, class T, class Accessor, class Functor, class... Base>
struct proxy_impl : public Base...
{};

template<class GlobalNamespaceHelper, Enum E>
struct enum_value_adapter_impl
{
    constexpr static std::array<enum_entry_description_t<E>, 0> values{};
};

struct global_t
{};

#endif

}

#if !defined(TSMP_INTROSPECT_PASS) && defined(TSMP_REFLECTION_ENABLED)
#include "reflection.hpp"
#endif

namespace tsmp {

template<class T>
using reflect = reflect_impl<global_t, std::remove_const_t<T>>;

template<Enum E>
using enum_value_adapter = enum_value_adapter_impl<global_t, std::remove_const_t<E>>;

template<Enum E>
constexpr auto enum_values = []() {
    constexpr auto enum_fields = enum_value_adapter<E>::values;
    std::array<E, enum_fields.size()> result;
    ranges::transform(enum_fields, result.begin(), enum_entry_description_t<E>::get_value);
    return result;
}();

template<Enum E>
constexpr auto enum_names = []() {
    constexpr auto enum_fields = enum_value_adapter<E>::values;
    std::array<std::string_view, enum_fields.size()> result;
    ranges::transform(enum_fields, result.begin(), enum_entry_description_t<E>::get_name);
    return result;
}();

template<Enum E>
constexpr std::string_view enum_to_string(E value)
{
    const auto it = ranges::find(enum_value_adapter<E>::values, value, enum_entry_description_t<E>::get_value);
    if (it == enum_value_adapter<E>::values.end()) {
        throw std::runtime_error("Value is not part of enumeration.");
    }
    return it->name;
}

template<Enum E>
constexpr E enum_from_string(std::string_view name)
{
    const auto it = ranges::find(enum_value_adapter<E>::values, name, enum_entry_description_t<E>::get_name);
    if (it == enum_value_adapter<E>::values.end()) {
        throw std::runtime_error("Name is not part of enumeration.");
    }
    return it->value;
}

}