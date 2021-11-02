#pragma once

#include <vector>
#include <string>
#include <fmt/format.h>

namespace data {

struct field_decl_t {
    std::string name;
};

struct record_decl_t {
    std::string name;
    std::vector<field_decl_t> fields;
};

bool operator==(const field_decl_t& lhs, const field_decl_t& rhs);

bool operator==(const record_decl_t& lhs, const record_decl_t& rhs);
}

template<>
struct fmt::formatter<data::field_decl_t> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const data::field_decl_t & field, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "{0}", field.name);
    }
};