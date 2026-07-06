#pragma once

#include "brain/brain_network_builder.hpp"

#include <cstddef>

namespace brain {

struct BrainSimulationConfig {
    double dt_ms = 0.1;
};

neuron::SynapseNetwork::StepStats step_brain_network(BrainNetwork& network, double dt_ms);

void run_brain_steps(BrainNetwork& network, double dt_ms, std::size_t steps);

}  // namespace brain
