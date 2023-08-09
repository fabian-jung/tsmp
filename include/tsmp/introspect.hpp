#pragma once

#include "reflect.hpp"
#include "string_literal.hpp"
#include <algorithm>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <variant>

namespace tsmp {

namespace detail {
template<class T, class... Args>
constexpr auto fetch_impl(T& internal, const std::string_view name, std::tuple<field_description_t<T, Args>...>)
{
    return std::apply(
        [&](auto... decls) {
            std::optional<std::variant<Args...>> result;
            const bool found = ((decls.name == name ? (result = internal.*(decls.ptr), true) : false) || ...);
            if (!found) {
                throw std::runtime_error("field name does not exist");
            }
            return *result;
        },
        tsmp::reflect<T>::fields());
}

template<class T, class V, class U>
static constexpr bool try_set(T& internal, V T::*ptr, U&& value)
{
    if constexpr (std::is_same_v<V, U>) {
        internal.*ptr = std::forward<U>(value);
        return true;
    } else {
        throw std::runtime_error("type mismatch between ");
    }
}

template<class T>
using reference_wrapper_t =
    std::conditional_t<std::is_reference_v<T>, std::reference_wrapper<std::remove_reference_t<T>>, T>;

template<class T>
using monostate_wrapper_t = std::conditional_t<std::is_same_v<T, void>, std::monostate, T>;

template<class T>
struct remove_duplicates;

template<class T>
using remove_duplicates_t = typename remove_duplicates<T>::type;

template<>
struct remove_duplicates<std::variant<>>
{
    using type = std::variant<>;
};

struct not_invokeable_t
{};

template<class T, class Variant>
struct add_type;

template<class T, class... Ts>
struct add_type<T, std::variant<Ts...>>
{
    using type = std::conditional_t<(std::is_same_v<T, Ts> || ...), std::variant<Ts...>, std::variant<T, Ts...>>;
};

template<class T, class... Ts>
struct remove_duplicates<std::variant<T, Ts...>>
{
    using type = typename add_type<T, remove_duplicates_t<std::variant<Ts...>>>::type;
};

template<class Fn, class... Args>
struct result
{};

template<class Fn, class... Args>
    requires std::is_invocable<Fn, Args...>::value
struct result<Fn, Args...>
{
    using type = std::invoke_result_t<Fn, Args...>;
};

template<class Fn, class... Args>
    requires(!std::is_invocable<Fn, Args...>::value)
struct result<Fn, Args...>
{
    using type = not_invokeable_t;
};

template<class Fn, class... Args>
using result_t = typename result<Fn, Args...>::type;

template<class FunctionsTuple, class... Args>
struct result_variant;

template<class... MemberFunctionPtr, class... Args>
struct result_variant<std::tuple<tsmp::function_description_t<MemberFunctionPtr>...>, Args...>
{
    using type = remove_duplicates_t<
        std::variant<reference_wrapper_t<monostate_wrapper_t<result_t<MemberFunctionPtr, Args...>>>...>>;
};

template<class Functions, class... Args>
using result_variant_t = typename result_variant<Functions, Args...>::type;

template<class T>
struct has_monostate
{
    constexpr static auto value = false;
};

template<template<class...> class T, class... Ts>
struct has_monostate<T<std::monostate, Ts...>>
{
    constexpr static auto value = true;
};

template<template<class...> class T, class Tf, class... Ts>
struct has_monostate<T<Tf, Ts...>>
{
    constexpr static auto value = has_monostate<T<Ts...>>::value;
};

}

template<class T>
struct introspect
{
    T& internal;

    constexpr introspect(T& lvalue) noexcept
        : internal{lvalue}
    {
    }

    static constexpr auto field_id(const std::string_view name)
    {
        return std::apply(
            [name](auto... decls) {
                size_t id = 0;
                bool found = (((decls.name == name) ? true : (++id, false)) || ...);
                if (!found) {
                    throw std::runtime_error("field name does not exist");
                }
                return id;
            },
            reflect<T>::fields());
    }

    static constexpr auto function_id(const std::string_view name)
    {
        return std::apply(
            [&](auto... decls) {
                size_t id = 0;
                bool found = (((decls.name == name) ? true : (++id, false)) || ...);
                if (!found) {
                    throw std::runtime_error("function name does not exist");
                }
                return id;
            },
            reflect<T>::functions());
    }

    template<size_t id, class... Args>
    constexpr decltype(auto) call(Args&&... args) const
    {
        constexpr auto ptr = std::get<id>(reflect<T>::functions()).ptr;
        return std::invoke(ptr, internal, std::forward<Args>(args)...);
    }

    template<string_literal_t name, class... Args>
    constexpr decltype(auto) call(Args&&... args) const
    {
        constexpr auto id = function_id(name);
        constexpr auto ptr = std::get<id>(reflect<T>::functions()).ptr;
        return std::invoke(ptr, internal, std::forward<Args>(args)...);
    }

    template<class... Args>
    constexpr decltype(auto) invoke(const size_t id, Args&&... args)
    {
        constexpr auto functions = std::apply(
            [](auto... elements) {
                return std::array<std::variant<decltype(elements.ptr)...>, sizeof...(elements)>{{elements.ptr...}};
            },
            reflect<T>::functions());
        auto variant = functions[id];
        using result_t = detail::result_variant_t<decltype(reflect<T>::functions()), T, Args...>;

        return std::visit(
            [&](const auto& ptr) -> result_t {
                if constexpr (std::is_invocable_v<decltype(ptr), T, Args...>) {
                    if constexpr (std::is_same_v<std::invoke_result_t<decltype(ptr), T, Args...>, void>) {
                        std::invoke(ptr, internal, std::forward<Args>(args)...);
                        if constexpr (detail::has_monostate<result_t>::value) {
                            return std::monostate{};
                        }
                    } else {
                        return {std::invoke(ptr, internal, std::forward<Args>(args)...)};
                    }
                } else {
                    throw std::runtime_error("member function parameter mismatch.");
                }
            },
            variant);
    }

    template<class... Args>
    constexpr decltype(auto) invoke(const std::string_view name, Args&&... args)
    {
        const auto id = function_id(name);
        return invoke(id, std::forward<Args>(args)...);
    }

    template<size_t id>
    constexpr auto& get() const
    {
        constexpr auto ptr = std::get<id>(reflect<T>::fields()).ptr;
        return internal.*ptr;
    }

    template<string_literal_t name>
    constexpr auto& get() const
    {
        constexpr auto id = field_id(name);
        return get<id>();
    }

    constexpr auto fetch(const std::string_view name) const
    {
        return detail::fetch_impl(internal, name, reflect<T>::fields());
    }

    template<class Arg>
    constexpr auto set(const std::string_view name, Arg&& arg)
    {
        std::apply(
            [this, name, &arg](auto... decls) {
                bool found = ((decls.name == name ? (detail::try_set(internal, decls.ptr, std::forward<Arg>(arg)), true)
                                                  : false) ||
                              ...);
                if (!found) {
                    throw std::runtime_error("field name does not exist");
                }
            },
            reflect<T>::fields());
    }

    constexpr bool has_fields() const noexcept
    {
        constexpr auto fields = reflect<T>::fields();
        return std::tuple_size_v<decltype(fields)> > 0;
    }

    template<class Visitor>
    constexpr auto visit_fields(Visitor&& visitor) const
    {
        constexpr bool results_match_void = std::apply(
            [](auto... decls) {
                return (std::is_same_v<void,
                                       std::invoke_result_t<Visitor,
                                                            decltype(decls.id),
                                                            decltype(decls.name),
                                                            decltype(std::declval<T&>().*(decls.ptr))>> &&
                        ... && true);
            },
            reflect<T>::fields());

        return std::apply(
            [visitor = std::forward<Visitor>(visitor), this](auto... decls) {
                if constexpr (results_match_void) {
                    (visitor(decls.id, decls.name, internal.*(decls.ptr)), ...);
                } else {
                    return std::array{visitor(decls.id, decls.name, internal.*(decls.ptr))...};
                }
            },
            reflect<T>::fields());
    }
};
}