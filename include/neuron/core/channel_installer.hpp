#pragma once

#include "neuron/core/cell.hpp"

namespace neuron {

// Small installation helpers keep channel construction out of MultiCompartmentNeuron.
void install_nca_channels(Cell& cell);
void install_irk_channels(Cell& cell);
void install_kqt3_channels(Cell& cell);
void install_egl2_channels(Cell& cell);
void install_shk1_channels(Cell& cell);
void install_kvs1_channels(Cell& cell);
void install_shl1_channels(Cell& cell);
void install_egl36_channels(Cell& cell);
void install_egl19_channels(Cell& cell);
void install_cca1_channels(Cell& cell);
void install_unc2_channels(Cell& cell);
void install_kcnl_channels(Cell& cell);
void install_slo1_egl19_channels(Cell& cell);
void install_slo1_unc2_channels(Cell& cell);
void install_slo2_egl19_channels(Cell& cell);
void install_slo2_unc2_channels(Cell& cell);
void enable_calcium_internal(Cell& cell);

}  // namespace neuron
