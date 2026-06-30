#pragma once

#include "neuron/neuron/multi_compartment_neuron.hpp"
#include "neuron/recording/recording.hpp"

#include <cstddef>
#include <string>

namespace neuron {

double leak_current_pA(const Cell& cell, std::size_t compartment_index);
double axial_current_pA(const Cell& cell, std::size_t compartment_index);
double ion_current_pA(const Cell& cell, std::size_t compartment_index);
double channel_current_pA(const Cell& cell, std::size_t compartment_index, const std::string& channel_name);

ChannelDiagnosticPoint sample_channel_diagnostics(
    const MultiCompartmentNeuron& neuron,
    double time_ms,
    std::size_t compartment_index);

}  // namespace neuron
