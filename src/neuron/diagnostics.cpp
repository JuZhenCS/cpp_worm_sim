#include "neuron/core/diagnostics.hpp"

#include <stdexcept>

namespace neuron {

double leak_current_pA(const Cell& cell, std::size_t compartment_index) {
    const auto& c = cell.compartments.at(compartment_index);
    return c.leak_conductance_nS * (c.voltage_mV - c.leak_reversal_mV);
}

double axial_current_pA(const Cell& cell, std::size_t compartment_index) {
    const std::size_t n = cell.compartments.size();
    const auto& c = cell.compartments.at(compartment_index);
    double current = 0.0;
    if (c.parent_index >= 0) {
        const auto parent = static_cast<std::size_t>(c.parent_index);
        if (parent >= n) {
            throw std::runtime_error("Parent compartment index out of range");
        }
        current += c.axial_conductance_to_parent_nS * (c.voltage_mV - cell.compartments[parent].voltage_mV);
    }
    for (const auto& child : cell.compartments) {
        if (child.parent_index == static_cast<int>(compartment_index)) {
            current += child.axial_conductance_to_parent_nS * (c.voltage_mV - child.voltage_mV);
        }
    }
    return current;
}

double ion_current_pA(const Cell& cell, std::size_t compartment_index) {
    const auto& c = cell.compartments.at(compartment_index);
    double current = 0.0;
    for (const auto& channel : c.channels) {
        current += channel->current_pA(c.voltage_mV);
    }
    return current;
}

double channel_current_pA(const Cell& cell, std::size_t compartment_index, const std::string& channel_name) {
    const auto& c = cell.compartments.at(compartment_index);
    double current = 0.0;
    for (const auto& channel : c.channels) {
        if (channel->name() == channel_name) {
            current += channel->current_pA(c.voltage_mV);
        }
    }
    return current;
}

ChannelDiagnosticPoint sample_channel_diagnostics(
    const MultiCompartmentNeuron& neuron,
    double time_ms,
    std::size_t compartment_index) {
    const auto& cell = neuron.cell();
    const auto& compartment = cell.compartments.at(compartment_index);
    ChannelDiagnosticPoint point;
    point.time_ms = time_ms;
    point.voltage_mV = compartment.voltage_mV;
    point.leak_current_pA = leak_current_pA(cell, compartment_index);
    point.axial_current_pA = axial_current_pA(cell, compartment_index);
    point.ion_total_current_pA = ion_current_pA(cell, compartment_index);
    point.nca_current_pA = channel_current_pA(cell, compartment_index, "nca");
    point.nca_gbnca_nS = compartment.nca_conductance_nS;
    point.nca_ena_mV = 30.0;
    point.irk_current_pA = channel_current_pA(cell, compartment_index, "irk");
    point.irk_gbirk_nS = compartment.irk_conductance_nS;
    point.irk_ek_mV = -80.0;
    point.kqt3_current_pA = channel_current_pA(cell, compartment_index, "kqt3");
    point.kqt3_gbkqt3_nS = compartment.kqt3_conductance_nS;
    point.kqt3_ek_mV = -80.0;
    point.egl2_current_pA = channel_current_pA(cell, compartment_index, "egl2");
    point.egl2_gbegl2_nS = compartment.egl2_conductance_nS;
    point.egl2_ek_mV = -80.0;
    point.shk1_current_pA = channel_current_pA(cell, compartment_index, "shk1");
    point.shk1_gbshk1_nS = compartment.shk1_conductance_nS;
    point.shk1_ek_mV = -80.0;
    point.kvs1_current_pA = channel_current_pA(cell, compartment_index, "kvs1");
    point.kvs1_gbkvs1_nS = compartment.kvs1_conductance_nS;
    point.kvs1_ek_mV = -80.0;
    point.shl1_current_pA = channel_current_pA(cell, compartment_index, "shl1");
    point.shl1_gbshl1_nS = compartment.shl1_conductance_nS;
    point.shl1_ek_mV = -80.0;
    point.egl36_current_pA = channel_current_pA(cell, compartment_index, "egl36");
    point.egl36_gbegl36_nS = compartment.egl36_conductance_nS;
    point.egl36_ek_mV = -80.0;
    point.egl19_current_pA = channel_current_pA(cell, compartment_index, "egl19");
    point.egl19_gbegl19_nS = compartment.egl19_conductance_nS;
    point.egl19_eca_mV = 60.0;
    point.cca1_current_pA = channel_current_pA(cell, compartment_index, "cca1");
    point.cca1_gbcca1_nS = compartment.cca1_conductance_nS;
    point.cca1_eca_mV = 60.0;
    point.unc2_current_pA = channel_current_pA(cell, compartment_index, "unc2");
    point.unc2_gbunc2_nS = compartment.unc2_conductance_nS;
    point.unc2_eca_mV = 60.0;
    point.kcnl_current_pA = channel_current_pA(cell, compartment_index, "kcnl");
    point.kcnl_gbkcnl_nS = compartment.kcnl_conductance_nS;
    point.slo1_egl19_current_pA = channel_current_pA(cell, compartment_index, "slo1_egl19");
    point.slo1_egl19_gbslo1_nS = compartment.slo1_egl19_conductance_nS;
    point.slo1_unc2_current_pA = channel_current_pA(cell, compartment_index, "slo1_unc2");
    point.slo1_unc2_gbslo1_nS = compartment.slo1_unc2_conductance_nS;
    point.slo2_egl19_current_pA = channel_current_pA(cell, compartment_index, "slo2_egl19");
    point.slo2_egl19_gbslo2_nS = compartment.slo2_egl19_conductance_nS;
    point.slo2_unc2_current_pA = channel_current_pA(cell, compartment_index, "slo2_unc2");
    point.slo2_unc2_gbslo2_nS = compartment.slo2_unc2_conductance_nS;
    point.cai_uM_per_um2 = compartment.cai_uM_per_um2;
    return point;
}

}  // namespace neuron
