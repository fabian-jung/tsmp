#include "types.hpp"
#include "fmt/core.h"
#include <algorithm>

namespace data {

std::string remove_substring(std::string str, std::string removal) {
    std::string::size_type pos = 0;
    while(pos = str.find(removal, pos), pos != str.npos) {
        str.erase(pos, removal.size());
    }
    return str;
}


std::string template_argument_t::get_value(std::string prefix) const {
    if(auto* type = std::get_if<type_t*>(&value)) {
        return (*type)->get_name(prefix);
    } else {
        return std::get<std::string>(value);
    }
}

decl_t::decl_t(std::string name, std::string qualified_namespace) : 
    name(std::move(name)),
    qualified_namespace(std::move(qualified_namespace))
{}

std::string decl_t::get_namespace(namespace_option_t option) const {
    switch(option) {
        case namespace_option_t::qualified:
            return qualified_namespace;
        case namespace_option_t::unqualified:
            return remove_substring(qualified_namespace, "inline ");
        case namespace_option_t::none:
            return "";
    }
    return "";
}

std::string decl_t::get_name(std::string prefix, namespace_option_t namespace_option) const {
    std::string namespace_str = get_namespace(namespace_option);
    return fmt::format(
        "{}{}{}{}",
        prefix,
        namespace_str,
        namespace_str.empty() ? "" : "::",
        name
    );
}

pointer_t::pointer_t(const type_t* type, cv_qualifier_t cv_qualifier) : type(std::move(type)), cv_qualifier(cv_qualifier) {}

std::string pointer_t::get_name(std::string prefix, namespace_option_t namespace_option) const {
    return fmt::format(
        "{}{}{}{}*",
        cv_qualifier == cv_qualifier_t::const_ ? "const " : "",
        cv_qualifier == cv_qualifier_t::volatile_ ? "volatile " : "",
        cv_qualifier == cv_qualifier_t::const_volatile ? "const volatile " : "",
        type->get_name(prefix, namespace_option)
    );
}

constant_array_t::constant_array_t(
    const type_t* type,
    std::size_t size
) : 
    type(type),
    size(size)
{}

std::string constant_array_t::get_name(std::string prefix, namespace_option_t namespace_option) const {
    return fmt::format(
        "{}[{}]",
        type->get_name(prefix, namespace_option),
        size
    );
}

cv_qualified_type_t::cv_qualified_type_t(const type_t* type, cv_qualifier_t cv_qualifier) : 
    type(std::move(type)),
    cv_qualifier(cv_qualifier)
{}

std::string cv_qualified_type_t::get_name(std::string prefix, namespace_option_t namespace_option) const {
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

reference_t::reference_t(const type_t* type, cv_qualifier_t cv_qualifier, ref_qualifier_t ref_qualifier) : type(std::move(type)), cv_qualifier(cv_qualifier), ref_qualifier(ref_qualifier) {}

std::string reference_t::get_name(std::string prefix , namespace_option_t namespace_option) const  {
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

enum_t::enum_t(
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

std::string function_decl_t::signature(std::string record_name) const {
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

builtin_t::builtin_t(std::string name) : name(name) {}

std::string builtin_t::get_name(std::string prefix, namespace_option_t namespace_option) const {
    return name;
}

record_t::record_t(
    std::string name,
    std::string qualified_namespace,
    bool is_struct
) :
    decl_t(std::move(name),
    std::move(qualified_namespace)),
    is_struct(is_struct)
{}

std::string record_t::get_name(std::string prefix, namespace_option_t namespace_option) const {
    auto template_parameter_list = template_arguments.empty() ? "" : fmt::format("<{}>", transform_join(template_arguments, ", ", [prefix](const template_argument_t& arg) { return arg.get_value(prefix); }));
    std::string namespace_str;
    if(parent != nullptr) {
        namespace_str = fmt::format("{}{}{}", namespace_str, namespace_str.empty() ? "" : "::", parent->get_name("", namespace_option_t::unqualified));
    } else {
        namespace_str = get_namespace(namespace_option);
    }
    auto decl = fmt::format(
        "{}{}{}{}{}",
        prefix,
        namespace_str,
        namespace_str.empty() ? "" : "::",
        template_parameter_list.empty() ? "" : "template ",
        name
    );
    return fmt::format(
        "{}{}",
        decl,
        template_parameter_list
    );
}

}