#pragma once

#include "neuron/core/neuron_model.hpp"

#include <cstddef>
#include <memory>
#include <string>

namespace neuron {

enum class ChemicalComponentType {
    Excitatory,
    Inhibitory
};

struct GradedChemicalSynapseConfig {
    double g_uS = 0.0;
    double tau_ms = 10.0;
    double v_half_mV = -20.0;
    double k_s_mV = 5.0;
    double e_rev_mV = 30.0;
};

class GradedChemicalSynapse {
public:
    GradedChemicalSynapse(
        std::shared_ptr<NeuronModel> pre,
        std::shared_ptr<NeuronModel> post,
        std::size_t pre_compartment,
        std::size_t post_compartment,
        ChemicalComponentType component_type,
        GradedChemicalSynapseConfig config,
        bool active = true,
        std::string label = {});

    double update_and_current_pA(double dt_ms);
    double apply_and_current_pA(double dt_ms);
    void step(double dt_ms);

    double s() const { return s_; }
    double effective_conductance_uS() const { return config_.g_uS; }
    bool active() const { return active_; }
    ChemicalComponentType component_type() const { return component_type_; }
    const std::string& label() const { return label_; }
    double last_current_pA() const { return last_current_pA_; }
    double peak_abs_current_pA() const { return peak_abs_current_pA_; }
    double peak_current_pA() const { return peak_current_pA_; }

private:
    std::shared_ptr<NeuronModel> pre_;
    std::shared_ptr<NeuronModel> post_;
    std::size_t pre_compartment_ = 0;
    std::size_t post_compartment_ = 0;
    ChemicalComponentType component_type_ = ChemicalComponentType::Excitatory;
    GradedChemicalSynapseConfig config_;
    bool active_ = true;
    std::string label_;
    double s_ = 0.0;
    double last_current_pA_ = 0.0;
    double peak_abs_current_pA_ = 0.0;
    double peak_current_pA_ = 0.0;

    double s_inf(double v_pre_mV) const;
};

struct GapJunctionConfig {
    double g_uS = 0.0;
};

class GapJunction {
public:
    GapJunction(
        std::shared_ptr<NeuronModel> a,
        std::shared_ptr<NeuronModel> b,
        std::size_t comp_a,
        std::size_t comp_b,
        GapJunctionConfig config,
        bool active = true,
        std::string label = {});

    void apply();
    double apply_and_current_to_a_pA();

    double current_to_a_pA() const;
    double effective_conductance_uS() const { return config_.g_uS; }
    bool active() const { return active_; }
    const std::string& label() const { return label_; }
    double last_current_to_a_pA() const { return last_current_to_a_pA_; }
    double peak_abs_current_pA() const { return peak_abs_current_pA_; }
    double peak_current_to_a_pA() const { return peak_current_to_a_pA_; }

private:
    std::shared_ptr<NeuronModel> a_;
    std::shared_ptr<NeuronModel> b_;
    std::size_t comp_a_ = 0;
    std::size_t comp_b_ = 0;
    GapJunctionConfig config_;
    bool active_ = true;
    std::string label_;
    double last_current_to_a_pA_ = 0.0;
    double peak_abs_current_pA_ = 0.0;
    double peak_current_to_a_pA_ = 0.0;
};

}  // namespace neuron
