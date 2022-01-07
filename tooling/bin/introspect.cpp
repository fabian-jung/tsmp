#include <llvm/Support/CommandLine.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include "engine/utils.h"
#include "engine/frontendaction.hpp"
#include "data/aggregator.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>

int main(int argc, const char* argv[]) {
    if(argc<3) {
        std::cerr << "Not enought parameters supplied. Usage is ./introspect_tool <input> [<input args>] <output>";
        return 1;
    }
    std::string output_file(argv[argc-1]);

    std::cout << "running in: " << std::filesystem::current_path() << std::endl;

    auto paramCout = argc - 1;
    for(auto &sourceFile : std::vector<const char*>(argv+1, argv+argc-1))
    {
        if(utils::fileExists(sourceFile) == false)
        {
            llvm::errs() << "File: " << sourceFile << " does not exist!\n";
            return -1;
        }

        auto sourcetxt = utils::getSourceCode(sourceFile);
        std::string error;
        const auto compilation_database = clang::tooling::CompilationDatabase::loadFromDirectory(std::filesystem::current_path().native(),  error);
        if(!compilation_database) {
            std::cerr << "compilation database not found under " << std::filesystem::current_path() << std::endl;
        }
        auto compileCommands = compilation_database->getCompileCommands(clang::tooling::getAbsolutePath(sourceFile));

        std::vector<std::string> compileArgs = utils::getCompileArgs(compileCommands);
        // compileArgs.push_back("-I" + utils::getClangBuiltInIncludePath(argv[0]));
        compileArgs.push_back("-ferror-limit=0");
        compileArgs.push_back("-DINTROSPECT_PASS"); 
        compileArgs.push_back("-I/usr/lib/gcc/x86_64-pc-linux-gnu/11.1.0/include"); // Hard coded path to gcc includes       
        compileArgs.push_back("-I/usr/local/include");
        compileArgs.push_back("-I/usr/lib/gcc/x86_64-pc-linux-gnu/11.1.0/include-fixed");
        compileArgs.push_back("-I/usr/include");    
        
        // for(auto &s : compileArgs)
        //     llvm::outs() << s << "\n";

        data::reflection_aggregator_t aggregator;
        auto xfrontendAction = std::make_unique<XFrontendAction>(aggregator);
        utils::customRunToolOnCodeWithArgs(std::move(xfrontendAction), sourcetxt, compileArgs, sourceFile);

        const auto dir = std::filesystem::path(output_file).remove_filename();
        if(!std::filesystem::exists(dir)) {
            if(!std::filesystem::create_directories(dir)) {
                std::cerr << "Could not generate directory requested for output.\n" << dir << " is not writeable." << std::endl;
                return 1;
            }
        }
        std::cout << "Write to " << output_file << std::endl;
        aggregator.generate(std::ofstream(output_file));
        // std::cout << "Generated Code:" << std::endl;
        // aggregator.generate(std::cout);
    }

    return 0;
}