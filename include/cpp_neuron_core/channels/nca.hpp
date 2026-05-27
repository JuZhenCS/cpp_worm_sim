#pragma once

#include "cpp_neuron_core/channel.hpp"

namespace cpp_neuron {

class NcaChannel final : public Channel {
public:
    explicit NcaChannel(double conductance_nS, double reversal_mV = 30.0);

    void step(double voltage_mV, double dt_ms) override;
    double current_pA(double voltage_mV) const override;
    std::string name() const override;
    std::vector<ChannelStateValue> state() const override;
    std::unique_ptr<Channel> clone() const override;

    double conductance_nS() const { return conductance_nS_; }
    double reversal_mV() const { return reversal_mV_; }

private:
    double conductance_nS_ = 0.0;
    double reversal_mV_ = 30.0;
};

}  // namespace cpp_neuron
