#include "cpp_neuron_core/types.hpp"

namespace cpp_neuron {

Compartment::Compartment(const Compartment& other)
    : index(other.index),
      parent_index(other.parent_index),
      label(other.label),
      capacitance_pF(other.capacitance_pF),
      leak_conductance_nS(other.leak_conductance_nS),
      leak_reversal_mV(other.leak_reversal_mV),
      axial_conductance_to_parent_nS(other.axial_conductance_to_parent_nS),
      area_um2(other.area_um2),
      voltage_mV(other.voltage_mV),
      cai_uM_per_um2(other.cai_uM_per_um2),
      calcium_internal_enabled(other.calcium_internal_enabled),
      nca_conductance_nS(other.nca_conductance_nS),
      irk_conductance_nS(other.irk_conductance_nS),
      kqt3_conductance_nS(other.kqt3_conductance_nS),
      egl2_conductance_nS(other.egl2_conductance_nS),
      shk1_conductance_nS(other.shk1_conductance_nS),
      kvs1_conductance_nS(other.kvs1_conductance_nS),
      shl1_conductance_nS(other.shl1_conductance_nS),
      egl36_conductance_nS(other.egl36_conductance_nS),
      egl19_conductance_nS(other.egl19_conductance_nS),
      cca1_conductance_nS(other.cca1_conductance_nS),
      unc2_conductance_nS(other.unc2_conductance_nS),
      kcnl_conductance_nS(other.kcnl_conductance_nS),
      slo1_egl19_conductance_nS(other.slo1_egl19_conductance_nS),
      slo1_unc2_conductance_nS(other.slo1_unc2_conductance_nS),
      slo2_egl19_conductance_nS(other.slo2_egl19_conductance_nS),
      slo2_unc2_conductance_nS(other.slo2_unc2_conductance_nS) {
    channels.reserve(other.channels.size());
    for (const auto& channel : other.channels) {
        channels.push_back(channel->clone());
    }
}

Compartment& Compartment::operator=(const Compartment& other) {
    if (this == &other) {
        return *this;
    }
    index = other.index;
    parent_index = other.parent_index;
    label = other.label;
    capacitance_pF = other.capacitance_pF;
    leak_conductance_nS = other.leak_conductance_nS;
    leak_reversal_mV = other.leak_reversal_mV;
    axial_conductance_to_parent_nS = other.axial_conductance_to_parent_nS;
    area_um2 = other.area_um2;
    voltage_mV = other.voltage_mV;
    cai_uM_per_um2 = other.cai_uM_per_um2;
    calcium_internal_enabled = other.calcium_internal_enabled;
    nca_conductance_nS = other.nca_conductance_nS;
    irk_conductance_nS = other.irk_conductance_nS;
    kqt3_conductance_nS = other.kqt3_conductance_nS;
    egl2_conductance_nS = other.egl2_conductance_nS;
    shk1_conductance_nS = other.shk1_conductance_nS;
    kvs1_conductance_nS = other.kvs1_conductance_nS;
    shl1_conductance_nS = other.shl1_conductance_nS;
    egl36_conductance_nS = other.egl36_conductance_nS;
    egl19_conductance_nS = other.egl19_conductance_nS;
    cca1_conductance_nS = other.cca1_conductance_nS;
    unc2_conductance_nS = other.unc2_conductance_nS;
    kcnl_conductance_nS = other.kcnl_conductance_nS;
    slo1_egl19_conductance_nS = other.slo1_egl19_conductance_nS;
    slo1_unc2_conductance_nS = other.slo1_unc2_conductance_nS;
    slo2_egl19_conductance_nS = other.slo2_egl19_conductance_nS;
    slo2_unc2_conductance_nS = other.slo2_unc2_conductance_nS;
    channels.clear();
    channels.reserve(other.channels.size());
    for (const auto& channel : other.channels) {
        channels.push_back(channel->clone());
    }
    return *this;
}

}  // namespace cpp_neuron
