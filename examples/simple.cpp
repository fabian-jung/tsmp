#include <fmt/core.h>
#include <fmt/ranges.h>
#include <tsmp/introspect.hpp>

#include <concepts>
#include <range/v3/view/transform.hpp>
#include <type_traits>
#include <iostream>

template <typename T> 
concept arithmetic = std::floating_point<T> || std::integral<T>;

std::string to_json(const char* value) {
    return fmt::format("\"{}\"", value);
}

std::string to_json(const arithmetic auto& value) {
    return fmt::format("{}", value);
}

std::string to_json(const std::string& value) {
    return fmt::format("\"{}\"", value);
}


template<class T>
std::string to_json(const T& value) {
    using internal_type = typename std::remove_pointer<T>::type;
    if constexpr (std::is_pointer_v<T>) {
        if constexpr (std::is_void_v<internal_type>) {
            return fmt::format("{}", fmt::ptr(value));
        }
        else {
            if (value == nullptr) {
                return std::string("null");
            } else {
                return to_json(*value);
            }
        }
    }
    else if constexpr(std::is_array_v<T>)
    {
        std::string fields;
        for(auto& v : value)
        {
            fields += to_json(v);
        }

        return fmt::format(" [{}]", fmt::join(fields, ", "));
    }
    else {
        tsmp::introspect introspecter{value};
        std::string result;
        if constexpr (introspecter.has_fields()) {
            const auto fields = introspecter.visit_fields([&](size_t, std::string_view name, const auto &value) {
                return fmt::format("\"{}\":{}", name, to_json(value));
            });
            return fmt::format("{{{}}}", fmt::join(fields, ", "));
        } else {
            return "{}";
        }
    }
}

struct bar_t {
    const char* baba = "baba";
    std::string s = "asdas";
};

struct foo_t {
    int a { 42 };
    // std::string s { "test" };
    float f = 3.14;
    //int arr[4]{1,2,3,4};
    bar_t bar;
    struct foo_t* foo{};
};

int main(int, const char**) {

    const foo_t foo{};
	std::cout << to_json(&foo) << std::endl;
    
    // float f;
    // std::cout<< tsmp::reflect<float>::name << std::endl;
    return 0;
}