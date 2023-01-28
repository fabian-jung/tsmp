#include "code_generator.hpp"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Tooling/Tooling.h"
#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/TemplateBase.h>
#include <iostream>
#include <llvm/ADT/APSInt.h>
#include <llvm/Support/raw_ostream.h>
#include "data/aggregator.hpp"
#include "data/types.hpp"
#include "fmt/core.h"
#include <fmt/ostream.h>

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using MatchFinder = clang::ast_matchers::MatchFinder;

void reflect_enum(const EnumDecl* decl, data::reflection_aggregator_t& aggregator) {
    if(!decl) return;
    fmt::print("enum analysis for {} yields values:\n", decl->getDeclName().getAsString());
    data::enum_decl_t enum_decl;
    for(auto enumValue : decl->enumerators()) {
        fmt::print("    {}\n", enumValue->getDeclName().getAsString());
        enum_decl.values.emplace_back(enumValue->getDeclName().getAsString());
    }
    aggregator.add_enum_decl(enum_decl);
}


std::string get_qualified_name(const clang::NamespaceDecl* decl) {
    std::string name = decl->getName().str();
    if(decl->isInlineNamespace()) {
        name = "inline "+name;
    }
    if(decl->getParent()->getDeclKind() == Decl::Kind::TranslationUnit) {
        return name;
    }
    return get_qualified_name(static_cast<const clang::NamespaceDecl*>(decl->getParent()))+"::"+name;
}

bool isStdNamespace(const DeclContext* ns) {
    if(ns->isStdNamespace()) {
        return true;
    }
    if(ns->getDeclKind()==Decl::Kind::Namespace) {
        return isStdNamespace(ns->getParent());
    }
    return false;
}

void reflect_type(const CXXRecordDecl* decl, data::reflection_aggregator_t& aggregator) {
    if(decl == nullptr) return;
    data::record_decl_t record_decl;
    record_decl.name = decl->getName().str();
    if(auto parent = decl->getParent()) {
        if(parent->isNamespace()) {
            record_decl.qualified_namespace = get_qualified_name(static_cast<const clang::NamespaceDecl*>(parent));
            //TODO check: static_cast<const clang::NamespaceDecl*>(parent)->isInlineNamespace()
            record_decl.is_std_type = isStdNamespace(parent);
            record_decl.is_forward_declareable = true;
        } else if(parent->getDeclKind() == clang::Decl::Kind::TranslationUnit) {
            // Global type
            record_decl.is_forward_declareable = true;
        } else if(parent->getDeclKind() == clang::Decl::Kind::CXXRecord) {
            record_decl.is_nested_type = true;
        }
    }

    const auto* specialization = dynamic_cast<const ClassTemplateSpecializationDecl*>(decl);
    if(specialization) {
        llvm::outs() << record_decl.name << "\n";
        const auto params = specialization->getTemplateArgs().asArray();
        const auto args = specialization->getSpecializedTemplate()->getTemplateParameters()->asArray();
        if(args.size() != params.size()) {
            fmt::print(std::cerr, "Not all template parameters are specified. This indicates that reflect<{0}> has been called, but {0} is not fully specialized.", record_decl.name);
        }
        for(int i = 0; i < args.size(); ++i) {
            auto arg = args[i];
            auto param = params[i];

            std::string value;
            llvm::raw_string_ostream s(value);
            LangOptions LO; // FIXME! see also TemplateName::dump().
            LO.CPlusPlus = true;
            LO.Bool = true;
            param.print(LO, s, false);

            if(arg->isTemplateParameterPack())  {
                fmt::print("Template argument of template parameter pack type. Fall back to duck-type identification. May be implemented later.\n");
                record_decl.is_forward_declareable = false;
            }
            if(const auto* type_decl = dynamic_cast<TemplateTypeParmDecl*>(arg)) {
                fmt::print("type template arg. {} {} {}\n", "typename", arg->getName().str(), value);
                record_decl.template_arguments.emplace_back(data::template_argument_t{ "typename", arg->getName().str(), value });
                // reflect_type(dynamic_cast<const CXXRecordDecl*>(arg), aggregator);
                record_decl.is_forward_declareable = false; // Would require arg to forward declared transitively
            } else if(const auto* value_decl = dynamic_cast<NonTypeTemplateParmDecl*>(arg)) {
                fmt::print("value template arg: {} {} {}\n", value_decl->getType().getAsString(), value_decl->getName().str(), value);
                if(!value_decl->getType()->isBuiltinType()) {
                    fmt::print("Template argument is not of a builtin type. Fall back to duck-type identification.\n");
                    record_decl.is_forward_declareable = false;
                }
                record_decl.template_arguments.emplace_back(data::template_argument_t{ value_decl->getType().getAsString(), arg->getName().str(), value });
            } else {
                fmt::print("Template argument type not implemented.\n");
            }
        }
    }

    record_decl.is_struct = decl->isStruct();
    fmt::print(
        "member analysis for {}{} {} in {}\n",
        record_decl.is_nested_type ? "nested ": "",
        record_decl.is_struct ? "struct" : "class",
        record_decl.name,
        record_decl.qualified_namespace
    );
    for(const auto& field : decl->fields()) {
        const auto access = field->getAccess();
        const auto type = field->getType();
        const auto builtin = type.getTypePtr()->isBuiltinType();
        auto name = field->getNameAsString();
        if(!record_decl.is_std_type) {
            if(type->isEnumeralType()) {
                reflect_enum(dynamic_cast<const EnumDecl*>(type->getAsTagDecl()), aggregator);
            }
            if(!builtin) {
                reflect_type(type->getAsCXXRecordDecl(), aggregator);
            } else {
                aggregator.add_trivial_type(type.getCanonicalType().getAsString());
            }
        }
        if(access != AS_public) {
            continue;
        }
        if(name.size() > 0) {
            record_decl.fields.emplace_back(data::field_decl_t{std::move(name)});
        }
    }
    for(const auto& method : decl->methods()) {
        auto name = method->getNameAsString();
        if(
            name.size() > 0 &&
            name != record_decl.name && // Addresses of constructors can not be taken, therefore we can not reflect them
            name.at(0) != '~' // Addresses of constructors can not be taken, therefore we can not reflect them
        ) {
            record_decl.functions.emplace_back(data::function_decl_t{std::move(name)});
        }
    }
    aggregator.add_record_decl(std::move(record_decl));
}

class CodeGenerator : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
    virtual void run(const MatchFinder::MatchResult &Result) {
        for (const auto& [id, node] : Result.Nodes.getMap()) {
            const auto args = node.get<ClassTemplateSpecializationDecl>()->getTemplateArgs().asArray();
            for(const auto& arg : args) {
                if(arg.getKind()!=TemplateArgument::ArgKind::Type) {
                    std::string s;
                    llvm::raw_string_ostream stream(s);
                    LangOptions LO; // FIXME! see also TemplateName::dump().
                    LO.CPlusPlus = true;
                    LO.Bool = true;
                    arg.print(LO, stream, true);
                    fmt::print("Matcher Result from binding \"{}\" is not of kind type. Here is the dump:\"{}\"\n ", id, s);
                    continue;
                }
                if(id=="type") {
                    reflect_type(arg.getAsType()->getAsCXXRecordDecl(), aggregator);
                } else if(id=="enum") {
                    reflect_enum(dynamic_cast<EnumDecl*>(arg.getAsType()->getAsTagDecl()), aggregator);
                } else {
                    fmt::print("arg is not a type or enum, skip template argument.\n");
                }
            }
        }
    }

    data::reflection_aggregator_t aggregator;
};

data::reflection_aggregator_t generate_code_for_target(clang::tooling::ClangTool& tool) {
    DeclarationMatcher reflection_matcher = classTemplateSpecializationDecl(matchesName("reflect")).bind("type");
    DeclarationMatcher proxy_matcher = classTemplateSpecializationDecl(matchesName("proxy")).bind("type");
    DeclarationMatcher enum_matcher = classTemplateSpecializationDecl(matchesName("enum_value_adapter")).bind("enum");
    MatchFinder finder;
    CodeGenerator generator;
    finder.addMatcher(reflection_matcher, &generator);
    finder.addMatcher(proxy_matcher, &generator);
    finder.addMatcher(enum_matcher, &generator);
    tool.run(newFrontendActionFactory(&finder).get());
    return generator.aggregator;
}
