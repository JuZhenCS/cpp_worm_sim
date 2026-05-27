#pragma once

#include "cpp_neuron_core/types.hpp"

#include <string>
#include <vector>

namespace cpp_neuron {

void write_trace_csv(const std::string& path, const std::vector<TracePoint>& trace);

struct ChannelDiagnosticPoint {
    double time_ms = 0.0;
    double voltage_mV = 0.0;
    double leak_current_pA = 0.0;
    double axial_current_pA = 0.0;
    double ion_total_current_pA = 0.0;
    double nca_current_pA = 0.0;
    double nca_gbnca_nS = 0.0;
    double nca_ena_mV = 30.0;
    double irk_current_pA = 0.0;
    double irk_gbirk_nS = 0.0;
    double irk_ek_mV = -80.0;
    double kqt3_current_pA = 0.0;
    double kqt3_gbkqt3_nS = 0.0;
    double kqt3_ek_mV = -80.0;
    double egl2_current_pA = 0.0;
    double egl2_gbegl2_nS = 0.0;
    double egl2_ek_mV = -80.0;
    double shk1_current_pA = 0.0;
    double shk1_gbshk1_nS = 0.0;
    double shk1_ek_mV = -80.0;
    double kvs1_current_pA = 0.0;
    double kvs1_gbkvs1_nS = 0.0;
    double kvs1_ek_mV = -80.0;
    double shl1_current_pA = 0.0;
    double shl1_gbshl1_nS = 0.0;
    double shl1_ek_mV = -80.0;
    double egl36_current_pA = 0.0;
    double egl36_gbegl36_nS = 0.0;
    double egl36_ek_mV = -80.0;
    double egl19_current_pA = 0.0;
    double egl19_gbegl19_nS = 0.0;
    double egl19_eca_mV = 60.0;
    double cca1_current_pA = 0.0;
    double cca1_gbcca1_nS = 0.0;
    double cca1_eca_mV = 60.0;
    double unc2_current_pA = 0.0;
    double unc2_gbunc2_nS = 0.0;
    double unc2_eca_mV = 60.0;
    double kcnl_current_pA = 0.0;
    double kcnl_gbkcnl_nS = 0.0;
    double slo1_egl19_current_pA = 0.0;
    double slo1_egl19_gbslo1_nS = 0.0;
    double slo1_unc2_current_pA = 0.0;
    double slo1_unc2_gbslo1_nS = 0.0;
    double slo2_egl19_current_pA = 0.0;
    double slo2_egl19_gbslo2_nS = 0.0;
    double slo2_unc2_current_pA = 0.0;
    double slo2_unc2_gbslo2_nS = 0.0;
    double cai_uM_per_um2 = 0.05;
};

void write_channel_diagnostics_csv(
    const std::string& path,
    const std::vector<ChannelDiagnosticPoint>& diagnostics);

}  // namespace cpp_neuron
