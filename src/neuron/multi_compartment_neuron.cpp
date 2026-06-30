#include "neuron/neuron/multi_compartment_neuron.hpp"

#include "neuron/neuron/calcium_internal.hpp"
#include "neuron/neuron/channel_installer.hpp"
#include "neuron/neuron/diagnostics.hpp"

#include <algorithm>
#include <stdexcept>

namespace neuron {

namespace {

void update_calcium_internal(Cell& cell, double dt_ms) {
    CalciumInternalState calcium;
    for (auto& c : cell.compartments) {
        if (!c.calcium_internal_enabled || c.area_um2 <= 0.0) {
            continue;
        }
        double calcium_current_total_pA = 0.0;
        for (const auto& channel : c.channels) {
            const auto channel_name = channel->name();
            if (channel_name == "egl19" || channel_name == "cca1" || channel_name == "unc2") {
                calcium_current_total_pA += channel->current_pA(c.voltage_mV);
            }
        }
        const double calcium_current_density = calcium_current_total_pA / (c.area_um2 * 10.0);
        c.cai_uM_per_um2 = calcium.step(c.cai_uM_per_um2, calcium_current_density, c.voltage_mV, dt_ms);
    }
}

}  // namespace

MultiCompartmentNeuron::MultiCompartmentNeuron(Cell cell)
    : cell_(std::move(cell)),
      solver_(cell_.compartments.size()),
      injected_current_pA_(cell_.compartments.size(), 0.0),
      zero_conductance_nS_(cell_.compartments.size(), 0.0),
      zero_reversal_mV_(cell_.compartments.size(), 0.0) {
    if (cell_.compartments.empty()) {
        throw std::runtime_error("MultiCompartmentNeuron requires at least one compartment");
    }
}

void MultiCompartmentNeuron::set_all_voltages(double voltage_mV) {
    for (auto& compartment : cell_.compartments) {
        compartment.voltage_mV = voltage_mV;
    }
}

void MultiCompartmentNeuron::add_current_pA(std::size_t compartment_index, double current_pA) {
    injected_current_pA_.at(compartment_index) += current_pA;
}

void MultiCompartmentNeuron::clear_currents() {
    std::fill(injected_current_pA_.begin(), injected_current_pA_.end(), 0.0);
}

void MultiCompartmentNeuron::step(double dt_ms) {
    step(dt_ms, injected_current_pA_);
    clear_currents();
}

void MultiCompartmentNeuron::step(double dt_ms, const std::vector<double>& injected_current_pA) {
    step_with_conductance(dt_ms, injected_current_pA, zero_conductance_nS_, zero_reversal_mV_);
}

void MultiCompartmentNeuron::step_with_conductance(
    double dt_ms,
    const std::vector<double>& injected_current_pA,
    const std::vector<double>& extra_conductance_nS,
    const std::vector<double>& extra_reversal_mV) {
    solver_.advance_voltages(cell_, dt_ms, injected_current_pA, extra_conductance_nS, extra_reversal_mV);
    update_calcium_internal(cell_, dt_ms);
}

double MultiCompartmentNeuron::voltage_mV(std::size_t compartment_index) const {
    return cell_.compartments.at(compartment_index).voltage_mV;
}

double MultiCompartmentNeuron::soma_voltage_mV() const {
    if (cell_.compartments.size() == 1) {
        return cell_.compartments.front().voltage_mV;
    }
    return 0.5 * (cell_.compartments[0].voltage_mV + cell_.compartments[1].voltage_mV);
}

void MultiCompartmentNeuron::attach_nca_channels() {
    install_nca_channels(cell_);
}

void MultiCompartmentNeuron::attach_irk_channels() {
    install_irk_channels(cell_);
}

void MultiCompartmentNeuron::attach_kqt3_channels() {
    install_kqt3_channels(cell_);
}

void MultiCompartmentNeuron::attach_egl2_channels() {
    install_egl2_channels(cell_);
}

void MultiCompartmentNeuron::attach_shk1_channels() {
    install_shk1_channels(cell_);
}

void MultiCompartmentNeuron::attach_kvs1_channels() {
    install_kvs1_channels(cell_);
}

void MultiCompartmentNeuron::attach_shl1_channels() {
    install_shl1_channels(cell_);
}

void MultiCompartmentNeuron::attach_egl36_channels() {
    install_egl36_channels(cell_);
}

void MultiCompartmentNeuron::attach_egl19_channels() {
    install_egl19_channels(cell_);
}

void MultiCompartmentNeuron::attach_cca1_channels() {
    install_cca1_channels(cell_);
}

void MultiCompartmentNeuron::attach_unc2_channels() {
    install_unc2_channels(cell_);
}

void MultiCompartmentNeuron::attach_kcnl_channels() {
    install_kcnl_channels(cell_);
}

void MultiCompartmentNeuron::attach_slo1_egl19_channels() {
    install_slo1_egl19_channels(cell_);
}

void MultiCompartmentNeuron::attach_slo1_unc2_channels() {
    install_slo1_unc2_channels(cell_);
}

void MultiCompartmentNeuron::attach_slo2_egl19_channels() {
    install_slo2_egl19_channels(cell_);
}

void MultiCompartmentNeuron::attach_slo2_unc2_channels() {
    install_slo2_unc2_channels(cell_);
}

void MultiCompartmentNeuron::enable_calcium_internal() {
    neuron::enable_calcium_internal(cell_);
}

double MultiCompartmentNeuron::leak_current_pA(std::size_t compartment_index) const {
    return neuron::leak_current_pA(cell_, compartment_index);
}

double MultiCompartmentNeuron::axial_current_pA(std::size_t compartment_index) const {
    return neuron::axial_current_pA(cell_, compartment_index);
}

double MultiCompartmentNeuron::ion_current_pA(std::size_t compartment_index) const {
    return neuron::ion_current_pA(cell_, compartment_index);
}

double MultiCompartmentNeuron::channel_current_pA(std::size_t compartment_index, const std::string& channel_name) const {
    return neuron::channel_current_pA(cell_, compartment_index, channel_name);
}

}  // namespace neuron
