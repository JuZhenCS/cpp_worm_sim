#include "cpp_neuron_core/channels/unc2.hpp"

#include <cmath>
#include <memory>

namespace cpp_neuron {

Unc2Channel::Unc2Channel(double conductance_nS, double reversal_mV)
    : conductance_nS_(conductance_nS), reversal_mV_(reversal_mV) {}

double Unc2Channel::minf(double voltage_mV) const {
    return 1.0 / (1.0 + std::exp(-(voltage_mV + 12.2) / 4.0));
}

double Unc2Channel::hinf(double voltage_mV) const {
    return 1.0 / (1.0 + std::exp((voltage_mV + 52.5) / 5.6));
}

double Unc2Channel::tau_m_ms(double voltage_mV) const {
    return 1.5 / (std::exp(-(voltage_mV + 8.2) / 9.1) + std::exp((voltage_mV + 8.2) / 15.4)) + 0.1;
}

double Unc2Channel::tau_h_ms(double voltage_mV) const {
    return 83.8 / (1.0 + std::exp(-(voltage_mV - 52.9) / -3.5))
           + 72.1 / (1.0 + std::exp((voltage_mV - 23.9) / -3.6));
}

void Unc2Channel::set_steady_state(double voltage_mV) {
    m_ = minf(voltage_mV);
    h_ = hinf(voltage_mV);
    initialized_ = true;
}

void Unc2Channel::step(double voltage_mV, double dt_ms) {
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

double Unc2Channel::current_pA(double voltage_mV) const {
    return conductance_nS_ * m_ * h_ * (voltage_mV - reversal_mV_);
}

std::string Unc2Channel::name() const {
    return "unc2";
}

std::vector<ChannelStateValue> Unc2Channel::state() const {
    return {{"gbunc2_nS", conductance_nS_}, {"m", m_}, {"h", h_}, {"eca_mV", reversal_mV_}};
}

std::unique_ptr<Channel> Unc2Channel::clone() const {
    return std::make_unique<Unc2Channel>(*this);
}

}  // namespace cpp_neuron
