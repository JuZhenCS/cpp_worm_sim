#include "cpp_neuron_core/neuron_factory.hpp"

#include "cpp_neuron_core/cell_loader.hpp"

namespace cpp_neuron {

namespace {

void attach_channels(PassiveNeuron& neuron, const NeuronChannelConfig& channels) {
    if (channels.enable_nca) {
        neuron.attach_nca_channels();
    }
    if (channels.enable_irk) {
        neuron.attach_irk_channels();
    }
    if (channels.enable_kqt3) {
        neuron.attach_kqt3_channels();
    }
    if (channels.enable_egl2) {
        neuron.attach_egl2_channels();
    }
    if (channels.enable_shk1) {
        neuron.attach_shk1_channels();
    }
    if (channels.enable_kvs1) {
        neuron.attach_kvs1_channels();
    }
    if (channels.enable_shl1) {
        neuron.attach_shl1_channels();
    }
    if (channels.enable_egl36) {
        neuron.attach_egl36_channels();
    }
    if (channels.enable_egl19) {
        neuron.attach_egl19_channels();
    }
    if (channels.enable_cca1) {
        neuron.attach_cca1_channels();
    }
    if (channels.enable_unc2) {
        neuron.attach_unc2_channels();
    }
    if (channels.enable_calcium_internal) {
        neuron.enable_calcium_internal();
    }
    if (channels.enable_kcnl) {
        neuron.attach_kcnl_channels();
    }
    if (channels.enable_slo1_egl19) {
        neuron.attach_slo1_egl19_channels();
    }
    if (channels.enable_slo1_unc2) {
        neuron.attach_slo1_unc2_channels();
    }
    if (channels.enable_slo2_egl19) {
        neuron.attach_slo2_egl19_channels();
    }
    if (channels.enable_slo2_unc2) {
        neuron.attach_slo2_unc2_channels();
    }
}

}  // namespace

std::unique_ptr<PassiveNeuron> create_passive_neuron(const NeuronBuildConfig& config) {
    auto cell = load_cell_csv(config.cell_file, config.name);
    auto neuron = std::make_unique<PassiveNeuron>(std::move(cell));
    attach_channels(*neuron, config.channels);
    return neuron;
}

std::unique_ptr<NeuronModel> create_neuron(const NeuronBuildConfig& config) {
    return create_passive_neuron(config);
}

}  // namespace cpp_neuron
