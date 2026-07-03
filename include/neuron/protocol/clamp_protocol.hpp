#pragma once

#include "neuron/core/multi_compartment_neuron.hpp"
#include "neuron/recording/recording.hpp"

#include <vector>

namespace neuron {

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

std::vector<TracePoint> run_current_clamp(MultiCompartmentNeuron neuron, const IClampProtocol& protocol);
std::vector<TracePoint> run_seclamp(MultiCompartmentNeuron neuron, const SEClampProtocol& protocol);
ProtocolResult run_current_clamp_with_diagnostics(
    MultiCompartmentNeuron neuron,
    const IClampProtocol& protocol,
    std::size_t diagnostic_compartment = 0);
ProtocolResult run_seclamp_with_diagnostics(
    MultiCompartmentNeuron neuron,
    const SEClampProtocol& protocol,
    std::size_t diagnostic_compartment = 0);

}  // namespace neuron
