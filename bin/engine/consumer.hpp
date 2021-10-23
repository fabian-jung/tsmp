#pragma once

#include <clang/AST/ASTContext.h>
#include <clang/AST/ASTConsumer.h>

#include "data/aggregator.hpp"
namespace clang
{
    class ASTContext;
}

class XConsumer : public clang::ASTConsumer
{
    public:

        explicit XConsumer(clang::ASTContext &context, data::reflection_aggregator_t& m_aggregator);
        virtual void HandleTranslationUnit(clang::ASTContext &context) override;

    private:
        data::reflection_aggregator_t& m_aggregator;
};