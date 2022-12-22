#pragma once

#include "types.hpp"

namespace data {

class prefix_splitter_t {
public:
    prefix_splitter_t(const std::vector<record_decl_t>& records = {}, std::vector<std::string> trivial_types = {});

    void add_record(record_decl_t record);

    std::set<field_decl_t> fields() const;
    std::set<function_decl_t> functions() const;
    std::vector<record_decl_t> records() const;
    std::vector<std::string> trivial_types() const;
   
private:

    bool has_field(const field_decl_t& field, const std::set<field_decl_t>& field_list) const;
    bool has_function(const function_decl_t& function, const std::set<function_decl_t>& function_list) const;

    void add_field(const field_decl_t& field);
    void add_function(const function_decl_t& function);

    bool field_list_match(const record_decl_t& lhs, const record_decl_t& rhs) const;

    std::set<field_decl_t> m_fields;
    std::set<function_decl_t> m_functions;
    std::vector<record_decl_t> m_records;
    std::vector<std::string> m_trivial_types;
};

}