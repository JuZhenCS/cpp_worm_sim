#include "neuron/core/mechanism_registry.hpp"

#include "neuron/core/mechanism_config.hpp"
#include "neuron/core/multi_compartment_neuron.hpp"

namespace neuron {

const std::vector<MechanismDescriptor>& all_mechanisms() {
    static const std::vector<MechanismDescriptor> mechanisms = {
        {"nca", "--enable-nca", &NeuronMechanismConfig::enable_nca,
         &MultiCompartmentNeuron::attach_nca_channels},
        {"irk", "--enable-irk", &NeuronMechanismConfig::enable_irk,
         &MultiCompartmentNeuron::attach_irk_channels},
        {"kqt3", "--enable-kqt3", &NeuronMechanismConfig::enable_kqt3,
         &MultiCompartmentNeuron::attach_kqt3_channels},
        {"egl2", "--enable-egl2", &NeuronMechanismConfig::enable_egl2,
         &MultiCompartmentNeuron::attach_egl2_channels},
        {"shk1", "--enable-shk1", &NeuronMechanismConfig::enable_shk1,
         &MultiCompartmentNeuron::attach_shk1_channels},
        {"kvs1", "--enable-kvs1", &NeuronMechanismConfig::enable_kvs1,
         &MultiCompartmentNeuron::attach_kvs1_channels},
        {"shl1", "--enable-shl1", &NeuronMechanismConfig::enable_shl1,
         &MultiCompartmentNeuron::attach_shl1_channels},
        {"egl36", "--enable-egl36", &NeuronMechanismConfig::enable_egl36,
         &MultiCompartmentNeuron::attach_egl36_channels},
        {"egl19", "--enable-egl19", &NeuronMechanismConfig::enable_egl19,
         &MultiCompartmentNeuron::attach_egl19_channels},
        {"cca1", "--enable-cca1", &NeuronMechanismConfig::enable_cca1,
         &MultiCompartmentNeuron::attach_cca1_channels},
        {"unc2", "--enable-unc2", &NeuronMechanismConfig::enable_unc2,
         &MultiCompartmentNeuron::attach_unc2_channels},
        {"calcium_internal", "--enable-calcium-internal",
         &NeuronMechanismConfig::enable_calcium_internal,
         &MultiCompartmentNeuron::enable_calcium_internal},
        {"kcnl", "--enable-kcnl", &NeuronMechanismConfig::enable_kcnl,
         &MultiCompartmentNeuron::attach_kcnl_channels},
        {"slo1_egl19", "--enable-slo1-egl19", &NeuronMechanismConfig::enable_slo1_egl19,
         &MultiCompartmentNeuron::attach_slo1_egl19_channels},
        {"slo1_unc2", "--enable-slo1-unc2", &NeuronMechanismConfig::enable_slo1_unc2,
         &MultiCompartmentNeuron::attach_slo1_unc2_channels},
        {"slo2_egl19", "--enable-slo2-egl19", &NeuronMechanismConfig::enable_slo2_egl19,
         &MultiCompartmentNeuron::attach_slo2_egl19_channels},
        {"slo2_unc2", "--enable-slo2-unc2", &NeuronMechanismConfig::enable_slo2_unc2,
         &MultiCompartmentNeuron::attach_slo2_unc2_channels},
    };

    return mechanisms;
}

}  // namespace neuron
