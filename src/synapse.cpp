#include "synapse/synapse.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <utility>

namespace neuron {
namespace {

constexpr double kMicroSiemensMillivoltToPicoamp = 1000.0; // μS · mV → pA

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
    bool active,
    std::string label)
    : pre_(std::move(pre)),
      post_(std::move(post)),
      pre_compartment_(pre_compartment),
      post_compartment_(post_compartment),
      component_type_(component_type),
      config_(config),
      active_(active),
      label_(std::move(label)) {
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
    if (dt_ms <= 0.0) { // 检查时间步长必须为正数
        throw std::runtime_error("Chemical component dt_ms must be positive"); // 时间步长非法时抛出错误
    }
    if (!active_) { // 如果这个化学突触没有启用
        last_current_pA_ = 0.0; // 记录本次电流为 0
        return 0.0; // 不产生电流，直接返回 0
    }

    const double v_pre = pre_->voltage_mV(pre_compartment_); // 读取前突触神经元指定 compartment 的电压
    const double v_post = post_->voltage_mV(post_compartment_); // 读取后突触神经元指定 compartment 的电压
    s_ += dt_ms * (s_inf(v_pre) - s_) / config_.tau_ms; // 按一阶动力学更新突触门控变量 s_
    s_ = std::clamp(s_, 0.0, 1.0); // 把 s_ 限制在 0 到 1 之间

    // uS * mV = nA, so multiply by 1000 to inject pA into NeuronModel.
    const double current = kMicroSiemensMillivoltToPicoamp * config_.g_uS * s_ * (config_.e_rev_mV - v_post); // 计算本时间步的化学突触电流，单位 pA
    last_current_pA_ = current; // 保存最近一次计算得到的电流
    const double abs_current = std::abs(current); // 计算电流绝对值，方便统计峰值大小
    if (abs_current > peak_abs_current_pA_) { // 如果当前绝对电流超过历史峰值
        peak_abs_current_pA_ = abs_current; // 更新历史最大绝对电流
        peak_current_pA_ = current; // 保存达到峰值时的带符号电流
    }
    return current; // 返回本次计算得到的突触电流
}

void GradedChemicalSynapse::step(double dt_ms) {
    (void)apply_and_current_pA(dt_ms);
}

double GradedChemicalSynapse::apply_and_current_pA(double dt_ms) {
    const double current = update_and_current_pA(dt_ms);
    post_->add_current_pA(post_compartment_, current); // add_current_pA(...)是神经元对象的函数，作用是把外部输入电流累加到指定 compartment，后面神经元 step() 更新电压时会用到这些输入电流。把这个化学突触产生的电流施加到后突触神经元的指定 compartment 上。
    return current;
}

GapJunction::GapJunction(
    std::shared_ptr<NeuronModel> a,
    std::shared_ptr<NeuronModel> b,
    std::size_t comp_a,
    std::size_t comp_b,
    GapJunctionConfig config,
    bool active,
    std::string label)
    : a_(std::move(a)),
      b_(std::move(b)),
      comp_a_(comp_a),
      comp_b_(comp_b),
      config_(config),
      active_(active),
      label_(std::move(label)) {
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
    (void)apply_and_current_to_a_pA();
}

double GapJunction::apply_and_current_to_a_pA() {
    const double i_to_a = current_to_a_pA();
    last_current_to_a_pA_ = i_to_a;
    const double abs_current = std::abs(i_to_a);
    if (abs_current > peak_abs_current_pA_) {
        peak_abs_current_pA_ = abs_current;
        peak_current_to_a_pA_ = i_to_a;
    }
    a_->add_current_pA(comp_a_, i_to_a);
    b_->add_current_pA(comp_b_, -i_to_a);
    return i_to_a;
}

}  // namespace neuron
