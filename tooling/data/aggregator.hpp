#pragma once

#include "types.hpp"
#include "prefix_splitter.hpp"
#include "enum_splitter.hpp"

namespace data {

class reflection_aggregator_t {
public:

    void add_record_decl(record_decl_t decl);
    void add_trivial_type(std::string name);
    void add_proxy_decl(record_decl_t decl);
    void add_enum_decl(enum_decl_t decl);

    std::vector<record_decl_t> records() const;
    std::vector<std::string> trivial_types() const;
    std::vector<enum_decl_t> enums() const;
    
private:
    std::vector<record_decl_t> m_records;
    std::vector<std::string> m_trivial_types;
    std::vector<enum_decl_t> m_enums;
};

}