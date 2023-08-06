#pragma once

#include "fmt/core.h"
#include <algorithm>
#include <compare>
#include <cstddef>
#include <memory>
#include <optional>
#include <variant>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <fmt/format.h>
#include <functional>

namespace data {

std::string remove_substring(std::string str, std::string removal);

template<class Range, class Fn>
auto transform_join(const Range& range, std::string delimiter, Fn fn) {
    std::string result;
    for(const auto& elem : range) {
        result += fmt::format("{}{}", std::invoke(fn, elem), delimiter);
    }
    if(result.size() > delimiter.size()) {
        result.resize(result.size()-delimiter.size());
    }
    return result;
}

enum class ref_qualifier_t {
    lvalue,
    rvalue,
    nothing
};

enum class cv_qualifier_t {
    nothing,
    const_,
    volatile_,
    const_volatile
};

enum class namespace_option_t {
    qualified,
    unqualified,
    none
};

struct type_t {
    virtual ~type_t() = default;
    virtual std::string get_name(std::string prefix = "", namespace_option_t namespace_option = data::namespace_option_t::unqualified) const = 0;
};

struct template_argument_t {
    std::string kind;
    std::string name;
    std::variant<type_t*, std::string> value;
    bool is_pack = false;
    std::vector<template_argument_t> template_template_arguments;

    std::string get_value(std::string prefix = "") const;
};

struct decl_t : public type_t {
    std::string name;
    std::string qualified_namespace;

    decl_t(std::string name, std::string qualified_namespace);

    std::string get_namespace(namespace_option_t option) const;
    std::string get_name(std::string prefix = "", namespace_option_t namespace_option = data::namespace_option_t::unqualified) const override;
};

struct pointer_t : public type_t {
    const type_t* type;
    cv_qualifier_t cv_qualifier; 

    pointer_t(const type_t* type, cv_qualifier_t cv_qualifier);

    std::string get_name(std::string prefix = "", namespace_option_t namespace_option = data::namespace_option_t::unqualified) const override;
};

struct constant_array_t : public type_t {
    const type_t* type;
    std::size_t size;

    constant_array_t(
        const type_t* type,
        std::size_t size
    );

    std::string get_name(std::string prefix = "", namespace_option_t namespace_option = data::namespace_option_t::unqualified) const override;
};

struct cv_qualified_type_t : public type_t {
    const type_t* type;
    cv_qualifier_t cv_qualifier;

    cv_qualified_type_t(const type_t* type, cv_qualifier_t cv_qualifier);

    std::string get_name(std::string prefix = "", namespace_option_t namespace_option = data::namespace_option_t::unqualified) const override;
};

struct reference_t : public type_t {
    const type_t* type;
    cv_qualifier_t cv_qualifier;
    ref_qualifier_t ref_qualifier;

    reference_t(const type_t* type, cv_qualifier_t cv_qualifier, ref_qualifier_t ref_qualifier);

    std::string get_name(std::string prefix = "", namespace_option_t namespace_option = data::namespace_option_t::unqualified) const override;
};

struct enum_t : public decl_t {
    bool scoped;
    std::vector<std::string> values;
    std::optional<const type_t*> underlying_type;

    enum_t(
        std::string name,
        std::string qualified_namespace,
        bool scoped,
        std::vector<std::string> values,
        std::optional<const type_t*> underlying_type
    );
};

struct field_decl_t {
    std::string name;
    const type_t* type;
};

struct parameter_decl_t {
    std::string name;
    const type_t* type;
    bool is_pack = false;
};

struct function_decl_t {
    std::string name;
    std::vector<parameter_decl_t> parameter;
    const type_t* result; 
    bool is_virtual = false;
    bool is_const = false;
    ref_qualifier_t ref_qualifier = ref_qualifier_t::nothing;
    bool is_constexpr = false;
    bool is_noexcept = false;
    bool is_static = false;

    std::string signature(std::string record_name) const;
};

struct builtin_t : public type_t {
    std::string name;

    builtin_t(std::string name);

    std::string get_name(std::string prefix = "", namespace_option_t namespace_option = data::namespace_option_t::unqualified) const override;
};

struct record_t : public decl_t {
    std::vector<template_argument_t> template_arguments;
    std::vector<field_decl_t> fields;
    std::vector<function_decl_t> functions;
    bool is_struct = false;
    const type_t* parent = nullptr;
    
    record_t(
        std::string name,
        std::string qualified_namespace,
        bool is_struct
    );

    std::string get_name(std::string prefix = "", namespace_option_t namespace_option = data::namespace_option_t::unqualified) const override;
};

}

namespace fmt {

template<>
struct formatter<data::ref_qualifier_t> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const data::ref_qualifier_t & ref_qualifier, FormatContext& ctx) const
    {   
        switch(ref_qualifier) {
            case data::ref_qualifier_t::nothing:
                return ctx.out();
            case data::ref_qualifier_t::lvalue:
                return fmt::format_to(ctx.out(), "&");
            case data::ref_qualifier_t::rvalue:
                return fmt::format_to(ctx.out(), "&&");
        }
        return ctx.out();
    }
};

template<>
struct formatter<data::cv_qualifier_t> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const data::cv_qualifier_t & ref_qualifier, FormatContext& ctx) const
    {   
        switch(ref_qualifier) {
            case data::cv_qualifier_t::nothing:
                return ctx.out();
            case data::cv_qualifier_t::const_:
                return fmt::format_to(ctx.out(), "const ");
            case data::cv_qualifier_t::volatile_:
                return fmt::format_to(ctx.out(), "volatile ");
            case data::cv_qualifier_t::const_volatile:
                return fmt::format_to(ctx.out(), "const volatile ");
        }
    }
};

template<>
struct formatter<data::parameter_decl_t> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const data::parameter_decl_t & param, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "{} {}{}", param.type ? param.type->get_name() : "<unknown>", param.name, param.is_pack ? "..." : "");
    }
};

template<>
struct formatter<data::function_decl_t> {
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
struct formatter<data::field_decl_t> {
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
struct formatter<data::template_argument_t>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const 
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(data::template_argument_t const& arg, FormatContext& ctx) const
    {
        std::string template_decl;
        if(!arg.template_template_arguments.empty()) {
            template_decl = fmt::format("template <{}> ", fmt::join(arg.template_template_arguments, ", "));
        }
        return fmt::format_to(ctx.out(), "{}{}{} {}", template_decl, arg.kind, arg.is_pack ? "..." : "" ,arg.name);
    }
};

}