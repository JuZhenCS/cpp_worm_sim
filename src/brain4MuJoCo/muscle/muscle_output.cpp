#include "brain4MuJoCo/muscle/muscle_output.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <stdexcept>
#include <utility>
#include <unordered_map>

namespace brain4MuJoCo {
namespace {

std::vector<std::string> split_csv_line(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool in_quotes = false;
    for (char ch : line) {
        if (ch == '"') {
            in_quotes = !in_quotes;
        } else if (ch == ',' && !in_quotes) {
            fields.push_back(field);
            field.clear();
        } else {
            field.push_back(ch);
        }
    }
    fields.push_back(field);
    return fields;
}

std::unordered_map<std::string, std::size_t> header_index(const std::vector<std::string>& header) {
    std::unordered_map<std::string, std::size_t> index;
    for (std::size_t i = 0; i < header.size(); ++i) {
        index.emplace(header[i], i);
    }
    return index;
}

std::string field(
    const std::vector<std::string>& row,
    const std::unordered_map<std::string, std::size_t>& header,
    const std::string& name) {
    const auto found = header.find(name);
    if (found == header.end() || found->second >= row.size()) {
        throw std::runtime_error("Missing muscle projection CSV field: " + name);
    }
    return row[found->second];
}

double clamp01(double value) {
    if (!std::isfinite(value)) {
        return 0.0;
    }
    return std::max(0.0, std::min(1.0, value));
}

}  // namespace

std::vector<MuscleProjectionEdge> load_muscle_projection_csv(const std::string& projection_csv) {
    std::ifstream input(projection_csv);
    if (!input) {
        throw std::runtime_error("Could not open muscle projection CSV: " + projection_csv);
    }

    std::string line;
    if (!std::getline(input, line)) {
        throw std::runtime_error("Empty muscle projection CSV: " + projection_csv);
    }
    const auto header = header_index(split_csv_line(line));

    std::vector<MuscleProjectionEdge> edges;
    while (std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }
        const auto row = split_csv_line(line);
        MuscleProjectionEdge edge;
        edge.pre_neuron = field(row, header, "pre_neuron");
        edge.muscle_index = static_cast<std::size_t>(std::stoull(field(row, header, "muscle_index")));
        edge.weight = std::stod(field(row, header, "weight"));
        edge.sign = std::stoi(field(row, header, "sign"));
        if (edge.muscle_index >= kBodyMuscleCount) {
            throw std::runtime_error("muscle_index out of range in projection CSV: " + std::to_string(edge.muscle_index));
        }
        if (!(edge.weight > 0.0) || !std::isfinite(edge.weight)) {
            throw std::runtime_error("Invalid non-positive muscle projection weight for neuron: " + edge.pre_neuron);
        }
        if (edge.sign != -1 && edge.sign != 1) {
            throw std::runtime_error("Invalid muscle projection sign for neuron: " + edge.pre_neuron);
        }
        edges.push_back(std::move(edge));
    }
    if (edges.empty()) {
        throw std::runtime_error("No muscle projection edges loaded from: " + projection_csv);
    }
    return edges;
}

void compute_muscle_output(
    const brain::BrainNetwork& network,
    const MuscleOutputConfig& config,
    std::array<float, kBodyMuscleCount>& output) {
    output.fill(0.0F);

    // motor-to-muscle projection v0: aggregate direct Cook2019 SI5 neuron-to-body-wall-muscle edges.
    // Voltage is converted to a simple clamped activation; this is not a final NMJ model.
    std::array<double, kBodyMuscleCount> signed_sum{};
    std::array<double, kBodyMuscleCount> weight_sum{};

    const double scale = config.v_scale_mV > 0.0 ? config.v_scale_mV : 40.0;
    for (const auto& edge : config.projection_edges) {
        if (edge.muscle_index >= kBodyMuscleCount) {
            continue;
        }
        const auto neuron_it = network.index.find(edge.pre_neuron);
        if (neuron_it == network.index.end() || !neuron_it->second) {
            continue;
        }
        const double voltage = neuron_it->second->soma_voltage_mV();
        const double activation = clamp01((voltage - config.v_rest_mV) / scale);
        signed_sum[edge.muscle_index] += static_cast<double>(edge.sign) * edge.weight * activation;
        weight_sum[edge.muscle_index] += edge.weight;
    }

    for (std::size_t idx = 0; idx < kBodyMuscleCount; ++idx) {
        if (weight_sum[idx] <= 0.0) {
            output[idx] = 0.0F;
            continue;
        }
        output[idx] = static_cast<float>(clamp01(signed_sum[idx] / weight_sum[idx]));
    }
}

}  // namespace brain4MuJoCo