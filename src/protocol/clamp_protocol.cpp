#include "cpp_neuron_core/protocol/clamp_protocol.hpp"

#include <cmath>
#include <utility>

namespace cpp_neuron {

namespace {

bool in_window(double t_ms, double start_ms, double duration_ms) {
    return t_ms >= start_ms && t_ms < start_ms + duration_ms;
}

ChannelDiagnosticPoint sample_diagnostics(
    const MultiCompartmentNeuron& neuron,
    double time_ms,
    std::size_t compartment_index) {
    const auto& compartment = neuron.cell().compartments.at(compartment_index);
    ChannelDiagnosticPoint point;
    point.time_ms = time_ms;
    point.voltage_mV = compartment.voltage_mV;
    point.leak_current_pA = neuron.leak_current_pA(compartment_index);
    point.axial_current_pA = neuron.axial_current_pA(compartment_index);
    point.ion_total_current_pA = neuron.ion_current_pA(compartment_index);
    point.nca_current_pA = neuron.channel_current_pA(compartment_index, "nca");
    point.nca_gbnca_nS = compartment.nca_conductance_nS;
    point.nca_ena_mV = 30.0;
    point.irk_current_pA = neuron.channel_current_pA(compartment_index, "irk");
    point.irk_gbirk_nS = compartment.irk_conductance_nS;
    point.irk_ek_mV = -80.0;
    point.kqt3_current_pA = neuron.channel_current_pA(compartment_index, "kqt3");
    point.kqt3_gbkqt3_nS = compartment.kqt3_conductance_nS;
    point.kqt3_ek_mV = -80.0;
    point.egl2_current_pA = neuron.channel_current_pA(compartment_index, "egl2");
    point.egl2_gbegl2_nS = compartment.egl2_conductance_nS;
    point.egl2_ek_mV = -80.0;
    point.shk1_current_pA = neuron.channel_current_pA(compartment_index, "shk1");
    point.shk1_gbshk1_nS = compartment.shk1_conductance_nS;
    point.shk1_ek_mV = -80.0;
    point.kvs1_current_pA = neuron.channel_current_pA(compartment_index, "kvs1");
    point.kvs1_gbkvs1_nS = compartment.kvs1_conductance_nS;
    point.kvs1_ek_mV = -80.0;
    point.shl1_current_pA = neuron.channel_current_pA(compartment_index, "shl1");
    point.shl1_gbshl1_nS = compartment.shl1_conductance_nS;
    point.shl1_ek_mV = -80.0;
    point.egl36_current_pA = neuron.channel_current_pA(compartment_index, "egl36");
    point.egl36_gbegl36_nS = compartment.egl36_conductance_nS;
    point.egl36_ek_mV = -80.0;
    point.egl19_current_pA = neuron.channel_current_pA(compartment_index, "egl19");
    point.egl19_gbegl19_nS = compartment.egl19_conductance_nS;
    point.egl19_eca_mV = 60.0;
    point.cca1_current_pA = neuron.channel_current_pA(compartment_index, "cca1");
    point.cca1_gbcca1_nS = compartment.cca1_conductance_nS;
    point.cca1_eca_mV = 60.0;
    point.unc2_current_pA = neuron.channel_current_pA(compartment_index, "unc2");
    point.unc2_gbunc2_nS = compartment.unc2_conductance_nS;
    point.unc2_eca_mV = 60.0;
    point.kcnl_current_pA = neuron.channel_current_pA(compartment_index, "kcnl");
    point.kcnl_gbkcnl_nS = compartment.kcnl_conductance_nS;
    point.slo1_egl19_current_pA = neuron.channel_current_pA(compartment_index, "slo1_egl19");
    point.slo1_egl19_gbslo1_nS = compartment.slo1_egl19_conductance_nS;
    point.slo1_unc2_current_pA = neuron.channel_current_pA(compartment_index, "slo1_unc2");
    point.slo1_unc2_gbslo1_nS = compartment.slo1_unc2_conductance_nS;
    point.slo2_egl19_current_pA = neuron.channel_current_pA(compartment_index, "slo2_egl19");
    point.slo2_egl19_gbslo2_nS = compartment.slo2_egl19_conductance_nS;
    point.slo2_unc2_current_pA = neuron.channel_current_pA(compartment_index, "slo2_unc2");
    point.slo2_unc2_gbslo2_nS = compartment.slo2_unc2_conductance_nS;
    point.cai_uM_per_um2 = compartment.cai_uM_per_um2;
    return point;
}

}  // namespace

std::vector<TracePoint> run_current_clamp(MultiCompartmentNeuron neuron, const IClampProtocol& protocol) {
    return run_current_clamp_with_diagnostics(std::move(neuron), protocol).trace;
}

ProtocolResult run_current_clamp_with_diagnostics(
    MultiCompartmentNeuron neuron,
    const IClampProtocol& protocol,
    std::size_t diagnostic_compartment) {
    neuron.set_all_voltages(protocol.v_init_mV);
    const int steps = static_cast<int>(std::ceil(protocol.tstop_ms / protocol.dt_ms));
    ProtocolResult result;
    result.trace.reserve(static_cast<std::size_t>(steps));
    result.diagnostics.reserve(static_cast<std::size_t>(steps));

    for (int step = 0; step < steps; ++step) {
        const double t_ms = step * protocol.dt_ms;
        const double current_pA = in_window(t_ms, protocol.delay_ms, protocol.duration_ms)
                                      ? protocol.amplitude_pA
                                      : 0.0;
        std::vector<double> injected(neuron.size(), 0.0);
        if (neuron.size() >= 2) {
            injected[0] = 0.5 * current_pA;
            injected[1] = 0.5 * current_pA;
        } else {
            injected[0] = current_pA;
        }

        neuron.step(protocol.dt_ms, injected);
        const double sample_time = t_ms + protocol.dt_ms;
        result.trace.push_back({sample_time, neuron.soma_voltage_mV(), current_pA, 0.0});
        result.diagnostics.push_back(sample_diagnostics(neuron, sample_time, diagnostic_compartment));
    }
    return result;
}

std::vector<TracePoint> run_seclamp(MultiCompartmentNeuron neuron, const SEClampProtocol& protocol) {
    return run_seclamp_with_diagnostics(std::move(neuron), protocol).trace;
}

ProtocolResult run_seclamp_with_diagnostics(
    MultiCompartmentNeuron neuron,
    const SEClampProtocol& protocol,
    std::size_t diagnostic_compartment) {
    neuron.set_all_voltages(protocol.v_init_mV);
    const int steps = static_cast<int>(std::ceil(protocol.tstop_ms / protocol.dt_ms));
    const double g_total_nS = 1000.0 / protocol.series_resistance_MOhm;
    ProtocolResult result;
    result.trace.reserve(static_cast<std::size_t>(steps));
    result.diagnostics.reserve(static_cast<std::size_t>(steps));

    for (int step = 0; step < steps; ++step) {
        const double t_ms = step * protocol.dt_ms;
        const double v_cmd_mV = in_window(t_ms, protocol.delay_ms, protocol.duration_ms)
                                    ? protocol.v_command_mV
                                    : protocol.v_hold_mV;
        std::vector<double> injected(neuron.size(), 0.0);
        std::vector<double> clamp_g(neuron.size(), 0.0);
        std::vector<double> clamp_e(neuron.size(), v_cmd_mV);
        if (neuron.size() >= 2) {
            clamp_g[0] = 0.5 * g_total_nS;
            clamp_g[1] = 0.5 * g_total_nS;
        } else {
            clamp_g[0] = g_total_nS;
        }

        std::vector<double> previous_voltage;
        previous_voltage.reserve(neuron.size());
        for (const auto& compartment : neuron.cell().compartments) {
            previous_voltage.push_back(compartment.voltage_mV);
        }
        neuron.step_with_conductance(protocol.dt_ms, injected, clamp_g, clamp_e);
        double electrode_current_pA = 0.0;
        double capacitive_current_pA = 0.0;
        const auto& compartments = neuron.cell().compartments;
        for (std::size_t i = 0; i < compartments.size(); ++i) {
            electrode_current_pA += clamp_g[i] * (v_cmd_mV - compartments[i].voltage_mV);
            if (clamp_g[i] > 0.0) {
                capacitive_current_pA +=
                    compartments[i].capacitance_pF * (compartments[i].voltage_mV - previous_voltage[i]) / protocol.dt_ms;
            }
        }
        const double comparable_current_pA = electrode_current_pA - capacitive_current_pA;
        const double sample_time = t_ms + protocol.dt_ms;
        result.trace.push_back({sample_time, neuron.soma_voltage_mV(), v_cmd_mV, comparable_current_pA});
        result.diagnostics.push_back(sample_diagnostics(neuron, sample_time, diagnostic_compartment));
    }
    return result;
}

}  // namespace cpp_neuron
