#pragma once

#include "neuron/channel.hpp"

namespace neuron {

class Kqt3Channel final : public Channel {
public:
    explicit Kqt3Channel(double conductance_nS, double reversal_mV = -80.0);

    void step(double voltage_mV, double dt_ms) override;
    double current_pA(double voltage_mV) const override;
    std::string name() const override;
    std::vector<ChannelStateValue> state() const override;
    std::unique_ptr<Channel> clone() const override;

    void set_steady_state(double voltage_mV);

    double minf(double voltage_mV) const;
    double tmf_ms(double voltage_mV) const;
    double tms_ms(double voltage_mV) const;
    double winf(double voltage_mV) const;
    double sinf(double voltage_mV) const;
    double tw_ms(double voltage_mV) const;
    double ts_ms(double voltage_mV) const;

    double mf() const { return mf_; }
    double ms() const { return ms_; }
    double w() const { return w_; }
    double s() const { return s_; }

private:
    double conductance_nS_ = 0.0;
    double reversal_mV_ = -80.0;
    double mf_ = 0.0;
    double ms_ = 0.0;
    double w_ = 0.0;
    double s_ = 0.0;
    bool initialized_ = false;
};

}  // namespace neuron
