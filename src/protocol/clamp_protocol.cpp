#include "neuron/protocol/clamp_protocol.hpp"

#include "neuron/core/diagnostics.hpp"

#include <cmath>
#include <utility>

namespace neuron {

namespace {

bool in_window(double t_ms, double start_ms, double duration_ms) {
    return t_ms >= start_ms && t_ms < start_ms + duration_ms;
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
        result.diagnostics.push_back(sample_channel_diagnostics(neuron, sample_time, diagnostic_compartment));
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
        result.diagnostics.push_back(sample_channel_diagnostics(neuron, sample_time, diagnostic_compartment));
    }
    return result;
}

}  // namespace neuron
