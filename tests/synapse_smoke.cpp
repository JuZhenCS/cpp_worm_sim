#include "cpp_neuron_core/neuron.hpp"
#include "cpp_neuron_core/neuron_factory.hpp"
#include "cpp_neuron_core/synapse_loader.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

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

double double_field(
    const std::vector<std::string>& row,
    const std::unordered_map<std::string, std::size_t>& header,
    const std::string& name) {
    return std::stod(field(row, header, name));
}

void collect_chemical_names(const std::string& path, std::set<std::string>& names) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Could not open chemical CSV: " + path);
    }
    std::string line;
    std::getline(input, line);
    const auto header = header_index(split_csv_line(line));
    while (std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }
        const auto row = split_csv_line(line);
        names.insert(field(row, header, "pre"));
        names.insert(field(row, header, "post"));
    }
}

void collect_gap_names(const std::string& path, std::set<std::string>& names) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Could not open gap CSV: " + path);
    }
    std::string line;
    std::getline(input, line);
    const auto header = header_index(split_csv_line(line));
    while (std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }
        const auto row = split_csv_line(line);
        names.insert(field(row, header, "cell_a"));
        names.insert(field(row, header, "cell_b"));
    }
}

struct NeuronReference {
    std::string parameter_reference;
    std::string functional_group;
};

std::unordered_map<std::string, NeuronReference> load_neuron_references(const std::string& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Could not open neuron reference CSV: " + path);
    }

    std::string line;
    if (!std::getline(input, line)) {
        throw std::runtime_error("Empty neuron reference CSV: " + path);
    }
    const auto header = header_index(split_csv_line(line));

    std::unordered_map<std::string, NeuronReference> references;
    while (std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }
        const auto row = split_csv_line(line);
        NeuronReference reference;
        reference.parameter_reference = field(row, header, "parameter_reference");
        reference.functional_group = field(row, header, "functional_group");
        references.emplace(field(row, header, "neuron"), reference);
    }
    return references;
}

struct RepresentativeTemplate {
    std::string cell_file;
    cpp_neuron::NeuronChannelConfig channels;
    double initial_voltage_mV = -60.0;
};

cpp_neuron::NeuronChannelConfig channels_for_reference(const std::string& reference) {
    cpp_neuron::NeuronChannelConfig channels;
    if (reference == "AWC") {
        channels.enable_kqt3 = true;
        channels.enable_shl1 = true;
        channels.enable_egl19 = true;
        channels.enable_unc2 = true;
    } else if (reference == "AIY") {
        channels.enable_nca = true;
        channels.enable_irk = true;
        channels.enable_kqt3 = true;
        channels.enable_egl2 = true;
        channels.enable_shk1 = true;
        channels.enable_kvs1 = true;
        channels.enable_shl1 = true;
        channels.enable_egl36 = true;
        channels.enable_egl19 = true;
        channels.enable_cca1 = true;
        channels.enable_calcium_internal = true;
        channels.enable_kcnl = true;
        channels.enable_slo1_egl19 = true;
        channels.enable_slo1_unc2 = true;
        channels.enable_slo2_egl19 = true;
        channels.enable_slo2_unc2 = true;
    } else if (reference == "AVA") {
        channels.enable_nca = true;
        channels.enable_shk1 = true;
        channels.enable_shl1 = true;
        channels.enable_egl19 = true;
        channels.enable_cca1 = true;
        channels.enable_unc2 = true;
        channels.enable_calcium_internal = true;
        channels.enable_kcnl = true;
        channels.enable_slo1_unc2 = true;
    } else if (reference == "RIM") {
        channels.enable_nca = true;
        channels.enable_irk = true;
        channels.enable_kqt3 = true;
        channels.enable_egl2 = true;
        channels.enable_shk1 = true;
        channels.enable_kvs1 = true;
        channels.enable_shl1 = true;
        channels.enable_egl36 = true;
        channels.enable_slo1_egl19 = true;
        channels.enable_slo1_unc2 = true;
        channels.enable_slo2_egl19 = true;
    } else if (reference == "VD5") {
        channels.enable_nca = true;
        channels.enable_kqt3 = true;
        channels.enable_egl2 = true;
        channels.enable_shk1 = true;
        channels.enable_shl1 = true;
        channels.enable_egl36 = true;
        channels.enable_egl19 = true;
        channels.enable_cca1 = true;
        channels.enable_slo1_unc2 = true;
        channels.enable_slo2_egl19 = true;
        channels.enable_slo2_unc2 = true;
    } else {
        throw std::runtime_error("Unknown neuron parameter reference: " + reference);
    }
    return channels;
}

RepresentativeTemplate template_for_reference(const std::string& reference, const std::string& template_data_dir) {
    RepresentativeTemplate result;
    result.channels = channels_for_reference(reference);
    if (reference == "AWC") {
        result.cell_file = template_data_dir + "/awcl/AWCL_cell.csv";
        result.initial_voltage_mV = -65.0;
    } else if (reference == "AIY") {
        result.cell_file = template_data_dir + "/aiyl/AIYL_cell.csv";
        result.initial_voltage_mV = -45.0;
    } else if (reference == "AVA") {
        result.cell_file = template_data_dir + "/aval/AVAL_cell.csv";
        result.initial_voltage_mV = -30.0;
    } else if (reference == "RIM") {
        result.cell_file = template_data_dir + "/riml/RIML_cell.csv";
        result.initial_voltage_mV = -39.3;
    } else if (reference == "VD5") {
        result.cell_file = template_data_dir + "/vd05/VD05_cell.csv";
        result.initial_voltage_mV = -75.0;
    } else {
        throw std::runtime_error("Unknown neuron parameter reference: " + reference);
    }
    return result;
}

std::string arg_value(int argc, char** argv, const std::string& name, const std::string& fallback) {
    for (int i = 1; i + 1 < argc; ++i) {
        if (argv[i] == name) {
            return argv[i + 1];
        }
    }
    return fallback;
}

double arg_double(int argc, char** argv, const std::string& name, double fallback) {
    return std::stod(arg_value(argc, argv, name, std::to_string(fallback)));
}

std::size_t arg_size(int argc, char** argv, const std::string& name, std::size_t fallback) {
    return static_cast<std::size_t>(std::stoull(arg_value(argc, argv, name, std::to_string(fallback))));
}

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

}  // namespace

int main(int argc, char** argv) {
    try {
        const std::string data_dir =
            arg_value(argc, argv, "--data-dir", std::string(CPP_WORM_SIM_SOURCE_DIR) + "/data/synapse_v0");
        const std::string chemical_csv =
            arg_value(argc, argv, "--chemical-csv", data_dir + "/chemical_components_neuron_neuron_v0.csv");
        const std::string gap_csv = arg_value(argc, argv, "--gap-csv", data_dir + "/gap_junctions_v0.csv");
        const std::string neuron_reference_csv =
            arg_value(argc, argv, "--neuron-reference-csv", data_dir + "/neuron_parameter_reference_v0.csv");
        const std::string template_data_dir =
            arg_value(argc, argv, "--template-data-dir", std::string(CPP_WORM_SIM_SOURCE_DIR) + "/data");
        const double dt_ms = arg_double(argc, argv, "--dt-ms", 0.1);
        const double tstop_ms = arg_double(argc, argv, "--tstop-ms", 10.0);
        const double explode_voltage_mV = arg_double(argc, argv, "--explode-voltage-mv", 100.0);
        const std::size_t top_current_edges = arg_size(argc, argv, "--top-current-edges", 10);
        const std::size_t steps = static_cast<std::size_t>(std::ceil(tstop_ms / dt_ms));

        std::set<std::string> names;
        collect_chemical_names(chemical_csv, names);
        collect_gap_names(gap_csv, names);
        const auto neuron_references = load_neuron_references(neuron_reference_csv);

        cpp_neuron::NeuronIndex neurons;
        std::vector<std::shared_ptr<cpp_neuron::NeuronModel>> owned_neurons;
        std::vector<std::string> owned_neuron_names;
        owned_neurons.reserve(names.size());
        owned_neuron_names.reserve(names.size());
        std::map<std::string, std::size_t> parameter_reference_counts;
        std::map<std::string, std::size_t> functional_group_counts;
        std::size_t idx = 0;
        for (const auto& name : names) {
            const auto reference_it = neuron_references.find(name);
            if (reference_it == neuron_references.end()) {
                throw std::runtime_error("Missing neuron parameter reference for neuron: " + name);
            }
            const auto& reference = reference_it->second;
            const auto representative = template_for_reference(reference.parameter_reference, template_data_dir);
            cpp_neuron::NeuronBuildConfig build_config;
            build_config.name = name;
            build_config.cell_file = representative.cell_file;
            build_config.channels = representative.channels;

            std::shared_ptr<cpp_neuron::NeuronModel> neuron(cpp_neuron::create_neuron(build_config));
            neuron->set_all_voltages(representative.initial_voltage_mV + 0.1 * (static_cast<double>(idx % 7) - 3.0));
            neurons.emplace(name, neuron);
            owned_neurons.push_back(std::move(neuron));
            owned_neuron_names.push_back(name);
            ++parameter_reference_counts[reference.parameter_reference];
            ++functional_group_counts[reference.functional_group];
            ++idx;
        }

        auto network = cpp_neuron::load_synapse_network_csv(chemical_csv, gap_csv, neurons);

        double min_voltage = std::numeric_limits<double>::infinity();
        double max_voltage = -std::numeric_limits<double>::infinity();
        double max_chemical_current = 0.0;
        double max_gap_current = 0.0;
        std::size_t nan_count = 0;
        std::vector<ExplodingNeuron> exploding_neurons;
        std::vector<char> exploding_neuron_seen(owned_neurons.size(), 0);
        std::vector<double> step_voltages(owned_neurons.size(), 0.0);

        for (std::size_t step = 0; step < steps; ++step) {
            const auto neuron_count = static_cast<std::ptrdiff_t>(owned_neurons.size());
#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
            for (std::ptrdiff_t neuron_idx = 0; neuron_idx < neuron_count; ++neuron_idx) {
                owned_neurons[static_cast<std::size_t>(neuron_idx)]->clear_currents();
            }
            const auto stats = network.apply(dt_ms);
            max_chemical_current = std::max(max_chemical_current, stats.max_abs_chemical_current_pA);
            max_gap_current = std::max(max_gap_current, stats.max_abs_gap_current_pA);
            const double time_ms = static_cast<double>(step + 1) * dt_ms;

            double step_min_voltage = std::numeric_limits<double>::infinity();
            double step_max_voltage = -std::numeric_limits<double>::infinity();
            std::size_t step_nan_count = 0;
#ifdef _OPENMP
#pragma omp parallel for schedule(static) reduction(min : step_min_voltage) reduction(max : step_max_voltage) \
    reduction(+ : step_nan_count)
#endif
            for (std::ptrdiff_t neuron_idx = 0; neuron_idx < neuron_count; ++neuron_idx) {
                const auto idx = static_cast<std::size_t>(neuron_idx);
                const auto& neuron = owned_neurons[idx];
                neuron->step(dt_ms);
                const double v = neuron->soma_voltage_mV();
                step_voltages[idx] = v;
                if (!std::isfinite(v)) {
                    ++step_nan_count;
                    continue;
                }
                step_min_voltage = std::min(step_min_voltage, v);
                step_max_voltage = std::max(step_max_voltage, v);
            }
            nan_count += step_nan_count;
            min_voltage = std::min(min_voltage, step_min_voltage);
            max_voltage = std::max(max_voltage, step_max_voltage);
            for (std::size_t neuron_idx = 0; neuron_idx < owned_neurons.size(); ++neuron_idx) {
                const double v = step_voltages[neuron_idx];
                if ((!std::isfinite(v) || std::abs(v) > explode_voltage_mV) && !exploding_neuron_seen[neuron_idx]) {
                    exploding_neuron_seen[neuron_idx] = 1;
                    const auto& name = owned_neuron_names[neuron_idx];
                    exploding_neurons.push_back({name, time_ms, v});
                }
            }
        }

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

        std::cout << std::setprecision(10);
        std::cout << "tstop_ms=" << tstop_ms << '\n';
        std::cout << "dt_ms=" << dt_ms << '\n';
#ifdef _OPENMP
        std::cout << "openmp_threads=" << omp_get_max_threads() << '\n';
#else
        std::cout << "openmp_threads=1\n";
#endif
        std::cout << "neurons=" << owned_neurons.size() << '\n';
        for (const auto& item : parameter_reference_counts) {
            std::cout << "parameter_reference_" << item.first << '=' << item.second << '\n';
        }
        for (const auto& item : functional_group_counts) {
            std::cout << "functional_group_" << item.first << '=' << item.second << '\n';
        }
        std::cout << "chemical_components=" << network.chemical_synapses.size() << '\n';
        std::cout << "gap_junctions=" << network.gap_junctions.size() << '\n';
        std::cout << "min_voltage_mV=" << min_voltage << '\n';
        std::cout << "max_voltage_mV=" << max_voltage << '\n';
        std::cout << "max_chemical_current_pA=" << max_chemical_current << '\n';
        std::cout << "max_gap_current_pA=" << max_gap_current << '\n';
        std::cout << "nan_count=" << nan_count << '\n';
        std::cout << "explode_voltage_mV=" << explode_voltage_mV << '\n';
        std::cout << "exploding_neurons=" << exploding_neurons.size() << '\n';
        for (const auto& neuron : exploding_neurons) {
            std::cout << "exploding_neuron name=" << neuron.name << " first_time_ms=" << neuron.time_ms
                      << " voltage_mV=" << neuron.voltage_mV << '\n';
        }
        std::cout << "top_current_edges=" << std::min(top_current_edges, current_edges.size()) << '\n';
        for (std::size_t edge_idx = 0; edge_idx < std::min(top_current_edges, current_edges.size()); ++edge_idx) {
            const auto& edge = current_edges[edge_idx];
            std::cout << "top_current_edge rank=" << edge_idx + 1 << " type=" << edge.type << " label=" << edge.label
                      << " peak_abs_current_pA=" << edge.peak_abs_current_pA
                      << " peak_current_pA=" << edge.peak_current_pA << '\n';
        }

        if (nan_count > 0 || !exploding_neurons.empty() || !std::isfinite(min_voltage) || !std::isfinite(max_voltage)) {
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    } catch (const std::exception& exc) {
        std::cerr << "ERROR: " << exc.what() << '\n';
        return EXIT_FAILURE;
    }
}
