#pragma once

#include "types.hpp"

namespace data {

class reflection_aggregator_t {
public:

    void add_record_decl(record_decl_t decl);
    
    template <class OStream>
    void generate(OStream& output) {
        output << "#include <tuple>\n";
        output << "namespace tsmp {\n";
        for(const auto& field_decl : m_records) {
            output << "\n\ntemplate <class T>\n";
            output << "requires requires(T) {\n";
            for(const auto& field : field_decl.fields) {
                output << "   T::"<< field.name<<";\n";
            }
            output << "}\n";
            output << "struct reflect<T> {\n";
            output << "    static constexpr bool reflectable = true;\n";
            output << "    constexpr static auto name() {\n";
            output << "        return \"" << field_decl.name << "\";\n";
            output << "    }\n";
            output << "    constexpr static auto fields() {\n";
            output << "        return std::make_tuple(\n";
            size_t i = 0;
            for(const auto& field : field_decl.fields) {
                output << "               field_description_t{ "<<i<<", \"" << field.name << "\", &T::"<< field.name << " }";
                ++i;
                if(i < field_decl.fields.size()) {
                    output << ",\n";
                } else {
                    output << "\n";
                }
            }
            output << "        );\n";
            output << "    }\n";
            output << "};\n";
        }
        output << "}\n";
    }
    
    template <class OStream>
    void generate(OStream&& output) {
        generate(output);
    }

private:
    std::vector<record_decl_t> m_records;
};

}