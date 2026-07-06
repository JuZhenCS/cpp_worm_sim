#pragma once

#include "brain/brain_network_builder.hpp"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

namespace brain {

struct BrainDiagnosticsMetadata {
    std::map<std::string, std::size_t> parameter_reference_counts;
    std::map<std::string, std::size_t> functional_group_counts;
};

BrainDiagnosticsMetadata load_brain_diagnostics_metadata_csv(const std::string& neuron_reference_csv);
struct BrainDiagnosticsConfig {
    bool enabled = false;
    double explode_voltage_mV = 100.0;
    std::size_t top_current_edges = 10;
};

struct ExplodingNeuron {
    std::string name;
    double time_ms = 0.0;
    double voltage_mV = 0.0;
};

struct TopCurrentEdge {
    std::string type;
    std::string label;
    double peak_abs_current_pA = 0.0;
    double peak_current_pA = 0.0;
};

struct BrainDiagnosticsResult {
    double tstop_ms = 0.0;
    double dt_ms = 0.0;
    int openmp_threads = 1;
    std::size_t neuron_count = 0;
    std::map<std::string, std::size_t> parameter_reference_counts;
    std::map<std::string, std::size_t> functional_group_counts;
    std::size_t chemical_components = 0;
    std::size_t gap_junctions = 0;
    double min_voltage_mV = 0.0;
    double max_voltage_mV = 0.0;
    double max_chemical_current_pA = 0.0;
    double max_gap_current_pA = 0.0;
    std::size_t nan_count = 0;
    double explode_voltage_mV = 0.0;
    std::vector<ExplodingNeuron> exploding_neurons;
    std::vector<TopCurrentEdge> top_current_edges;

    bool ok() const;
};

class BrainDiagnosticsCollector {
public:
    BrainDiagnosticsCollector(const BrainNetwork& network, BrainDiagnosticsConfig config, BrainDiagnosticsMetadata metadata);

    void observe_after_step(
        const BrainNetwork& network,
        const neuron::SynapseNetwork::StepStats& synapse_stats,
        double time_ms);

    BrainDiagnosticsResult finish(const BrainNetwork& network, double tstop_ms, double dt_ms) const;

private:
    BrainDiagnosticsConfig config_;
    BrainDiagnosticsResult result_;
    std::vector<char> exploding_neuron_seen_;
};

}  // namespace brain

