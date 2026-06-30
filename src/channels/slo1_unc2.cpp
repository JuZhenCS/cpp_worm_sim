#include "neuron/channels/slo1_unc2.hpp"

#include <cmath>
#include <memory>

namespace neuron {

Slo1Unc2Channel::Slo1Unc2Channel(double conductance_nS) : conductance_nS_(conductance_nS) {}

void Slo1Unc2Channel::update_rates(double voltage_mV) {
    constexpr double vhm = -12.2;
    constexpr double ka = 4.0;
    constexpr double vhh = -52.5;
    constexpr double ki = 5.6;
    constexpr double atm = 1.5;
    constexpr double btm = -8.2;
    constexpr double ctm = 9.1;
    constexpr double dtm = 15.4;
    constexpr double etm = 0.1;
    constexpr double ath = 83.8;
    constexpr double bth = 52.9;
    constexpr double cth = -3.5;
    constexpr double dth = 72.1;
    constexpr double eth = 23.9;
    constexpr double fth = -3.6;

    constexpr double wyx = 0.013;
    constexpr double wxy = -0.028;
    constexpr double wom = 3.15;
    constexpr double wop = 0.16;
    constexpr double kxy = 55.73;
    constexpr double nxy = 1.30;
    constexpr double kyx = 34.34;
    constexpr double nyx = 0.0001;
    constexpr double canci = 0.05;
    constexpr double faraday = 96485.0;
    constexpr double gsc = 0.04;
    constexpr double r = 0.013;
    constexpr double dca = 250.0;
    constexpr double kb = 500.0;
    constexpr double btot = 30.0;
    constexpr double pi = 3.1415926;

    mcavinf_ = 1.0 / (1.0 + std::exp(-(voltage_mV - vhm) / ka));
    hcavinf_ = 1.0 / (1.0 + std::exp((voltage_mV - vhh) / ki));
    tmcav_ms_ = atm / (std::exp(-(voltage_mV - btm) / ctm) + std::exp((voltage_mV - btm) / dtm)) + etm;
    thcav_ms_ = ath / (1.0 + std::exp(-(voltage_mV - bth) / cth))
                + dth / (1.0 + std::exp((voltage_mV - eth) / fth));

    if (voltage_mV < 60.0) {
        cain_uM_per_um3_ =
            -gsc * (voltage_mV - 60.0) * 1.0e9 / (8.0 * pi * r * dca * faraday)
            * std::exp(-r / std::sqrt(dca / (kb * btot)));
    } else {
        cain_uM_per_um3_ = 0.0001;
    }

    const double alpha = mcavinf_ / tmcav_ms_;
    const double beta = 1.0 / tmcav_ms_ - alpha;
    const double wm = wom * std::exp(-wyx * voltage_mV);
    const double wp = wop * std::exp(-wxy * voltage_mV);
    const double fm = 1.0 / (1.0 + std::pow(cain_uM_per_um3_ / kyx, nyx));
    const double fp = 1.0 / (1.0 + std::pow(kxy / cain_uM_per_um3_, nxy));
    const double kom = wm * fm;
    const double kop = wp * fp;
    const double kcm = wm / (1.0 + std::pow(canci / kyx, nyx));
    const double denominator = (kop + kom) * (kcm + alpha) + beta * kcm;
    minf_ = mcav_ * kop * (alpha + beta + kcm) / denominator;
    tm_ms_ = (alpha + beta + kcm) / denominator;
}

void Slo1Unc2Channel::set_steady_state(double voltage_mV) {
    update_rates(voltage_mV);
    mcav_ = mcavinf_;
    hcav_ = hcavinf_;
    update_rates(voltage_mV);
    m_ = minf_;
    initialized_ = true;
}

void Slo1Unc2Channel::step(double voltage_mV, double dt_ms) {
    if (!initialized_) {
        update_rates(voltage_mV);
        hcav_ = hcavinf_;
        mcav_ = mcavinf_;
        m_ = 0.0;
        initialized_ = true;
        return;
    }
    update_rates(voltage_mV);
    const double old_mcav = mcav_;
    const double old_hcav = hcav_;
    const double old_m = m_;
    mcav_ = mcavinf_ + (old_mcav - mcavinf_) * std::exp(-dt_ms / tmcav_ms_);
    hcav_ = hcavinf_ + (old_hcav - hcavinf_) * std::exp(-dt_ms / thcav_ms_);
    m_ = minf_ + (old_m - minf_) * std::exp(-dt_ms / tm_ms_);
}

double Slo1Unc2Channel::current_pA(double voltage_mV) const {
    return conductance_nS_ * m_ * hcav_ * (voltage_mV + 80.0);
}

std::string Slo1Unc2Channel::name() const {
    return "slo1_unc2";
}

std::vector<ChannelStateValue> Slo1Unc2Channel::state() const {
    return {
        {"gbslo1_unc2_nS", conductance_nS_},
        {"m", m_},
        {"hcav", hcav_},
        {"mcav", mcav_},
        {"minf", minf_},
        {"tm_ms", tm_ms_},
        {"mcavinf", mcavinf_},
        {"tmcav_ms", tmcav_ms_},
        {"hcavinf", hcavinf_},
        {"thcav_ms", thcav_ms_},
        {"cain_uM_per_um3", cain_uM_per_um3_},
        {"ek_mV", -80.0},
    };
}

std::unique_ptr<Channel> Slo1Unc2Channel::clone() const {
    return std::make_unique<Slo1Unc2Channel>(*this);
}

}  // namespace neuron
