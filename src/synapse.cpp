#include "cpp_neuron_core/synapse.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <utility>

namespace cpp_neuron {
namespace {

constexpr double kMicroSiemensMillivoltToPicoamp = 1000.0;

void require_neuron(const std::shared_ptr<NeuronModel>& neuron, const char* label) {
    if (!neuron) {
        throw std::runtime_error(std::string(label) + " neuron pointer is null");
    }
}

void require_compartment(const NeuronModel& neuron, std::size_t compartment, const char* label) {
    if (compartment >= neuron.size()) {
        throw std::runtime_error(std::string(label) + " compartment index out of range");
    }
}

}  // namespace

GradedChemicalSynapse::GradedChemicalSynapse(
    std::shared_ptr<NeuronModel> pre,
    std::shared_ptr<NeuronModel> post,
    std::size_t pre_compartment,
    std::size_t post_compartment,
    ChemicalComponentType component_type,
    GradedChemicalSynapseConfig config,
    bool active)
    : pre_(std::move(pre)),
      post_(std::move(post)),
      pre_compartment_(pre_compartment),
      post_compartment_(post_compartment),
      component_type_(component_type),
      config_(config),
      active_(active) {
    require_neuron(pre_, "Presynaptic");
    require_neuron(post_, "Postsynaptic");
    require_compartment(*pre_, pre_compartment_, "Presynaptic");
    require_compartment(*post_, post_compartment_, "Postsynaptic");
    if (config_.g_uS < 0.0) {
        throw std::runtime_error("Chemical component conductance must be nonnegative");
    }
    if (config_.tau_ms <= 0.0) {
        throw std::runtime_error("Chemical component tau_ms must be positive");
    }
    if (config_.k_s_mV == 0.0) {
        throw std::runtime_error("Chemical component k_s_mV must be nonzero");
    }
}

double GradedChemicalSynapse::s_inf(double v_pre_mV) const {
    return 1.0 / (1.0 + std::exp(-(v_pre_mV - config_.v_half_mV) / config_.k_s_mV));
}

double GradedChemicalSynapse::update_and_current_pA(double dt_ms) {
    if (dt_ms <= 0.0) {
        throw std::runtime_error("Chemical component dt_ms must be positive");
    }
    if (!active_) {
        return 0.0;
    }

    const double v_pre = pre_->voltage_mV(pre_compartment_);
    const double v_post = post_->voltage_mV(post_compartment_);
    s_ += dt_ms * (s_inf(v_pre) - s_) / config_.tau_ms;
    s_ = std::clamp(s_, 0.0, 1.0);

    // uS * mV = nA, so multiply by 1000 to inject pA into NeuronModel.
    return kMicroSiemensMillivoltToPicoamp * config_.g_uS * s_ * (config_.e_rev_mV - v_post);
}

void GradedChemicalSynapse::step(double dt_ms) {
    (void)apply_and_current_pA(dt_ms);
}

double GradedChemicalSynapse::apply_and_current_pA(double dt_ms) {
    const double current = update_and_current_pA(dt_ms);
    post_->add_current_pA(post_compartment_, current);
    return current;
}

GapJunction::GapJunction(
    std::shared_ptr<NeuronModel> a,
    std::shared_ptr<NeuronModel> b,
    std::size_t comp_a,
    std::size_t comp_b,
    GapJunctionConfig config,
    bool active)
    : a_(std::move(a)),
      b_(std::move(b)),
      comp_a_(comp_a),
      comp_b_(comp_b),
      config_(config),
      active_(active) {
    require_neuron(a_, "Gap junction first");
    require_neuron(b_, "Gap junction second");
    require_compartment(*a_, comp_a_, "Gap junction first");
    require_compartment(*b_, comp_b_, "Gap junction second");
    if (config_.g_uS < 0.0) {
        throw std::runtime_error("Gap junction conductance must be nonnegative");
    }
}

double GapJunction::current_to_a_pA() const {
    if (!active_) {
        return 0.0;
    }
    const double v_a = a_->voltage_mV(comp_a_);
    const double v_b = b_->voltage_mV(comp_b_);
    return kMicroSiemensMillivoltToPicoamp * config_.g_uS * (v_b - v_a);
}

void GapJunction::apply() {
    const double i_to_a = current_to_a_pA();
    a_->add_current_pA(comp_a_, i_to_a);
    b_->add_current_pA(comp_b_, -i_to_a);
}

}  // namespace cpp_neuron
