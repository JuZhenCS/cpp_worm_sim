#include "neuron/channels/egl36.hpp"

#include <cmath>
#include <memory>

namespace neuron {

Egl36Channel::Egl36Channel(double conductance_nS) : conductance_nS_(conductance_nS) {}

double Egl36Channel::minf(double voltage_mV) const {
    return 1.0 / (1.0 + std::exp(-(voltage_mV - 63.0) / 28.5));
}

void Egl36Channel::set_steady_state(double voltage_mV) {
    mf_ = minf(voltage_mV);
    mm_ = minf(voltage_mV);
    ms_ = minf(voltage_mV);
    initialized_ = true;
}

void Egl36Channel::step(double voltage_mV, double dt_ms) {
    const double target = minf(voltage_mV);
    if (!initialized_) {
        set_steady_state(voltage_mV);
        return;
    }
    mf_ = target + (mf_ - target) * std::exp(-dt_ms / 13.0);
    mm_ = target + (mm_ - target) * std::exp(-dt_ms / 63.0);
    ms_ = target + (ms_ - target) * std::exp(-dt_ms / 355.0);
}

double Egl36Channel::current_pA(double voltage_mV) const {
    return conductance_nS_ * (0.33 * mf_ + 0.36 * mm_ + 0.39 * ms_) * (voltage_mV + 80.0);
}

std::string Egl36Channel::name() const {
    return "egl36";
}

std::vector<ChannelStateValue> Egl36Channel::state() const {
    return {{"gbegl36_nS", conductance_nS_}, {"mf", mf_}, {"mm", mm_}, {"ms", ms_}, {"ek_mV", -80.0}};
}

std::unique_ptr<Channel> Egl36Channel::clone() const {
    return std::make_unique<Egl36Channel>(*this);
}

}  // namespace neuron
