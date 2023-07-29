#include "renderer.hpp"
#include "data/aggregator.hpp"
#include "data/types.hpp"
#include "fmt/core.h"
#include "fmt/format.h"
#include "fmt/ostream.h"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

struct namespace_wrapper_t {
    //TODO use a proper tree structure
    namespace_wrapper_t* parent = nullptr;
    std::string name;
    std::set<const data::record_t*> records;
    std::set<const data::enum_t*> enums;
    std::map<std::string, std::unique_ptr<namespace_wrapper_t>> children;

    static std::pair<std::string, std::string> split_namespace(std::string ns) {
        if(ns.find("inline ") == 0) {
            ns = ns.substr(7);
        }
        auto first_split = ns.find("::");
        if(first_split != ns.npos) {
            return { ns.substr(0, first_split), ns.substr(first_split+2)};
        }
        return { ns, "" };
    }

    void insert(const data::record_t* record, std::string ns) {
        if(ns.empty()) {
            records.emplace(record);
            return;
        }

        auto [outer, inner] = split_namespace(ns);

        auto pos = children.find(outer);
        if(pos != children.end()) {
            pos->second->insert(record, inner);
        } else {
            auto wrapper = std::make_unique<namespace_wrapper_t>(namespace_wrapper_t{this, outer});
            wrapper->insert(record, inner);
            children.emplace_hint(pos, outer, std::move(wrapper));
        }
    }

    void insert(const data::enum_t* e, std::string ns) {
        if(ns.empty()) {
            enums.emplace(e);
            return;
        }

        auto [outer, inner] = split_namespace(ns);

        auto pos = children.find(outer);
        if(pos != children.end()) {
            pos->second->insert(e, inner);
        } else {
            auto wrapper = std::make_unique<namespace_wrapper_t>(namespace_wrapper_t{this, outer});
            wrapper->insert(e, inner);
            children.emplace_hint(pos, outer, std::move(wrapper));
        }
    }

    std::string full_namespace() const {
        if(parent) {
            return fmt::format("{}::{}", parent->full_namespace(), name);
        }
        return "";
    }
};

template<>
struct fmt::formatter<namespace_wrapper_t>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) const 
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(namespace_wrapper_t const& arg, FormatContext& ctx) const
    {
        std::string ns = arg.full_namespace();
        std::map<std::string, std::string> definitions;
        for(const auto& record: arg.records) {
            std::string template_definition;
            std::string record_definition;
            if(!record->template_arguments.empty()) {
                fmt::format_to(std::back_inserter(record_definition), "    template <{0}>\n", fmt::join(record->template_arguments, ", "));
                std::vector<std::string_view> names;
                std::transform(record->template_arguments.begin(), record->template_arguments.end(), std::back_inserter(names), [](const auto& arg) -> std::string_view {
                    return arg.name;
                });
                template_definition = fmt::format("<{}>", fmt::join(names, ", "));
            }
            fmt::format_to(std::back_inserter(record_definition), "    using {0} = {1}::{0}{2};\n", record->name, ns, template_definition);
            definitions.emplace(record->name, record_definition);
        }
        for(const auto& e: arg.enums) {
            std::string template_definition;
            definitions.emplace(e->name, fmt::format("    using {0} = {1}::{0};\n", e->name, ns));
        }
        for(const auto& [name, def] : definitions) {
            fmt::format_to(ctx.out(), "{}\n", def);
        }
        for(const auto& child: arg.children) {
            fmt::format_to(ctx.out(), "    struct {} {{\n{}    }};\n", child.first, *child.second);
        }
        return ctx.out();
    }
};


namespace data {

// std::string erase_substring(std::string input, const std::string& substring) {
//     std::string::size_type pos = 0u;
//     while((pos = input.find(substring, pos)) < input.size()) {
//         input.erase(pos, substring.size());
//     }
//     return input;
// }

// std::string generate_unique_parameter_name() {
//     static std::uint64_t id = 0;
//     return fmt::format("p{}", ++id);
// }

std::string render_class_definition(const data::reflection_aggregator_t::entry_pointer_t<data::record_t>& record) {
    std::string template_declaration;
    if(!record->template_arguments.empty()) {
        template_declaration += fmt::format("template<{}> ", fmt::join(record->template_arguments, ", ") );
    }

    return fmt::format("{}{} {};", template_declaration, record->is_struct ? "struct" : "class", record->name);
}

std::string render_forward_declaration(const reflection_aggregator_t::entry_container_t<record_t>& records) {
    std::set<std::string> result;
    for(const auto& record : records) {
        if(record->parent != nullptr) {
             // skip nested classes, they dont need a forward declaration because the name is accessible via the parent
            continue;
        }
        std::string declaration = render_class_definition(record);
        if(record->qualified_namespace.empty()) {
            result.emplace(declaration);
        } else {
            result.emplace(fmt::format("namespace {} {{ {} }}", record->qualified_namespace, declaration));
        }
    }
    return fmt::format("{}", fmt::join(result, "\n"));
}

std::string render_forward_declaration(const reflection_aggregator_t::entry_container_t<enum_t>& enums) {
    std::string result;
    for(const auto& e : enums) {
        const std::string declaration = fmt::format(
            "enum {}{}{};",
            e->scoped ? "class " : "",
            e->name,
            e->underlying_type ? fmt::format(" : {}", e->underlying_type.value()->get_name()) : ""
        );
        if(e->qualified_namespace.empty()) {
            result += fmt::format("{}\n", declaration);
        } else {
            result += fmt::format("namespace {} {{ {} }}\n", e->qualified_namespace, declaration);
        }
    }
    return result;
}

std::string render_tsmp_global(
    const reflection_aggregator_t::entry_container_t<record_t>& records,
    const reflection_aggregator_t::entry_container_t<enum_t>& enums
) {
    namespace_wrapper_t global;
    for(const auto& record : records) {
        if(record->parent != nullptr) {
            // skip nested classes, they dont need an alias in global_t because the name is accessible via the parent
            continue;
        }
        global.insert(record.get(), record->qualified_namespace);
    }
    for(const auto& e : enums) {
        global.insert(e.get(), e->qualified_namespace);
    }

constexpr auto global_template =
R"(struct global_t {{
{}
}};
)";    
    return fmt::format(global_template, global);
}

// bool is_overloaded(const function_decl_t& function, const std::set<function_decl_t>& function_list) {
//     const auto count = std::count_if(function_list.begin(), function_list.end(), [&function](const auto& elem){ return function.name == elem.name;});
//     return count > 1;
// }

std::string render_field_description(const std::vector<field_decl_t>& fields) {
    std::string result;
    int i = 0;
    for(const auto& f : fields) {
        result += fmt::format("\t\t\tfield_description_t{{ {0}, \"{1}\", &value_type::{1} }},\n", i++, f.name);
    }
    if(!fields.empty()) {
        result.resize(result.size()-2);
    }
    return result;
}

std::string render_function_description(const std::vector<function_decl_t>& functions) {
    std::string result;
    int i = 0;
    for(const auto& f : functions) {
        if(f.name == "~") continue;
        result += fmt::format(
            "\t\t\tfunction_description_t{{ {0}, \"{1}\", static_cast<{2} (value_type::*)({3}) {4}{5}>(&value_type::{1}) }},\n",
            i++,
            f.name,
            f.result ? f.result->get_name("typename GlobalNamespaceHelper::") : "<unknown>",
            transform_join(f.parameter, ", ", [](const parameter_decl_t& p){ return p.type ? p.type->get_name("typename GlobalNamespaceHelper::") : "<unknown>"; }),
            f.is_const ? "const"  : "",
            f.ref_qualifier
        );
    }
    
    if(!functions.empty()) {
        result.resize(result.size()-2);
    }
    return result;
}

std::string render_proxy_functions(const std::vector<function_decl_t>& functions) {
    std::string result;
    for(const auto& function : functions) {
        if(function.name == "~") continue;      
        std::vector<std::string> param_list;
        std::uint64_t placeholder_id = 0;
        for(auto& p : function.parameter) {
            if(auto ref = dynamic_cast<const data::reference_t*>(p.type); ref != nullptr && ref->ref_qualifier == data::ref_qualifier_t::lvalue) {
                param_list.emplace_back(p.name);
            } else {
                param_list.emplace_back(fmt::format("std::move({})", p.name));
            }
        }
        result += fmt::format(
R"(    {10}{4} {0}({1}) {2}{3}{{
        if constexpr ((std::is_base_of_v<Base, value_type> || ...)) {{
            auto base_function = [](auto* proxy, auto&&... argv) -> decltype(auto) {{ return proxy->value_type::{0}(std::forward<decltype(argv)>(argv)...); }};
            return fn(base_function, "{0}"{9}{5}{6});
        }} else {{
            using Fn = {7} ({8}*)({1}) {2}{3};
            Fn base_function = &value_type::{0};
            return fn(base_function, "{0}"{9}{5}{6});
        }}
    }}
)",
            function.name,
            transform_join(function.parameter, ", ", [](const parameter_decl_t& param) {
                return fmt::format(
                    "{} {}{}",
                    param.type ? param.type->get_name("typename GlobalNamespaceHelper::") : "<unknown>",
                    param.name, param.is_pack ? "..." : ""
                );
            }),
            function.is_const ? "const" : "",
            function.ref_qualifier,
            function.is_virtual ? fmt::format("{}", function.result->get_name("typename GlobalNamespaceHelper::")) : "decltype(auto)",
            param_list.empty() ? "" : ", ",
            fmt::join(param_list, ", "),
            function.result ? function.result->get_name("typename GlobalNamespaceHelper::") : "<unknown>",
            function.is_static ? "" : "value_type::",
            function.is_static ? "" : ", accessor(this)",
            function.is_constexpr ? "constexpr " : ""
        );
    }

    return result;
}

std::string render_record_declaration(const reflection_aggregator_t::entry_container_t<record_t>& records) {
    constexpr std::string_view reflect_trait_template =
R"(template <class GlobalNamespaceHelper>
struct reflect_impl{} {{
    static constexpr bool reflectable = true;

    using value_type = {};

    constexpr static auto name() {{
        return "{}";
    }}
    constexpr static auto fields() {{
        return std::make_tuple(
    {}
        );
    }}

    constexpr static auto functions() {{
        return std::make_tuple(
    {}
        );
    }}
}};

)";

    constexpr std::string_view proxy_trait_template =
R"(template <class GlobalNamespaceHelper, class Accessor, class Functor, class... Base> 
struct proxy_impl{} : public Base... {{
    Accessor accessor;
    Functor fn;

    using value_type = {};

    proxy_impl(Accessor accessor = {{}}, Functor fn = {{}}) :
        accessor(std::move(accessor)),
        fn(std::move(fn))
    {{}}
{}
}};

)";

    constexpr std::string_view trivial_type_trait_template =
R"(template <>
struct reflect<{0}> {{
    static constexpr bool reflectable = true;
    constexpr static auto name() {{
        return "{0}";
    }}
    constexpr static auto fields() {{
        return std::make_tuple();
    }}

    constexpr static auto functions() {{
        return std::make_tuple();
    }}
}};

)";

    std::string result;
    for(const auto& record : records) {
        const std::string fields = render_field_description(record->fields);
        const std::string functions = render_function_description(record->functions);
        const std::string proxy_functions = render_proxy_functions(record->functions);

        const auto decorated_name = record->get_name("typename GlobalNamespaceHelper::"); 
        result += 
            fmt::format(
                reflect_trait_template,
                fmt::format("<GlobalNamespaceHelper, {}>", decorated_name),
                decorated_name,
                record->name,
                fields,
                functions
            );

        std::string unqualified_namespace = fmt::format("{1}{0}",
            record->get_namespace(data::namespace_option_t::unqualified),
            record->qualified_namespace.empty() ? "" : "::"
        );

        result+=
            fmt::format(
                proxy_trait_template,
                fmt::format("<GlobalNamespaceHelper, {}, Accessor, Functor, Base...>", decorated_name),
                decorated_name,
                proxy_functions
            );
    }

    return result;
}

renderer_t::renderer_t(std::string header) :
    header(std::move(header))
{}

std::string render_enum_declaration(const reflection_aggregator_t::entry_container_t<enum_t>& enum_declarations) {
    std::string result;
    for(const auto& decl : enum_declarations) {
        const std::string entries = transform_join(decl->values, ",\n", [](const std::string& value){
            return fmt::format("        enum_entry_description_t {{ \"{0}\", value_type::{0} }}", value);
        });
        result += fmt::format(
R"(
template <class GlobalNamespaceHelper>
struct enum_value_adapter_impl<GlobalNamespaceHelper, {0}> {{
    using value_type = {0};
    constexpr static std::array values {{
{1}
    }};
}};

)", 
            decl->get_name("typename GlobalNamespaceHelper::"),
            entries
        );
    }
    return result;
}

void renderer_t::render(const data::reflection_aggregator_t& aggregator) {

// FIXME: including cstdint is a workaround to get integer literals working as template arguments
// it currently does not work for custom aliases
constexpr std::string_view code_template = 
R"(#pragma once
#include <tuple>
#include <stdbool.h>
#include <cstdint>
#include <cstddef>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wgnu-redeclared-enum"
{}
{}
#pragma GCC diagnostic pop
namespace tsmp {{

template <class GlobalNamespaceHelper, class T>
struct reflect_impl;

template <class GlobalNamespaceHelper, class T, class Accessor, class Functor, class... Base>
struct proxy_impl;

template <class GlobalNamespaceHelper, Enum E>
struct enum_value_adapter_impl;

{}
{}
{}
}}
)";

    fmt::print(
        "List of records to be rendered:\n{}\n",
        transform_join(aggregator.fetch<record_t>(), "\n", [](const auto& record){ return record->get_name("typename GlobalNamespaceHelper::"); })
    );

    fmt::print(
        header,
        code_template,
        render_forward_declaration(aggregator.fetch<data::record_t>()),
        render_forward_declaration(aggregator.fetch<data::enum_t>()),
        render_tsmp_global(aggregator.fetch<data::record_t>(), aggregator.fetch<data::enum_t>()),
        render_record_declaration(aggregator.fetch<data::record_t>()),
        render_enum_declaration(aggregator.fetch<data::enum_t>())
    );
}

}