#include "brain/diagnostics/brain_diagnostics_recorder.hpp"

#include <iomanip>
#include <ostream>

namespace brain {

void write_brain_summary(const BrainDiagnosticsResult& result, std::ostream& output) {
    output << std::setprecision(10);
    output << "tstop_ms=" << result.tstop_ms << '\n';
    output << "dt_ms=" << result.dt_ms << '\n';
    output << "openmp_threads=" << result.openmp_threads << '\n';
    output << "neurons=" << result.neuron_count << '\n';
    for (const auto& item : result.parameter_reference_counts) {
        output << "parameter_reference_" << item.first << '=' << item.second << '\n';
    }
    for (const auto& item : result.functional_group_counts) {
        output << "functional_group_" << item.first << '=' << item.second << '\n';
    }
    output << "chemical_components=" << result.chemical_components << '\n';
    output << "gap_junctions=" << result.gap_junctions << '\n';
    output << "min_voltage_mV=" << result.min_voltage_mV << '\n';
    output << "max_voltage_mV=" << result.max_voltage_mV << '\n';
    output << "max_chemical_current_pA=" << result.max_chemical_current_pA << '\n';
    output << "max_gap_current_pA=" << result.max_gap_current_pA << '\n';
    output << "nan_count=" << result.nan_count << '\n';
    output << "explode_voltage_mV=" << result.explode_voltage_mV << '\n';
    output << "exploding_neurons=" << result.exploding_neurons.size() << '\n';
    for (const auto& neuron : result.exploding_neurons) {
        output << "exploding_neuron name=" << neuron.name << " first_time_ms=" << neuron.time_ms
               << " voltage_mV=" << neuron.voltage_mV << '\n';
    }
    output << "top_current_edges=" << result.top_current_edges.size() << '\n';
    for (std::size_t edge_idx = 0; edge_idx < result.top_current_edges.size(); ++edge_idx) {
        const auto& edge = result.top_current_edges[edge_idx];
        output << "top_current_edge rank=" << edge_idx + 1 << " type=" << edge.type << " label=" << edge.label
               << " peak_abs_current_pA=" << edge.peak_abs_current_pA << " peak_current_pA=" << edge.peak_current_pA
               << '\n';
    }
}

}  // namespace brain
