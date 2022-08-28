#include "tsmp/reflect.hpp"
#include "tsmp/string_literal.hpp"
#include <fmt/format.h>
#include <stdexcept>
#include <optional>
#include <tsmp/introspect.hpp>

#include <concepts>
#include <ranges>
#include <type_traits>
#include <stdexcept>

#include <nlohmann/json.hpp>

template <class T>
concept Arithmetic = std::floating_point<T> || std::integral<T>;

template <auto value>
struct immutable_t {
    using value_type = decltype(value);

    constexpr immutable_t() noexcept = default;
    constexpr immutable_t(immutable_t&&) noexcept = default;
    constexpr immutable_t(const immutable_t&) noexcept = default;
    constexpr immutable_t& operator=(immutable_t&&) noexcept = default;
    constexpr immutable_t& operator=(const immutable_t&) noexcept = default;

    constexpr immutable_t(value_type input) {
        if(input != value) {
            throw std::runtime_error(fmt::format("Value missmatch: {} != {}", input, value));
        }
    }

    constexpr operator value_type() const {
        return value;
    }

    constexpr value_type get() const {
        return value;
    }
};

std::string to_json(const auto& value);

std::string to_json(const char* const& cstr) {
    return fmt::format("\"{}\"", cstr);
}

std::string to_json(const std::string& str) {
    return fmt::format("\"{}\"", str);
}

std::string to_json(const Arithmetic auto& number) {
    return std::to_string(number);
}

std::string to_json(const tsmp::Enum auto& e) {
    return fmt::format("\"{}\"", tsmp::enum_to_string(e));
}

template <size_t N>
std::string to_json(const tsmp::string_literal_t<N>& string_literal) {
    return to_json(std::string(string_literal));
}

template <auto value>
std::string to_json(const immutable_t<value>& immutable) {
    return to_json(immutable.get());
}

template <std::ranges::input_range Range>
std::string to_json(const Range& range) {
    using value_type = typename Range::value_type;
    using signature_t = std::string(*)(const value_type&);
    const auto elements = std::ranges::transform_view(range, static_cast<signature_t>(to_json));
    return fmt::format("[{}]", fmt::join(elements, ","));
}

template <class T>
std::string to_json(const std::optional<T>& optional) {
    if(optional) {
        return to_json(*optional);
    } else {
        return "null";
    }
}

template <class... Ts>
std::string to_json(const std::variant<Ts...>& variant) {
    return std::visit([](const auto& value) {
        return to_json(value);
    }, variant);
}

std::string to_json(const auto& value) {
    tsmp::introspect introspect { value };
    if constexpr (introspect.has_fields()) {
        const auto fields = introspect.visit_fields([](size_t, std::string_view name, const auto& field) {
            return fmt::format("\"{}\":{}", name, to_json(field));
        });
        return fmt::format("{{{}}}", fmt::join(fields, ","));
    } else {
        return "{}";
    }
}

template <class T>
struct from_json;

template <Arithmetic T>
struct from_json<T> {
    T operator()(const nlohmann::json& json) {
        if(!json.is_number()) {
            throw std::runtime_error{fmt::format("{} is not a number", json.dump())};
        }
        return json;
    }
};

template <tsmp::Enum T>
struct from_json<T> {
    T operator()(const nlohmann::json& json) {
        if(!json.is_string()) {
            throw std::runtime_error{fmt::format("{} is not a string", json.dump())};
        }
        return tsmp::enum_from_string<T>(static_cast<std::string>(json));
    }
};


template <>
struct from_json<std::string> {

    std::string operator()(const nlohmann::json& json) {
        if(!json.is_string()) {
            throw std::runtime_error{fmt::format("{} is not a string", json.dump())};
        }
        return json;
    }
};

template <class T, size_t N>
struct from_json<std::array<T, N>> {
    std::array<T, N> operator()(const nlohmann::json& json)
    {
        std::array<T, N> result;
        if(!json.is_array()) {
            throw std::runtime_error{fmt::format("{} is not an array", json.dump())};
        }
        if(json.size() != result.size()) {
            throw std::runtime_error{fmt::format("{} is not of requested size {}", json.dump(), N)};
        }
        std::vector<nlohmann::json> intermediate(json.begin(), json.end());
        auto view = std::ranges::transform_view(intermediate, from_json<T>{});
        std::copy(view.begin(), view.end(), result.begin());
        return result;
    }
};

template <size_t N>
struct from_json<tsmp::string_literal_t<N>> {
    tsmp::string_literal_t<N> operator()(const nlohmann::json& json)
    {
        tsmp::string_literal_t<N> result;
        if(!json.is_string()) {
            throw std::runtime_error{fmt::format("{} is not a string", json.dump())};
        }

        std::string str = json;
        if(str.size() != result.size()) {
            throw std::runtime_error{fmt::format("{} is not of requested size {}", json.dump(), N)};
        }
        std::copy(str.begin(), str.end(), result.begin());
        return result;
    }
};

template <auto value>
struct from_json<immutable_t<value>> {
    immutable_t<value> operator()(const nlohmann::json& json) {
        using value_type = typename immutable_t<value>::value_type;
        return { from_json<std::remove_const_t<value_type>>{}(json) };
    }
};

template <std::ranges::range Range>
struct from_json<Range> {
    Range operator()(const nlohmann::json& json) {
        if(!json.is_array()) {
            throw std::runtime_error{fmt::format("{} is not an array", json.dump())};
        }
        using value_type = typename Range::value_type;
        std::vector<nlohmann::json> intermediate(json.begin(), json.end());
        auto view = std::ranges::transform_view(intermediate, from_json<value_type>{});
        return Range{view.begin(), view.end()};
    }
};

namespace detail {
    template <class T, class Variant>
    std::optional<Variant> decode(const nlohmann::json& json) {
        try {
            return from_json<T>{}(json);
        } catch(...) {
            return std::nullopt;
        }
    }
}

template <class... Args>
struct from_json<std::variant<Args...>> {

    std::variant<Args...> operator()(const nlohmann::json& json) {
        std::optional<std::variant<Args...>> result;
        ((result = detail::decode<Args, std::variant<Args...>>(json)) || ...);
        if(result) {
            return *result;
        }
        throw std::runtime_error(fmt::format("Could not match {} against type {}", json.dump(), tsmp::reflect<std::variant<Args...>>::name()));
    }
};

template <class T>
concept is_optional = std::is_same_v<T, std::optional<typename T::value_type>>;

template <class T>
struct from_json {
    T operator()(const nlohmann::json& json) {
        T result {};
        tsmp::introspect introspect { result };
        if(!json.is_object()) {
            throw std::runtime_error(fmt::format("{} is not an object", json.dump()));
        }
        if constexpr(introspect.has_fields()) {
            introspect.visit_fields([&json](size_t, std::string_view name_view, auto& field) {
                using field_t = std::remove_reference_t<decltype(field)>;
                std::string name(name_view);

                if constexpr (is_optional<field_t>) {
                    if(json.contains(name)) {
                        try {
                            field = from_json<typename field_t::value_type>{}(json[std::string(name)]);
                        } catch(...) {
                            field = std::nullopt;
                        }
                    } else {
                        field = std::nullopt;
                    }
                } else {
                    if(json.contains(name)) {
                        field = from_json<field_t>{}(json[std::string(name)]);
                    } else {
                        throw std::runtime_error(fmt::format("{} has no field {}", json.dump(), name_view));
                    }
                }
            });
        }
        return result;
    }
};

int main(int, char*[]) {
    struct foo_t {
        enum class state_t {
            A,
            B,
            C
        } state = state_t::A;

        int i { 42 };
        // immutable_t<4> version;
        tsmp::string_literal_t<4> sl { "asdf" };
        immutable_t<tsmp::string_literal_t{"some_string"}> immutable_string;
        float f { 1337.0f };
        std::string s = "Hello World!";
        struct bar_t {
            int i { 0 };
        } bar;
        std::array<int, 4> numbers_array { 1, 2, 3, 4 };
        std::vector<int> numbers_vector { 1, 2, 3, 4 };
        std::optional<int> oint = std::nullopt;
        std::variant<std::string, int> variant { 5 };
    } foo;

    fmt::print("{}\n", to_json(foo));

    foo_t foo2 = from_json<foo_t>{}(nlohmann::json::parse(to_json(foo_t{})));
    fmt::print("{}\n", to_json(foo2));
    return 0;
}