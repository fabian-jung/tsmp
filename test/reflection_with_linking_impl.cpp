#include <string>
#include "tsmp/reflect.hpp"

struct foo_t {
    int i;
    float f;
};

std::string function_with_reflection() {
    return tsmp::reflect<foo_t>::name();
}