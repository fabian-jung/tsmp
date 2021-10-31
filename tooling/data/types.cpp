#include "types.hpp"
#include <algorithm>

namespace data {

bool operator==(const field_decl_t& lhs, const field_decl_t& rhs) {
    return lhs.name == rhs.name;
}

bool operator==(const record_decl_t& lhs, const record_decl_t& rhs) {
    return std::equal(
        lhs.fields.begin(), lhs.fields.end(),
        rhs.fields.begin(), rhs.fields.end()
    );
}

}