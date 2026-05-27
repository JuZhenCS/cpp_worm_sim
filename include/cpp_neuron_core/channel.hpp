#pragma once

#include <memory>
#include <string>
#include <vector>

namespace cpp_neuron {

struct ChannelStateValue {
    std::string name;
    double value = 0.0;
};

class Channel {
public:
    virtual ~Channel() = default;

    virtual void step(double voltage_mV, double dt_ms) = 0;
    virtual void set_calcium(double /*cai_uM_per_um2*/) {}
    virtual double current_pA(double voltage_mV) const = 0;
    virtual std::string name() const = 0;
    virtual std::vector<ChannelStateValue> state() const { return {}; }
    virtual std::unique_ptr<Channel> clone() const = 0;
};

}  // namespace cpp_neuron
