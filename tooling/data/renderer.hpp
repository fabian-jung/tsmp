#pragma once

#include "aggregator.hpp"

#include <fstream>
#include <string>

namespace data {

class renderer_t
{
public:
    renderer_t(std::string header);

    void render(const data::reflection_aggregator_t& aggregator);

private:
    std::ofstream header;
};

}