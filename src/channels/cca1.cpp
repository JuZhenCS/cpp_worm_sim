#include "cpp_neuron_core/channels/cca1.hpp"

#include <cmath>
#include <memory>

namespace cpp_neuron {

Cca1Channel::Cca1Channel(double conductance_nS, double reversal_mV)
    : conductance_nS_(conductance_nS), reversal_mV_(reversal_mV) {}

double Cca1Channel::minf(double voltage_mV) const {
    return 1.0 / (1.0 + std::exp(-(voltage_mV + 43.32) / 7.6));
}

double Cca1Channel::hinf(double voltage_mV) const {
    return 1.0 / (1.0 + std::exp((voltage_mV + 58.0) / 7.0));
}

double Cca1Channel::tau_m_ms(double voltage_mV) const {
    return 40.0 / (1.0 + std::exp(-(voltage_mV + 62.5) / -12.6)) + 0.7;
}

double Cca1Channel::tau_h_ms(double voltage_mV) const {
    return 280.0 / (1.0 + std::exp((voltage_mV + 60.7) / 8.5)) + 19.8;
}

void Cca1Channel::set_steady_state(double voltage_mV) {
    m_ = minf(voltage_mV);
    h_ = hinf(voltage_mV);
    initialized_ = true;
}

void Cca1Channel::step(double voltage_mV, double dt_ms) {
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

double Cca1Channel::current_pA(double voltage_mV) const {
    return conductance_nS_ * m_ * m_ * h_ * (voltage_mV - reversal_mV_);
}

std::string Cca1Channel::name() const {
    return "cca1";
}

std::vector<ChannelStateValue> Cca1Channel::state() const {
    return {{"gbcca1_nS", conductance_nS_}, {"m", m_}, {"h", h_}, {"eca_mV", reversal_mV_}};
}

std::unique_ptr<Channel> Cca1Channel::clone() const {
    return std::make_unique<Cca1Channel>(*this);
}

}  // namespace cpp_neuron
