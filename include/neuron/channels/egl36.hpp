#pragma once

#include "neuron/channel.hpp"

namespace neuron {

class Egl36Channel final : public Channel {
public:
    explicit Egl36Channel(double conductance_nS);

    void step(double voltage_mV, double dt_ms) override;
    double current_pA(double voltage_mV) const override;
    std::string name() const override;
    std::vector<ChannelStateValue> state() const override;
    std::unique_ptr<Channel> clone() const override;

    void set_steady_state(double voltage_mV);

    double mf() const { return mf_; }
    double mm() const { return mm_; }
    double ms() const { return ms_; }
    double minf(double voltage_mV) const;

private:
    double conductance_nS_ = 0.0;
    double mf_ = 0.0;
    double mm_ = 0.0;
    double ms_ = 0.0;
    bool initialized_ = false;
};

}  // namespace neuron
