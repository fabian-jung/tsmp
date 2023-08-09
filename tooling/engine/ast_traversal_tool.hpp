#pragma once
#include "data/aggregator.hpp"
#include "data/types.hpp"
#include "clang/Tooling/Tooling.h"
#include <clang/AST/DeclCXX.h>
#include <clang/Basic/IdentifierTable.h>

namespace engine {

class ast_traversal_tool_t
{
public:
    void run(clang::tooling::ClangTool& tool);
    const data::reflection_aggregator_t& state() const;

    const data::type_t* register_type(const clang::EnumDecl* decl);
    const data::type_t* register_type(const clang::CXXRecordDecl* record);

private:
    std::vector<data::template_argument_t> template_argument_analysis(const clang::CXXRecordDecl* decl);
    std::vector<data::field_decl_t> field_analysis(const clang::CXXRecordDecl* decl);
    std::vector<data::parameter_decl_t> parameter_analysis(const clang::CXXMethodDecl* method);
    std::vector<data::function_decl_t> method_analysis(const clang::CXXRecordDecl* decl, const std::string& decl_name);

    std::string get_nested_name_specifier(const clang::NestedNameSpecifier* specifier);
    std::string get_qualified_name(const clang::QualType& type);
    std::string get_name(const clang::Type* type);
    std::string get_name(const clang::IdentifierInfo* type);
    std::string get_name(const clang::TemplateArgument& argument);
    std::string get_name(const clang::TypeDecl* type);
    std::string get_name(const clang::CXXRecordDecl* type);

    std::string get_namespace(const clang::DeclContext* context, bool qualified = false);

    const data::type_t* register_type(const clang::Type* type);
    const data::type_t* register_type(const clang::QualType& qtype);

    data::reflection_aggregator_t aggregator;
    std::map<const void*, const data::type_t*> visited_nodes;
};

}