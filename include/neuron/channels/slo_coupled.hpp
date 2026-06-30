#pragma once

#include "neuron/channel.hpp"

namespace neuron {

enum class SloKind { Slo1, Slo2 };
enum class CalciumPartner { Egl19, Unc2 };

class SloCoupledChannel final : public Channel {
public:
    SloCoupledChannel(std::string channel_name, double conductance_nS, SloKind slo_kind, CalciumPartner partner);

    void step(double voltage_mV, double dt_ms) override;
    double current_pA(double voltage_mV) const override;
    std::string name() const override;
    std::vector<ChannelStateValue> state() const override;
    std::unique_ptr<Channel> clone() const override;

    void set_steady_state(double voltage_mV);

    double m() const { return m_; }
    double hcav() const { return hcav_; }
    double mcav() const { return mcav_; }
    double minf() const { return minf_; }
    double tm_ms() const { return tm_ms_; }
    double mcavinf() const { return mcavinf_; }
    double tmcav_ms() const { return tmcav_ms_; }
    double hcavinf() const { return hcavinf_; }
    double thcav_ms() const { return thcav_ms_; }
    double cain_uM_per_um2() const { return cain_uM_per_um2_; }

private:
    void update_rates(double voltage_mV);

    std::string channel_name_;
    double conductance_nS_ = 0.0;
    SloKind slo_kind_ = SloKind::Slo1;
    CalciumPartner partner_ = CalciumPartner::Unc2;
    double m_ = 0.0;
    double hcav_ = 0.0;
    double mcav_ = 0.0;
    double minf_ = 0.0;
    double tm_ms_ = 1.0;
    double mcavinf_ = 0.0;
    double tmcav_ms_ = 1.0;
    double hcavinf_ = 0.0;
    double thcav_ms_ = 1.0;
    double cain_uM_per_um2_ = 0.0001;
    bool initialized_ = false;
};

}  // namespace neuron
