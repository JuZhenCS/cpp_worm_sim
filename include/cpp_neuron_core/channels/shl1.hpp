#pragma once

#include "cpp_neuron_core/channel.hpp"

namespace cpp_neuron {

class Shl1Channel final : public Channel {
public:
    explicit Shl1Channel(double conductance_nS);

    void step(double voltage_mV, double dt_ms) override;
    double current_pA(double voltage_mV) const override;
    std::string name() const override;
    std::vector<ChannelStateValue> state() const override;
    std::unique_ptr<Channel> clone() const override;

    void set_steady_state(double voltage_mV);

    double m() const { return m_; }
    double hf() const { return hf_; }
    double hs() const { return hs_; }
    double minf(double voltage_mV) const;
    double hfinf(double voltage_mV) const;
    double hsinf(double voltage_mV) const;
    double tau_m_base_ms(double voltage_mV) const;
    double tau_hf_base_ms(double voltage_mV) const;
    double tau_hs_base_ms(double voltage_mV) const;

private:
    double conductance_nS_ = 0.0;
    double m_ = 0.0;
    double hf_ = 0.0;
    double hs_ = 0.0;
    bool initialized_ = false;
};

}  // namespace cpp_neuron
