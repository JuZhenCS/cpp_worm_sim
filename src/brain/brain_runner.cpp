#include "brain/brain_runner.hpp"

#include <cmath>
#include <exception>
#include <stdexcept>

namespace brain {
namespace {

std::size_t simulation_steps(double tstop_ms, double dt_ms) {
    if (tstop_ms <= 0.0) {
        throw std::runtime_error("--tstop-ms must be positive");
    }
    if (dt_ms <= 0.0) {
        throw std::runtime_error("--dt-ms must be positive");
    }
    return static_cast<std::size_t>(std::ceil(tstop_ms / dt_ms));
}

}  // namespace

BrainRunnerResult run_brain(const BrainRunnerConfig& config) {
    BrainRunnerResult result;
    try {
        auto network = build_brain_network(config.network);
        const std::size_t steps = simulation_steps(config.tstop_ms, config.simulation.dt_ms);

        if (!config.diagnostics.enabled) {
            run_brain_steps(network, config.simulation.dt_ms, steps);
            return result;
        }

        const auto diagnostics_metadata = load_brain_diagnostics_metadata_csv(config.network.neuron_reference_csv);
        BrainDiagnosticsCollector diagnostics(network, config.diagnostics, diagnostics_metadata);
        for (std::size_t step = 0; step < steps; ++step) {
            const auto stats = step_brain_network(network, config.simulation.dt_ms);
            const double time_ms = static_cast<double>(step + 1) * config.simulation.dt_ms;
            diagnostics.observe_after_step(network, stats, time_ms);
        }
        result.diagnostics = diagnostics.finish(network, config.tstop_ms, config.simulation.dt_ms);
        return result;
    } catch (const std::exception& exc) {
        result.ok = false;
        result.error_message = exc.what();
        return result;
    }
}

}  // namespace brain

