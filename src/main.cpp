#include "cpp_neuron_core/neuron.hpp"
#include "cpp_neuron_core/neuron_factory.hpp"
#include "cpp_neuron_core/protocol.hpp"
#include "cpp_neuron_core/recorder.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

// 从命令行读取形如 "--name value" 的参数；没有提供时使用 fallback。
std::string arg_value(int argc, char** argv, const std::string& name, const std::string& fallback) {
    for (int i = 1; i + 1 < argc; ++i) {
        if (argv[i] == name) {
            return argv[i + 1];
        }
    }
    return fallback;
}

// 读取 double 参数。注意：如果用户传入的值不是数字，std::stod 会抛异常并由 main 捕获。
double arg_double(int argc, char** argv, const std::string& name, double fallback) {
    return std::stod(arg_value(argc, argv, name, std::to_string(fallback)));
}

// 检查布尔开关是否出现，例如 "--enable-nca"。
bool has_arg(int argc, char** argv, const std::string& name) {
    for (int i = 1; i < argc; ++i) {
        if (argv[i] == name) {
            return true;
        }
    }
    return false;
}

// 不同代表神经元的 current-clamp 默认参数。
// main 里仍允许用命令行参数覆盖 amp/dt/tstop。
cpp_neuron::IClampProtocol default_iclamp(const std::string& cell) {
    cpp_neuron::IClampProtocol p;
    if (cell == "AIYL") {
        p.v_init_mV = -45.0;
        p.amplitude_pA = 10.0;
    } else if (cell == "AVAL") {
        p.v_init_mV = -30.0;
        p.duration_ms = 5015.0;
        p.amplitude_pA = 10.0;
    } else if (cell == "RIML") {
        p.v_init_mV = -39.3;
        p.amplitude_pA = 10.0;
    }
    return p;
}

// 不同代表神经元的 voltage-clamp 默认参数。
// main 里仍允许用命令行参数覆盖 vcmd/dt/tstop。
cpp_neuron::SEClampProtocol default_seclamp(const std::string& cell) {
    cpp_neuron::SEClampProtocol p;
    if (cell == "AWCL") {
        p.tstop_ms = 1600.0;
        p.duration_ms = 100.0;
        p.v_command_mV = 30.0;
    } else if (cell == "VD05") {
        p.tstop_ms = 2600.0;
        p.duration_ms = 1210.0;
        p.v_command_mV = 20.0;
    }
    return p;
}

}  // namespace

int main(int argc, char** argv) {
    try {
        // 这个可执行文件的最小运行信息：细胞类型、protocol、输入 cell CSV、输出 trace/diagnostics。
        const std::string cell_name = arg_value(argc, argv, "--cell", "AIYL");
        const std::string protocol = arg_value(argc, argv, "--protocol", "iclamp");
        const std::string data_dir = arg_value(argc, argv, "--data-dir", "data/cells");
        const std::string output = arg_value(argc, argv, "--output", "trace.csv");
        const std::string diag_output = arg_value(argc, argv, "--diag-output", "");
        const std::string cell_path = arg_value(argc, argv, "--cell-file", data_dir + "/" + cell_name + ".csv");

        // 通道默认全部关闭；README 示例通过这些 --enable-* 开关打开对应 conductance。
        cpp_neuron::NeuronChannelConfig channels;
        channels.enable_nca = has_arg(argc, argv, "--enable-nca");
        channels.enable_irk = has_arg(argc, argv, "--enable-irk");
        channels.enable_kqt3 = has_arg(argc, argv, "--enable-kqt3");
        channels.enable_egl2 = has_arg(argc, argv, "--enable-egl2");
        channels.enable_shk1 = has_arg(argc, argv, "--enable-shk1");
        channels.enable_kvs1 = has_arg(argc, argv, "--enable-kvs1");
        channels.enable_shl1 = has_arg(argc, argv, "--enable-shl1");
        channels.enable_egl36 = has_arg(argc, argv, "--enable-egl36");
        channels.enable_egl19 = has_arg(argc, argv, "--enable-egl19");
        channels.enable_cca1 = has_arg(argc, argv, "--enable-cca1");
        channels.enable_unc2 = has_arg(argc, argv, "--enable-unc2");
        channels.enable_calcium_internal = has_arg(argc, argv, "--enable-calcium-internal");
        channels.enable_kcnl = has_arg(argc, argv, "--enable-kcnl");
        channels.enable_slo1_egl19 = has_arg(argc, argv, "--enable-slo1-egl19");
        channels.enable_slo1_unc2 = has_arg(argc, argv, "--enable-slo1-unc2");
        channels.enable_slo2_egl19 = has_arg(argc, argv, "--enable-slo2-egl19");
        channels.enable_slo2_unc2 = has_arg(argc, argv, "--enable-slo2-unc2");

        // 从 cell CSV 读取多隔室结构和每个 compartment 的参数，再按 channels 挂载 active channels。
        auto neuron = cpp_neuron::create_passive_neuron({cell_name, cell_path, channels});

        if (protocol == "iclamp") {
            // 电流钳：设置刺激电流、步长、总时长；输出 soma voltage trace 和可选通道诊断。
            auto p = default_iclamp(cell_name);
            p.amplitude_pA = arg_double(argc, argv, "--amp-pa", p.amplitude_pA);
            p.dt_ms = arg_double(argc, argv, "--dt-ms", p.dt_ms);
            p.tstop_ms = arg_double(argc, argv, "--tstop-ms", p.tstop_ms);
            auto result = cpp_neuron::run_current_clamp_with_diagnostics(*neuron, p);
            cpp_neuron::write_trace_csv(output, result.trace);
            if (!diag_output.empty()) {
                cpp_neuron::write_channel_diagnostics_csv(diag_output, result.diagnostics);
            }
        } else if (protocol == "seclamp") {
            // 电压钳：设置 command voltage、步长、总时长；输出 comparable clamp current。
            auto p = default_seclamp(cell_name);
            p.v_command_mV = arg_double(argc, argv, "--vcmd-mv", p.v_command_mV);
            p.dt_ms = arg_double(argc, argv, "--dt-ms", p.dt_ms);
            p.tstop_ms = arg_double(argc, argv, "--tstop-ms", p.tstop_ms);
            auto result = cpp_neuron::run_seclamp_with_diagnostics(*neuron, p);
            cpp_neuron::write_trace_csv(output, result.trace);
            if (!diag_output.empty()) {
                cpp_neuron::write_channel_diagnostics_csv(diag_output, result.diagnostics);
            }
        } else {
            throw std::runtime_error("Unknown protocol: " + protocol);
        }

        std::cout << "Wrote " << output << '\n';
        return EXIT_SUCCESS;
    } catch (const std::exception& exc) {
        // 参数错误、文件打不开、CSV 格式问题、未知 protocol 等都会走到这里。
        std::cerr << "ERROR: " << exc.what() << '\n';
        return EXIT_FAILURE;
    }
}
