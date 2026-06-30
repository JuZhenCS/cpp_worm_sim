#include "neuron/channels/slo_coupled.hpp"

#include <cmath>
#include <memory>
#include <utility>

namespace neuron {

SloCoupledChannel::SloCoupledChannel(
    std::string channel_name,
    double conductance_nS,
    SloKind slo_kind,
    CalciumPartner partner)
    : channel_name_(std::move(channel_name)),
      conductance_nS_(conductance_nS),
      slo_kind_(slo_kind),
      partner_(partner) {}

void SloCoupledChannel::update_rates(double voltage_mV) {
    if (partner_ == CalciumPartner::Egl19) {
        constexpr double vhm = 5.6;
        constexpr double ka = 7.5;
        constexpr double vhh = 24.9;
        constexpr double ki = 12.0;
        constexpr double vhhb = -20.5;
        constexpr double kib = 8.1;
        constexpr double ahinf = 1.43;
        constexpr double bhinf = 0.14;
        constexpr double chinf = 5.96;
        constexpr double dhinf = 0.60;
        constexpr double atm = 2.9;
        constexpr double btm = 5.2;
        constexpr double ctm = 6.0;
        constexpr double dtm = 1.9;
        constexpr double etm = 1.4;
        constexpr double ftm = 30.0;
        constexpr double gtm = 2.3;
        constexpr double ath = 0.4;
        constexpr double bth = 44.6;
        constexpr double cth = -23.0;
        constexpr double dth = 5.0;
        constexpr double eth = 36.4;
        constexpr double fth = 28.7;
        constexpr double gth = 3.7;
        constexpr double hth = 43.1;
        mcavinf_ = 1.0 / (1.0 + std::exp(-(voltage_mV - vhm) / ka));
        hcavinf_ = (ahinf / (1.0 + std::exp(-(voltage_mV - vhh) / ki)) + bhinf)
                   * (chinf / (1.0 + std::exp((voltage_mV - vhhb) / kib)) + dhinf);
        tmcav_ms_ = atm * std::exp(-std::pow((voltage_mV - btm) / ctm, 2.0))
                    + dtm * std::exp(-std::pow((voltage_mV - etm) / ftm, 2.0)) + gtm;
        thcav_ms_ = ath
                    * ((bth / (1.0 + std::exp((voltage_mV - cth) / dth)))
                       + (eth / (1.0 + std::exp((voltage_mV - fth) / gth))) + hth);
    } else {
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
        mcavinf_ = 1.0 / (1.0 + std::exp(-(voltage_mV - vhm) / ka));
        hcavinf_ = 1.0 / (1.0 + std::exp((voltage_mV - vhh) / ki));
        tmcav_ms_ = atm / (std::exp(-(voltage_mV - btm) / ctm) + std::exp((voltage_mV - btm) / dtm)) + etm;
        thcav_ms_ = ath / (1.0 + std::exp(-(voltage_mV - bth) / cth))
                    + dth / (1.0 + std::exp((voltage_mV - eth) / fth));
    }

    constexpr double faraday = 96485.0;
    constexpr double gsc = 0.04;
    constexpr double r = 0.013;
    constexpr double dca = 250.0;
    constexpr double kb = 500.0;
    constexpr double btot = 30.0;
    constexpr double pi = 3.1415926;
    cain_uM_per_um2_ = voltage_mV < 60.0
                           ? -gsc * (voltage_mV - 60.0) * 1.0e9 / (8.0 * pi * r * dca * faraday)
                                 * std::exp(-r / std::sqrt(dca / (kb * btot)))
                           : 0.0001;

    const double wyx = slo_kind_ == SloKind::Slo1 ? 0.013 : 0.019;
    const double wxy = slo_kind_ == SloKind::Slo1 ? -0.028 : -0.024;
    const double wom = slo_kind_ == SloKind::Slo1 ? 3.15 : 0.90;
    const double wop = slo_kind_ == SloKind::Slo1 ? 0.16 : 0.027;
    const double kxy = slo_kind_ == SloKind::Slo1 ? 55.73 : 93.45;
    const double nxy = slo_kind_ == SloKind::Slo1 ? 1.30 : 1.84;
    const double kyx = slo_kind_ == SloKind::Slo1 ? 34.34 : 3294.55;
    const double nyx = slo_kind_ == SloKind::Slo1 ? 0.0001 : 0.00001;
    constexpr double canci = 0.05;

    const double alpha = mcavinf_ / tmcav_ms_;
    const double beta = 1.0 / tmcav_ms_ - alpha;
    const double wm = wom * std::exp(-wyx * voltage_mV);
    const double wp = wop * std::exp(-wxy * voltage_mV);
    const double fm = 1.0 / (1.0 + std::pow(cain_uM_per_um2_ / kyx, nyx));
    const double fp = 1.0 / (1.0 + std::pow(kxy / cain_uM_per_um2_, nxy));
    const double kom = wm * fm;
    const double kop = wp * fp;
    const double kcm = wm / (1.0 + std::pow(canci / kyx, nyx));
    const double denominator = (kop + kom) * (kcm + alpha) + beta * kcm;
    minf_ = mcav_ * kop * (alpha + beta + kcm) / denominator;
    tm_ms_ = (alpha + beta + kcm) / denominator;
}

void SloCoupledChannel::set_steady_state(double voltage_mV) {
    update_rates(voltage_mV);
    mcav_ = mcavinf_;
    hcav_ = hcavinf_;
    update_rates(voltage_mV);
    m_ = minf_;
    initialized_ = true;
}

void SloCoupledChannel::step(double voltage_mV, double dt_ms) {
    if (!initialized_) {
        update_rates(voltage_mV);
        hcav_ = hcavinf_;
        mcav_ = mcavinf_;
        update_rates(voltage_mV);
        m_ = 0.0;
        initialized_ = true;
        return;
    }
    update_rates(voltage_mV);
    mcav_ = mcavinf_ + (mcav_ - mcavinf_) * std::exp(-dt_ms / tmcav_ms_);
    hcav_ = hcavinf_ + (hcav_ - hcavinf_) * std::exp(-dt_ms / thcav_ms_);
    m_ = minf_ + (m_ - minf_) * std::exp(-dt_ms / tm_ms_);
}

double SloCoupledChannel::current_pA(double voltage_mV) const {
    return conductance_nS_ * m_ * hcav_ * (voltage_mV + 80.0);
}

std::string SloCoupledChannel::name() const {
    return channel_name_;
}

std::vector<ChannelStateValue> SloCoupledChannel::state() const {
    return {
        {"conductance_nS", conductance_nS_},
        {"m", m_},
        {"hcav", hcav_},
        {"mcav", mcav_},
        {"minf", minf_},
        {"tm_ms", tm_ms_},
        {"mcavinf", mcavinf_},
        {"tmcav_ms", tmcav_ms_},
        {"hcavinf", hcavinf_},
        {"thcav_ms", thcav_ms_},
        {"cain_uM_per_um2", cain_uM_per_um2_},
    };
}

std::unique_ptr<Channel> SloCoupledChannel::clone() const {
    return std::make_unique<SloCoupledChannel>(*this);
}

}  // namespace neuron
