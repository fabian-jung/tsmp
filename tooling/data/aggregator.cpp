#include "aggregator.hpp"
#include "data/types.hpp"
#include <algorithm>
#include <iostream>
#include <fmt/core.h>
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

int reflection_aggregator_t::generate(std::filesystem::path path) const {

    const auto dir = std::filesystem::path(path).remove_filename();
    if(!std::filesystem::exists(dir)) {
        if(!std::filesystem::create_directories(dir)) {
            fmt::print(std::cerr, "Could not generate directory requested for output {} is not writeable.\n", dir.string());
            return 1;
        }
    }

    fmt::print("Write to {}\n", path.string());

    prefix_splitter_t splitter(m_records, m_trivial_types);
    enum_splitter_t enum_splitter(m_enums);
    auto output = fmt::output_file(path.string());
    output.print("#pragma once\n");
    output.print("#include <tuple>\n");
    output.print("#include <tsmp/reflect.hpp>\n");
    output.print("#include <tsmp/proxy.hpp>\n");
    output.print("namespace tsmp {{\n");
    output.print("{}\n", splitter.render());
    output.print("{}\n", enum_splitter.render());
    output.print("}}\n");

    return 0;
}


}