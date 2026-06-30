#pragma once

#include "neuron/channel.hpp"

namespace neuron {

class KcnlChannel final : public Channel {
public:
    explicit KcnlChannel(double conductance_nS);

    void set_calcium(double cai_uM_per_um2) override;
    void step(double voltage_mV, double dt_ms) override;
    double current_pA(double voltage_mV) const override;
    std::string name() const override;
    std::vector<ChannelStateValue> state() const override;
    std::unique_ptr<Channel> clone() const override;

    void set_steady_state(double cai_uM_per_um2);
    double m() const { return m_; }
    double minf(double cai_uM_per_um2) const;
    double tau_m_ms() const { return 6.3; }

private:
    double conductance_nS_ = 0.0;
    double cai_uM_per_um2_ = 0.05;
    double m_ = 0.0;
    bool initialized_ = false;
};

}  // namespace neuron
