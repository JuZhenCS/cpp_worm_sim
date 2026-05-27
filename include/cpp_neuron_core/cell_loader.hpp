#pragma once

#include "cpp_neuron_core/types.hpp"

#include <string>

namespace cpp_neuron {

Cell load_cell_csv(const std::string& path, const std::string& cell_name);

}  // namespace cpp_neuron

