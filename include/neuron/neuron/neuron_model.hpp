#pragma once

#include <cstddef>
#include <string>

namespace neuron {

// 神经元运行时的最小抽象接口。
// synapse / network 层只依赖这些方法，不需要知道具体神经元是否是 MultiCompartmentNeuron 或别的实现。
class NeuronModel {
public:
    virtual ~NeuronModel() = default;

    // 运行时名称和 compartment 数量。
    virtual const std::string& name() const = 0;
    virtual std::size_t size() const = 0;

    // 读取单个 compartment 电压，以及对外常用的 soma 电压。
    virtual double voltage_mV(std::size_t compartment_index) const = 0;
    virtual double soma_voltage_mV() const = 0;

    // 初始化或外部驱动 neuron 的状态。
    virtual void set_all_voltages(double voltage_mV) = 0;
    virtual void add_current_pA(std::size_t compartment_index, double current_pA) = 0;
    virtual void clear_currents() = 0;

    // 前进一步；调用方负责提供 dt，具体实现决定如何积分。
    virtual void step(double dt_ms) = 0;
};

}  // namespace neuron
