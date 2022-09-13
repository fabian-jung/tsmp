#include "introspect_visitor.hpp"
#include "data/types.hpp"
#include "clang/AST/Decl.h"
#include <iostream>
#include <sstream>
#include <unordered_set>

using namespace clang;

introspect_visitor_t::introspect_visitor_t(ASTContext *Context, data::reflection_aggregator_t& aggregator) :
    m_aggregator(aggregator),
    m_context(Context)
{}

bool introspect_visitor_t::shouldVisitTemplateInstantiations() const {
    return true;
}

bool introspect_visitor_t::shouldVisitImplicitCode() const {
    return true;
}

void reflect_enum(const EnumDecl* decl, data::reflection_aggregator_t& aggregator) {

    llvm::outs() << "enum analysis for" << decl->getDeclName() << "  yields values:\n";
    data::enum_decl_t enum_decl;
    for(auto enumValue : decl->enumerators()) {
        llvm::outs() << "    " << enumValue->getDeclName() << "\n";
        enum_decl.values.emplace_back(enumValue->getDeclName().getAsString());
    }
    aggregator.add_enum_decl(enum_decl);
}

void reflect_type(const CXXRecordDecl* decl, data::reflection_aggregator_t& aggregator, std::unordered_set<const CXXRecordDecl*> recursion_guard) {
    if(decl == nullptr) return;
    if(recursion_guard.find(decl) != recursion_guard.end())
        return;

    recursion_guard.insert(decl);

    llvm::outs() << "member analysis for " << decl->getNameAsString() << "\n";
    data::record_decl_t record_decl;
    record_decl.name = decl->getNameAsString();
    for(const auto& field : decl->fields()) {
        // const auto index = field->getFieldIndex();
        // auto name = field->getQualifiedNameAsString();
        const auto access = field->getAccess();
        const auto type = field->getType();
        const auto builtin = type.getTypePtr()->isBuiltinType();
        const auto cxx_type = type->getAsCXXRecordDecl();
        const auto cxx_pointee_type = type->getPointeeCXXRecordDecl();
        auto name = field->getNameAsString();
        if(type->isEnumeralType()) {
            reflect_enum(dynamic_cast<const EnumDecl*>(type->getAsTagDecl()), aggregator);
        }
        if(!builtin) {
            if(cxx_type)
            {
                reflect_type(cxx_type, aggregator, recursion_guard);
            }
            else
            {
                reflect_type(cxx_pointee_type, aggregator, recursion_guard);
            }
        } else {
            aggregator.add_trivial_type(type.getAsString());
        }
        // llvm::outs() << "field:" << field->getNameAsString() << ":" << type.getAsString() << " is builtin " << builtin << "\n";
        if(access != AS_public) {
            llvm::outs() << "ignore private " << record_decl.name << "::" << name << "\n";
            continue;
        }
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

void proxy_type(const CXXRecordDecl* decl, data::reflection_aggregator_t& aggregator) {
    reflect_type(decl, aggregator, {});
}

bool introspect_visitor_t::VisitClassTemplateSpecializationDecl(ClassTemplateSpecializationDecl *Declaration) {
    // llvm::outs() << "Found ClassTemplateSpecializationDecl decl " << Declaration->getQualifiedNameAsString() << " " << Declaration->getDeclName() << "\n";
    if(Declaration->getDeclName().getAsString() == "reflect") {
        llvm::outs() << "Found ClassTemplateSpecializationDecl decl " << Declaration->getQualifiedNameAsString() << " " << Declaration->getDeclName() << "\n";
        const auto args = Declaration->getTemplateArgs().asArray();
        for(auto i : args) {
            if(i.getKind() == TemplateArgument::ArgKind::Type) {
                const auto type = i.getAsType();
                if(type->isBuiltinType()) {
                    m_aggregator.add_trivial_type(type.getAsString());
                } else {
                    const auto cxx_type = type->getAsCXXRecordDecl();
                    const auto cxx_pointee_type = type->getPointeeCXXRecordDecl();

                    if(cxx_type)
                    {
                        reflect_type(cxx_type, m_aggregator, {});
                    }
                    else
                    {
                        reflect_type(cxx_pointee_type, m_aggregator, {});
                    }
                }
            }
        }
    }

    if(Declaration->getDeclName().getAsString() == "proxy") {
        llvm::outs() << "Found ClassTemplateSpecializationDecl decl " << Declaration->getQualifiedNameAsString() << " " << Declaration->getDeclName() << "\n";
        const auto args = Declaration->getTemplateArgs().asArray();
        if(args.size() >= 1) {
            const auto type = args.front().getAsType();
            if(!type->isBuiltinType()) {
                const auto decl = type.getTypePtr()->getAsCXXRecordDecl();

                const auto cxx_type = type->getAsCXXRecordDecl();
                const auto cxx_pointee_type = type->getPointeeCXXRecordDecl();

                if(cxx_type)
                {
                    proxy_type(cxx_type, m_aggregator);
                }
                else
                {
                    proxy_type(cxx_pointee_type, m_aggregator);
                }
            } else {
                std::cerr << "Built in types can not be proxied." << std::endl;
            }
        }
    }

    if(Declaration->getDeclName().getAsString() == "enum_value_adapter") {
        llvm::outs() << "Found ClassTemplateSpecializationDecl decl " << Declaration->getQualifiedNameAsString() << " " << Declaration->getDeclName() << "\n";
        const auto args = Declaration->getTemplateArgs().asArray();
        if(args.size() >= 1) {
            const auto type = args.front().getAsType();
            const auto* enumDecl = dynamic_cast<EnumDecl*>(type->getAsTagDecl());
            if(type->isEnumeralType() && enumDecl != nullptr) {
                reflect_enum(enumDecl, m_aggregator);
            } else {
                std::cerr << "Parameter of enum_value_adapter is not a enumeral type." << std::endl;
            }
        }
    }

    return true;
}