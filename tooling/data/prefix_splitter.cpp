#include "prefix_splitter.hpp"

#include <algorithm>

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
    record.functions.erase(
        std::remove_if(
            record.functions.begin(),
            record.functions.end(),
            [](const auto& function) {
                return function.name.find("operator") == 0; 
            }
        ),
        record.functions.end()
    );
    if(!record.is_forward_declarable) {
        for(auto& r : m_records) {
            if(field_list_match(r, record)) {
                r.name = "<unknown>";
                fmt::print("Reject introspection for {} because of duplication.\n", record.name);
                return;
            }
        }
        for(const auto& field : record.fields) {
            add_field(field);
        }
        for(auto function : record.functions) {
            add_function(function);
        }
    }
    fmt::print("Add introspection for class {}\n", record.name);

    m_records.emplace_back(record);
}

std::vector<field_decl_t> prefix_splitter_t::fields() const {
    return m_fields;
};

std::vector<function_decl_t> prefix_splitter_t::functions() const {
    return m_functions;
};

std::vector<record_decl_t> prefix_splitter_t::records() const {
    return m_records;
};

std::vector<std::string> prefix_splitter_t::trivial_types() const {
    return m_trivial_types;
};

bool prefix_splitter_t::has_field(const field_decl_t& field, const std::vector<field_decl_t>& field_list) const {
    const auto it = std::find_if(
        field_list.begin(),
        field_list.end(),
        [name = field.name](const auto& f){
                return f.name == name; 
        }
    );
    return it != field_list.end();
}

bool prefix_splitter_t::has_function(const function_decl_t& function, const std::vector<function_decl_t>& function_list) const {
    const auto it = std::find_if(
        function_list.begin(),
        function_list.end(),
        [name = function.name](const auto& f){
                return f.name == name; 
        }
    );
    return it != function_list.end();
}

void prefix_splitter_t::add_field(const field_decl_t& field) {
    if(!has_field(field, m_fields)) {
        m_fields.emplace_back(field);
    }
}

void prefix_splitter_t::add_function(const function_decl_t& function) {
    if(function.name == "~") return;
    if(!has_function(function, m_functions)) {
        m_functions.emplace_back(function);
    }
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