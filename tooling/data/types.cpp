#include "types.hpp"
#include "fmt/core.h"
#include <algorithm>

namespace data {

bool operator==(const field_decl_t& lhs, const field_decl_t& rhs) {
    return lhs.name == rhs.name;
}

bool operator<(const field_decl_t& lhs, const field_decl_t& rhs) {
    return lhs.name < rhs.name;
}

bool operator==(const parameter_decl_t& lhs, const parameter_decl_t& rhs) {
    return lhs.type == rhs.type; 
}

bool operator<(const parameter_decl_t& lhs, const parameter_decl_t& rhs) {
    return lhs.type < rhs.type; 
}

bool operator==(const function_decl_t& lhs, const function_decl_t& rhs) {
    return 
        lhs.name == rhs.name && 
        lhs.is_const == rhs.is_const &&
        lhs.ref_qualifier == rhs.ref_qualifier &&
        std::equal(lhs.parameter.begin(), lhs.parameter.end(), rhs.parameter.begin(), rhs.parameter.end());
}

bool operator<(const function_decl_t& lhs, const function_decl_t& rhs) {
    if(lhs.name != rhs.name) {
        return lhs.name < rhs.name;
    } 
    if(lhs.is_const != rhs.is_const) {
        return lhs.is_const < rhs.is_const;
    }
    if(lhs.ref_qualifier != rhs.ref_qualifier) {
        return lhs.ref_qualifier < rhs.ref_qualifier;
    }
    return std::lexicographical_compare(lhs.parameter.begin(), lhs.parameter.end(), rhs.parameter.begin(), rhs.parameter.end());
}

}