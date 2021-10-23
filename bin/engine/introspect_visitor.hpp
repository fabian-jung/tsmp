#include <clang/AST/RecursiveASTVisitor.h>
#include "data/aggregator.hpp"

class introspect_visitor_t : public clang::RecursiveASTVisitor<introspect_visitor_t> {
public:
    explicit introspect_visitor_t(clang::ASTContext *context, data::reflection_aggregator_t& aggregator);

    bool shouldVisitTemplateInstantiations() const;
    bool shouldVisitImplicitCode() const;

    bool VisitClassTemplateSpecializationDecl(clang::ClassTemplateSpecializationDecl *declaration);

    std::string generate_code() const;

private:
    data::reflection_aggregator_t& m_aggregator;

    clang::ASTContext *m_context;
};