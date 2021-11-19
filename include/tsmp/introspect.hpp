#pragma once

#include "reflect.hpp"
#include "string_literal.hpp"
#include <variant>
#include <optional>
namespace tsmp {
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

    template <class... Args>
    constexpr auto get_impl(const std::string_view name, std::tuple<field_description_t<T, Args>...> fields) const {
        const std::array matches = std::apply(
            [&](auto... decls){
                return std::array {
                    (name == decls.name ?
                        std::optional<std::variant<Args...>>( internal.*(decls.ptr) ) :
                        std::optional<std::variant<Args...>>()
                    )...
                };
            },
            reflect<T>::fields()
        );
        const auto pos = std::find_if(matches.begin(), matches.end(), [](const auto& optional){ return optional.has_value(); });
        if(pos == matches.end()) {
            throw std::runtime_error("field name does not exist");
        }
        return pos->value();
    }

    constexpr auto get(const std::string_view name) const {
        return get_impl(name, reflect<T>::fields());
    }

};
}