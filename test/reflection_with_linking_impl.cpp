#include <string>
#include "tsmp/reflect.hpp"

std::string function_with_reflection() {
    struct foo_t {
        int i;
        float f;
    };
    return tsmp::reflect<foo_t>::name();
}