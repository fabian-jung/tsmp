#include "code_generator.hpp"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Tooling/Tooling.h"
#include <clang/AST/Decl.h>
#include <clang/AST/DeclTemplate.h>
#include <llvm/Support/raw_ostream.h>
#include "data/aggregator.hpp"
// #include "clang/Frontend/FrontendActions.h"
// #include <clang/AST/DeclTemplate.h>
// #include <clang/AST/PrettyPrinter.h>
// #include <clang/AST/Stmt.h>
// #include <clang/Basic/LangOptions.h>
// #include <iostream>
// #include <llvm/Support/raw_ostream.h>

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

void reflect_type(const CXXRecordDecl* decl, data::reflection_aggregator_t& aggregator) {
    if(decl == nullptr) return;
    fmt::print("member analysis for {}\n", decl->getNameAsString());
    data::record_decl_t record_decl;
    record_decl.name = decl->getNameAsString();
    for(const auto& field : decl->fields()) {
        // const auto index = field->getFieldIndex();
        // auto name = field->getQualifiedNameAsString();
        const auto access = field->getAccess();
        const auto type = field->getType();
        const auto builtin = type.getTypePtr()->isBuiltinType();
        auto name = field->getNameAsString();
        if(type->isEnumeralType()) {
            reflect_enum(dynamic_cast<const EnumDecl*>(type->getAsTagDecl()), aggregator);
        }
        if(!builtin) {
            reflect_type(type->getAsCXXRecordDecl(), aggregator);
        } else {
            aggregator.add_trivial_type(type.getCanonicalType().getAsString());
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
                    fmt::print("arg is not a type, skip template argument.\n");
                    continue;
                }
                if(id=="type") {
                    reflect_type(arg.getAsType()->getAsCXXRecordDecl(), aggregator);
                } else if(id=="enum") {
                    reflect_enum(dynamic_cast<EnumDecl*>(arg.getAsType()->getAsTagDecl()), aggregator);
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
