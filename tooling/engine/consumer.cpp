#include "consumer.hpp"
#include "introspect_visitor.hpp"

#include <iostream>

XConsumer::XConsumer(clang::ASTContext &context, data::reflection_aggregator_t& aggregator) :
    m_aggregator(aggregator)
{}

void XConsumer::HandleTranslationUnit(clang::ASTContext &context)
{
    introspect_visitor_t visitor(&context, m_aggregator);
    visitor.TraverseAST(context);
}