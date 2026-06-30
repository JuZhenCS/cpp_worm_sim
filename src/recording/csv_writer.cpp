#include "neuron/recording/csv_writer.hpp"

#include <fstream>
#include <stdexcept>

namespace neuron {

void write_trace_csv(const std::string& path, const std::vector<TracePoint>& trace) {
    std::ofstream out(path);
    if (!out) {
        throw std::runtime_error("Failed to open output CSV: " + path);
    }
    out << "time_ms,soma_v_mV,stimulus,clamp_current_pA\n";
    for (const auto& point : trace) {
        out << point.time_ms << ',' << point.soma_v_mV << ',' << point.stimulus << ','
            << point.clamp_current_pA << '\n';
    }
}

void write_channel_diagnostics_csv(
    const std::string& path,
    const std::vector<ChannelDiagnosticPoint>& diagnostics) {
    std::ofstream out(path);
    if (!out) {
        throw std::runtime_error("Failed to open channel diagnostics CSV: " + path);
    }
    out << "time_ms,V_mV,I_leak_pA,I_axial_pA,I_ion_total_pA,"
           "I_nca_pA,nca_gbnca_nS,nca_ena_mV,I_irk_pA,irk_gbirk_nS,irk_ek_mV,"
           "I_kqt3_pA,kqt3_gbkqt3_nS,kqt3_ek_mV,I_egl2_pA,egl2_gbegl2_nS,egl2_ek_mV,"
           "I_shk1_pA,shk1_gbshk1_nS,shk1_ek_mV,I_kvs1_pA,kvs1_gbkvs1_nS,kvs1_ek_mV,"
           "I_shl1_pA,shl1_gbshl1_nS,shl1_ek_mV,I_egl36_pA,egl36_gbegl36_nS,egl36_ek_mV,"
           "I_egl19_pA,egl19_gbegl19_nS,egl19_eca_mV,I_cca1_pA,cca1_gbcca1_nS,cca1_eca_mV,"
           "I_unc2_pA,unc2_gbunc2_nS,unc2_eca_mV,I_kcnl_pA,kcnl_gbkcnl_nS,"
           "I_slo1_egl19_pA,slo1_egl19_gbslo1_nS,I_slo1_unc2_pA,slo1_unc2_gbslo1_nS,"
           "I_slo2_egl19_pA,slo2_egl19_gbslo2_nS,I_slo2_unc2_pA,slo2_unc2_gbslo2_nS,"
           "cai_uM_per_um2\n";
    for (const auto& point : diagnostics) {
        out << point.time_ms << ',' << point.voltage_mV << ',' << point.leak_current_pA << ','
            << point.axial_current_pA << ',' << point.ion_total_current_pA << ','
            << point.nca_current_pA << ',' << point.nca_gbnca_nS << ',' << point.nca_ena_mV << ','
            << point.irk_current_pA << ',' << point.irk_gbirk_nS << ',' << point.irk_ek_mV << ','
            << point.kqt3_current_pA << ',' << point.kqt3_gbkqt3_nS << ',' << point.kqt3_ek_mV << ','
            << point.egl2_current_pA << ',' << point.egl2_gbegl2_nS << ',' << point.egl2_ek_mV << ','
            << point.shk1_current_pA << ',' << point.shk1_gbshk1_nS << ',' << point.shk1_ek_mV << ','
            << point.kvs1_current_pA << ',' << point.kvs1_gbkvs1_nS << ',' << point.kvs1_ek_mV << ','
            << point.shl1_current_pA << ',' << point.shl1_gbshl1_nS << ',' << point.shl1_ek_mV << ','
            << point.egl36_current_pA << ',' << point.egl36_gbegl36_nS << ',' << point.egl36_ek_mV << ','
            << point.egl19_current_pA << ',' << point.egl19_gbegl19_nS << ',' << point.egl19_eca_mV << ','
            << point.cca1_current_pA << ',' << point.cca1_gbcca1_nS << ',' << point.cca1_eca_mV << ','
            << point.unc2_current_pA << ',' << point.unc2_gbunc2_nS << ',' << point.unc2_eca_mV << ','
            << point.kcnl_current_pA << ',' << point.kcnl_gbkcnl_nS << ','
            << point.slo1_egl19_current_pA << ',' << point.slo1_egl19_gbslo1_nS << ','
            << point.slo1_unc2_current_pA << ',' << point.slo1_unc2_gbslo1_nS << ','
            << point.slo2_egl19_current_pA << ',' << point.slo2_egl19_gbslo2_nS << ','
            << point.slo2_unc2_current_pA << ',' << point.slo2_unc2_gbslo2_nS << ','
            << point.cai_uM_per_um2 << '\n';
    }
}

}  // namespace neuron
