#pragma once

#include "neuron/neuron/cell.hpp"

#include <string>

namespace neuron {

Cell load_cell_csv(const std::string& path, const std::string& cell_name);

}  // namespace neuron

