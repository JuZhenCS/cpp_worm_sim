#pragma once

#include "cpp_neuron_core/neuron/neuron_factory.hpp"
#include "cpp_neuron_core/protocol/clamp_protocol.hpp"

#include <string>

namespace cpp_neuron::runner {

enum class ProtocolKind {
    IClamp,
    SEClamp,
};

// 完整描述一次单神经元运行；main.cpp 只负责获得并执行这个配置。
struct RunnerConfig {
    NeuronBuildConfig neuron;
    ProtocolKind protocol = ProtocolKind::IClamp;
    IClampProtocol iclamp;
    SEClampProtocol seclamp;
    std::string output = "trace.csv";
    std::string diagnostic_output;
};

RunnerConfig parse_config(int argc, char** argv);
void run(const RunnerConfig& config);

}  // namespace cpp_neuron::runner
