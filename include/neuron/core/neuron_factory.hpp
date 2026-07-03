#pragma once

#include "neuron/core/mechanism_config.hpp"
#include "neuron/core/multi_compartment_neuron.hpp"

#include <memory>
#include <string>

namespace neuron {

// 构建一个 neuron 所需的最小配置：
// name 用作运行时细胞名，cell_file 指向多隔室参数 CSV，mechanisms 决定启用哪些动力学机制。
struct NeuronBuildConfig {
    std::string name;
    std::string cell_file;
    NeuronMechanismConfig mechanisms;
};

// 创建当前具体实现类型 MultiCompartmentNeuron；单细胞 neuron_runner 需要访问诊断接口时会用这个版本。
std::unique_ptr<MultiCompartmentNeuron> create_multi_compartment_neuron(const NeuronBuildConfig& config);

// 创建统一 NeuronModel 接口；synapse / network 层只依赖这个抽象接口。
std::unique_ptr<NeuronModel> create_neuron(const NeuronBuildConfig& config);

}  // namespace neuron
