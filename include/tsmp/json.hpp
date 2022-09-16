#pragma once

#include "tsmp/reflect.hpp"
#include "tsmp/string_literal.hpp"
#include <algorithm>
#include <fmt/format.h>
#include <stdexcept>
#include <functional>
#include <iterator>
#include <optional>
#include <tsmp/introspect.hpp>

#include <concepts>
#include <range/v3/view/transform.hpp>
#include <range/v3/algorithm/transform.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/subrange.hpp>

#include <type_traits>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace tsmp {

template <class T>
concept Arithmetic = std::floating_point<T> || std::integral<T>;

template <class T>
concept is_optional = std::is_same_v<T, std::optional<typename T::value_type>>;

template <auto value>
struct immutable_t {
    using value_type = decltype(value);

    constexpr immutable_t() noexcept = default;
    constexpr immutable_t(immutable_t&&) noexcept = default;
    constexpr immutable_t(const immutable_t&) noexcept = default;
    constexpr immutable_t& operator=(immutable_t&&) noexcept = default;
    constexpr immutable_t& operator=(const immutable_t&) noexcept = default;

    constexpr immutable_t(value_type input){
        if(input != value) {
            throw std::runtime_error("Value missmatch");
        }
    }

    [[nodiscard]] constexpr operator value_type() const noexcept {
        return value;
    }

    [[nodiscard]] constexpr value_type get() const noexcept {
        return value;
    }
};

[[nodiscard]] std::string to_json(const auto& value);

[[nodiscard]] inline std::string to_json(const char* const& cstr) {
    return fmt::format("\"{}\"", cstr);
}

[[nodiscard]] inline std::string to_json(const std::string& str) {
    return fmt::format("\"{}\"", str);
}

[[nodiscard]] std::string to_json(const Arithmetic auto& number) {
    return fmt::format("{}", number);
}

[[nodiscard]] std::string to_json(const Enum auto& e) {
    return fmt::format("\"{}\"", enum_to_string(e));
}

template <size_t N>
[[nodiscard]] std::string to_json(const string_literal_t<N>& string_literal) {
    return to_json(std::string(string_literal));
}

template <auto value>
[[nodiscard]] std::string to_json(const immutable_t<value>& immutable) {
    return to_json(immutable.get());
}

template <std::ranges::input_range Range>
[[nodiscard]] std::string to_json(const Range& range) {
    using value_type = typename Range::value_type;
    using signature_t = std::string(*)(const value_type&);
    const auto elements = ranges::transform_view(range, static_cast<signature_t>(to_json));
    return fmt::format("[{}]", fmt::join(elements, ","));
}

template <class T>
[[nodiscard]] std::string to_json(const std::optional<T>& optional) {
    if(optional) {
        return to_json(*optional);
    } else {
        return "null";
    }
}

template <class... Ts>
[[nodiscard]] std::string to_json(const std::variant<Ts...>& variant) {
    return std::visit([](const auto& value) {
        return to_json(value);
    }, variant);
}

[[nodiscard]] std::string to_json(const auto& value) {
    introspect introspect { value };
    if constexpr (introspect.has_fields()) {
        const auto fields = introspect.visit_fields([](size_t, std::string_view name, const auto& field) {
            return fmt::format("\"{}\":{}", name, to_json(field));
        });
        return fmt::format("{{{}}}", fmt::join(fields, ","));
    } else {
        return "{}";
    }
}

namespace detail {

template <class T>
struct throw_handler_t {
    using value_type = T;
    T operator()(std::string msg) {
        throw std::runtime_error(std::move(msg));
    }
};

template <class T>
struct nullopt_handler_t {
    using value_type = std::optional<T>;
    std::optional<T> operator()(std::string) {
        return std::nullopt;
    }
};

template <class T, template<class> class ErrorHandler>
struct from_json_t;

template <Arithmetic T, template <class> class ErrorHandler>
struct from_json_t<T, ErrorHandler> {
    using value_type = typename ErrorHandler<T>::value_type;
    [[nodiscard]] value_type operator()(const nlohmann::json& json) {
        if(!json.is_number()) {
            return ErrorHandler<T>{}(fmt::format("{} is not a number", json.dump()));
        }
        return json;
    }
};

template <Enum T, template <class> class ErrorHandler>
struct from_json_t<T, ErrorHandler> {
    using value_type = typename ErrorHandler<T>::value_type;
    [[nodiscard]] value_type operator()(const nlohmann::json& json) {
        if(!json.is_string()) {
            return ErrorHandler<T>{}(fmt::format("{} is not a string", json.dump()));
        }
        return enum_from_string<T>(static_cast<std::string>(json));
    }
};


template <template <class> class ErrorHandler>
struct from_json_t<std::string, ErrorHandler> {
    using value_type = typename ErrorHandler<std::string>::value_type;
    [[nodiscard]] value_type operator()(const nlohmann::json& json) {
        if(!json.is_string()) {
            return ErrorHandler<std::string>{}(fmt::format("{} is not a string", json.dump()));
        }
        return static_cast<value_type>(json);
    }
};

template <class T, size_t N, template <class> class ErrorHandler>
struct from_json_t<std::array<T, N>, ErrorHandler> {
    using value_type = typename ErrorHandler<std::array<T, N>>::value_type;
    [[nodiscard]] value_type operator()(const nlohmann::json& json)
    {
        if(!json.is_array()) {
            return ErrorHandler<std::array<T, N>>{}(fmt::format("{} is not an array", json.dump()));
        }
        std::array<T, N> result;
        if(json.size() != result.size()) {
            return ErrorHandler<std::array<T, N>>{} (fmt::format("{} is not of requested size {}", json.dump(), N));
        }
        ranges::transform(json, result.begin(), from_json_t<T, ErrorHandler>{});
        return result;
    }
};

template <size_t N, template <class> class ErrorHandler>
struct from_json_t<string_literal_t<N>, ErrorHandler> {
    using value_type = typename ErrorHandler<string_literal_t<N>>::value_type;
    [[nodiscard]] value_type operator()(const nlohmann::json& json)
    {
        if(!json.is_string()) {
            return ErrorHandler<string_literal_t<N>>{}(fmt::format("{} is not a string", json.dump()));
        }
        std::string str = json;
        value_type result(str.c_str());
        if(str.size() > result.size()) {
            return ErrorHandler<string_literal_t<N>>{}(fmt::format("{} is bigger than requested size {}", json.dump(), N));
        }
        return result;
    }
};

template <auto value, template <class> class ErrorHandler>
struct from_json_t<immutable_t<value>, ErrorHandler> {
    using value_type = typename ErrorHandler<immutable_t<value>>::value_type;
    [[nodiscard]] value_type operator()(const nlohmann::json& json) {
        using capture_type = typename immutable_t<value>::value_type;
        return { from_json_t<std::remove_const_t<capture_type>, ErrorHandler>{}(json) };
    }
};

template <ranges::range Range, template <class> class ErrorHandler>
struct from_json_t<Range, ErrorHandler> {
    using value_type = typename ErrorHandler<Range>::value_type;
    [[nodiscard]] value_type operator()(const nlohmann::json& json) {
        if(!json.is_array()) {
            return ErrorHandler<Range>{}(fmt::format("{} is not an array", json.dump()));
        }
        using range_value_type = typename Range::value_type;
        std::vector<range_value_type> buffer;
        ranges::transform(json, std::back_insert_iterator(buffer), from_json_t<range_value_type, throw_handler_t>{});
        return Range{ buffer.begin(), buffer.end() };
    }
};

template <class... Args, template <class> class ErrorHandler>
struct from_json_t<std::variant<Args...>, ErrorHandler> {
    using value_type = typename ErrorHandler<std::variant<Args...>>::value_type;

    template <class T>
    [[nodiscard]] static std::optional<value_type> try_decode(const nlohmann::json& json) noexcept {
        const auto result = from_json_t<T, nullopt_handler_t>{}(json);
        if(result) {
            return value_type{ result.value() };
        } else {
            return std::nullopt;
        }
    }

    [[nodiscard]] value_type operator()(const nlohmann::json& json) {
        std::optional<value_type> result;
        ((result = try_decode<Args>(json)) || ...);
        if(result) {
            return *result;
        }
        return ErrorHandler<std::variant<Args...>>{}(fmt::format("Could not match {} against type {}", json.dump(), reflect<std::variant<Args...>>::name()));
    }
};

template <class T, template <class> class ErrorHandler>
struct from_json_t {
    using value_type = typename ErrorHandler<T>::value_type;

    template <class C, class MaybeOptional>
    static constexpr bool unwrap_assign(C& ref, MaybeOptional&& moptional) noexcept {
        if constexpr(is_optional<MaybeOptional>) {
            if(moptional) {
                ref = std::move(moptional.value());
                return false;
            }
            return true;
        } else {
            ref = std::forward<MaybeOptional>(moptional);
            return false;
        }
    }

    value_type operator()(const nlohmann::json& json) {
        T result {};
        introspect introspect { result };
        if(!json.is_object()) {
            return ErrorHandler<T>{}(fmt::format("{} is not an object", json.dump()));
        }
        if constexpr(introspect.has_fields()) {
            const auto fields_initialised_failed = introspect.visit_fields([&json](size_t, std::string_view name_view, auto& field) -> bool {
                using field_t = std::remove_reference_t<decltype(field)>;
                std::string name(name_view);

                if constexpr (is_optional<field_t>) {
                    if(json.contains(name)) {
                        field = from_json_t<typename field_t::value_type, nullopt_handler_t>{}(json[std::string(name)]);
                    }
                    return false;
                } else {
                    if(!json.contains(name)) {
                        ErrorHandler<field_t>{}(fmt::format("{} has no field {}", json.dump(), name_view));
                        return true;
                    }
                    return unwrap_assign(field, from_json_t<field_t, ErrorHandler>{}(json[name]));
                }
            });
            if constexpr(std::is_same_v<ErrorHandler<T>, nullopt_handler_t<T>>) {
                if(std::any_of(fields_initialised_failed.begin(), fields_initialised_failed.end(), std::identity())) {
                    return std::nullopt;
                }
            }
        }
        return result;
    }
};

}

template <class T, class... Validator>
constexpr T from_json(std::string_view string, Validator&&... validator) {
    const auto result = detail::from_json_t<T, detail::throw_handler_t>{}(nlohmann::json::parse(string));
    if((true && ... && validator(result))) {
        return result;
    } else {
        throw std::runtime_error("Validator was not satisfied.");
    }
}

template <class T, class... Validator>
constexpr std::optional<T> try_from_json(std::string_view string, Validator&&... validator) noexcept {
    const auto result = detail::from_json_t<T, detail::nullopt_handler_t>{}(nlohmann::json::parse(string));
    if((true && ... && validator(result))) {
        return result;
    } else {
        return std::nullopt;
    }
}

}