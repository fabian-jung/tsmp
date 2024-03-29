#include "ast_traversal_tool.hpp"
#include "data/aggregator.hpp"
#include "data/types.hpp"
#include "fmt/core.h"
#include "fmt/ostream.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Tooling/Tooling.h"
#include <algorithm>
#include <clang/AST/DeclTemplate.h>
#include <iostream>

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using MatchFinder = clang::ast_matchers::MatchFinder;

namespace engine {

std::string get_unqualified_namespace(const clang::NamespaceDecl* decl)
{
    if (!decl)
        return "";
    std::string name = decl->getName().str();
    if (auto parent = decl->getParent()) {
        if (parent->getDeclKind() == Decl::Kind::TranslationUnit) {
            return name;
        }
    }
    return fmt::format("{}{}{}",
                       get_unqualified_namespace(static_cast<const clang::NamespaceDecl*>(decl->getParent())),
                       decl->isInlineNamespace() ? "" : "::",
                       decl->isInlineNamespace() ? "" : name);
}

const data::type_t* ast_traversal_tool_t::register_type(const EnumDecl* decl)
{
    if (!decl)
        return nullptr;
    if (auto it = visited_nodes.find(decl); it != visited_nodes.end()) {
        return it->second;
    }
    const auto name = decl->getDeclName().getAsString();
    std::string qualified_namespace = get_namespace(decl);

    std::optional<const data::type_t*> underlying_type =
        [this](const EnumDecl* decl) -> std::optional<const data::type_t*> {
        if (decl->isFixed()) {
            return register_type(decl->getIntegerType());
        } else {
            return std::nullopt;
        }
    }(decl);

    fmt::print("enum analysis for {}{}{}{} yields values:\n",
               qualified_namespace,
               qualified_namespace.empty() ? "" : "::",
               name,
               underlying_type ? fmt::format(" : {}", underlying_type.value()->get_name()) : "");
    std::vector<std::string> values;
    for (auto enumValue : decl->enumerators()) {
        fmt::print("    {}\n", enumValue->getDeclName().getAsString());
        values.emplace_back(enumValue->getDeclName().getAsString());
    }
    auto [it, success] = visited_nodes.emplace(
        decl,
        aggregator.create<data::enum_t>(
            std::move(name), std::move(qualified_namespace), decl->isScoped(), std::move(values), underlying_type));
    return it->second;
}

std::string ast_traversal_tool_t::get_name(const clang::TemplateArgument& argument)
{
    std::string result;
    if (argument.getKind() == clang::TemplateArgument::Integral) {
        return std::to_string(argument.getAsIntegral().getExtValue());
    }
    llvm::raw_string_ostream s(result);
    LangOptions LO; // FIXME! see also TemplateName::dump().
    LO.CPlusPlus = true;
    LO.Bool = true;
    argument.print(LO, s, false);
    if (result.size() >= 2 && result.front() == '<') {
        result = result.substr(1, result.size() - 2);
    }
    return result;
}

std::vector<data::template_argument_t> ast_traversal_tool_t::template_argument_analysis(const CXXRecordDecl* decl)
{
    std::vector<data::template_argument_t> result;
    const auto* specialization = dynamic_cast<const ClassTemplateSpecializationDecl*>(decl);
    if (specialization) {
        const auto params = specialization->getTemplateArgs().asArray();
        const auto args = specialization->getSpecializedTemplate()->getTemplateParameters()->asArray();
        if (args.size() != params.size()) {
            fmt::print(std::cerr,
                       "Not all template parameters are specified. This indicates that reflect has been called, but T "
                       "is not fully specialized.");
        }
        for (int i = 0; i < args.size(); ++i) {
            auto arg = args[i];
            auto param = params[i];
            const auto value = get_name(param);

            std::string name = arg->getName().str();
            if (name.empty()) {
                name = fmt::format("tsmp_template_argument_{}", i);
            }
            if (const auto* type_decl = dynamic_cast<TemplateTypeParmDecl*>(arg)) {
                result.emplace_back(data::template_argument_t{"typename", name, value, arg->isTemplateParameterPack()});
                if (param.getKind() == clang::TemplateArgument::ArgKind::Type) {
                    register_type(param.getAsType()->getAsCXXRecordDecl());
                }
            } else if (const auto* value_decl = dynamic_cast<NonTypeTemplateParmDecl*>(arg)) {
                auto type_name = register_type(value_decl->getType())->get_name("::");
                result.emplace_back(data::template_argument_t{type_name, name, value});
            } else if (const auto* template_template_decl = dynamic_cast<TemplateTemplateParmDecl*>(arg)) {
                std::vector<data::template_argument_t> template_template_args;
                for (auto p : template_template_decl->getTemplateParameters()->asArray()) {
                    template_template_args.emplace_back(data::template_argument_t{
                        "typename", p->getDeclName().getAsString(), "", p->isTemplateParameterPack()});
                }
                const bool is_pack = arg->isTemplateParameterPack();
                result.emplace_back(
                    data::template_argument_t{"typename", name, value, is_pack, std::move(template_template_args)});
            } else {
                fmt::print("Template argument type not implemented.\n");
            }
        }
    }
    return result;
}

std::string ast_traversal_tool_t::get_name(const clang::IdentifierInfo* identifier)
{
    return identifier->getName().str();
}

std::string ast_traversal_tool_t::get_nested_name_specifier(const NestedNameSpecifier* specifier)
{
    switch (specifier->getKind()) {
        case clang::NestedNameSpecifier::Identifier:
            return get_name(specifier->getAsIdentifier());
        case clang::NestedNameSpecifier::Namespace:
            return get_unqualified_namespace(specifier->getAsNamespace());
        case clang::NestedNameSpecifier::NamespaceAlias:
            return get_unqualified_namespace(specifier->getAsNamespaceAlias()->getNamespace());
        case clang::NestedNameSpecifier::TypeSpec:
            return get_name(specifier->getAsType());
        case clang::NestedNameSpecifier::TypeSpecWithTemplate:
            return fmt::format("template {}", get_name(specifier->getAsType()));
        case clang::NestedNameSpecifier::Global:
            return "::";
        case clang::NestedNameSpecifier::Super:
            return "__super";
    }
    return "";
}

std::vector<data::field_decl_t> ast_traversal_tool_t::field_analysis(const CXXRecordDecl* decl)
{
    std::vector<data::field_decl_t> result;

    for (const auto& field : decl->fields()) {
        if (field->getAccess() != AS_public) {
            continue;
        }

        const auto* type = register_type(field->getType());
        auto name = field->getNameAsString();

        if (name.size() > 0) {

            fmt::print("-field: {} {}\n", type->get_name(), name);
            result.emplace_back(data::field_decl_t{std::move(name), type});
        }
    }
    return result;
}

std::string ast_traversal_tool_t::get_name(const clang::CXXRecordDecl* decl)
{
    std::string name = decl->getName().str();
    auto template_parameter_list = template_argument_analysis(decl);
    if (!template_parameter_list.empty()) {
        std::vector<std::string> parameter_types;
        std::transform(template_parameter_list.begin(),
                       template_parameter_list.end(),
                       std::back_inserter(parameter_types),
                       [](const auto& param) { return param.get_value(); });
        name = fmt::format("{}<{}>", name, fmt::join(parameter_types, ", "));
    }
    return name;
}

std::string ast_traversal_tool_t::get_name(const clang::Type* type)
{
    switch (type->getTypeClass()) {
        case clang::Type::TypeClass::Enum: {
            const auto* decl = type->getAsTagDecl();
            std::string name = decl->getName().str();
            return name;
        }
        case clang::Type::TypeClass::Record:
            return get_name(type->getAsCXXRecordDecl());
        case clang::Type::TypeClass::Auto:
            return get_qualified_name(type->getContainedAutoType()->getContainedDeducedType()->getDeducedType());
        case clang::Type::TypeClass::Decltype:
            return get_qualified_name(type->getAs<const DecltypeType>()->getUnderlyingType());
        case clang::Type::TypeClass::Builtin: {
            LangOptions lo;
            PrintingPolicy policy{lo};
            policy.adjustForCPlusPlus();
            return type->getAs<BuiltinType>()->getName(policy).str();
        }
        case clang::Type::TypeClass::Typedef:
            return fmt::format("{}", get_qualified_name(type->getAs<TypedefType>()->desugar()));
        case clang::Type::TypeClass::Pointer:
            return fmt::format("{}*", get_qualified_name(type->getPointeeType()));
        case clang::Type::TypeClass::LValueReference:
            return fmt::format("{}&", get_qualified_name(type->getPointeeType()));
        case clang::Type::TypeClass::RValueReference:
            return fmt::format("{}&&", get_qualified_name(type->getPointeeType()));
        case clang::Type::Elaborated: {
            if (type->isElaboratedTypeSpecifier()) {
                const auto elaborated = type->getAs<const ElaboratedType>();
                if (elaborated) {
                    const auto qualifier = elaborated->getQualifier();
                    const auto named_type = elaborated->getNamedType();
                    if (qualifier) {
                        return fmt::format(
                            "{}{}", get_nested_name_specifier(qualifier), get_qualified_name(named_type));
                    }
                }
                return "<UnspecifiedElaboratedType>";
            } else {
                return get_name(type->getUnqualifiedDesugaredType());
            }
        }
        case clang::Type::TypeClass::SubstTemplateTypeParm: {
            const auto* subst_template_type = type->getAs<SubstTemplateTypeParmType>();
            if (subst_template_type) {
                return get_name(subst_template_type->getReplacementType());
            } else {
                return "<SubstTemplateTypeParm>";
            }
        }
        case clang::Type::TypeClass::TemplateSpecialization: {
            const auto specialisation = type->getAs<TemplateSpecializationType>();
            const auto name = specialisation->getTemplateName();
            const auto arguments = specialisation->template_arguments();
            std::vector<std::string> argument_names;
            for (const auto& arg : arguments) {
                argument_names.emplace_back(get_name(arg));
            }
            return fmt::format("{}<{}>", get_name(name), fmt::join(argument_names, ", "));
        }
        default:
            fmt::print("trouble identifying {}\n", type->getTypeClassName());
            return "<unknown>";
    }
}

std::string ast_traversal_tool_t::get_namespace(const clang::DeclContext* context, bool qualified)
{
    auto parent = context->getParent();
    if (!parent->isNamespace()) {
        return "";
    }
    auto ns = static_cast<const clang::NamespaceDecl*>(parent);
    auto parent_namespace = get_namespace(parent);
    return fmt::format("{}{}{}{}",
                       parent_namespace,
                       parent_namespace.empty() ? "" : "::",
                       ns->isInline() ? "inline " : "",
                       ns->getName().str());
}

std::string ast_traversal_tool_t::get_qualified_name(const clang::QualType& qtype)
{
    auto [type, qualifiers] = qtype.split().asPair();
    return fmt::format("{}{}{}{}{}",
                       qualifiers.hasConst() && !qtype->isPointerType() ? "const " : "",
                       qualifiers.hasVolatile() ? "volatile " : "",
                       qualifiers.hasRestrict() ? "restrict " : "",
                       get_name(type),
                       qualifiers.hasConst() && qtype->isPointerType() ? " const " : "");
}

data::cv_qualifier_t from_qual(clang::Qualifiers quals)
{
    constexpr auto const_case = 0b01;
    constexpr auto volatile_case = 0b10;
    constexpr auto const_volatile_case = 0b11;
    const auto qual_mask = quals.hasConst() & const_case | quals.hasVolatile() & volatile_case;
    switch (qual_mask) {
        case const_case:
            return data::cv_qualifier_t::const_;
        case volatile_case:
            return data::cv_qualifier_t::volatile_;
        case const_volatile_case:
            return data::cv_qualifier_t::const_volatile;
        default:
            return data::cv_qualifier_t::nothing;
    }
}

const data::type_t* ast_traversal_tool_t::register_type(const clang::CXXRecordDecl* record_decl)
{
    if (record_decl == nullptr)
        return nullptr;
    if (auto it = visited_nodes.find(record_decl); it != visited_nodes.end()) {
        return it->second;
    }
    std::string name = record_decl->getName().str();
    if (name.empty()) {
        std::string s;
        llvm::raw_string_ostream stream(s);
        record_decl->dump(stream);
        fmt::print("Can not reflect unnamed type:{}\n", s);
        return nullptr;
    }
    auto qualified_namespace = get_namespace(record_decl);

    auto record = std::make_unique<data::record_t>(name, qualified_namespace, record_decl->isStruct());
    record->template_arguments = template_argument_analysis(record_decl);
    fmt::print("CXXRecordDecl analysis for: {}\n", record->get_name());

    visited_nodes.emplace(record_decl, record.get());
    record->fields = field_analysis(record_decl);
    record->functions = method_analysis(record_decl, name);
    if (auto parent = record_decl->getParent(); parent->getDeclKind() == clang::Decl::Kind::CXXRecord) {
        record->parent = register_type(static_cast<const CXXRecordDecl*>(parent));
    }

    const data::type_t* result = aggregator.emplace(std::unique_ptr<const data::record_t>(std::move(record)));

    return result;
}

const data::type_t* ast_traversal_tool_t::register_type(const clang::Type* type)
{
    if (auto it = visited_nodes.find(type); it != visited_nodes.end()) {
        return it->second;
    }
    switch (type->getTypeClass()) {
        case clang::Type::TypeClass::Enum: {
            return register_type(dynamic_cast<clang::EnumDecl*>(type->getAsTagDecl()));
        }
        case clang::Type::TypeClass::Record: {
            return register_type(type->getAsCXXRecordDecl());
        }
        case clang::Type::TypeClass::Auto:
            return register_type(type->getContainedAutoType()->getContainedDeducedType()->getDeducedType());
        case clang::Type::TypeClass::Decltype:
            return register_type(type->getAs<DecltypeType>()->getUnderlyingType());
        case clang::Type::TypeClass::Builtin: {
            LangOptions lo;
            PrintingPolicy policy{lo};
            policy.adjustForCPlusPlus();
            return aggregator.create<data::builtin_t>(type->getAs<BuiltinType>()->getName(policy).str());
        }
        case clang::Type::TypeClass::Typedef:
            return register_type(type->getAs<TypedefType>()->desugar().getTypePtr());
        case clang::Type::TypeClass::Pointer: {
            auto [t, qual] = type->getPointeeType().split();
            return aggregator.create<data::pointer_t>(register_type(t), from_qual(qual));
        }
        case clang::Type::TypeClass::LValueReference:
            return register_type(type->getPointeeType());
        case clang::Type::TypeClass::RValueReference:
            return register_type(type->getPointeeType());
        case clang::Type::Elaborated:
            return register_type(type->getUnqualifiedDesugaredType());
        case clang::Type::TypeClass::SubstTemplateTypeParm: {
            const auto* subst_template_type = type->getAs<SubstTemplateTypeParmType>();
            if (subst_template_type) {
                return register_type(subst_template_type->getReplacementType());
            } else {
                return nullptr;
            }
        }
        case clang::Type::TypeClass::TemplateSpecialization:
            return register_type(type->getAsCXXRecordDecl());
        case clang::Type::TypeClass::ConstantArray: {
            const auto* array_type = static_cast<const ConstantArrayType*>(type->getAsArrayTypeUnsafe());
            return aggregator.create<data::constant_array_t>(register_type(array_type->getElementType()),
                                                             array_type->getSize().getZExtValue());
        }
        case clang::Type::TypeClass::Paren:
            return register_type(type->getAs<ParenType>()->desugar());
        case clang::Type::TypeClass::FunctionProto:
            return aggregator.create<data::builtin_t>("void");
        case clang::Type::TypeClass::Using:
            return register_type(type->getAs<UsingType>()->desugar());
        default:
            fmt::print(std::cerr, "trouble identifying {}\n", type->getTypeClassName());
            return nullptr;
    }
}

const data::type_t* ast_traversal_tool_t::register_type(const clang::QualType& qtype)
{
    auto [type, qualifiers] = qtype.split();
    const std::uint32_t cv_flags = 0b1 * qualifiers.hasConst() || 0b10 * qualifiers.hasVolatile();
    auto cv_qualifier = [](std::uint32_t cv) {
        switch (cv) {
            case 0b01:
                return data::cv_qualifier_t::const_;
            case 0b10:
                return data::cv_qualifier_t::volatile_;
            case 0b11:
                return data::cv_qualifier_t::const_volatile;
            default:
                return data::cv_qualifier_t::nothing;
        }
    }(cv_flags);
    auto ref_qualifier = [](const auto& qual_type) {
        if (qual_type->isLValueReferenceType()) {
            return data::ref_qualifier_t::lvalue;
        }
        if (qual_type->isRValueReferenceType()) {
            return data::ref_qualifier_t::rvalue;
        }
        return data::ref_qualifier_t::nothing;
    }(qtype);
    const auto* result = register_type(type);
    if (ref_qualifier != data::ref_qualifier_t::nothing) {
        return aggregator.create<data::reference_t>(result, cv_qualifier, ref_qualifier);
    }
    if (cv_qualifier != data::cv_qualifier_t::nothing) {
        return aggregator.create<data::cv_qualified_type_t>(result, cv_qualifier);
    }
    return result;
}

auto to_ref(clang::RefQualifierKind clang)
{
    switch (clang) {
        case clang::RefQualifierKind::RQ_LValue:
            return data::ref_qualifier_t::lvalue;
        case clang::RefQualifierKind::RQ_RValue:
            return data::ref_qualifier_t::rvalue;
        case clang::RefQualifierKind::RQ_None:
            return data::ref_qualifier_t::nothing;
    }
    return data::ref_qualifier_t::nothing;
};

auto find_variardic_expansion(std::vector<data::parameter_decl_t>& parameters)
{
    using iterator_t = std::vector<data::parameter_decl_t>::iterator;
    using subrange_t = std::pair<iterator_t, iterator_t>;
    std::vector<subrange_t> result;
    for (iterator_t begin = parameters.begin(); begin != parameters.end(); ++begin) {
        iterator_t end = std::find_if_not(begin + 1, parameters.end(), [name = begin->name](const auto& end_value) {
            return name == end_value.name;
        });
        if (std::distance(begin, end) > 0) {
            result.emplace_back(begin, end);
        }
    }
    return result;
}

std::vector<data::parameter_decl_t> ast_traversal_tool_t::parameter_analysis(const clang::CXXMethodDecl* method)
{
    std::vector<data::parameter_decl_t> result;
    int i = 0;
    for (const auto* decl : method->parameters()) {
        auto name = decl->getName().str();
        if (name.empty()) {
            name = fmt::format("tsmp_parameter_{}", i++);
        }
        auto* type = register_type(decl->getType());
        auto is_pack = decl->isParameterPack();
        result.emplace_back(data::parameter_decl_t{std::move(name), type, is_pack});
    }
    for (auto [begin, end] : find_variardic_expansion(result)) {
        for (auto it = begin; it != end; ++it) {
            it->name = fmt::format("{}_{}", it->name, std::distance(begin, it));
        }
    }
    return result;
}

std::vector<data::function_decl_t> ast_traversal_tool_t::method_analysis(const CXXRecordDecl* decl,
                                                                         const std::string& decl_name)
{
    std::vector<data::function_decl_t> result;
    std::string record_name = decl->getName().str();
    for (const auto& method : decl->methods()) {
        auto name = method->getNameAsString();
        const auto access = method->getAccess();
        if (name.size() > 0 &&
            name != decl_name && // Addresses of constructors can not be taken, therefore we can not reflect them
            name.at(0) != '~' && // Addresses of constructors can not be taken, therefore we can not reflect them
            name.find("operator ") != 0 && // TODO: reflecting conversion operators may be added later
            access == AS_public && method->isDeleted() == false) {
            const bool is_noexcept = method->getExceptionSpecType() == ExceptionSpecificationType::EST_BasicNoexcept;

            const auto ref_qualifier = to_ref(method->getRefQualifier());
            auto params = parameter_analysis(method);
            std::string return_type = get_qualified_name(method->getReturnType());

            auto& entry = result.emplace_back(data::function_decl_t{.name = std::move(name),
                                                                    .parameter = params,
                                                                    .result = register_type(method->getReturnType()),
                                                                    .is_virtual = method->isVirtual(),
                                                                    .is_const = method->isConst(),
                                                                    .ref_qualifier = ref_qualifier,
                                                                    .is_constexpr = method->isConstexpr(),
                                                                    .is_noexcept = is_noexcept,
                                                                    .is_static = method->isStatic()});
            fmt::print("-method: {}\n", entry.signature(record_name));
        }
    }
    return result;
}

class CodeGenerator : public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    CodeGenerator(ast_traversal_tool_t& traversal_tool)
        : traversal_tool(traversal_tool)
    {
    }

    virtual void run(const MatchFinder::MatchResult& Result)
    {
        for (const auto& [id, node] : Result.Nodes.getMap()) {
            const auto args = node.get<ClassTemplateSpecializationDecl>()->getTemplateArgs().asArray();
            for (const auto& arg : args) {
                if (arg.getKind() != TemplateArgument::ArgKind::Type) {
                    std::string s;
                    llvm::raw_string_ostream stream(s);
                    LangOptions LO; // FIXME! see also TemplateName::dump().
                    LO.CPlusPlus = true;
                    LO.Bool = true;
                    arg.print(LO, stream, true);
                    fmt::print(
                        "Matcher Result from binding \"{}\" is not of kind type. Here is the dump:\"{}\"\n ", id, s);
                    continue;
                }
                if (id == "type") {
                    traversal_tool.register_type(arg.getAsType()->getAsCXXRecordDecl());
                } else if (id == "enum") {
                    traversal_tool.register_type(dynamic_cast<EnumDecl*>(arg.getAsType()->getAsTagDecl()));
                } else {
                    fmt::print("arg is not a type or enum, skip template argument.\n");
                }
            }
        }
    }

    ast_traversal_tool_t& traversal_tool;
};

void ast_traversal_tool_t::run(clang::tooling::ClangTool& tool)
{
    DeclarationMatcher reflection_matcher = classTemplateSpecializationDecl(matchesName("reflect")).bind("type");
    DeclarationMatcher proxy_matcher = classTemplateSpecializationDecl(matchesName("proxy")).bind("type");
    DeclarationMatcher enum_matcher = classTemplateSpecializationDecl(matchesName("enum_value_adapter")).bind("enum");
    MatchFinder finder;
    CodeGenerator generator{*this};
    finder.addMatcher(reflection_matcher, &generator);
    finder.addMatcher(proxy_matcher, &generator);
    finder.addMatcher(enum_matcher, &generator);
    tool.run(newFrontendActionFactory(&finder).get());
}

const data::reflection_aggregator_t& ast_traversal_tool_t::state() const
{
    return aggregator;
}

}