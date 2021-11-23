#pragma once

#include "reflect.hpp"
#include "string_literal.hpp"
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <variant>
#include <optional>
namespace tsmp {

namespace detail {
    template <class T, class... Args>
    constexpr auto fetch_impl(T& internal, const std::string_view name, std::tuple<field_description_t<T, Args>...> fields) {
        const std::array matches = std::apply(
            [&](auto... decls){
                return std::array {
                    (name == decls.name ?
                        std::optional<std::variant<Args...>>( internal.*(decls.ptr) ) :
                        std::optional<std::variant<Args...>>()
                    )...
                };
            },
            tsmp::reflect<T>::fields()
        );
        const auto pos = std::find_if(matches.begin(), matches.end(), [](const auto& optional){ return optional.has_value(); });
        if(pos == matches.end()) {
            throw std::runtime_error("field name does not exist");
        }
        return pos->value();
    }

    template <class T, class V, class U>
    static constexpr bool try_set(T& internal, V T::*ptr, U value) {
        if constexpr(std::is_same_v<V, U>) {
            internal.*ptr =  value;
            return true;
        } else {
            throw std::runtime_error("type missmatch between ");
        }
    }
}

template <class T>
struct introspect {
    T& internal;

    constexpr introspect(T& lvalue) noexcept:
        internal{ lvalue }
    {}

    static constexpr auto field_id(const std::string_view name) {
        const std::array matches = std::apply(
            [&](auto... decls){
                return std::array { (name == decls.name ? true : false)... };
            },
            reflect<T>::fields()
        );
        const auto pos = std::find(matches.begin(), matches.end(), true);
        if(pos == matches.end()) {
            throw std::runtime_error("function name does not exist");
        }
        return pos - matches.begin();
    }

    static constexpr auto function_id(const std::string_view name) {
        const std::array matches = std::apply(
            [&](auto... decls){
                return std::array { (name == decls.name ? true : false)... };
            },
            reflect<T>::functions()
        );
        const auto pos = std::find(matches.begin(), matches.end(), true);
        if(pos == matches.end()) {
            throw std::runtime_error("function name does not exist");
        }
        return pos - matches.begin();
    }

    template <size_t id, class... Args>
    constexpr decltype(auto) call(Args... args) const {
        constexpr auto ptr = std::get<id>(reflect<T>::functions()).ptr;
        return (internal.*ptr)(args...);
    }

    template <string_literal_t name, class... Args>
    constexpr decltype(auto) call(Args... args) const {
        constexpr auto id = function_id(name);
        constexpr auto ptr = std::get<id>(reflect<T>::functions()).ptr;
        return (internal.*ptr)(args...);
    }

    template <size_t id>
    constexpr auto& get() const {
        constexpr auto ptr = std::get<id>(reflect<T>::fields()).ptr;
        return internal.*ptr;
    }

    template <string_literal_t name>
    constexpr auto& get() const {
        constexpr auto id = field_id(name);
        return get<id>();
    }

    constexpr auto fetch(const std::string_view name) const {
        return detail::fetch_impl(internal, name, reflect<T>::fields());
    }

    template <class Arg>
    constexpr auto set(const std::string_view name, Arg arg) const {
        const auto overwrite = std::apply(
            [&](auto... decls){
                return std::array {
                    (name == decls.name ? 
                        detail::try_set(internal, decls.ptr, arg) :
                        false
                ) ...};
            },
            reflect<T>::fields()
        );
        const auto pos = std::find(overwrite.begin(), overwrite.end(), true);
        if(pos == overwrite.end()) {
            throw std::runtime_error("field name does not exist");
        }
    }

    template <class Visitor>
    constexpr auto visit_fields(Visitor&& visitor) const {
        constexpr auto results_match_void =
            std::apply(
                [](auto... decls){
                    return std::array {
                        std::is_same_v<
                            void,
                            std::invoke_result_t<
                                Visitor,
                                decltype(decls.id),
                                decltype(decls.name),
                                decltype(std::declval<T&>().*(decls.ptr))
                            >
                        >...
                    };
                },
                reflect<T>::fields()
            );

        constexpr bool result_is_void = std::all_of(
            results_match_void.begin(), 
            results_match_void.end(),
            std::identity()
        );

        return std::apply(
            [result_is_void, visitor = std::forward<Visitor>(visitor), this](auto... decls) {
                if constexpr(result_is_void) {
                    (visitor(decls.id, decls.name,  internal.*(decls.ptr)), ...);
                } else {         
                    return std::array {
                        visitor(decls.id, decls.name,  internal.*(decls.ptr))...
                    };
                }
            },
            reflect<T>::fields()
        );
    } 
};
}