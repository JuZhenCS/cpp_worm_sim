#pragma once

#include "neuron/core/mechanism_config.hpp"

#include <string>

namespace brain {

struct RepresentativeNeuronTemplate {
    std::string cell_file;
    neuron::NeuronMechanismConfig mechanisms;
    double initial_voltage_mV = -60.0;
};

RepresentativeNeuronTemplate representative_neuron_template(
    const std::string& reference,
    const std::string& template_data_dir);

}  // namespace brain
