#include "cpp_neuron_core/neuron/calcium_internal.hpp"

#include <cmath>

namespace cpp_neuron {

CalciumInternalState::CalciumInternalState(double vcell_um3) : vcell_um3_(vcell_um3) {}

double CalciumInternalState::step(
    double cai_uM_per_um2,
    double ica_density_pA_per_um2,
    double voltage_mV,
    double dt_ms) const {
    constexpr double faraday = 96485.0;
    double source = 0.0;
    if (voltage_mV <= 60.0) {
        source = -free_fraction_ * ica_density_pA_per_um2 * 1.0e6 / (2.0 * faraday * vcell_um3_);
    }
    const double target = caeq_uM_per_um2_ + source * removal_tau_ms_;
    return target + (cai_uM_per_um2 - target) * std::exp(-dt_ms / removal_tau_ms_);
}

}  // namespace cpp_neuron
