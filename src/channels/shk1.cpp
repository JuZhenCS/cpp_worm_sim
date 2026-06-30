#include "neuron/channels/shk1.hpp"

#include <cmath>
#include <memory>

namespace neuron {

Shk1Channel::Shk1Channel(double conductance_nS) : conductance_nS_(conductance_nS) {}

double Shk1Channel::minf(double voltage_mV) const {
    constexpr double vhm = 20.4;
    constexpr double ka = 7.7;
    return 1.0 / (1.0 + std::exp(-(voltage_mV - vhm) / ka));
}

double Shk1Channel::hinf(double voltage_mV) const {
    constexpr double vhh = -7.0;
    constexpr double ki = 5.8;
    return 1.0 / (1.0 + std::exp((voltage_mV - vhh) / ki));
}

double Shk1Channel::tau_m_ms(double voltage_mV) const {
    constexpr double atm = 26.6;
    constexpr double btm = -33.7;
    constexpr double ctm = 15.8;
    constexpr double dtm = -33.7;
    constexpr double etm = 15.4;
    constexpr double ftm = 2.0;
    return atm / (std::exp(-(voltage_mV - btm) / ctm) + std::exp((voltage_mV - dtm) / etm)) + ftm;
}

double Shk1Channel::tau_h_ms(double /*voltage_mV*/) const {
    return 1400.0;
}

void Shk1Channel::set_steady_state(double voltage_mV) {
    m_ = minf(voltage_mV);
    h_ = hinf(voltage_mV);
    initialized_ = true;
}

void Shk1Channel::step(double voltage_mV, double dt_ms) {
    const double m_target = minf(voltage_mV);
    const double h_target = hinf(voltage_mV);
    if (!initialized_) {
        m_ = m_target;
        h_ = h_target;
        initialized_ = true;
        return;
    }
    m_ = m_target + (m_ - m_target) * std::exp(-dt_ms / tau_m_ms(voltage_mV));
    h_ = h_target + (h_ - h_target) * std::exp(-dt_ms / tau_h_ms(voltage_mV));
}

double Shk1Channel::current_pA(double voltage_mV) const {
    constexpr double ek_mV = -80.0;
    return conductance_nS_ * m_ * h_ * (voltage_mV - ek_mV);
}

std::string Shk1Channel::name() const {
    return "shk1";
}

std::vector<ChannelStateValue> Shk1Channel::state() const {
    return {{"gbshk1_nS", conductance_nS_}, {"m", m_}, {"h", h_}, {"ek_mV", -80.0}};
}

std::unique_ptr<Channel> Shk1Channel::clone() const {
    return std::make_unique<Shk1Channel>(*this);
}

}  // namespace neuron
