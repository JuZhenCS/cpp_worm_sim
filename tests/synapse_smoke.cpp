#include "cpp_neuron_core/neuron.hpp"
#include "cpp_neuron_core/neuron_factory.hpp"
#include "cpp_neuron_core/synapse_loader.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
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

}  // namespace

int main(int argc, char** argv) {
    try {
        const std::string data_dir = arg_value(argc, argv, "--data-dir", "../C.elegans.network/synapse_v0");
        const std::string chemical_csv = arg_value(argc, argv, "--chemical-csv", data_dir + "/chemical_components_v0.csv");
        const std::string gap_csv = arg_value(argc, argv, "--gap-csv", data_dir + "/gap_junctions_v0.csv");
        const std::string neuron_reference_csv =
            arg_value(argc, argv, "--neuron-reference-csv", data_dir + "/neuron_parameter_reference_v0.csv");
        const std::string template_data_dir =
            arg_value(argc, argv, "--template-data-dir", std::string(CPP_WORM_SIM_SOURCE_DIR) + "/data");
        const double dt_ms = arg_double(argc, argv, "--dt-ms", 0.1);
        const double tstop_ms = arg_double(argc, argv, "--tstop-ms", 10.0);
        const std::size_t steps = static_cast<std::size_t>(std::ceil(tstop_ms / dt_ms));

        std::set<std::string> names;
        collect_chemical_names(chemical_csv, names);
        collect_gap_names(gap_csv, names);
        const auto neuron_references = load_neuron_references(neuron_reference_csv);

        cpp_neuron::NeuronIndex neurons;
        std::vector<std::shared_ptr<cpp_neuron::NeuronModel>> owned_neurons;
        owned_neurons.reserve(names.size());
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

        for (std::size_t step = 0; step < steps; ++step) {
            for (const auto& neuron : owned_neurons) {
                neuron->clear_currents();
            }
            const auto stats = network.apply(dt_ms);
            max_chemical_current = std::max(max_chemical_current, stats.max_abs_chemical_current_pA);
            max_gap_current = std::max(max_gap_current, stats.max_abs_gap_current_pA);
            for (const auto& neuron : owned_neurons) {
                neuron->step(dt_ms);
                const double v = neuron->soma_voltage_mV();
                if (!std::isfinite(v)) {
                    ++nan_count;
                    continue;
                }
                min_voltage = std::min(min_voltage, v);
                max_voltage = std::max(max_voltage, v);
            }
        }

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

        if (nan_count > 0 || !std::isfinite(min_voltage) || !std::isfinite(max_voltage)) {
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    } catch (const std::exception& exc) {
        std::cerr << "ERROR: " << exc.what() << '\n';
        return EXIT_FAILURE;
    }
}
