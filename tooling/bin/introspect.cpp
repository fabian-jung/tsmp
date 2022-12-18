#include <algorithm>
#include <clang/Basic/LLVM.h>
#include <clang/Tooling/ArgumentsAdjusters.h>
#include <clang/Tooling/Tooling.h>
#include <iterator>
#include <llvm/Support/CommandLine.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include "engine/utils.h"
#include "engine/code_generator.hpp"

#include <iostream>
#include <fmt/ostream.h>
#include <fmt/format.h>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#include <clang/Parse/ParseAST.h>
#include <clang/Parse/Parser.h>

struct tsmp_argument_adjuster_t {
    std::string output_file;
    clang::tooling::CommandLineArguments operator()(clang::tooling::CommandLineArguments args, clang::StringRef file) {
        args.push_back("-ferror-limit=0");
        args.push_back("-DTSMP_INTROSPECT_PASS");
        
        const auto builtinIncludePath = utils::getClangBuiltInIncludePath(output_file);
        std::transform(builtinIncludePath.begin(), builtinIncludePath.end(), std::back_inserter(args), add_include_flag);
        return args;
    };

    static std::string add_include_flag(std::string path) {
        return "-I"+path;
    }
};


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

    const auto dir = std::filesystem::path(output_file).remove_filename();
    if(!std::filesystem::exists(dir)) {
        fmt::print(std::cerr, "Could not generate output. Directory {} requested for output does not exist.\n", dir.string());
        return 1;
    }
    
    clang::tooling::ClangTool tool(*compilation_database, std::vector<std::string>(argv+1, argv+argc-1));
    tool.appendArgumentsAdjuster(tsmp_argument_adjuster_t{output_file});
    tool.appendArgumentsAdjuster(clang::tooling::getClangSyntaxOnlyAdjuster());
    const auto aggregator = generate_code_for_target(tool);
    

    fmt::print("Write to {}.\n ", output_file);
    aggregator.generate(output_file);

    return 0;
}