#include "aggregator.hpp"
#include "data/types.hpp"
#include <algorithm>
#include <fstream>
#include <fmt/format.h>
#include <fmt/ostream.h>

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

void reflection_aggregator_t::add_enum_decl(enum_decl_t decl) {
    const auto pos = std::find(m_enums.begin(), m_enums.end(), decl);
    if(pos != m_enums.end()) {
        // duplicate declaration
        return;
    }
    m_enums.emplace_back(std::move(decl));
}

std::vector<record_decl_t> reflection_aggregator_t::records() const {
    return m_records;
}

std::vector<std::string> reflection_aggregator_t::trivial_types() const {
    return m_trivial_types;
}

std::vector<enum_decl_t> reflection_aggregator_t::enums() const {
    return m_enums;
}

}
