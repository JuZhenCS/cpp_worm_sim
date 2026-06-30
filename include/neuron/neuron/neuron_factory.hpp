#pragma once

#include "neuron/neuron/multi_compartment_neuron.hpp"

#include <memory>
#include <string>

namespace neuron {

// 构建神经元时启用的动力学机制，包括active channels和内部钙动力学。
struct NeuronMechanismConfig {
    bool enable_nca = false;
    bool enable_irk = false;
    bool enable_kqt3 = false;
    bool enable_egl2 = false;
    bool enable_shk1 = false;
    bool enable_kvs1 = false;
    bool enable_shl1 = false;
    bool enable_egl36 = false;
    bool enable_egl19 = false;
    bool enable_cca1 = false;
    bool enable_unc2 = false;
    bool enable_calcium_internal = false;
    bool enable_kcnl = false;
    bool enable_slo1_egl19 = false;
    bool enable_slo1_unc2 = false;
    bool enable_slo2_egl19 = false;
    bool enable_slo2_unc2 = false;
};

// 构建一个 neuron 所需的最小配置：
// name 用作运行时细胞名，cell_file 指向多隔室参数 CSV，mechanisms 决定启用哪些动力学机制。
struct NeuronBuildConfig {
    std::string name;
    std::string cell_file;
    NeuronMechanismConfig mechanisms;
};

// 创建当前具体实现类型 MultiCompartmentNeuron；单细胞 runner 需要访问诊断接口时会用这个版本。
std::unique_ptr<MultiCompartmentNeuron> create_multi_compartment_neuron(const NeuronBuildConfig& config);

// 创建统一 NeuronModel 接口；synapse / network 层只依赖这个抽象接口。
std::unique_ptr<NeuronModel> create_neuron(const NeuronBuildConfig& config);

}  // namespace neuron
