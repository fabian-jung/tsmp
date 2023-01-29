#pragma once

#include "fmt/core.h"
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

struct template_argument_t {
    std::string kind;
    std::string name;
    std::string value;
};
struct record_decl_t {
    std::string name;
    std::vector<field_decl_t> fields;
    std::vector<function_decl_t> functions;
    std::string qualified_namespace {};
    std::vector<template_argument_t> template_arguments {};
    bool is_forward_declarable = false;
    bool is_std_type = false;
    bool is_nested_type = false;
    bool is_struct = false;
};

struct enum_decl_t {
    std::vector<std::string> values;
};

bool operator==(const field_decl_t& lhs, const field_decl_t& rhs);
bool operator==(const function_decl_t& lhs, const function_decl_t& rhs);
bool operator==(const record_decl_t& lhs, const record_decl_t& rhs);
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
        return fmt::format_to(ctx.out(), "{} {}", arg.kind, arg.name);
    }
};