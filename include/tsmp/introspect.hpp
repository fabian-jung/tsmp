#pragma once

#include "reflect.hpp"
#include "string_literal.hpp"
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <variant>
#include <optional>
namespace tsmp {

    namespace detail {
        template <class T, class... Args>
        constexpr auto fetch_impl(T& internal, const std::string_view name, std::tuple<field_description_t<T, Args>...>) {
            return std::apply(
                    [&](auto... decls) {
                        std::optional<std::variant<Args...>> result;
                        const bool found = ((decls.name == name ? (result = internal.*(decls.ptr), true) : false ) || ...);
                        if(!found) {
                            throw std::runtime_error("field name does not exist");
                        }
                        return *result;
                    },
                    tsmp::reflect<T>::fields()
            );
        }

        template<class... FunctionsTuple>
        struct result_variant;

        template <class MemberFunctionPtr>
        struct result;

        template <class T, class V, class U>
        static constexpr bool try_set(T& internal, V T::*ptr, U&& value) {
            if constexpr(std::is_same_v<V, U>) {
                internal.*ptr =  std::forward<U>(value);
                return true;
            } else {
                throw std::runtime_error("type missmatch between ");
            }
        }
        template <class T, class Result, class... Args>
        struct result<Result (T::*)(Args...)> {
            using type = Result;
        };

        template <class Result, class... Args>
        struct result<Result(Args...)> {
            using type = Result;
        };

        template <class Result, class... Args>
        struct result<Result(Args...) const> {
            using type = Result;
        };

        template <class T>
        using result_t = typename result<T>::type;

        template <class T>
        using reference_wrapper_t = std::conditional_t<std::is_reference_v<T>, std::reference_wrapper<std::remove_reference_t<T>>, T>;

        template <class T>
        using monostate_wrapper_t = std::conditional_t<std::is_same_v<T, void>, std::monostate, T>;

        template <class T>
        struct remove_duplicates;

        template <class T>
        using remove_duplicates_t = typename remove_duplicates<T>::type;

        template <>
        struct remove_duplicates<std::variant<>> {
            using type = std::variant<>;
        };

        template <class T, class Variant>
        struct add_type;

        template <class T, class... Ts>
        struct add_type<T, std::variant<Ts...>> {
            using type = std::conditional_t<(std::is_same_v<T, Ts> || ...), std::variant<Ts...>, std::variant<T, Ts...>>;
        };

        template <class T, class... Ts>
        struct remove_duplicates<std::variant<T, Ts...>> {
            using type = typename add_type<T, remove_duplicates_t<std::variant<Ts...>>>::type;
        };

        template <class Base, class... MemberFunctionPtr>
        struct result_variant<std::tuple<tsmp::field_description_t<Base, MemberFunctionPtr>...>> {
            using type = remove_duplicates_t<std::variant<reference_wrapper_t<monostate_wrapper_t<result_t<MemberFunctionPtr>>>...>>;
        };

        template <class Functions>
        using result_variant_t = typename result_variant<Functions>::type;

    }

    template <class T>
    struct introspect {
        T &internal;
        static constexpr bool is_pointer = std::is_pointer_v<T>;

        constexpr introspect(T &lvalue) noexcept:
                internal{lvalue} {}

        using internal_type = typename std::remove_pointer<T>::type;

        static constexpr auto field_id(const std::string_view name) {
            return std::apply([name](auto... decls) {
                                  size_t id = 0;
                                  bool found = (((decls.name == name) ? true : (++id, false)) || ...);
                                  if (!found) {
                                      throw std::runtime_error("field name does not exist");
                                  }
                                  return id;
                              },
                              reflect<internal_type>::fields()
            );
        }

        static constexpr auto function_id(const std::string_view name) {
            return std::apply(
                    [&](auto... decls) {
                        size_t id = 0;
                        bool found = (((decls.name == name) ? true : (++id, false)) || ...);
                        if (!found) {
                            throw std::runtime_error("function name does not exist");
                        }
                        return id;
                    },
                    reflect<internal_type>::functions()
            );
        }

        template<size_t id, class... Args>
        constexpr decltype(auto) call(Args &&... args) const {
            constexpr auto ptr = std::get<id>(reflect<internal_type>::functions()).ptr;
            return (internal.*ptr)(std::forward<Args>(args)...);
        }

        template<string_literal_t name, class... Args>
        constexpr decltype(auto) call(Args &&... args) const {
            constexpr auto id = function_id(name);
            constexpr auto ptr = std::get<id>(reflect<internal_type>::functions()).ptr;
            return (internal.*ptr)(std::forward<Args>(args)...);
        }

        template<class... Args>
        constexpr decltype(auto) invoke(const size_t id, Args &&... args) {
            constexpr auto functions = std::apply(
                    [](auto... elements) {
                        return std::array<std::variant<decltype(elements.ptr)...>, sizeof...(elements)>{{elements.ptr...}};
                    },
                    reflect<internal_type>::functions()
            );
            auto variant = functions[id];
            using result_t = detail::result_variant_t<decltype(reflect<internal_type>::functions())>;
            if constexpr (!is_pointer) {
                return std::visit(
                        [&](auto &&ptr) -> result_t {
                            if constexpr (std::is_invocable_v<decltype(ptr), T, Args...>) {
                                if constexpr (std::is_same_v<std::invoke_result_t<decltype(ptr), T, Args...>, void>) {
                                    (internal.*ptr)(std::forward<Args>(args)...);
                                    return {};
                                } else {
                                    return {(internal.*ptr)(std::forward<Args>(args)...)};
                                }
                            } else {
                                throw std::runtime_error("member function parameter missmatch.");
                            }
                        },
                        variant
                );
            } else {
                return std::visit(
                        [&](auto &&ptr) -> result_t {
                            if constexpr (std::is_invocable_v<decltype(ptr), T, Args...>) {
                                if constexpr (std::is_same_v<std::invoke_result_t<decltype(ptr), T, Args...>, void>) {
                                    (*internal.*ptr)(std::forward<Args>(args)...);
                                    return {};
                                } else {
                                    return {(*internal.*ptr)(std::forward<Args>(args)...)};
                                }
                            } else {
                                throw std::runtime_error("member function parameter missmatch.");
                            }
                        },
                        variant
                );
            }
        }

        template<class... Args>
        constexpr decltype(auto) invoke(const std::string_view name, Args &&... args) {
            const auto id = function_id(name);
            return invoke(id, std::forward<Args>(args)...);
        }

        template<size_t id>
        constexpr auto &get() const {
            constexpr auto ptr = std::get<id>(reflect<internal_type>::fields()).ptr;
            return internal.*ptr;
        }

        template<string_literal_t name>
        constexpr auto &get() const {
            constexpr auto id = field_id(name);
            return get<id>();
        }

        constexpr auto fetch(const std::string_view name) const {
            return detail::fetch_impl(internal, name, reflect<internal_type>::fields());
        }

        template<class Arg>
        constexpr auto set(const std::string_view name, Arg &&arg) {
            std::apply(
                    [this, name, &arg](auto... decls) {
                        bool found = ((decls.name == name ? (detail::try_set(internal, decls.ptr,
                                                                             std::forward<Arg>(arg)), true)
                                                          : false) || ...);
                        if (!found) {
                            throw std::runtime_error("field name does not exist");
                        }
                    },
                    reflect<internal_type>::fields()
            );
        }

        constexpr bool has_fields() const noexcept {
            constexpr auto fields = reflect<internal_type>::fields();
            return std::tuple_size_v<decltype(fields)> > 0;
        }

        template<class Visitor>
        constexpr auto visit_fields(Visitor &&visitor) const {
            constexpr bool results_match_void =
                    std::apply(
                            [](auto... decls) {
                                return (
                                        std::is_same_v<
                                                void,
                                                std::invoke_result_t<
                                                        Visitor,
                                                        decltype(decls.id),
                                                        decltype(decls.name),
                                                        decltype(std::declval<internal_type &>().*(decls.ptr))
                                                >
                                        > && ... && true);
                            },
                            reflect<internal_type>::fields()
                    );
            if constexpr (!is_pointer) {
                return std::apply(
                        [visitor = std::forward<Visitor>(visitor), this](auto... decls) {
                            if constexpr (results_match_void) {
                                (visitor(decls.id, decls.name, internal.*(decls.ptr)), ...);
                            } else {
                                return std::array{
                                        visitor(decls.id, decls.name, internal.*(decls.ptr))...
                                };
                            }
                        },
                        reflect<internal_type>::fields()
                );
            } else {
                return std::apply(
                        [results_match_void, visitor = std::forward<Visitor>(visitor), this](auto... decls) {

                            if constexpr (results_match_void) {
                                (visitor(decls.id, decls.name, *internal.*(decls.ptr)), ...);
                            } else {
                                return std::array{
                                        visitor(decls.id, decls.name, *internal.*(decls.ptr))...
                                };
                            }
                        },
                        reflect<internal_type>::fields()
                );
            }
        }
    };
}