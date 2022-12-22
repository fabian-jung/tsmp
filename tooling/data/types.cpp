#include "types.hpp"
#include <algorithm>

namespace data {

bool operator==(const field_decl_t& lhs, const field_decl_t& rhs) {
    return lhs.name == rhs.name;
}

bool operator<(const field_decl_t& lhs, const field_decl_t& rhs) {
    return lhs.name < rhs.name;
}

bool operator==(const function_decl_t& lhs, const function_decl_t& rhs) {
    return lhs.name == rhs.name;
}

bool operator<(const function_decl_t& lhs, const function_decl_t& rhs) {
    return lhs.name < rhs.name;
}

bool operator==(const record_decl_t& lhs, const record_decl_t& rhs) {
    return
        lhs.name == rhs.name && 
        std::equal(lhs.fields.begin(), lhs.fields.end(), rhs.fields.begin(), rhs.fields.end()) &&
        std::equal(lhs.functions.begin(), lhs.functions.end(), rhs.functions.begin(), rhs.functions.end());
}

bool operator==(const enum_decl_t& lhs, const enum_decl_t& rhs) {
    return  std::equal(lhs.values.begin(), lhs.values.end(), rhs.values.begin(), rhs.values.end());
}

}