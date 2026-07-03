#include "neuron/core/mechanism_config.hpp"
#include "neuron/core/multi_compartment_neuron.hpp"
#include "neuron/core/neuron_factory.hpp"
#include "neuron/synapse_loader.hpp"
#include "neuron/synapse.hpp"

#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

namespace {

std::shared_ptr<neuron::MultiCompartmentNeuron> neuron(const std::string& name, double voltage_mV) {
    const std::string cell_file = std::string(CPP_WORM_SIM_SOURCE_DIR) + "/data/aiyl/AIYL_cell.csv";
    auto neuron = std::shared_ptr<neuron::MultiCompartmentNeuron>(
        neuron::create_multi_compartment_neuron({name, cell_file, neuron::NeuronMechanismConfig{}}));
    neuron->set_all_voltages(voltage_mV);
    return neuron;
}

void test_gate_stays_bounded_and_increases_with_pre_voltage() {
    auto pre_low = neuron("pre_low", -80.0);
    auto pre_high = neuron("pre_high", 20.0);
    auto post = neuron("post", -60.0);

    neuron::GradedChemicalSynapseConfig config;
    config.g_uS = 1.0e-4;

    neuron::GradedChemicalSynapse low(
        pre_low, post, 0, 0, neuron::ChemicalComponentType::Excitatory, config);
    neuron::GradedChemicalSynapse high(
        pre_high, post, 0, 0, neuron::ChemicalComponentType::Excitatory, config);

    low.update_and_current_pA(1.0);
    high.update_and_current_pA(1.0);

    assert(low.s() >= 0.0 && low.s() <= 1.0);
    assert(high.s() >= 0.0 && high.s() <= 1.0);
    assert(high.s() > low.s());
}

void test_excitatory_component_current_direction() {
    auto pre = neuron("pre", 20.0);
    auto post = neuron("post", -60.0);

    neuron::GradedChemicalSynapseConfig config;
    config.g_uS = 1.0e-4;
    config.e_rev_mV = 30.0;

    neuron::GradedChemicalSynapse synapse(
        pre, post, 0, 0, neuron::ChemicalComponentType::Excitatory, config);

    const double current = synapse.update_and_current_pA(1.0);
    assert(current > 0.0);
}

void test_inhibitory_component_current_direction() {
    auto pre = neuron("pre", 20.0);
    auto post = neuron("post", -40.0);

    neuron::GradedChemicalSynapseConfig config;
    config.g_uS = 1.0e-4;
    config.e_rev_mV = -70.0;

    neuron::GradedChemicalSynapse synapse(
        pre, post, 0, 0, neuron::ChemicalComponentType::Inhibitory, config);

    const double current = synapse.update_and_current_pA(1.0);
    assert(current < 0.0);
}

void test_micro_siemens_millivolt_to_picoamp_conversion() {
    auto pre = neuron("pre", 100.0);
    auto post = neuron("post", 0.0);

    neuron::GradedChemicalSynapseConfig config;
    config.g_uS = 1.0;
    config.tau_ms = 1.0;
    config.v_half_mV = -100.0;
    config.k_s_mV = 1.0;
    config.e_rev_mV = 1.0;

    neuron::GradedChemicalSynapse synapse(
        pre, post, 0, 0, neuron::ChemicalComponentType::Excitatory, config);

    const double current = synapse.update_and_current_pA(1.0);
    assert(std::abs(current - 1000.0) < 1.0e-6);
}

void test_gap_junction_conserves_current() {
    auto a = neuron("a", -40.0);
    auto b = neuron("b", -70.0);

    neuron::GapJunctionConfig config;
    config.g_uS = 1.0e-4;
    neuron::GapJunction gap(a, b, 0, 0, config);

    const double i_to_a = gap.current_to_a_pA();
    assert(i_to_a < 0.0);
    assert(std::abs(i_to_a - 1000.0 * config.g_uS * (b->voltage_mV(0) - a->voltage_mV(0))) < 1.0e-9);
}

void test_csv_loader_builds_synapse_network() {
    const auto dir = std::filesystem::temp_directory_path() / "cpp_worm_sim_synapse_loader_test";
    std::filesystem::create_directories(dir);
    const auto chemical_csv = dir / "chemical_components_v0.csv";
    const auto gap_csv = dir / "gap_junctions_v0.csv";

    {
        std::ofstream out(chemical_csv);
        out << "component_id,connection_id,pre,post,component_type,fenyves_polarity,anatomical_weight,"
               "normalized_weight,rho,g0_uS,g_uS,e_rev_mV,tau_ms,v_half_mV,k_s_mV,active_in_v0,evidence\n";
        out << "A__B__exc,A__B,A,B,exc,complex,2,0.25,0.5,0.00049,0.00006125,30,10,-20,5,True,test\n";
        out << "A__B__inh,A__B,A,B,inh,complex,2,0.25,0.5,0.0002,0.000025,-70,10,-20,5,True,test\n";
    }
    {
        std::ofstream out(gap_csv);
        out << "gap_id,cell_a,cell_b,anatomical_weight,normalized_weight,g0_uS,g_uS,active_in_v0,evidence\n";
        out << "A__B,A,B,1,0.1,0.0001,0.00001,True,test\n";
    }

    neuron::NeuronIndex neurons;
    neurons.emplace("A", neuron("A", 20.0));
    neurons.emplace("B", neuron("B", -60.0));

    auto network = neuron::load_synapse_network_csv(chemical_csv.string(), gap_csv.string(), neurons);
    assert(network.chemical_synapses.size() == 2);
    assert(network.gap_junctions.size() == 1);

    network.step(1.0);
    neurons["B"]->step(1.0);
    assert(std::isfinite(neurons["B"]->voltage_mV(0)));
}

void test_csv_loader_rejects_unknown_neuron() {
    const auto dir = std::filesystem::temp_directory_path() / "cpp_worm_sim_synapse_loader_test";
    std::filesystem::create_directories(dir);
    const auto chemical_csv = dir / "chemical_components_unknown_v0.csv";

    {
        std::ofstream out(chemical_csv);
        out << "component_id,connection_id,pre,post,component_type,fenyves_polarity,anatomical_weight,"
               "normalized_weight,rho,g0_uS,g_uS,e_rev_mV,tau_ms,v_half_mV,k_s_mV,active_in_v0,evidence\n";
        out << "A__Missing__exc,A__Missing,A,Missing,exc,+,1,1,1,0.00049,0.00049,30,10,-20,5,True,test\n";
    }

    neuron::NeuronIndex neurons;
    neurons.emplace("A", neuron("A", 20.0));

    bool threw = false;
    try {
        (void)neuron::load_chemical_synapses_csv(chemical_csv.string(), neurons);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    assert(threw);
}

}  // namespace

int main() {
    test_gate_stays_bounded_and_increases_with_pre_voltage();
    test_excitatory_component_current_direction();
    test_inhibitory_component_current_direction();
    test_micro_siemens_millivolt_to_picoamp_conversion();
    test_gap_junction_conserves_current();
    test_csv_loader_builds_synapse_network();
    test_csv_loader_rejects_unknown_neuron();

    std::cout << "synapse tests passed\n";
    return 0;
}
