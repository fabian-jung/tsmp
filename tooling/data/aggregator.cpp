#include "aggregator.hpp"
#include <algorithm>

namespace data {

void reflection_aggregator_t::add_record_decl(record_decl_t decl) {
    if(std::find(m_records.begin(), m_records.end(), decl) != m_records.end()) {
        // duplicate declaration
        return;
    }
    m_records.emplace_back(std::move(decl));
}

void reflection_aggregator_t::add_trivial_type(std::string name) {
    if(std::find(m_trivial_types.begin(), m_trivial_types.end(), name) != m_trivial_types.end()) {
        // duplicate declaration
        return;
    }
    m_trivial_types.emplace_back(std::move(name));
}

void reflection_aggregator_t::add_proxy_decl(record_decl_t decl) {
    if(std::find(m_records.begin(), m_records.end(), decl) != m_records.end()) {
        // duplicate declaration
        return;
    }
    m_records.emplace_back(std::move(decl));
}


}