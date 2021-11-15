
#include "frontendaction.hpp"

#include <clang/AST/ASTContext.h>
// #include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/CompilerInstance.h>

#include "consumer.hpp"

#include "introspect_visitor.hpp"

std::unique_ptr<clang::ASTConsumer> XFrontendAction::CreateASTConsumer(clang::CompilerInstance &compiler, llvm::StringRef inFile)
{
    return std::unique_ptr<clang::ASTConsumer>(new XConsumer(compiler.getASTContext(), m_aggregator));
}

XFrontendAction::XFrontendAction(data::reflection_aggregator_t& aggregator) :
    m_aggregator(aggregator)
{}