#include "runner/runner.hpp"

#include "cpp_neuron_core/recording/csv_writer.hpp"

#include <array>
#include <stdexcept>
#include <string>

namespace cpp_neuron::runner {

namespace {

std::string arg_value(int argc, char** argv, const std::string& name, const std::string& fallback) {
    for (int i = 1; i < argc; ++i) {
        if (argv[i] == name) {
            if (i + 1 >= argc) {
                throw std::runtime_error("Missing value for argument: " + name);
            }
            return argv[i + 1];
        }
    }
    return fallback;
}

double arg_double(int argc, char** argv, const std::string& name, double fallback) {
    return std::stod(arg_value(argc, argv, name, std::to_string(fallback)));
}

bool has_arg(int argc, char** argv, const std::string& name) {
    for (int i = 1; i < argc; ++i) {
        if (argv[i] == name) {
            return true;
        }
    }
    return false;
}

// bool Type::* 是成员指针；它把一个 CLI flag 映射到配置对象中的 bool 字段。
struct MechanismFlagBinding {
    const char* flag;
    bool NeuronMechanismConfig::* enabled;
};

NeuronMechanismConfig parse_mechanism_config(int argc, char** argv) {
    static constexpr std::array<MechanismFlagBinding, 17> bindings{{
        {"--enable-nca", &NeuronMechanismConfig::enable_nca},
        {"--enable-irk", &NeuronMechanismConfig::enable_irk},
        {"--enable-kqt3", &NeuronMechanismConfig::enable_kqt3},
        {"--enable-egl2", &NeuronMechanismConfig::enable_egl2},
        {"--enable-shk1", &NeuronMechanismConfig::enable_shk1},
        {"--enable-kvs1", &NeuronMechanismConfig::enable_kvs1},
        {"--enable-shl1", &NeuronMechanismConfig::enable_shl1},
        {"--enable-egl36", &NeuronMechanismConfig::enable_egl36},
        {"--enable-egl19", &NeuronMechanismConfig::enable_egl19},
        {"--enable-cca1", &NeuronMechanismConfig::enable_cca1},
        {"--enable-unc2", &NeuronMechanismConfig::enable_unc2},
        {"--enable-calcium-internal", &NeuronMechanismConfig::enable_calcium_internal},
        {"--enable-kcnl", &NeuronMechanismConfig::enable_kcnl},
        {"--enable-slo1-egl19", &NeuronMechanismConfig::enable_slo1_egl19},
        {"--enable-slo1-unc2", &NeuronMechanismConfig::enable_slo1_unc2},
        {"--enable-slo2-egl19", &NeuronMechanismConfig::enable_slo2_egl19},
        {"--enable-slo2-unc2", &NeuronMechanismConfig::enable_slo2_unc2},
    }};

    NeuronMechanismConfig mechanisms;
    for (const auto& binding : bindings) {
        mechanisms.*(binding.enabled) = has_arg(argc, argv, binding.flag);
    }
    return mechanisms;
}

IClampProtocol default_iclamp(const std::string& cell) {
    IClampProtocol protocol;
    if (cell == "AIYL") {
        protocol.v_init_mV = -45.0;
        protocol.amplitude_pA = 10.0;
    } else if (cell == "AVAL") {
        protocol.v_init_mV = -30.0;
        protocol.duration_ms = 5015.0;
        protocol.amplitude_pA = 10.0;
    } else if (cell == "RIML") {
        protocol.v_init_mV = -39.3;
        protocol.amplitude_pA = 10.0;
    }
    return protocol;
}

SEClampProtocol default_seclamp(const std::string& cell) {
    SEClampProtocol protocol;
    if (cell == "AWCL") {
        protocol.tstop_ms = 1600.0;
        protocol.duration_ms = 100.0;
        protocol.v_command_mV = 30.0;
    } else if (cell == "VD05") {
        protocol.tstop_ms = 2600.0;
        protocol.duration_ms = 1210.0;
        protocol.v_command_mV = 20.0;
    }
    return protocol;
}

void validate_timing(double dt_ms, double tstop_ms) {
    if (dt_ms <= 0.0) {
        throw std::runtime_error("--dt-ms must be positive");
    }
    if (tstop_ms <= 0.0) {
        throw std::runtime_error("--tstop-ms must be positive");
    }
}

}  // namespace

RunnerConfig parse_config(int argc, char** argv) {
    RunnerConfig config;
    const std::string cell_name = arg_value(argc, argv, "--cell", "AIYL");
    const std::string protocol_name = arg_value(argc, argv, "--protocol", "iclamp");
    const std::string data_dir = arg_value(argc, argv, "--data-dir", "../data/aiyl");
    const std::string cell_file =
        arg_value(argc, argv, "--cell-file", data_dir + "/" + cell_name + "_cell.csv");

    config.neuron = {cell_name, cell_file, parse_mechanism_config(argc, argv)};
    config.output = arg_value(argc, argv, "--output", config.output);
    config.diagnostic_output = arg_value(argc, argv, "--diag-output", "");

    if (protocol_name == "iclamp") {
        config.protocol = ProtocolKind::IClamp;
        config.iclamp = default_iclamp(cell_name);
        config.iclamp.amplitude_pA = arg_double(argc, argv, "--amp-pa", config.iclamp.amplitude_pA);
        config.iclamp.dt_ms = arg_double(argc, argv, "--dt-ms", config.iclamp.dt_ms);
        config.iclamp.tstop_ms = arg_double(argc, argv, "--tstop-ms", config.iclamp.tstop_ms);
        validate_timing(config.iclamp.dt_ms, config.iclamp.tstop_ms);
    } else if (protocol_name == "seclamp") {
        config.protocol = ProtocolKind::SEClamp;
        config.seclamp = default_seclamp(cell_name);
        config.seclamp.v_command_mV = arg_double(argc, argv, "--vcmd-mv", config.seclamp.v_command_mV);
        config.seclamp.dt_ms = arg_double(argc, argv, "--dt-ms", config.seclamp.dt_ms);
        config.seclamp.tstop_ms = arg_double(argc, argv, "--tstop-ms", config.seclamp.tstop_ms);
        validate_timing(config.seclamp.dt_ms, config.seclamp.tstop_ms);
    } else {
        throw std::runtime_error("Unknown protocol: " + protocol_name);
    }

    return config;
}

void run(const RunnerConfig& config) {
    auto neuron = create_multi_compartment_neuron(config.neuron);

    ProtocolResult result;
    switch (config.protocol) {
    case ProtocolKind::IClamp:
        result = run_current_clamp_with_diagnostics(*neuron, config.iclamp);
        break;
    case ProtocolKind::SEClamp:
        result = run_seclamp_with_diagnostics(*neuron, config.seclamp);
        break;
    default:
        throw std::runtime_error("Unsupported protocol kind");
    }

    write_trace_csv(config.output, result.trace);
    if (!config.diagnostic_output.empty()) {
        write_channel_diagnostics_csv(config.diagnostic_output, result.diagnostics);
    }
}

}  // namespace cpp_neuron::runner
