#include "data/aggregator.hpp"
#include "data/renderer.hpp"
#include "engine/utils.h"
#include "engine/ast_traversal_tool.hpp"

#include "fmt/ostream.h"

#include <iostream>
#include <filesystem>

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
        fmt::print(std::cerr, "Not enough parameters supplied. Usage is ./introspect_tool <input file> [<additional input files>] <header>\n");
        return 1;
    }

    std::string header(argv[argc-1]);

    std::string error;
    const auto compilation_database = clang::tooling::CompilationDatabase::loadFromDirectory(std::filesystem::current_path().native(),  error);
    if(!compilation_database) {
        const auto current_path = std::filesystem::current_path().string();
        fmt::print(std::cerr, "Compilation failed with error \"{}\". Maybe compilation database not found under {}\n", error, current_path);
    }

    const auto header_dir = std::filesystem::path(header).remove_filename();
    if(!std::filesystem::exists(header_dir)) {
        fmt::print(std::cerr, "Could not generate output. Directory {} requested for output does not exist.\n", header_dir.string());
        return 1;
    }
    
    clang::tooling::ClangTool tool(*compilation_database, std::vector<std::string>(argv+1, argv+argc-1));
    tool.appendArgumentsAdjuster(tsmp_argument_adjuster_t{header});
    tool.appendArgumentsAdjuster(clang::tooling::getClangSyntaxOnlyAdjuster());
    engine::ast_traversal_tool_t traversal_tool;
    traversal_tool.run(tool);

    fmt::print("Write to {}.\n", header);
    data::renderer_t renderer(header);
    renderer.render(traversal_tool.state());

    return 0;
}