#include <set>
#include <vector>
#include <string>

#include "types.hpp"

namespace data {
struct enum_splitter_t {

    enum_splitter_t(const std::vector<enum_decl_t>& enums);

    std::string render() const;

    std::vector<enum_decl_t> decls;
    std::set<std::string> fields;
};

} // End of namespace data