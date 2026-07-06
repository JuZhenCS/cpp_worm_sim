#include "brain/brain_simulation.hpp"

#include <stdexcept>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace brain {
namespace {

void validate_dt(double dt_ms) {
    if (dt_ms <= 0.0) {
        throw std::runtime_error("dt_ms must be positive");
    }
}

}  // namespace

neuron::SynapseNetwork::StepStats step_brain_network(BrainNetwork& network, double dt_ms) {
    validate_dt(dt_ms);

    const auto neuron_count = static_cast<std::ptrdiff_t>(network.neurons.size());
#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
    for (std::ptrdiff_t neuron_idx = 0; neuron_idx < neuron_count; ++neuron_idx) {
        network.neurons[static_cast<std::size_t>(neuron_idx)]->clear_currents(); // 清空上一时间步输入电流
    }

    const auto stats = network.synapses.apply(dt_ms); // 计算并施加本时间步突触电流

#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
    for (std::ptrdiff_t neuron_idx = 0; neuron_idx < neuron_count; ++neuron_idx) {
        network.neurons[static_cast<std::size_t>(neuron_idx)]->step(dt_ms); // 根据电流更新神经元电压
    }

    return stats;
}

void run_brain_steps(BrainNetwork& network, double dt_ms, std::size_t steps) {
    validate_dt(dt_ms);
    for (std::size_t step = 0; step < steps; ++step) {
        step_brain_network(network, dt_ms);
    }
}

}  // namespace brain
