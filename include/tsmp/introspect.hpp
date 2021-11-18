#pragma once

#include "reflect.hpp"
#include "string_literal.hpp"

namespace tsmp {
template <class T>
struct introspect {
    T& internal;

    constexpr introspect(T& lvalue) noexcept:
        internal{ lvalue }
    {}

    static constexpr auto field_id(const std::string_view name) {
        std::array matches = std::apply(
            [&](auto... decls){
                return std::array { (name == decls.name ? true : false)... };
            },
            reflect<T>::fields()
        );
        auto pos = std::find(matches.begin(), matches.end(), true);
        if(pos == matches.end()) {
            throw std::runtime_error("function name does not exist");
        }
        return pos - matches.begin();
    }

    static constexpr auto function_id(const std::string_view name) {
        std::array matches = std::apply(
            [&](auto... decls){
                return std::array { (name == decls.name ? true : false)... };
            },
            reflect<T>::functions()
        );
        auto pos = std::find(matches.begin(), matches.end(), true);
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

    constexpr auto& get(const std::string_view) const;

};
}