#include "enum_splitter.hpp"

namespace data {

enum_splitter_t::enum_splitter_t(const std::vector<enum_decl_t>& enums) :
    decls(enums)
{
    for(const auto& decl : enums) {
        fields.insert(decl.values.begin(), decl.values.end());
    }
}

std::string enum_splitter_t::render() const {
    std::string result;
    for(const auto& value : fields) {
        result += fmt::format("template<class E> concept has_enum_value_{0} = requires(E) {{ E::{0}; }};\n", value);
    }
    result += "\n";
    for(const auto& decl : decls) {
        std::string requirements = "    has_enum_value_", entries;
        requirements += fmt::format("{}", fmt::join(decl.values, "<E> && \n    has_enum_value_"));
        requirements += "<E>";

        for(auto value : decl.values) {
            entries += fmt::format("        enum_entry_description_t<E> {{ \"{0}\", E::{0} }},\n", value);
        }
        entries.resize(entries.size()-2); // remove last ",\n"

        result += fmt::format(
R"(
template <Enum E> requires
{}
struct enum_value_adapter<E> {{
    constexpr static std::array values {{
{}
    }};
}};

)", requirements, entries);
    }
    return result;
}

}