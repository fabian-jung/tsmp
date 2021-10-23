#pragma once

#include <vector>
#include <string>

namespace data {

struct field_decl_t {
    std::string name;
};

struct record_decl_t {
    std::string name;
    std::vector<field_decl_t> fields;
};

bool operator==(const field_decl_t& lhs, const field_decl_t& rhs);

bool operator==(const record_decl_t& lhs, const record_decl_t& rhs);
}