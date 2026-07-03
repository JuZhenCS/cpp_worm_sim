#include "neuron/core/neuron_factory.hpp"

#include "neuron/core/cell_loader.hpp"
#include "neuron/core/mechanism_config.hpp"
#include "neuron/core/mechanism_registry.hpp"

namespace neuron {

namespace {

// 根据mechanism配置挂载active channels并启用内部钙动力学。
// 每个 attach_* 函数会遍历所有 compartment，只在该通道 conductance > 0 的位置创建通道对象。
void enable_mechanisms(MultiCompartmentNeuron& neuron, const NeuronMechanismConfig& mechanisms) {
    for (const auto& mechanism : all_mechanisms()) {
        if (mechanisms.*(mechanism.enabled)) {
            (neuron.*(mechanism.install))();
        }
    }
}

}  // namespace

std::unique_ptr<MultiCompartmentNeuron> create_multi_compartment_neuron(const NeuronBuildConfig& config) {
    // 第一步：从 cell CSV 读取多隔室结构、漏电参数、轴向连接和各通道 conductance。
    auto cell = load_cell_csv(config.cell_file, config.name);

    // 第二步：用加载出的 Cell 构造当前运行时的神经元对象。
    auto neuron = std::make_unique<MultiCompartmentNeuron>(std::move(cell));

    // 第三步：按构建配置启用通道（channel）动力学机制。
    enable_mechanisms(*neuron, config.mechanisms);
    return neuron;
}

std::unique_ptr<NeuronModel> create_neuron(const NeuronBuildConfig& config) {
    // 对外暴露统一的 NeuronModel 接口，方便 synapse / network 层不依赖 MultiCompartmentNeuron 具体类型。
    return create_multi_compartment_neuron(config);
}

}  // namespace neuron
