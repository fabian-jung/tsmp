#include "renderer.hpp"
#include "data/prefix_splitter.hpp"
#include "data/types.hpp"
#include "fmt/ostream.h"

#include <algorithm>
#include <map>
#include <string_view>
namespace data {

std::string erase_substring(std::string input, const std::string& substring) {
    std::string::size_type pos = 0u;
    while((pos = input.find(substring, pos)) < input.size()) {
        input.erase(pos, substring.size());
    }
    return input;
}

std::string strip_special_chars(std::string input) {
    const std::map<char, char> replacement_map {
        {'~', 't'},
        {'=', 'e'},
        {'+', 'p'},
        {'-', 'm'},
        {'*', 'u'},
        {'/', 'd'},
        {'|', 'l'}        
    };
    auto is_special = [](char c) { 
        return !((c>='a'&& c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9') || c =='_');
    };
    auto pos = input.begin();
    while((pos=std::find_if(pos, input.end(), is_special)) != input.end()) {
        auto& c = *pos;
        const auto replacement = replacement_map.find(*pos);
        if(replacement != replacement_map.end()) {
            *pos = replacement->second;
        } else {
            *pos = 'x';
        }
    }
    return input;
}

function_decl_t strip_special_chars_from_function(function_decl_t input) {
    input.name = strip_special_chars(std::move(input.name));
    return input;
}

std::string render_forward_declaration(const std::vector<record_decl_t>& records) {
    std::string result;
    for(const auto& record : records) {
        if(record.is_forward_declarable) {
            std::string template_declaration;
            if(!record.template_arguments.empty()) {
                template_declaration += fmt::format("template<{}> ", fmt::join(record.template_arguments, ", ") );
            }
            std::string declaration = fmt::format("{}{} {};", template_declaration, record.is_struct ? "struct" : "class", record.name);
            if(record.qualified_namespace.empty()) {
                result += fmt::format("{}\n", declaration);
            } else {
                result += fmt::format("namespace {} {{ {} }}\n", record.qualified_namespace, declaration);
            }
        }
    }
    return result;
}

std::string render_concepts_for_fields(const std::set<field_decl_t>& fields) {
    std::string result;
    for(auto field : fields) {
        result += fmt::format(
            "template<class T> concept has_field_{0} = requires {{ T::{0}; }};\n",
                field.name
        );
    }
    return result;
}

std::string render_concepts_for_functions(const std::set<function_decl_t>& escaped_function_names) {
    std::string result;
    for(const auto& function_name : escaped_function_names) {
        if(!function_name.overloaded) {
            result += fmt::format(
                "template<class T> concept has_function_{0} = requires {{ &T::{0}; }};\n",
                function_name.name
            );
        }
    }
    return result;
}

std::set<function_decl_t> escape_function_names(const std::set<function_decl_t>& functions) {
    std::set<function_decl_t> result;
    std::transform(functions.begin(), functions.end(), std::inserter(result, result.begin()), strip_special_chars_from_function);

    return result;
}

std::vector<record_decl_t> escape_function_names(std::vector<record_decl_t> records) {
    for(auto& record : records) {
       record.functions = escape_function_names(record.functions);
    }
    return records;
}

std::string render_field_description(const std::set<field_decl_t>& fields) {
    std::string result;
    int i = 0;
    for(const auto& f : fields) {
        result += fmt::format("\t\t\tfield_description_t{{ {0}, \"{1}\", &T::{1} }},\n", i++, f.name);
    }
    if(!fields.empty()) {
        result.resize(result.size()-2);
    }
    return result;
}

std::string render_function_description(const std::set<function_decl_t>& functions) {
    std::string result;
    int i = 0;
    for(const auto& f : functions) {
        if(f.name == "~") continue;
        result += fmt::format("\t\t\tfunction_description_t{{ {0}, \"{1}\", &T::{1} }},\n", i++, f.name);
    }
    
    if(!functions.empty()) {
        result.resize(result.size()-2);
    }
    return result;
}

std::string render_proxy_member(const std::set<field_decl_t>& fields) {
    std::string result;
    int i = 0;
    for(const auto& f : fields) {
        result += fmt::format("\tdecltype(__tsmp_accessor(__tsmp_base).{0})& {0} = __tsmp_accessor(__tsmp_base).{0};\n", f.name);
    }

    return result;
}
std::string render_proxy_functions(const std::set<function_decl_t>& functions) {
    std::string result;

    std::set<std::string> function_names;
    std::transform(
        functions.begin(),
        functions.end(),
        std::inserter(function_names, function_names.end()),
        [](const auto& decl){ return decl.name;
    });

    for(const auto& name : function_names) {
        if(name == "~") continue;
        result += fmt::format(
R"(template <class... Args>
constexpr decltype(auto) {0}(Args&&... args) {{
    auto __tsmp_base_function = [this](auto... argv) -> decltype(auto) {{ return __tsmp_accessor(__tsmp_base).{0}(std::forward<decltype(argv)>(argv)...); }};
    return __tsmp_fn(__tsmp_base_function, "{0}", std::forward<Args>(args)...);
}}
)",
            name
        );
    }

    return result;
}

std::string render_requires_clauses(const record_decl_t& record) {
    std::string result;
    if(record.is_forward_declarable) {
        std::string unqualified_namespace = erase_substring(record.qualified_namespace, "inline ");
        std::string template_definition;
        if(!record.template_arguments.empty()) {
            std::string list;
            for(auto s : record.template_arguments) {
                list += s.value + ", ";
            }
            list.resize(list.size()-2);
            template_definition = fmt::format("<{}>",list);
        }
        result += fmt::format(
            "requires std::same_as<std::remove_cv_t<T>, {}{}{}{}>",
            unqualified_namespace, 
            unqualified_namespace.empty() ? "" : "::",
            record.name,
            template_definition
        );
    } else {
        result += (!record.functions.empty()||!record.fields.empty()) ? "requires " : "";
        if(!record.fields.empty()) {               
            result += fmt::format("has_field_{}<T>", fmt::join(record.fields, "<T> && has_field_"));
        }
        std::vector<function_decl_t> non_overloaded_functions;
        for(const auto& function : record.functions) {
            if(!function.overloaded) {
                non_overloaded_functions.emplace_back(function);
            }
        }
        if(!record.functions.empty()) {
            if(!record.fields.empty()) {
                result += " && " ;
            }
            result += fmt::format("has_function_{}<T>", fmt::join(non_overloaded_functions, "<T> && has_function_"));
        }
    }
    return result;
}

std::string render_record_declaration(const prefix_splitter_t& splitter) {
    constexpr std::string_view reflect_trait_template =
R"(template <class T>
{}
struct reflect{} {{
    static constexpr bool reflectable = true;
    static constexpr bool forward_declareable = {};
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
R"(template <class T, template<class> class Container, class Accessor, class Functor> 
{}
struct proxy{} {{
    Container<T> __tsmp_base;
    Accessor __tsmp_accessor; // maps container<foo_t> to foo_t&
    Functor __tsmp_fn; // User function

{}

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

    const auto records = escape_function_names(splitter.records());
    const auto fields = splitter.fields();
    const auto functions = escape_function_names(splitter.functions());
    const auto trivial_types = splitter.trivial_types();

    std::string result;
    result += render_concepts_for_fields(fields);
    result += render_concepts_for_functions(functions);  
    result += '\n';

    for(auto record : records) {
        const std::string fields = render_field_description(record.fields);
        const std::string proxy_members = render_proxy_member(record.fields);
        const std::string functions = render_function_description(record.functions);
        const std::string proxy_functions = render_proxy_functions(record.functions);
         
        std::string requirements = render_requires_clauses(record);

        result += 
            fmt::format(
                reflect_trait_template,
                requirements,
                requirements.empty() ? "" : "<T>",
                record.is_forward_declarable,
                record.name,
                fields,
                functions
            );

        result+=
            fmt::format(
                proxy_trait_template,
                requirements,
                requirements.empty() ? "" : "<T, Container, Accessor, Functor>",
                proxy_members,
                proxy_functions
            );
    }

    for(auto record : trivial_types) {
        result += 
            fmt::format(
                trivial_type_trait_template,
                record
            );
    }

    return result;
}

renderer_t::renderer_t(std::string output_file) :
    output_file(std::move(output_file))
{}

std::string render_enum_declaration(const enum_splitter_t& enum_splitter) {
    std::string result;
    for(const auto& value : enum_splitter.fields) {
        result += fmt::format("template<class E> concept has_enum_value_{0} = requires(E) {{ E::{0}; }};\n", value);
    }
    result += "\n";
    for(const auto& decl : enum_splitter.decls) {
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

void renderer_t::render(const data::reflection_aggregator_t& aggregator) {
   
constexpr std::string_view code_template = 
R"(#pragma once
#include <tuple>
#include <tsmp/reflect.hpp>
#include <tsmp/proxy.hpp>
{}
namespace tsmp {{
{}
{}
}}
)";

    const auto records = aggregator.records();
    const auto trivial_types = aggregator.trivial_types();
    const auto enums = aggregator.enums();

    const prefix_splitter_t splitter(records, trivial_types);
    const enum_splitter_t enum_splitter(enums);

    fmt::print(
        output_file,
        code_template,
        render_forward_declaration(records),
        render_record_declaration(splitter),
        render_enum_declaration(enum_splitter)
    );
}

}