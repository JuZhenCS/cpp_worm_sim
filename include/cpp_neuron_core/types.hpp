#pragma once

#include "cpp_neuron_core/channel.hpp"

#include <memory>
#include <string>
#include <vector>

namespace cpp_neuron {

struct Compartment {
    int index = 0;
    int parent_index = -1;
    std::string label;
    double capacitance_pF = 1.0;
    double leak_conductance_nS = 0.0;
    double leak_reversal_mV = -60.0;
    double axial_conductance_to_parent_nS = 0.0;
    double area_um2 = 0.0;
    double voltage_mV = -60.0;
    double cai_uM_per_um2 = 0.05;
    bool calcium_internal_enabled = false;
    double nca_conductance_nS = 0.0;
    double irk_conductance_nS = 0.0;
    double kqt3_conductance_nS = 0.0;
    double egl2_conductance_nS = 0.0;
    double shk1_conductance_nS = 0.0;
    double kvs1_conductance_nS = 0.0;
    double shl1_conductance_nS = 0.0;
    double egl36_conductance_nS = 0.0;
    double egl19_conductance_nS = 0.0;
    double cca1_conductance_nS = 0.0;
    double unc2_conductance_nS = 0.0;
    double kcnl_conductance_nS = 0.0;
    double slo1_egl19_conductance_nS = 0.0;
    double slo1_unc2_conductance_nS = 0.0;
    double slo2_egl19_conductance_nS = 0.0;
    double slo2_unc2_conductance_nS = 0.0;
    std::vector<std::unique_ptr<Channel>> channels;

    Compartment() = default;
    Compartment(const Compartment& other);
    Compartment& operator=(const Compartment& other);
    Compartment(Compartment&&) noexcept = default;
    Compartment& operator=(Compartment&&) noexcept = default;
};

struct Cell {
    std::string name;
    std::vector<Compartment> compartments;
};

struct TracePoint {
    double time_ms = 0.0;
    double soma_v_mV = 0.0;
    double stimulus = 0.0;
    double clamp_current_pA = 0.0;
};

}  // namespace cpp_neuron
