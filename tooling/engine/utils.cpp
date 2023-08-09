#include "utils.h"
#include <fstream>

namespace utils {

std::vector<std::string> getClangBuiltInIncludePath(const std::string& fullCallPath)
{
    auto currentPath = fullCallPath;
    currentPath.erase(currentPath.rfind("/"));

    std::string line;
    std::ifstream file(currentPath + "/builtInInclude.path");
    std::vector<std::string> result;
    while (std::getline(file, line)) {
        result.emplace_back(line);
    }

    return result;
}

}
