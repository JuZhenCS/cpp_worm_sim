#include "brain4MuJoCo/brain_runtime.hpp"

#include <stdexcept>
#include <utility>

namespace brain4MuJoCo {

BrainRuntime::BrainRuntime(BrainRuntimeConfig config) : config_(std::move(config)), network_(brain::build_brain_network(config_.network)) {
    if (config_.muscle.projection_edges.empty()) {
        if (config_.muscle.projection_csv.empty()) {
            throw std::runtime_error("Muscle projection CSV path is empty");
        }
        config_.muscle.projection_edges = load_muscle_projection_csv(config_.muscle.projection_csv);
    }
    compute_muscle_output(network_, config_.muscle, muscle_output_);
}

void BrainRuntime::reset() {
    network_ = brain::build_brain_network(config_.network);
    compute_muscle_output(network_, config_.muscle, muscle_output_);
}

const std::array<float, kBodyMuscleCount>& BrainRuntime::simulate_steps(std::int32_t steps) {
    if (steps < 0) {
        throw std::runtime_error("simulate step count must be non-negative");
    }
    for (std::int32_t step = 0; step < steps; ++step) {
        brain::step_brain_network(network_, config_.simulation.dt_ms);
    }
    compute_muscle_output(network_, config_.muscle, muscle_output_);
    return muscle_output_;
}

const std::array<float, kBodyMuscleCount>& BrainRuntime::muscle_output() const {
    return muscle_output_;
}

}  // namespace brain4MuJoCo