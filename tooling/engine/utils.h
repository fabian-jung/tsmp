#pragma once

#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>

#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>

#include <string>
#include <vector>

namespace utils {
std::vector<std::string> getClangBuiltInIncludePath(const std::string& fullCallPath);
}