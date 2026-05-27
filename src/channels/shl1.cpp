#include "cpp_neuron_core/channels/shl1.hpp"

#include <cmath>
#include <memory>

namespace cpp_neuron {

Shl1Channel::Shl1Channel(double conductance_nS) : conductance_nS_(conductance_nS) {}

double Shl1Channel::minf(double voltage_mV) const {
    return 1.0 / (1.0 + std::exp(-(voltage_mV - 11.2) / 14.1));
}

double Shl1Channel::hfinf(double voltage_mV) const {
    return 1.0 / (1.0 + std::exp((voltage_mV + 33.1) / 8.3));
}

double Shl1Channel::hsinf(double voltage_mV) const {
    return hfinf(voltage_mV);
}

double Shl1Channel::tau_m_base_ms(double voltage_mV) const {
    return 13.8 / (std::exp(-(voltage_mV + 17.5) / 12.9) + std::exp((voltage_mV + 3.7) / 6.5)) + 1.9;
}

double Shl1Channel::tau_hf_base_ms(double voltage_mV) const {
    return 539.2 / (1.0 + std::exp((voltage_mV + 28.2) / 4.9)) + 27.3;
}

double Shl1Channel::tau_hs_base_ms(double voltage_mV) const {
    return 8422.0 / (1.0 + std::exp((voltage_mV + 37.7) / 6.4)) + 118.9;
}

void Shl1Channel::set_steady_state(double voltage_mV) {
    m_ = minf(voltage_mV);
    hf_ = hfinf(voltage_mV);
    hs_ = hsinf(voltage_mV);
    initialized_ = true;
}

void Shl1Channel::step(double voltage_mV, double dt_ms) {
    const double m_target = minf(voltage_mV);
    const double hf_target = hfinf(voltage_mV);
    const double hs_target = hsinf(voltage_mV);
    if (!initialized_) {
        m_ = m_target;
        hf_ = hf_target;
        hs_ = hs_target;
        initialized_ = true;
        return;
    }
    m_ = m_target + (m_ - m_target) * std::exp(-dt_ms / (tau_m_base_ms(voltage_mV) * 0.4));
    hf_ = hf_target + (hf_ - hf_target) * std::exp(-dt_ms / (tau_hf_base_ms(voltage_mV) * 0.08));
    hs_ = hs_target + (hs_ - hs_target) * std::exp(-dt_ms / (tau_hs_base_ms(voltage_mV) * 0.3));
}

double Shl1Channel::current_pA(double voltage_mV) const {
    const double h_mix = 0.7 * hf_ + 0.3 * hs_;
    return conductance_nS_ * m_ * m_ * m_ * h_mix * (voltage_mV + 80.0);
}

std::string Shl1Channel::name() const {
    return "shl1";
}

std::vector<ChannelStateValue> Shl1Channel::state() const {
    return {{"gbshl1_nS", conductance_nS_}, {"m", m_}, {"hf", hf_}, {"hs", hs_}, {"ek_mV", -80.0}};
}

std::unique_ptr<Channel> Shl1Channel::clone() const {
    return std::make_unique<Shl1Channel>(*this);
}

}  // namespace cpp_neuron
