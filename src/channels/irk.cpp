#include "neuron/channels/irk.hpp"

#include <cmath>
#include <memory>

namespace neuron {

IrkChannel::IrkChannel(double conductance_nS) : conductance_nS_(conductance_nS) {}

double IrkChannel::minf(double voltage_mV) const {
    constexpr double vhm = -82.0;
    constexpr double ka = 13.0;
    return 1.0 / (1.0 + std::exp((voltage_mV - vhm) / ka));
}

double IrkChannel::tau_m_ms(double voltage_mV) const {
    constexpr double atm = 17.1;
    constexpr double btm = -17.8;
    constexpr double ctm = 20.3;
    constexpr double dtm = -43.4;
    constexpr double etm = 11.2;
    constexpr double ftm = 3.8;
    return atm / (std::exp(-(voltage_mV - btm) / ctm) + std::exp((voltage_mV - dtm) / etm)) + ftm;
}

void IrkChannel::set_steady_state(double voltage_mV) {
    m_ = minf(voltage_mV);
    initialized_ = true;
}

void IrkChannel::step(double voltage_mV, double dt_ms) {
    const double target = minf(voltage_mV);
    if (!initialized_) {
        m_ = target;
        initialized_ = true;
        return;
    }
    const double tau = tau_m_ms(voltage_mV);
    m_ = target + (m_ - target) * std::exp(-dt_ms / tau);
}

double IrkChannel::current_pA(double voltage_mV) const {
    constexpr double ek_mV = -80.0;
    return conductance_nS_ * m_ * (voltage_mV - ek_mV);
}

std::string IrkChannel::name() const {
    return "irk";
}

std::vector<ChannelStateValue> IrkChannel::state() const {
    return {{"gbirk_nS", conductance_nS_}, {"m", m_}, {"ek_mV", -80.0}};
}

std::unique_ptr<Channel> IrkChannel::clone() const {
    return std::make_unique<IrkChannel>(*this);
}

}  // namespace neuron
