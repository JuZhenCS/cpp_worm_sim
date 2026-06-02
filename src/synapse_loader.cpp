#include "cpp_neuron_core/synapse_loader.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace cpp_neuron {
namespace {

using CsvRow = std::unordered_map<std::string, std::string>;

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

std::vector<CsvRow> read_csv_rows(const std::string& csv_path) {
    std::ifstream input(csv_path);
    if (!input) {
        throw std::runtime_error("Could not open CSV: " + csv_path);
    }

    std::string line;
    if (!std::getline(input, line)) {
        throw std::runtime_error("CSV is empty: " + csv_path);
    }
    const auto header = split_csv_line(line);
    std::vector<CsvRow> rows;

    while (std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }
        const auto fields = split_csv_line(line);
        if (fields.size() != header.size()) {
            throw std::runtime_error("CSV row has wrong field count in " + csv_path + ": " + line);
        }
        CsvRow row;
        for (std::size_t i = 0; i < header.size(); ++i) {
            row.emplace(header[i], fields[i]);
        }
        rows.push_back(std::move(row));
    }
    return rows;
}

const std::string& require_field(const CsvRow& row, const std::string& field) {
    const auto it = row.find(field);
    if (it == row.end()) {
        throw std::runtime_error("Missing CSV field: " + field);
    }
    return it->second;
}

double field_double(const CsvRow& row, const std::string& field) {
    std::size_t parsed = 0;
    const auto& text = require_field(row, field);
    const double value = std::stod(text, &parsed);
    if (parsed != text.size()) {
        throw std::runtime_error("Invalid numeric value for " + field + ": " + text);
    }
    return value;
}

std::size_t field_size_or(const CsvRow& row, const std::string& field, std::size_t fallback) {
    const auto it = row.find(field);
    if (it == row.end() || it->second.empty()) {
        return fallback;
    }
    std::size_t parsed = 0;
    const unsigned long value = std::stoul(it->second, &parsed);
    if (parsed != it->second.size()) {
        throw std::runtime_error("Invalid integer value for " + field + ": " + it->second);
    }
    return static_cast<std::size_t>(value);
}

bool field_bool(const CsvRow& row, const std::string& field) {
    const auto& text = require_field(row, field);
    return text == "True" || text == "true" || text == "1";
}

std::shared_ptr<NeuronModel> require_neuron_by_name(const NeuronIndex& neurons, const std::string& name) {
    const auto it = neurons.find(name);
    if (it == neurons.end() || !it->second) {
        throw std::runtime_error("Synapse CSV references unknown neuron: " + name);
    }
    return it->second;
}

ChemicalComponentType parse_component_type(const std::string& text) {
    if (text == "exc" || text == "Excitatory") {
        return ChemicalComponentType::Excitatory;
    }
    if (text == "inh" || text == "Inhibitory") {
        return ChemicalComponentType::Inhibitory;
    }
    throw std::runtime_error("Unknown chemical component_type: " + text);
}

}  // namespace

SynapseNetwork::StepStats SynapseNetwork::apply(double dt_ms) {
    StepStats stats;
    for (auto& synapse : chemical_synapses) {
        const double current = synapse.apply_and_current_pA(dt_ms);
        stats.max_abs_chemical_current_pA = std::max(stats.max_abs_chemical_current_pA, std::abs(current));
    }
    for (auto& gap : gap_junctions) {
        const double current = gap.current_to_a_pA();
        stats.max_abs_gap_current_pA = std::max(stats.max_abs_gap_current_pA, std::abs(current));
        gap.apply();
    }
    return stats;
}

void SynapseNetwork::step(double dt_ms) {
    (void)apply(dt_ms);
}

std::vector<GradedChemicalSynapse> load_chemical_synapses_csv(
    const std::string& csv_path,
    const NeuronIndex& neurons,
    std::size_t default_pre_compartment,
    std::size_t default_post_compartment) {
    std::vector<GradedChemicalSynapse> synapses;
    for (const auto& row : read_csv_rows(csv_path)) {
        if (!field_bool(row, "active_in_v0")) {
            continue;
        }

        GradedChemicalSynapseConfig config;
        config.g_uS = field_double(row, "g_uS");
        config.e_rev_mV = field_double(row, "e_rev_mV");
        config.tau_ms = field_double(row, "tau_ms");
        config.v_half_mV = field_double(row, "v_half_mV");
        config.k_s_mV = field_double(row, "k_s_mV");

        const std::string& pre_name = require_field(row, "pre");
        const std::string& post_name = require_field(row, "post");
        const std::size_t pre_compartment = field_size_or(row, "pre_compartment", default_pre_compartment);
        const std::size_t post_compartment = field_size_or(row, "post_compartment", default_post_compartment);
        synapses.emplace_back(
            require_neuron_by_name(neurons, pre_name),
            require_neuron_by_name(neurons, post_name),
            pre_compartment,
            post_compartment,
            parse_component_type(require_field(row, "component_type")),
            config,
            true);
    }
    return synapses;
}

std::vector<GapJunction> load_gap_junctions_csv(
    const std::string& csv_path,
    const NeuronIndex& neurons,
    std::size_t default_compartment_a,
    std::size_t default_compartment_b) {
    std::vector<GapJunction> gaps;
    for (const auto& row : read_csv_rows(csv_path)) {
        if (!field_bool(row, "active_in_v0")) {
            continue;
        }

        GapJunctionConfig config;
        config.g_uS = field_double(row, "g_uS");
        const std::string& cell_a = require_field(row, "cell_a");
        const std::string& cell_b = require_field(row, "cell_b");
        const std::size_t compartment_a = field_size_or(row, "compartment_a", default_compartment_a);
        const std::size_t compartment_b = field_size_or(row, "compartment_b", default_compartment_b);
        gaps.emplace_back(
            require_neuron_by_name(neurons, cell_a),
            require_neuron_by_name(neurons, cell_b),
            compartment_a,
            compartment_b,
            config,
            true);
    }
    return gaps;
}

SynapseNetwork load_synapse_network_csv(
    const std::string& chemical_components_csv,
    const std::string& gap_junctions_csv,
    const NeuronIndex& neurons) {
    SynapseNetwork network;
    network.chemical_synapses = load_chemical_synapses_csv(chemical_components_csv, neurons);
    network.gap_junctions = load_gap_junctions_csv(gap_junctions_csv, neurons);
    return network;
}

}  // namespace cpp_neuron
