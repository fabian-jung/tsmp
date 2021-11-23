#pragma once

#include "types.hpp"


#include "prefix_splitter.hpp"

namespace data {

class reflection_aggregator_t {
public:

    void add_record_decl(record_decl_t decl);
    void add_trivial_type(std::string name);

    template <class OStream>
    void generate(OStream& output) {
        prefix_splitter_t splitter(m_records, m_trivial_types);

        output << "#include <tuple>\n";
        output << "namespace tsmp {\n";
        output << splitter.render();
        output << "}\n";
    }
    
    template <class OStream>
    void generate(OStream&& output) {
        generate(output);
    }

private:
    std::vector<record_decl_t> m_records;
    std::vector<std::string> m_trivial_types;
};

}