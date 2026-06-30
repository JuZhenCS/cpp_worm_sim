#include "neuron/channels/egl19.hpp"

#include <cmath>
#include <memory>

namespace neuron {

Egl19Channel::Egl19Channel(double conductance_nS, double reversal_mV)
    : conductance_nS_(conductance_nS), reversal_mV_(reversal_mV) {}

double Egl19Channel::minf(double voltage_mV) const {
    return 1.0 / (1.0 + std::exp(-(voltage_mV - 5.6) / 7.5));
}

double Egl19Channel::hinf(double voltage_mV) const {
    const double first = 1.43 / (1.0 + std::exp(-(voltage_mV - 24.9) / 12.0)) + 0.14;
    const double second = 5.96 / (1.0 + std::exp((voltage_mV + 20.5) / 8.1)) + 0.60;
    return first * second;
}

double Egl19Channel::tau_m_ms(double voltage_mV) const {
    const double term1 = 2.9 * std::exp(-std::pow((voltage_mV - 5.2) / 6.0, 2.0));
    const double term2 = 1.9 * std::exp(-std::pow((voltage_mV - 1.4) / 30.0, 2.0));
    return term1 + term2 + 2.3;
}

double Egl19Channel::tau_h_ms(double voltage_mV) const {
    const double inner = 44.6 / (1.0 + std::exp((voltage_mV + 23.0) / 5.0))
                         + 36.4 / (1.0 + std::exp((voltage_mV - 28.7) / 3.7)) + 43.1;
    return 0.4 * inner;
}

void Egl19Channel::set_steady_state(double voltage_mV) {
    m_ = minf(voltage_mV);
    h_ = hinf(voltage_mV);
    initialized_ = true;
}

void Egl19Channel::step(double voltage_mV, double dt_ms) {
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

double Egl19Channel::current_pA(double voltage_mV) const {
    return conductance_nS_ * m_ * h_ * (voltage_mV - reversal_mV_);
}

std::string Egl19Channel::name() const {
    return "egl19";
}

std::vector<ChannelStateValue> Egl19Channel::state() const {
    return {{"gbegl19_nS", conductance_nS_}, {"m", m_}, {"h", h_}, {"eca_mV", reversal_mV_}};
}

std::unique_ptr<Channel> Egl19Channel::clone() const {
    return std::make_unique<Egl19Channel>(*this);
}

}  // namespace neuron
