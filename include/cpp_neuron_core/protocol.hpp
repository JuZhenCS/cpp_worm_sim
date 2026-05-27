#pragma once

#include "cpp_neuron_core/neuron.hpp"
#include "cpp_neuron_core/recorder.hpp"
#include "cpp_neuron_core/types.hpp"

#include <vector>

namespace cpp_neuron {

struct IClampProtocol {
    double dt_ms = 0.2;
    double tstop_ms = 20000.0;
    double delay_ms = 11000.0;
    double duration_ms = 5000.0;
    double amplitude_pA = 0.0;
    double v_init_mV = -60.0;
};

struct SEClampProtocol {
    double dt_ms = 0.2;
    double tstop_ms = 1600.0;
    double delay_ms = 1000.0;
    double duration_ms = 100.0;
    double v_hold_mV = -60.0;
    double v_command_mV = 0.0;
    double v_init_mV = -60.0;
    double series_resistance_MOhm = 29.0;
};

struct ProtocolResult {
    std::vector<TracePoint> trace;
    std::vector<ChannelDiagnosticPoint> diagnostics;
};

std::vector<TracePoint> run_current_clamp(PassiveNeuron neuron, const IClampProtocol& protocol);
std::vector<TracePoint> run_seclamp(PassiveNeuron neuron, const SEClampProtocol& protocol);
ProtocolResult run_current_clamp_with_diagnostics(
    PassiveNeuron neuron,
    const IClampProtocol& protocol,
    std::size_t diagnostic_compartment = 0);
ProtocolResult run_seclamp_with_diagnostics(
    PassiveNeuron neuron,
    const SEClampProtocol& protocol,
    std::size_t diagnostic_compartment = 0);

}  // namespace cpp_neuron
