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
    fmt::print("called ./introspect {}\n", fmt::join(std::vector<const char*>{argv, argv+argc}, " "));
    if(argc<3) {
        fmt::print(std::cerr, "Not enought parameters supplied. Usage is ./introspect_tool <input file> [<additional input files>] <output file>\n");
        return 1;
    }
    std::string output_file(argv[argc-1]);

    std::string error;
    const auto compilation_database = clang::tooling::CompilationDatabase::loadFromDirectory(std::filesystem::current_path().native(),  error);
    if(!compilation_database) {
        const auto current_path = std::filesystem::current_path().string();
        fmt::print(std::cerr, "Compilation failed with error \"{}\". Maybe compilation database not found under {}\n", error, current_path);
    }

    data::reflection_aggregator_t aggregator;

    for(const auto &source_file : std::vector<const char*>(argv+1, argv+argc-1))
    {
        if(!utils::fileExists(source_file))
        {
            llvm::errs() << "File: " << source_file << " does not exist!\n";
            return -1;
        }
        fmt::print("Analyzing AST of {}\n", source_file );
        const auto compile_commands = compilation_database->getCompileCommands(clang::tooling::getAbsolutePath(source_file));
        const auto builtin_include_path = utils::getClangBuiltInIncludePath(output_file);
        const auto source_code_contents = utils::getSourceCode(source_file);
        
        for(const auto& compileCommand : compile_commands) {
            std::vector<std::string> compileArgs = utils::getCompileArgs(compileCommand);
            compileArgs.push_back("-ferror-limit=0");
            compileArgs.push_back("-DTSMP_INTROSPECT_PASS");
            std::transform(builtin_include_path.begin(), builtin_include_path.end(), std::back_inserter(compileArgs), add_include_flag);
            fmt::print("cxxflags: {}\n", fmt::join(compileArgs, " "));
        
            auto xfrontendAction = std::make_unique<XFrontendAction>(aggregator);
            utils::customRunToolOnCodeWithArgs(std::move(xfrontendAction), source_code_contents, compileArgs, source_file);
        }
    }

    return aggregator.generate(output_file);
}