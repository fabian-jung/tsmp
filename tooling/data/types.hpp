#pragma once

#include <algorithm>
#include <vector>
#include <string>
#include <set>
#include <fmt/format.h>

namespace data {

struct field_decl_t {
    std::string name;
};
bool operator==(const field_decl_t& lhs, const field_decl_t& rhs);
bool operator<(const field_decl_t& lhs, const field_decl_t& rhs);

struct function_decl_t {
    std::string name;
    mutable bool overloaded = false;
};

bool operator==(const function_decl_t& lhs, const function_decl_t& rhs);
bool operator<(const function_decl_t& lhs, const function_decl_t& rhs);

struct template_argument_t {
    std::string kind;
    std::string name;
    std::string value;
    bool is_pack = false;
};

struct record_decl_t {
    std::string name;
    std::set<field_decl_t> fields;
    std::set<function_decl_t> functions;
    std::string qualified_namespace {};
    std::vector<template_argument_t> template_arguments {};
    bool is_forward_declarable = false;
    bool is_std_type = false;
    bool is_nested_type = false;
    bool is_struct = false;
};

bool operator==(const record_decl_t& lhs, const record_decl_t& rhs);
struct enum_decl_t {
    std::vector<std::string> values;
};

bool operator==(const enum_decl_t& lhs, const enum_decl_t& rhs);
}

template<>
struct fmt::formatter<data::function_decl_t> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const data::function_decl_t & function, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "{0}", function.name);
    }
};

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

template<>
struct fmt::formatter<data::template_argument_t>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const 
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(data::template_argument_t const& arg, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "{}{} {}", arg.kind, arg.is_pack ? "..." : "" ,arg.name);
    }
};

template<>
struct fmt::formatter<data::record_decl_t> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const data::record_decl_t & record, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "{1}::{0} {{ fields:{{{2}}}, functions:{{{3}}} }}", record.name, record.qualified_namespace, fmt::join(record.fields, ", "), fmt::join(record.functions, ", "));
    }
};
