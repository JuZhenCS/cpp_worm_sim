#include "cpp_neuron_core/channels/kvs1.hpp"

#include <cmath>
#include <memory>

namespace cpp_neuron {

Kvs1Channel::Kvs1Channel(double conductance_nS) : conductance_nS_(conductance_nS) {}

double Kvs1Channel::minf(double voltage_mV) const {
    return 1.0 / (1.0 + std::exp(-(voltage_mV - 57.1) / 25.0));
}

double Kvs1Channel::hinf(double voltage_mV) const {
    return 1.0 / (1.0 + std::exp((voltage_mV - 47.3) / 11.1));
}

double Kvs1Channel::tau_m_ms(double voltage_mV) const {
    return 30.0 / (1.0 + std::exp((voltage_mV - 18.1) / 20.0)) + 1.0;
}

double Kvs1Channel::tau_h_ms(double voltage_mV) const {
    return 88.5 / (1.0 + std::exp((voltage_mV - 50.0) / 15.0)) + 53.4;
}

void Kvs1Channel::set_steady_state(double voltage_mV) {
    m_ = minf(voltage_mV);
    h_ = hinf(voltage_mV);
    initialized_ = true;
}

void Kvs1Channel::step(double voltage_mV, double dt_ms) {
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

double Kvs1Channel::current_pA(double voltage_mV) const {
    return conductance_nS_ * m_ * h_ * (voltage_mV + 80.0);
}

std::string Kvs1Channel::name() const {
    return "kvs1";
}

std::vector<ChannelStateValue> Kvs1Channel::state() const {
    return {{"gbkvs1_nS", conductance_nS_}, {"m", m_}, {"h", h_}, {"ek_mV", -80.0}};
}

std::unique_ptr<Channel> Kvs1Channel::clone() const {
    return std::make_unique<Kvs1Channel>(*this);
}

}  // namespace cpp_neuron
