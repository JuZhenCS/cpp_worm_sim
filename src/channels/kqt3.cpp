#include "neuron/channels/kqt3.hpp"

#include <cmath>
#include <memory>

namespace neuron {

Kqt3Channel::Kqt3Channel(double conductance_nS, double reversal_mV)
    : conductance_nS_(conductance_nS), reversal_mV_(reversal_mV) {}

double Kqt3Channel::minf(double voltage_mV) const {
    return 1.0 / (1.0 + std::exp(-(voltage_mV + 12.6726) / 15.8008));
}

double Kqt3Channel::tmf_ms(double voltage_mV) const {
    const double x = (voltage_mV + 38.1) / 33.59;
    return 395.3 / (1.0 + x * x);
}

double Kqt3Channel::tms_ms(double voltage_mV) const {
    return 5503.0 + (-5345.4) / (1.0 + std::pow(10.0, -0.02827 * (-23.9 - voltage_mV)))
           + (-4590.6) / (1.0 + std::pow(10.0, -0.0357 * (14.15 + voltage_mV)));
}

double Kqt3Channel::winf(double voltage_mV) const {
    return 0.49 + 0.51 / (1.0 + std::exp((voltage_mV + 1.084) / 28.78));
}

double Kqt3Channel::sinf(double voltage_mV) const {
    return 0.34 + 0.66 / (1.0 + std::exp((voltage_mV + 45.3) / 12.3));
}

double Kqt3Channel::tw_ms(double voltage_mV) const {
    const double x = (voltage_mV + 48.09) / 48.83;
    return 0.544 + 29.2 / (1.0 + x * x);
}

double Kqt3Channel::ts_ms(double /*voltage_mV*/) const {
    return 500000.0;
}

void Kqt3Channel::set_steady_state(double voltage_mV) {
    mf_ = minf(voltage_mV);
    ms_ = minf(voltage_mV);
    w_ = winf(voltage_mV);
    s_ = sinf(voltage_mV);
    initialized_ = true;
}

void Kqt3Channel::step(double voltage_mV, double dt_ms) {
    if (!initialized_) {
        mf_ = 0.0;
        ms_ = 0.0;
        w_ = winf(voltage_mV);
        s_ = sinf(voltage_mV);
        initialized_ = true;
    }
    const double m_target = minf(voltage_mV);
    mf_ = m_target + (mf_ - m_target) * std::exp(-dt_ms / tmf_ms(voltage_mV));
    ms_ = m_target + (ms_ - m_target) * std::exp(-dt_ms / tms_ms(voltage_mV));
    w_ = winf(voltage_mV) + (w_ - winf(voltage_mV)) * std::exp(-dt_ms / tw_ms(voltage_mV));
    s_ = sinf(voltage_mV) + (s_ - sinf(voltage_mV)) * std::exp(-dt_ms / ts_ms(voltage_mV));
}

double Kqt3Channel::current_pA(double voltage_mV) const {
    return conductance_nS_ * (0.3 * mf_ + 0.7 * ms_) * w_ * s_ * (voltage_mV - reversal_mV_);
}

std::string Kqt3Channel::name() const {
    return "kqt3";
}

std::vector<ChannelStateValue> Kqt3Channel::state() const {
    return {
        {"gbkqt3_nS", conductance_nS_},
        {"mf", mf_},
        {"ms", ms_},
        {"w", w_},
        {"s", s_},
        {"ek_mV", reversal_mV_},
    };
}

std::unique_ptr<Channel> Kqt3Channel::clone() const {
    return std::make_unique<Kqt3Channel>(*this);
}

}  // namespace neuron
