#pragma once

#include "neuron/neuron/neuron_model.hpp"
#include "neuron/synapse.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace neuron {

using NeuronIndex = std::unordered_map<std::string, std::shared_ptr<NeuronModel>>;

struct SynapseNetwork {
    std::vector<GradedChemicalSynapse> chemical_synapses;
    std::vector<GapJunction> gap_junctions;

    struct StepStats {
        double max_abs_chemical_current_pA = 0.0;
        double max_abs_gap_current_pA = 0.0;
    };

    StepStats apply(double dt_ms);
    void step(double dt_ms);
};

std::vector<GradedChemicalSynapse> load_chemical_synapses_csv(
    const std::string& csv_path,
    const NeuronIndex& neurons,
    std::size_t default_pre_compartment = 0,
    std::size_t default_post_compartment = 0);

std::vector<GapJunction> load_gap_junctions_csv(
    const std::string& csv_path,
    const NeuronIndex& neurons,
    std::size_t default_compartment_a = 0,
    std::size_t default_compartment_b = 0);

SynapseNetwork load_synapse_network_csv(
    const std::string& chemical_components_csv,
    const std::string& gap_junctions_csv,
    const NeuronIndex& neurons);

}  // namespace neuron
