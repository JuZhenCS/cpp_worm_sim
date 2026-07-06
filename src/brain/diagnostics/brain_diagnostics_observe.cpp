#include "brain/diagnostics/brain_diagnostics_observe.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <unordered_map>
#include <utility>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace brain {
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
        throw std::runtime_error("Missing CSV field: " + name);
    }
    return row[found->second];
}
int openmp_thread_count() {
#ifdef _OPENMP
    return omp_get_max_threads();
#else
    return 1;
#endif
}

std::vector<TopCurrentEdge> top_current_edges(const neuron::SynapseNetwork& network, std::size_t count) {
    std::vector<TopCurrentEdge> current_edges;
    current_edges.reserve(network.chemical_synapses.size() + network.gap_junctions.size());
    for (const auto& synapse : network.chemical_synapses) {
        current_edges.push_back(
            {"chemical", synapse.label(), synapse.peak_abs_current_pA(), synapse.peak_current_pA()});
    }
    for (const auto& gap : network.gap_junctions) {
        current_edges.push_back({"gap", gap.label(), gap.peak_abs_current_pA(), gap.peak_current_to_a_pA()});
    }
    std::sort(current_edges.begin(), current_edges.end(), [](const TopCurrentEdge& a, const TopCurrentEdge& b) {
        return a.peak_abs_current_pA > b.peak_abs_current_pA;
    });
    if (current_edges.size() > count) {
        current_edges.resize(count);
    }
    return current_edges;
}

}  // namespace

BrainDiagnosticsMetadata load_brain_diagnostics_metadata_csv(const std::string& neuron_reference_csv) {
    std::ifstream input(neuron_reference_csv);
    if (!input) {
        throw std::runtime_error("Could not open neuron reference CSV: " + neuron_reference_csv);
    }

    std::string line;
    if (!std::getline(input, line)) {
        throw std::runtime_error("Empty neuron reference CSV: " + neuron_reference_csv);
    }
    const auto header = header_index(split_csv_line(line));

    BrainDiagnosticsMetadata metadata;
    while (std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }
        const auto row = split_csv_line(line);
        ++metadata.parameter_reference_counts[field(row, header, "parameter_reference")];
        ++metadata.functional_group_counts[field(row, header, "functional_group")];
    }
    return metadata;
}
bool BrainDiagnosticsResult::ok() const {
    return nan_count == 0 && exploding_neurons.empty() && std::isfinite(min_voltage_mV) &&
           std::isfinite(max_voltage_mV);
}

BrainDiagnosticsCollector::BrainDiagnosticsCollector(
    const BrainNetwork& network,
    BrainDiagnosticsConfig config,
    BrainDiagnosticsMetadata metadata)
    : config_(config), exploding_neuron_seen_(network.neurons.size(), 0) {
    if (config_.explode_voltage_mV <= 0.0) {
        throw std::runtime_error("--explode-voltage-mv must be positive");
    }

    result_.openmp_threads = openmp_thread_count();
    result_.neuron_count = network.neurons.size();
    result_.parameter_reference_counts = std::move(metadata.parameter_reference_counts);
    result_.functional_group_counts = std::move(metadata.functional_group_counts);
    result_.chemical_components = network.synapses.chemical_synapses.size();
    result_.gap_junctions = network.synapses.gap_junctions.size();
    result_.min_voltage_mV = std::numeric_limits<double>::infinity();
    result_.max_voltage_mV = -std::numeric_limits<double>::infinity();
    result_.explode_voltage_mV = config_.explode_voltage_mV;
}

void BrainDiagnosticsCollector::observe_after_step(
    const BrainNetwork& network,
    const neuron::SynapseNetwork::StepStats& synapse_stats,
    double time_ms) {
    result_.max_chemical_current_pA =
        std::max(result_.max_chemical_current_pA, synapse_stats.max_abs_chemical_current_pA);
    result_.max_gap_current_pA = std::max(result_.max_gap_current_pA, synapse_stats.max_abs_gap_current_pA);

    for (std::size_t neuron_idx = 0; neuron_idx < network.neurons.size(); ++neuron_idx) {
        const double voltage = network.neurons[neuron_idx]->soma_voltage_mV();
        if (!std::isfinite(voltage)) {
            ++result_.nan_count;
        } else {
            result_.min_voltage_mV = std::min(result_.min_voltage_mV, voltage);
            result_.max_voltage_mV = std::max(result_.max_voltage_mV, voltage);
        }

        if ((!std::isfinite(voltage) || std::abs(voltage) > config_.explode_voltage_mV) &&
            !exploding_neuron_seen_[neuron_idx]) {
            exploding_neuron_seen_[neuron_idx] = 1;
            result_.exploding_neurons.push_back({network.neuron_names[neuron_idx], time_ms, voltage});
        }
    }
}

BrainDiagnosticsResult BrainDiagnosticsCollector::finish(
    const BrainNetwork& network,
    double tstop_ms,
    double dt_ms) const {
    BrainDiagnosticsResult result = result_;
    result.tstop_ms = tstop_ms;
    result.dt_ms = dt_ms;
    result.top_current_edges = top_current_edges(network.synapses, config_.top_current_edges);
    return result;
}

}  // namespace brain



