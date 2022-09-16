#include <algorithm>
#include <iterator>
#include <llvm/Support/CommandLine.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include "engine/utils.h"
#include "engine/frontendaction.hpp"
#include "data/aggregator.hpp"

#include <iostream>
#include <fmt/ostream.h>
#include <fmt/format.h>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

std::string add_include_flag(std::string path) {
    return "-I"+path;
}

int main(int argc, const char* argv[]) {
    if(argc<3) {
        fmt::print(std::cerr, "Not enought parameters supplied. Usage is ./introspect_tool <input> [<input args>] <output>\n");
        return 1;
    }
    std::string output_file(argv[argc-1]);

    fmt::print("running in: {}\n", std::filesystem::current_path().string());

    data::reflection_aggregator_t aggregator;
    
    for(auto &sourceFile : std::vector<const char*>(argv+1, argv+argc-1))
    {
        if(!utils::fileExists(sourceFile))
        {
            llvm::errs() << "File: " << sourceFile << " does not exist!\n";
            return -1;
        }
        std::cout << "Analysing AST of " << sourceFile << std::endl;

        auto sourcetxt = utils::getSourceCode(sourceFile);
        std::string error;
        const auto compilation_database = clang::tooling::CompilationDatabase::loadFromDirectory(std::filesystem::current_path().native(),  error);
        if(!compilation_database) {
            std::cerr << "compilation database not found under " << std::filesystem::current_path() << std::endl;
        }
        auto compileCommands = compilation_database->getCompileCommands(clang::tooling::getAbsolutePath(sourceFile));
        const auto builtinIncludePath = utils::getClangBuiltInIncludePath(output_file);

        std::vector<std::string> compileArgs = utils::getCompileArgs(compileCommands);
        // compileArgs.push_back("-I" + utils::getClangBuiltInIncludePath(argv[0]));
        compileArgs.push_back("-ferror-limit=0");
        compileArgs.push_back("-DTSMP_INTROSPECT_PASS");
        std::transform(builtinIncludePath.begin(), builtinIncludePath.end(), std::back_inserter(compileArgs), add_include_flag);
        fmt::print("With args: {}\n", fmt::join(compileArgs, " "));
    
        auto xfrontendAction = std::make_unique<XFrontendAction>(aggregator);
        utils::customRunToolOnCodeWithArgs(std::move(xfrontendAction), sourcetxt, compileArgs, sourceFile);
    }

    const auto dir = std::filesystem::path(output_file).remove_filename();
    if(!std::filesystem::exists(dir)) {
        if(!std::filesystem::create_directories(dir)) {
            std::cerr << "Could not generate directory requested for output.\n" << dir << " is not writeable." << std::endl;
            return 1;
        }
    }

    std::cout << "Write to " << output_file << std::endl;
    aggregator.generate(std::ofstream(output_file));

    return 0;
}