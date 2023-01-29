#pragma once

#include "aggregator.hpp"

#include <string>
#include <fstream>

namespace data {

class renderer_t {
public:
    renderer_t(std::string output_file);

    void render(const data::reflection_aggregator_t& aggregator);

private:
    std::ofstream output_file;
};

}