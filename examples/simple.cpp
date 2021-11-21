#include <string_view>
#include <tuple>

#include <tsmp/introspect.hpp>
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
    // using type = std::remove_cv_t<std::remove_reference_t<std::remove_cv_t<decltype(value)>>>;
    tsmp::introspect introspecter{ value };
    std::string result;
    result += '\n'+std::string(indentation, ' ')+"{\n";
    introspecter.visit_fields([&](size_t id, std::string_view name, const auto& value){
        result += std::string(indentation+4, ' ');
        result += name;
        result += ":";
        result += to_json(value, indentation+4);
        result += ",\n";
    });
    result += std::string(indentation, ' ')+"}";
    return result;

}

struct bar_t {
    const char* baba = "baba";
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