#include "neuron/channels/kcnl.hpp"

#include <cmath>
#include <memory>

namespace neuron {

KcnlChannel::KcnlChannel(double conductance_nS) : conductance_nS_(conductance_nS) {}

void KcnlChannel::set_calcium(double cai_uM_per_um2) {
    cai_uM_per_um2_ = cai_uM_per_um2;
}

double KcnlChannel::minf(double cai_uM_per_um2) const {
    return cai_uM_per_um2 / (0.33 + cai_uM_per_um2);
}

void KcnlChannel::set_steady_state(double cai_uM_per_um2) {
    cai_uM_per_um2_ = cai_uM_per_um2;
    m_ = minf(cai_uM_per_um2_);
    initialized_ = true;
}

void KcnlChannel::step(double /*voltage_mV*/, double dt_ms) {
    const double target = minf(cai_uM_per_um2_);
    if (!initialized_) {
        m_ = target;
        initialized_ = true;
        return;
    }
    m_ = target + (m_ - target) * std::exp(-dt_ms / tau_m_ms());
}

double KcnlChannel::current_pA(double voltage_mV) const {
    return conductance_nS_ * m_ * (voltage_mV + 80.0);
}

std::string KcnlChannel::name() const {
    return "kcnl";
}

std::vector<ChannelStateValue> KcnlChannel::state() const {
    return {{"gbkcnl_nS", conductance_nS_}, {"m", m_}, {"cai_uM_per_um2", cai_uM_per_um2_}, {"ek_mV", -80.0}};
}

std::unique_ptr<Channel> KcnlChannel::clone() const {
    return std::make_unique<KcnlChannel>(*this);
}

}  // namespace neuron
