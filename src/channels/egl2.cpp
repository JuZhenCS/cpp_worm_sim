#include "neuron/channels/egl2.hpp"

#include <cmath>
#include <memory>

namespace neuron {

Egl2Channel::Egl2Channel(double conductance_nS) : conductance_nS_(conductance_nS) {}

double Egl2Channel::minf(double voltage_mV) const {
    constexpr double vhm = -6.9;
    constexpr double ka = 14.9;
    return 1.0 / (1.0 + std::exp(-(voltage_mV - vhm) / ka));
}

double Egl2Channel::tau_m_ms(double voltage_mV) const {
    constexpr double atm = 1845.8;
    constexpr double btm = -122.6;
    constexpr double ctm = 13.8;
    constexpr double dtm = 1517.74;
    return atm / (1.0 + std::exp((voltage_mV - btm) / ctm)) + dtm;
}

void Egl2Channel::set_steady_state(double voltage_mV) {
    m_ = minf(voltage_mV);
    initialized_ = true;
}

void Egl2Channel::step(double voltage_mV, double dt_ms) {
    const double target = minf(voltage_mV);
    if (!initialized_) {
        m_ = target;
        initialized_ = true;
        return;
    }
    m_ = target + (m_ - target) * std::exp(-dt_ms / tau_m_ms(voltage_mV));
}

double Egl2Channel::current_pA(double voltage_mV) const {
    constexpr double ek_mV = -80.0;
    return conductance_nS_ * m_ * (voltage_mV - ek_mV);
}

std::string Egl2Channel::name() const {
    return "egl2";
}

std::vector<ChannelStateValue> Egl2Channel::state() const {
    return {{"gbegl2_nS", conductance_nS_}, {"m", m_}, {"ek_mV", -80.0}};
}

std::unique_ptr<Channel> Egl2Channel::clone() const {
    return std::make_unique<Egl2Channel>(*this);
}

}  // namespace neuron
