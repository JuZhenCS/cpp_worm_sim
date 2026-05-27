#include "cpp_neuron_core/channels/nca.hpp"

namespace cpp_neuron {

NcaChannel::NcaChannel(double conductance_nS, double reversal_mV)
    : conductance_nS_(conductance_nS), reversal_mV_(reversal_mV) {}

void NcaChannel::step(double /*voltage_mV*/, double /*dt_ms*/) {
    // nca.mod has no dynamic gate state.
}

double NcaChannel::current_pA(double voltage_mV) const {
    return conductance_nS_ * (voltage_mV - reversal_mV_);
}

std::string NcaChannel::name() const {
    return "nca";
}

std::vector<ChannelStateValue> NcaChannel::state() const {
    return {{"gbnca_nS", conductance_nS_}, {"ena_mV", reversal_mV_}};
}

std::unique_ptr<Channel> NcaChannel::clone() const {
    return std::make_unique<NcaChannel>(*this);
}

}  // namespace cpp_neuron
