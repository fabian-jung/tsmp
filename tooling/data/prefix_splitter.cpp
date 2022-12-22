#include "prefix_splitter.hpp"

#include <algorithm>
#include <iterator>
#include <set>
#include <iostream>

#include "backport.hpp"

namespace data {

prefix_splitter_t::prefix_splitter_t(const std::vector<record_decl_t>& records, std::vector<std::string> trivial_types) :
    m_trivial_types(std::move(trivial_types))
{
    for(const auto& record : records) {
        add_record(record);
    }
}

void prefix_splitter_t::add_record(record_decl_t record) {
    // Conversion Operator not supported for the moment
    std::erase_if(record.functions, [](const auto& function) {
        return function.name.find("operator") == 0; 
    });

    if(!record.is_forward_declarable) {
        for(auto& r : m_records) {
            if(!r.is_forward_declarable && field_list_match(r, record)) {
                r.name = "<unknown>";
                fmt::print("Reject introspection for {} because of duplication. Record  dump:\"{}\"\n", record.name, record);
                return;
            }
        }
        for(const auto& field : record.fields) {
            add_field(field);
        }
        for(auto function : record.functions) {
            if(!function.overloaded) {
                add_function(function);
            }
        }
    }
    fmt::print("Add introspection for class {}\n", record.name);

    m_records.emplace_back(record);
}

std::set<field_decl_t> prefix_splitter_t::fields() const {
    return m_fields;
};

std::set<function_decl_t> prefix_splitter_t::functions() const {
    return m_functions;
};

std::vector<record_decl_t> prefix_splitter_t::records() const {
    return m_records;
};

std::vector<std::string> prefix_splitter_t::trivial_types() const {
    return m_trivial_types;
};

bool prefix_splitter_t::has_field(const field_decl_t& field, const std::set<field_decl_t>& field_list) const {
    return field_list.count(field);
}

bool prefix_splitter_t::has_function(const function_decl_t& function, const std::set<function_decl_t>& function_list) const {
    return function_list.count(function);
}

void prefix_splitter_t::add_field(const field_decl_t& field) {
    m_fields.insert(field);
}

void prefix_splitter_t::add_function(const function_decl_t& function) {
    if(function.name == "~") return;
    m_functions.insert(function);
}

bool prefix_splitter_t::field_list_match(const record_decl_t& lhs, const record_decl_t& rhs) const {
    if(lhs.fields.size() != rhs.fields.size() || lhs.functions.size() != rhs.functions.size() ) {
        return false;
    }

    for(auto l : lhs.fields) {
        if(!has_field(l, rhs.fields)) {
            return false;
        }
    }

    for(auto l : lhs.functions) {
        if(!has_function(l, rhs.functions)) {
            return false;
        }
    }

    return true;
}

}