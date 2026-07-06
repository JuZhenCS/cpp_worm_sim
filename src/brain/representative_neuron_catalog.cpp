#include "brain/representative_neuron_catalog.hpp"

#include <stdexcept>

namespace brain {
namespace {

neuron::NeuronMechanismConfig mechanisms_for_reference(const std::string& reference) {
    neuron::NeuronMechanismConfig mechanisms;
    if (reference == "AWC") {
        mechanisms.enable_kqt3 = true;
        mechanisms.enable_shl1 = true;
        mechanisms.enable_egl19 = true;
        mechanisms.enable_unc2 = true;
    } else if (reference == "AIY") {
        mechanisms.enable_nca = true;
        mechanisms.enable_irk = true;
        mechanisms.enable_kqt3 = true;
        mechanisms.enable_egl2 = true;
        mechanisms.enable_shk1 = true;
        mechanisms.enable_kvs1 = true;
        mechanisms.enable_shl1 = true;
        mechanisms.enable_egl36 = true;
        mechanisms.enable_egl19 = true;
        mechanisms.enable_cca1 = true;
        mechanisms.enable_calcium_internal = true;
        mechanisms.enable_kcnl = true;
        mechanisms.enable_slo1_egl19 = true;
        mechanisms.enable_slo1_unc2 = true;
        mechanisms.enable_slo2_egl19 = true;
        mechanisms.enable_slo2_unc2 = true;
    } else if (reference == "AVA") {
        mechanisms.enable_nca = true;
        mechanisms.enable_shk1 = true;
        mechanisms.enable_shl1 = true;
        mechanisms.enable_egl19 = true;
        mechanisms.enable_cca1 = true;
        mechanisms.enable_unc2 = true;
        mechanisms.enable_calcium_internal = true;
        mechanisms.enable_kcnl = true;
        mechanisms.enable_slo1_unc2 = true;
    } else if (reference == "RIM") {
        mechanisms.enable_nca = true;
        mechanisms.enable_irk = true;
        mechanisms.enable_kqt3 = true;
        mechanisms.enable_egl2 = true;
        mechanisms.enable_shk1 = true;
        mechanisms.enable_kvs1 = true;
        mechanisms.enable_shl1 = true;
        mechanisms.enable_egl36 = true;
        mechanisms.enable_slo1_egl19 = true;
        mechanisms.enable_slo1_unc2 = true;
        mechanisms.enable_slo2_egl19 = true;
    } else if (reference == "VD5") {
        mechanisms.enable_nca = true;
        mechanisms.enable_kqt3 = true;
        mechanisms.enable_egl2 = true;
        mechanisms.enable_shk1 = true;
        mechanisms.enable_shl1 = true;
        mechanisms.enable_egl36 = true;
        mechanisms.enable_egl19 = true;
        mechanisms.enable_cca1 = true;
        mechanisms.enable_slo1_unc2 = true;
        mechanisms.enable_slo2_egl19 = true;
        mechanisms.enable_slo2_unc2 = true;
    } else {
        throw std::runtime_error("Unknown neuron parameter reference: " + reference);
    }
    return mechanisms;
}

}  // namespace

RepresentativeNeuronTemplate representative_neuron_template(
    const std::string& reference,
    const std::string& template_data_dir) {
    RepresentativeNeuronTemplate result;
    result.mechanisms = mechanisms_for_reference(reference);
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

}  // namespace brain
