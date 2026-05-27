#pragma once

#include "cpp_neuron_core/types.hpp"

#include <string>
#include <vector>

namespace cpp_neuron {

class PassiveNeuron {
public:
    explicit PassiveNeuron(Cell cell);

    void set_all_voltages(double voltage_mV);
    void step(double dt_ms, const std::vector<double>& injected_current_pA);
    void step_with_conductance(
        double dt_ms,
        const std::vector<double>& injected_current_pA,
        const std::vector<double>& extra_conductance_nS,
        const std::vector<double>& extra_reversal_mV);

    const Cell& cell() const { return cell_; }
    Cell& cell() { return cell_; }

    double soma_voltage_mV() const;
    std::size_t size() const { return cell_.compartments.size(); }
    void attach_nca_channels();
    void attach_irk_channels();
    void attach_kqt3_channels();
    void attach_egl2_channels();
    void attach_shk1_channels();
    void attach_kvs1_channels();
    void attach_shl1_channels();
    void attach_egl36_channels();
    void attach_egl19_channels();
    void attach_cca1_channels();
    void attach_unc2_channels();
    void attach_kcnl_channels();
    void attach_slo1_egl19_channels();
    void attach_slo1_unc2_channels();
    void attach_slo2_egl19_channels();
    void attach_slo2_unc2_channels();
    void enable_calcium_internal();

    double leak_current_pA(std::size_t compartment_index) const;
    double axial_current_pA(std::size_t compartment_index) const;
    double ion_current_pA(std::size_t compartment_index) const;
    double channel_current_pA(std::size_t compartment_index, const std::string& channel_name) const;

private:
    Cell cell_;
};

}  // namespace cpp_neuron
