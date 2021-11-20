#include <string_view>
#include <tuple>

#include <tsmp/reflect.hpp>
#include <iostream>

template <typename T> concept arithmetic = std::is_arithmetic_v<T>;

std::string to_json(const char* value, const size_t indentation = 0) {
    return "\""+std::string(value)+"\"";
}

template <arithmetic T>
std::string to_json(const T& value, const size_t indentation = 0) {
    return "\""+std::to_string(value)+"\"";
}

std::string to_json(const std::string& value, const size_t indentation = 0) {
    return "\""+value+"\"";
}

std::string to_json(const auto& value, const size_t indentation = 0) {
    using type = std::remove_cv_t<std::remove_reference_t<std::remove_cv_t<decltype(value)>>>;
    using reflect = typename tsmp::reflect<type>;
    std::string result;
    result += '\n'+std::string(indentation, ' ')+"{\n";
    std::apply(
        [&](auto... fields){
            ([&](auto field) {
                result += std::string(indentation+4, ' ');
                result += field.name;
                result += ":";
                result += to_json(value.*(field.ptr), indentation+4);
                result += ",\n";
            }(fields), ...);
        },
        reflect::fields()
    );
    result += std::string(indentation, ' ')+"}";
    return result;

}

struct bar_t {
    const char* baba = "baba ";
};

struct foo_t {
    int a { 42 };
    // std::string s { "test" };
    float f = 3.14;
    bar_t bar;
};

int main(int argc, const char** argv) {

	std::cout << to_json(foo_t{}) << std::endl;
    
    return 0;
}