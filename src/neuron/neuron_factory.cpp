#include "cpp_neuron_core/neuron/neuron_factory.hpp"

#include "cpp_neuron_core/neuron/cell_loader.hpp"

namespace cpp_neuron {

namespace {

// 根据mechanism配置挂载active channels并启用内部钙动力学。
// 每个 attach_* 函数会遍历所有 compartment，只在该通道 conductance > 0 的位置创建通道对象。
void enable_mechanisms(MultiCompartmentNeuron& neuron, const NeuronMechanismConfig& mechanisms) {
    if (mechanisms.enable_nca) {
        neuron.attach_nca_channels();
    }
    if (mechanisms.enable_irk) {
        neuron.attach_irk_channels();
    }
    if (mechanisms.enable_kqt3) {
        neuron.attach_kqt3_channels();
    }
    if (mechanisms.enable_egl2) {
        neuron.attach_egl2_channels();
    }
    if (mechanisms.enable_shk1) {
        neuron.attach_shk1_channels();
    }
    if (mechanisms.enable_kvs1) {
        neuron.attach_kvs1_channels();
    }
    if (mechanisms.enable_shl1) {
        neuron.attach_shl1_channels();
    }
    if (mechanisms.enable_egl36) {
        neuron.attach_egl36_channels();
    }
    if (mechanisms.enable_egl19) {
        neuron.attach_egl19_channels();
    }
    if (mechanisms.enable_cca1) {
        neuron.attach_cca1_channels();
    }
    if (mechanisms.enable_unc2) {
        neuron.attach_unc2_channels();
    }
    if (mechanisms.enable_calcium_internal) {
        neuron.enable_calcium_internal();
    }
    if (mechanisms.enable_kcnl) {
        neuron.attach_kcnl_channels();
    }
    if (mechanisms.enable_slo1_egl19) {
        neuron.attach_slo1_egl19_channels();
    }
    if (mechanisms.enable_slo1_unc2) {
        neuron.attach_slo1_unc2_channels();
    }
    if (mechanisms.enable_slo2_egl19) {
        neuron.attach_slo2_egl19_channels();
    }
    if (mechanisms.enable_slo2_unc2) {
        neuron.attach_slo2_unc2_channels();
    }
}

}  // namespace

std::unique_ptr<MultiCompartmentNeuron> create_multi_compartment_neuron(const NeuronBuildConfig& config) {
    // 第一步：从 cell CSV 读取多隔室结构、漏电参数、轴向连接和各通道 conductance。
    auto cell = load_cell_csv(config.cell_file, config.name);

    // 第二步：用加载出的 Cell 构造当前运行时的神经元对象。
    auto neuron = std::make_unique<MultiCompartmentNeuron>(std::move(cell));

    // 第三步：按构建配置启用动力学机制。
    enable_mechanisms(*neuron, config.mechanisms);
    return neuron;
}

std::unique_ptr<NeuronModel> create_neuron(const NeuronBuildConfig& config) {
    // 对外暴露统一的 NeuronModel 接口，方便 synapse / network 层不依赖 MultiCompartmentNeuron 具体类型。
    return create_multi_compartment_neuron(config);
}

}  // namespace cpp_neuron
