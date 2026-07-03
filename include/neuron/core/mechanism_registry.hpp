#pragma once

#include "neuron/core/mechanism_config.hpp"

#include <vector>

namespace neuron {

class MultiCompartmentNeuron;

struct MechanismDescriptor {
    const char* name;
    const char* cli_flag;
    bool NeuronMechanismConfig::* enabled;
    void (MultiCompartmentNeuron::* install)();
};

const std::vector<MechanismDescriptor>& all_mechanisms();

}  // namespace neuron
