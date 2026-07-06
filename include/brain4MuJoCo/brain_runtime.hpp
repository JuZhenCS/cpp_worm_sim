#pragma once

#include "brain/brain_network_builder.hpp"
#include "brain/brain_simulation.hpp"
#include "brain4MuJoCo/muscle/muscle_output.hpp"

#include <array>
#include <cstdint>

namespace brain4MuJoCo {

struct BrainRuntimeConfig {
    brain::BrainNetworkConfig network;
    brain::BrainSimulationConfig simulation;
    MuscleOutputConfig muscle;
};

class BrainRuntime {
public:
    explicit BrainRuntime(BrainRuntimeConfig config);

    void reset();

    const std::array<float, kBodyMuscleCount>& simulate_steps(std::int32_t steps);

    const std::array<float, kBodyMuscleCount>& muscle_output() const;

private:
    BrainRuntimeConfig config_;
    brain::BrainNetwork network_;
    std::array<float, kBodyMuscleCount> muscle_output_{};
};

}  // namespace brain4MuJoCo