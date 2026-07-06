#pragma once

#include "brain/brain_network_builder.hpp"
#include "brain/brain_simulation.hpp"
#include "brain/diagnostics/brain_diagnostics_observe.hpp"

#include <optional>
#include <string>

namespace brain {

struct BrainRunnerConfig {
    BrainNetworkConfig network;
    BrainSimulationConfig simulation;
    BrainDiagnosticsConfig diagnostics;
    double tstop_ms = 10.0;
};

struct BrainRunnerResult {
    bool ok = true;
    std::optional<BrainDiagnosticsResult> diagnostics;
    std::string error_message;
};

BrainRunnerResult run_brain(const BrainRunnerConfig& config);

}  // namespace brain
