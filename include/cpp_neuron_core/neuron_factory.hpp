#pragma once

#include "cpp_neuron_core/neuron.hpp"
#include "cpp_neuron_core/neuron_model.hpp"

#include <memory>
#include <string>

namespace cpp_neuron {

struct NeuronChannelConfig {
    bool enable_nca = false;
    bool enable_irk = false;
    bool enable_kqt3 = false;
    bool enable_egl2 = false;
    bool enable_shk1 = false;
    bool enable_kvs1 = false;
    bool enable_shl1 = false;
    bool enable_egl36 = false;
    bool enable_egl19 = false;
    bool enable_cca1 = false;
    bool enable_unc2 = false;
    bool enable_calcium_internal = false;
    bool enable_kcnl = false;
    bool enable_slo1_egl19 = false;
    bool enable_slo1_unc2 = false;
    bool enable_slo2_egl19 = false;
    bool enable_slo2_unc2 = false;
};

struct NeuronBuildConfig {
    std::string name;
    std::string cell_file;
    NeuronChannelConfig channels;
};

std::unique_ptr<PassiveNeuron> create_passive_neuron(const NeuronBuildConfig& config);
std::unique_ptr<NeuronModel> create_neuron(const NeuronBuildConfig& config);

}  // namespace cpp_neuron
