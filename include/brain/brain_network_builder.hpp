#pragma once

#include "neuron/core/neuron_model.hpp"
#include "synapse/synapse_loader.hpp"

#include <memory>
#include <string>
#include <vector>

namespace brain {

struct BrainNetworkConfig {
    std::string chemical_csv;
    std::string gap_csv;
    std::string neuron_reference_csv;
    std::string template_data_dir;
};

struct BrainNetwork {
    neuron::NeuronIndex index; // 让 synapse loader 根据 CSV 里的 neuron 名字快速找到对应神经元对象。总之：名字 -> neuron 对象，给突触加载和查找用
    std::vector<std::shared_ptr<neuron::NeuronModel>> neurons; // 所有神经元模型的数组。每个元素是一个 shared_ptr<NeuronModel>。也就是说，neurons 保存的是所有可被仿真推进的 neuron 对象。总之：所有 neuron 对象，给仿真遍历用
    std::vector<std::string> neuron_names; // neuron 名字列表，顺序和 neurons 对齐。
    neuron::SynapseNetwork synapses; // 所有化学突触和电突触的连接信息。总之：chemical synapse + gap junction 网络
};

BrainNetwork build_brain_network(const BrainNetworkConfig& config);

}  // namespace brain

