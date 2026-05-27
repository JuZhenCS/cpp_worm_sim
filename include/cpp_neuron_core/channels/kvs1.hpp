#pragma once

#include "cpp_neuron_core/channel.hpp"

namespace cpp_neuron {

class Kvs1Channel final : public Channel {
public:
    explicit Kvs1Channel(double conductance_nS);

    void step(double voltage_mV, double dt_ms) override;
    double current_pA(double voltage_mV) const override;
    std::string name() const override;
    std::vector<ChannelStateValue> state() const override;
    std::unique_ptr<Channel> clone() const override;

    void set_steady_state(double voltage_mV);

    double m() const { return m_; }
    double h() const { return h_; }
    double minf(double voltage_mV) const;
    double hinf(double voltage_mV) const;
    double tau_m_ms(double voltage_mV) const;
    double tau_h_ms(double voltage_mV) const;

private:
    double conductance_nS_ = 0.0;
    double m_ = 0.0;
    double h_ = 0.0;
    bool initialized_ = false;
};

}  // namespace cpp_neuron
