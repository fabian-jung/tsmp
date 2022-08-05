#pragma once

#include <vector>
#include <string>
#include <fmt/format.h>

namespace data {

struct field_decl_t {
    std::string name;
};

struct function_decl_t {
    std::string name;
};

struct record_decl_t {
    std::string name;
    std::vector<field_decl_t> fields;
    std::vector<function_decl_t> functions;
};

bool operator==(const field_decl_t& lhs, const field_decl_t& rhs);
bool operator==(const function_decl_t& lhs, const function_decl_t& rhs);

bool operator==(const record_decl_t& lhs, const record_decl_t& rhs);

}

template<>
struct fmt::formatter<data::field_decl_t> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const data::field_decl_t & field, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "{0}", field.name);
    }
};