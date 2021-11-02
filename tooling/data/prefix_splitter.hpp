#pragma once

#include "types.hpp"
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <algorithm>
namespace data {

class prefix_splitter_t {
public:
    prefix_splitter_t(const std::vector<record_decl_t>& records = {}) {
        for(const auto& record : records) {
            add_record(record);
        }
    };

    void add_record(const record_decl_t& record) {
        for(const auto& field : record.fields) {
            add_field(field);
        }
        for(auto& r : m_records) {
            if(field_list_match(r.fields, record.fields)) {
                r.name = "<unknown>";
                return;
            }
        }
        m_records.emplace_back(record);
    }

    const auto& fields() const {
        return m_fields;
    };

    const auto& records() const {
        return m_records;
    };
    
    std::string render() const {
        std::string result;
        for(auto field : m_fields) {
            result += fmt::format(
                "template<class T> concept has_{0} = requires(T) {{ T::{0}; }};\n",
                 field.name
            );
        }
        result += '\n';
        for(auto record : m_records) {
            // trait impl ...
            std::string fields;
            int i = 0;
            for(const auto& f : record.fields) {
                fields += fmt::format("field_description_t{{ {0}, \"{1}\", &T::{1} }},\n", i++, f.name);
            }
            fields.resize(fields.size()-2);

            result += fmt::format(
R"(template <class T>
requires has_{}<T>
struct reflect<T> {{
    static constexpr bool reflectable = true;
    constexpr static auto name() {{
        return "{}";
    }}
    constexpr static auto fields() {{
        return std::make_tuple(
            {}
        );
    }}
}};

)",
                fmt::join(record.fields, "<T> && has_"),
                record.name,
                fields
            );
        }
        return result;
    }

private:

    bool has_field(const field_decl_t& field, const std::vector<field_decl_t>& field_list) const {
        const auto it = std::find_if(
            field_list.begin(),
            field_list.end(),
            [name = field.name](const auto& f){
                 return f.name == name; 
            }
        );
        return it != field_list.end();
    }

    void add_field(const field_decl_t& field) {
        if(!has_field(field, m_fields)) {
            m_fields.emplace_back(field);
        }
    }

    bool field_list_match(const std::vector<field_decl_t>& lhs, const std::vector<field_decl_t>& rhs) const {
        if(lhs.size() != rhs.size()) {
            return false;
        }

        for(auto l : lhs) {
            if(!has_field(l, rhs)) {
                return false;
            }
        }
        return true;
    }

    std::vector<field_decl_t> m_fields;
    std::vector<record_decl_t> m_records;
};

}