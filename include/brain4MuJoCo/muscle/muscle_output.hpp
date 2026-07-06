#pragma once

#include "brain/brain_network_builder.hpp"

#include <array>
#include <cstddef>
#include <string>
#include <vector>

namespace brain4MuJoCo {

constexpr std::size_t kBodyMuscleCount = 96;

struct MuscleProjectionEdge {
    std::string pre_neuron;
    std::size_t muscle_index = 0;
    double weight = 0.0;
    int sign = 1;
};

struct MuscleOutputConfig {
    double v_rest_mV = -60.0;
    double v_scale_mV = 40.0;
    std::string projection_csv;
    std::vector<MuscleProjectionEdge> projection_edges;
};

std::vector<MuscleProjectionEdge> load_muscle_projection_csv(const std::string& projection_csv);

void compute_muscle_output(
    const brain::BrainNetwork& network,
    const MuscleOutputConfig& config,
    std::array<float, kBodyMuscleCount>& output);

}  // namespace brain4MuJoCo