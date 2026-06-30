#include "cpp_neuron_core/neuron/cell_loader.hpp"

#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace cpp_neuron {

namespace {

std::vector<std::string> split_csv_line(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string field;
    while (std::getline(ss, field, ',')) {
        fields.push_back(field);
    }
    return fields;
}

std::map<std::string, std::size_t> header_index(const std::vector<std::string>& fields) {
    std::map<std::string, std::size_t> index;
    for (std::size_t i = 0; i < fields.size(); ++i) {
        index[fields[i]] = i;
    }
    return index;
}

std::string require_field(
    const std::vector<std::string>& fields,
    const std::map<std::string, std::size_t>& header,
    const std::string& name) {
    const auto found = header.find(name);
    if (found == header.end() || found->second >= fields.size()) {
        throw std::runtime_error("Missing CSV field: " + name);
    }
    return fields[found->second];
}

double optional_double(
    const std::vector<std::string>& fields,
    const std::map<std::string, std::size_t>& header,
    const std::string& name,
    double fallback) {
    const auto found = header.find(name);
    if (found == header.end() || found->second >= fields.size() || fields[found->second].empty()) {
        return fallback;
    }
    return std::stod(fields[found->second]);
}

}  // namespace

Cell load_cell_csv(const std::string& path, const std::string& cell_name) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("Failed to open cell CSV: " + path);
    }

    std::string line;
    std::getline(in, line);
    const auto header = header_index(split_csv_line(line));
    Cell cell;
    cell.name = cell_name;

    while (std::getline(in, line)) {
        if (line.empty()) {
            continue;
        }
        const auto fields = split_csv_line(line);
        if (fields.size() < header.size()) {
            throw std::runtime_error("Malformed cell CSV row: " + line);
        }
        Compartment c;
        c.index = std::stoi(require_field(fields, header, "idx"));
        c.parent_index = std::stoi(require_field(fields, header, "parent_idx"));
        c.label = require_field(fields, header, "label");
        c.capacitance_pF = std::stod(require_field(fields, header, "C_m_pF"));
        c.leak_conductance_nS = std::stod(require_field(fields, header, "g_L_nS"));
        c.leak_reversal_mV = std::stod(require_field(fields, header, "e_L_mV"));
        c.axial_conductance_to_parent_nS = std::stod(require_field(fields, header, "g_C_nS"));
        c.area_um2 = optional_double(fields, header, "area_um2", 0.0);
        c.nca_conductance_nS = optional_double(fields, header, "gbnca_nS", 0.0);
        c.irk_conductance_nS = optional_double(fields, header, "gbirk_nS", 0.0);
        c.kqt3_conductance_nS = optional_double(fields, header, "gbkqt3_nS", 0.0);
        c.egl2_conductance_nS = optional_double(fields, header, "gbegl2_nS", 0.0);
        c.shk1_conductance_nS = optional_double(fields, header, "gbshk1_nS", 0.0);
        c.kvs1_conductance_nS = optional_double(fields, header, "gbkvs1_nS", 0.0);
        c.shl1_conductance_nS = optional_double(fields, header, "gbshl1_nS", 0.0);
        c.egl36_conductance_nS = optional_double(fields, header, "gbegl36_nS", 0.0);
        c.egl19_conductance_nS = optional_double(fields, header, "gbegl19_nS", 0.0);
        c.cca1_conductance_nS = optional_double(fields, header, "gbcca1_nS", 0.0);
        c.unc2_conductance_nS = optional_double(fields, header, "gbunc2_nS", 0.0);
        c.kcnl_conductance_nS = optional_double(fields, header, "gbkcnl_nS", 0.0);
        c.slo1_egl19_conductance_nS = optional_double(fields, header, "gbslo1_egl19_nS", 0.0);
        c.slo1_unc2_conductance_nS = optional_double(fields, header, "gbslo1_unc2_nS", 0.0);
        c.slo2_egl19_conductance_nS = optional_double(fields, header, "gbslo2_egl19_nS", 0.0);
        c.slo2_unc2_conductance_nS = optional_double(fields, header, "gbslo2_unc2_nS", 0.0);
        c.voltage_mV = c.leak_reversal_mV;
        cell.compartments.push_back(c);
    }

    if (cell.compartments.empty()) {
        throw std::runtime_error("No compartments loaded from: " + path);
    }
    return cell;
}

}  // namespace cpp_neuron
