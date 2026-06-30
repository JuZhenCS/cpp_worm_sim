#pragma once

#include "neuron/neuron/cable_solver.hpp"
#include "neuron/neuron/cell.hpp"
#include "neuron/neuron/neuron_model.hpp"

#include <string>
#include <vector>

namespace neuron {

// 当前运行时使用的多隔室神经元实现。
// 它可以按配置挂载 active channels，并承担单细胞 runner 的主要动力学计算。
class MultiCompartmentNeuron : public NeuronModel {
public:
    // Cell 由 cell_loader 从 CSV 读入，包含 compartment 参数、轴向连接和通道 conductance。
    explicit MultiCompartmentNeuron(Cell cell);

    const std::string& name() const override { return cell_.name; }
    void set_all_voltages(double voltage_mV) override;
    void add_current_pA(std::size_t compartment_index, double current_pA) override;
    void clear_currents() override;

    // 使用已累积的 injected_current_pA_ 前进一步，随后清空外部注入电流。
    void step(double dt_ms) override;

    // current-clamp 等调用路径直接传入每个 compartment 的注入电流。
    void step(double dt_ms, const std::vector<double>& injected_current_pA);

    // voltage-clamp 使用这个版本：额外给某些 compartment 加 conductance 和 reversal/command voltage。
    void step_with_conductance(
        double dt_ms,
        const std::vector<double>& injected_current_pA,
        const std::vector<double>& extra_conductance_nS,
        const std::vector<double>& extra_reversal_mV);

    // 暴露底层 Cell，供 protocol/diagnostics 读取 compartment 细节。
    const Cell& cell() const { return cell_; }
    Cell& cell() { return cell_; }

    double voltage_mV(std::size_t compartment_index) const override;
    double soma_voltage_mV() const override;
    std::size_t size() const override { return cell_.compartments.size(); }

    // 根据每个 compartment 的 conductance 字段挂载对应通道对象。
    void attach_nca_channels();
    void attach_irk_channels();
    void attach_kqt3_channels();
    void attach_egl2_channels();
    void attach_shk1_channels();
    void attach_kvs1_channels();
    void attach_shl1_channels();
    void attach_egl36_channels();
    void attach_egl19_channels();
    void attach_cca1_channels();
    void attach_unc2_channels();
    void attach_kcnl_channels();
    void attach_slo1_egl19_channels();
    void attach_slo1_unc2_channels();
    void attach_slo2_egl19_channels();
    void attach_slo2_unc2_channels();
    void enable_calcium_internal();

    // 诊断输出用：按 compartment 读取各类电流。
    double leak_current_pA(std::size_t compartment_index) const;
    double axial_current_pA(std::size_t compartment_index) const;
    double ion_current_pA(std::size_t compartment_index) const;
    double channel_current_pA(std::size_t compartment_index, const std::string& channel_name) const;

private:
    // cell_ 保存静态参数和动态状态；solver_ 保存积分工作区。
    Cell cell_;
    CableSolver solver_;
    std::vector<double> injected_current_pA_;
    std::vector<double> zero_conductance_nS_;
    std::vector<double> zero_reversal_mV_;
};

}  // namespace neuron
