#pragma once

namespace neuron {

class CalciumInternalState {
public:
    explicit CalciumInternalState(double vcell_um3 = 31.16);

    double step(double cai_uM_per_um2, double ica_density_pA_per_um2, double voltage_mV, double dt_ms) const;
    double caeq_uM_per_um2() const { return caeq_uM_per_um2_; }

private:
    double vcell_um3_ = 31.16;
    double free_fraction_ = 0.001;
    double removal_tau_ms_ = 50.0;
    double caeq_uM_per_um2_ = 0.05;
};

}  // namespace neuron
