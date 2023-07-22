#pragma once

#include "fmt/core.h"
#include <algorithm>
#include <compare>
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

inline std::string remove_substring(std::string str, std::string removal) {
    std::string::size_type pos = 0;
    while(pos = str.find(removal, pos), pos != str.npos) {
        str.erase(pos, removal.size());
    }
    return str;
}


template<class Range, class Fn>
auto transform_join(Range range, std::string delimiter, Fn fn) {
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

    std::string get_value(std::string prefix = "") const {
        if(auto* type = std::get_if<type_t*>(&value)) {
            return (*type)->get_name(prefix);
        } else {
            return std::get<std::string>(value);
        }
    }
};

struct decl_t : public type_t {
    std::string name;
    std::string qualified_namespace;

    decl_t(std::string name, std::string qualified_namespace) : 
        name(std::move(name)),
        qualified_namespace(std::move(qualified_namespace))
    {}

    std::string get_namespace(namespace_option_t option) const {
        switch(option) {
            case namespace_option_t::qualified:
                return qualified_namespace;
            case namespace_option_t::unqualified:
                return remove_substring(qualified_namespace, "inline ");
            case namespace_option_t::none:
                return "";
        }
    }

    std::string get_name(std::string prefix = "", namespace_option_t namespace_option = data::namespace_option_t::unqualified) const override {
        std::string namespace_str = get_namespace(namespace_option);
        return fmt::format(
            "{}{}{}{}",
            prefix,
            namespace_str,
            namespace_str.empty() ? "" : "::",
            name
        );
    }
};

struct pointer_t : public type_t {
    const type_t* type;
    cv_qualifier_t cv_qualifier; 

    pointer_t(const type_t* type, cv_qualifier_t cv_qualifier) : type(std::move(type)), cv_qualifier(cv_qualifier) {}

    std::string get_name(std::string prefix = "", namespace_option_t namespace_option = data::namespace_option_t::unqualified) const override {
        return fmt::format(
            "{}{}{}{}*",
            cv_qualifier == cv_qualifier_t::const_ ? "const " : "",
            cv_qualifier == cv_qualifier_t::volatile_ ? "volatile " : "",
            cv_qualifier == cv_qualifier_t::const_volatile ? "const volatile " : "",
            type->get_name(prefix, namespace_option)
        );
    }
};

struct constant_array_t : public type_t {
    const type_t* type;
    std::size_t size;

    constant_array_t(
        const type_t* type,
        std::size_t size
    ) : 
        type(type),
        size(size)
    {}

    std::string get_name(std::string prefix = "", namespace_option_t namespace_option = data::namespace_option_t::unqualified) const override {
        return fmt::format(
            "{}[{}]",
            type->get_name(prefix, namespace_option),
            size
        );
    }
};

struct cv_qualified_type_t : public type_t {
    const type_t* type;
    cv_qualifier_t cv_qualifier;

    cv_qualified_type_t(const type_t* type, cv_qualifier_t cv_qualifier) : 
        type(std::move(type)),
        cv_qualifier(cv_qualifier)
    {}

    std::string get_name(std::string prefix = "", namespace_option_t namespace_option = data::namespace_option_t::unqualified) const override {
        if(dynamic_cast<const pointer_t*>(type) == nullptr) {
            return fmt::format(
                "{0}{1}{2}{3}",
                cv_qualifier == cv_qualifier_t::const_ ? "const " : "",
                cv_qualifier == cv_qualifier_t::volatile_ ? "volatile " : "",
                cv_qualifier == cv_qualifier_t::const_volatile ? "const volatile " : "",
                type->get_name(prefix, namespace_option)
            );
        } else {
            return fmt::format(
                "{0}{1}{2}{3}",
                type->get_name(prefix, namespace_option),
                cv_qualifier == cv_qualifier_t::const_ ? " const" : "",
                cv_qualifier == cv_qualifier_t::volatile_ ? " volatile" : "",
                cv_qualifier == cv_qualifier_t::const_volatile ? " const volatile" : ""
            );
        }
    }
};

struct reference_t : public type_t {
    const type_t* type;
    cv_qualifier_t cv_qualifier;
    ref_qualifier_t ref_qualifier;

    reference_t(const type_t* type, cv_qualifier_t cv_qualifier, ref_qualifier_t ref_qualifier) : type(std::move(type)), cv_qualifier(cv_qualifier), ref_qualifier(ref_qualifier) {}

    std::string get_name(std::string prefix = "", namespace_option_t namespace_option = data::namespace_option_t::unqualified) const override {
        return fmt::format(
            "{}{}{}{}{}{}",
            cv_qualifier == cv_qualifier_t::const_ ? "const " : "",
            cv_qualifier == cv_qualifier_t::volatile_ ? "volatile " : "",
            cv_qualifier == cv_qualifier_t::const_volatile ? "const volatile " : "",
            type->get_name(prefix, namespace_option),
            ref_qualifier == ref_qualifier_t::lvalue ? "&" : "",
            ref_qualifier == ref_qualifier_t::rvalue ? "&&" : ""
        );
    }
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
    ) : 
        decl_t(std::move(name),
        std::move(qualified_namespace)),
        scoped(scoped),
        values(std::move(values)),
        underlying_type(underlying_type)
    {}
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

    std::string signature(std::string record_name) const {
        return fmt::format(
            "{}{}{} {}::{}({}) {}{}",
            is_static ? "static " : "",
            is_constexpr ? "constexpr " : "",
            result ? result->get_name() : "<unknown>",
            record_name,
            name,
            fmt::join(parameter, ", "),
            is_const ? "const" : "",
            ref_qualifier
        );
    }
};

struct builtin_t : public type_t {
    std::string name;

    builtin_t(std::string name) : name(name) {}

    std::string get_name(std::string prefix = "", namespace_option_t namespace_option = data::namespace_option_t::unqualified) const override {
        return name;
    }
};

struct record_t : public decl_t {
    std::vector<template_argument_t> template_arguments {}; // TODO needs to be moved out. Emtpy templates are broken if not.
    cv_qualifier_t cv_qualifier; //TODO remove?
    std::vector<field_decl_t> fields;
    std::vector<function_decl_t> functions;
    bool is_struct = false;

    record_t(
        std::string name,
        std::string qualified_namespace,
        bool is_struct
    ) :
        decl_t(std::move(name),
        std::move(qualified_namespace)),
        is_struct(is_struct)
    {}

    std::string get_name(std::string prefix = "", namespace_option_t namespace_option = data::namespace_option_t::unqualified) const override {
        auto template_parameter_list = template_arguments.empty() ? "" : fmt::format("<{}>", transform_join(template_arguments, ", ", [prefix](const template_argument_t& arg) { return arg.get_value(prefix); }));
        std::string namespace_str = get_namespace(namespace_option);
        auto decl = fmt::format(
            "{}{}{}{}{}",
            prefix,
            namespace_str,
            namespace_str.empty() ? "" : "::",
            template_parameter_list.empty() ? "" : "template ",
            name
        );
        return fmt::format(
            "{}{}{}{}{}",
            cv_qualifier == cv_qualifier_t::const_ ? "const " : "",
            cv_qualifier == cv_qualifier_t::volatile_ ? "volatile " : "",
            cv_qualifier == cv_qualifier_t::const_volatile ? "const volatile " : "",
            decl,
            template_parameter_list
        );
    }
};

}

namespace fmt {

template<>
struct fmt::formatter<data::ref_qualifier_t> {
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
    }
};

template<>
struct fmt::formatter<data::cv_qualifier_t> {
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
struct fmt::formatter<data::parameter_decl_t> {
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

}