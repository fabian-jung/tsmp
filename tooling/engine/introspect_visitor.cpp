#include "introspect_visitor.hpp"
#include <sstream>
using namespace clang;

introspect_visitor_t::introspect_visitor_t(ASTContext *Context, data::reflection_aggregator_t& aggregator) :
    m_context(Context),
    m_aggregator(aggregator)
{}

bool introspect_visitor_t::shouldVisitTemplateInstantiations() const {
    return true;
}

bool introspect_visitor_t::shouldVisitImplicitCode() const {
    return true;
}

void reflect_type(const CXXRecordDecl* decl, data::reflection_aggregator_t& aggregator) {
    if(decl == nullptr) return;
    llvm::outs() << "member analysis for " << decl->getNameAsString() << "\n";
    data::record_decl_t record_decl;
    record_decl.name = decl->getNameAsString();
    for(const auto& field : decl->fields()) {
        // const auto index = field->getFieldIndex();
        // auto name = field->getQualifiedNameAsString();
        const auto type = field->getType();
        const auto builtin = type.getTypePtr()->isBuiltinType();
        if(!builtin) {
            reflect_type(type->getAsCXXRecordDecl(), aggregator);
        }
        // llvm::outs() << "field:" << field->getNameAsString() << ":" << type.getAsString() << " is builtin " << builtin << "\n";
        auto name = field->getNameAsString();
        if(name.size() > 0) {
            record_decl.fields.emplace_back(data::field_decl_t{std::move(name)});
        }
    }
    for(const auto& method : decl->methods()) {
        auto name = method->getNameAsString();
        // llvm::outs() << "found method with name = " << name << "\n";
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

bool introspect_visitor_t::VisitClassTemplateSpecializationDecl(ClassTemplateSpecializationDecl *Declaration) {
    if(Declaration->getDeclName().getAsString() != "reflect") {
        return true;
    }
    llvm::outs() << "Found ClassTemplateSpecializationDecl decl " << Declaration->getQualifiedNameAsString() << " " << Declaration->getDeclName() << "\n";
    const auto args = Declaration->getTemplateArgs().asArray();
    for(auto i : args) {
        if(i.getKind() == TemplateArgument::ArgKind::Type) {
            const auto type = i.getAsType();
            const auto decl = type.getTypePtr()->getAsCXXRecordDecl();
            reflect_type(decl, m_aggregator);
        }
    }
    return true;
}       