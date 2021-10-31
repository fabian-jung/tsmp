#pragma once

#include "types.hpp"

namespace data {

struct prefix_splitter_node_t {
    std::vector<const char*> record_names;
    std::vector<field_decl_t> fields;
    std::vector<prefix_splitter_node_t> m_children;
};

class prefix_splitter_t {
public:
    prefix_splitter_t(const std::vector<record_decl_t>& records);
    
private:
    std::vector<prefix_splitter_node_t> nodes;
    
};

}