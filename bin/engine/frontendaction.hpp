#pragma once

#include <llvm/ADT/StringRef.h>
#include <clang/Frontend/FrontendActions.h>

#include <memory>
#include <vector>
#include <string>

#include "data/aggregator.hpp"

namespace clang
{
    class CompilerInstance;
}

class XFrontendAction : public clang::ASTFrontendAction
{
    public:
        XFrontendAction(data::reflection_aggregator_t& aggregator);
        virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &compiler, llvm::StringRef inFile) override;

    private:
        data::reflection_aggregator_t& m_aggregator;
};