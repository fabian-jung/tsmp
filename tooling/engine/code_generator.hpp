#pragma once

#include "clang/Tooling/Tooling.h"
#include "data/aggregator.hpp"

data::reflection_aggregator_t generate_code_for_target(clang::tooling::ClangTool& tool);