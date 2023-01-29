#include "enum_splitter.hpp"

namespace data {

enum_splitter_t::enum_splitter_t(const std::vector<enum_decl_t>& enums) :
    decls(enums)
{
    for(const auto& decl : enums) {
        fields.insert(decl.values.begin(), decl.values.end());
    }
}

}