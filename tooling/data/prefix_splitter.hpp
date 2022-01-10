#pragma once

#include "types.hpp"
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <algorithm>
#include <iterator>
namespace data {

class prefix_splitter_t {
public:
    prefix_splitter_t(const std::vector<record_decl_t>& records = {}, std::vector<std::string> trivial_types = {});

    void add_record(record_decl_t record);

    const std::vector<field_decl_t>& fields() const;
    const std::vector<function_decl_t>& functions() const;
    const std::vector<record_decl_t>& records() const;
    
    static std::string strip_special_chars(std::string input);

    std::string render() const;

private:

    bool has_field(const field_decl_t& field, const std::vector<field_decl_t>& field_list) const;
    bool has_function(const function_decl_t& function, const std::vector<function_decl_t>& function_list) const;
    void add_field(const field_decl_t& field);
    void add_function(const function_decl_t& function);
    bool field_list_match(const record_decl_t& lhs, const record_decl_t& rhs) const;

    std::vector<field_decl_t> m_fields;
    std::vector<function_decl_t> m_functions;
    std::vector<record_decl_t> m_records;
    std::vector<std::string> m_trivial_types;
};

}