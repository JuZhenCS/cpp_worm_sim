#include "cpp_neuron_core/cell_loader.hpp"
#include "cpp_neuron_core/neuron.hpp"
#include "cpp_neuron_core/protocol.hpp"
#include "cpp_neuron_core/recorder.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

std::string arg_value(int argc, char** argv, const std::string& name, const std::string& fallback) {
    for (int i = 1; i + 1 < argc; ++i) {
        if (argv[i] == name) {
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
        const std::string cell_name = arg_value(argc, argv, "--cell", "AIYL");
        const std::string protocol = arg_value(argc, argv, "--protocol", "iclamp");
        const std::string data_dir = arg_value(argc, argv, "--data-dir", "data/cells");
        const std::string output = arg_value(argc, argv, "--output", "trace.csv");
        const std::string diag_output = arg_value(argc, argv, "--diag-output", "");
        const std::string cell_path = arg_value(argc, argv, "--cell-file", data_dir + "/" + cell_name + ".csv");

        auto cell = cpp_neuron::load_cell_csv(cell_path, cell_name);
        cpp_neuron::PassiveNeuron neuron(std::move(cell));
        if (has_arg(argc, argv, "--enable-nca")) {
            neuron.attach_nca_channels();
        }
        if (has_arg(argc, argv, "--enable-irk")) {
            neuron.attach_irk_channels();
        }
        if (has_arg(argc, argv, "--enable-kqt3")) {
            neuron.attach_kqt3_channels();
        }
        if (has_arg(argc, argv, "--enable-egl2")) {
            neuron.attach_egl2_channels();
        }
        if (has_arg(argc, argv, "--enable-shk1")) {
            neuron.attach_shk1_channels();
        }
        if (has_arg(argc, argv, "--enable-kvs1")) {
            neuron.attach_kvs1_channels();
        }
        if (has_arg(argc, argv, "--enable-shl1")) {
            neuron.attach_shl1_channels();
        }
        if (has_arg(argc, argv, "--enable-egl36")) {
            neuron.attach_egl36_channels();
        }
        if (has_arg(argc, argv, "--enable-egl19")) {
            neuron.attach_egl19_channels();
        }
        if (has_arg(argc, argv, "--enable-cca1")) {
            neuron.attach_cca1_channels();
        }
        if (has_arg(argc, argv, "--enable-unc2")) {
            neuron.attach_unc2_channels();
        }
        if (has_arg(argc, argv, "--enable-calcium-internal")) {
            neuron.enable_calcium_internal();
        }
        if (has_arg(argc, argv, "--enable-kcnl")) {
            neuron.attach_kcnl_channels();
        }
        if (has_arg(argc, argv, "--enable-slo1-egl19")) {
            neuron.attach_slo1_egl19_channels();
        }
        if (has_arg(argc, argv, "--enable-slo1-unc2")) {
            neuron.attach_slo1_unc2_channels();
        }
        if (has_arg(argc, argv, "--enable-slo2-egl19")) {
            neuron.attach_slo2_egl19_channels();
        }
        if (has_arg(argc, argv, "--enable-slo2-unc2")) {
            neuron.attach_slo2_unc2_channels();
        }

        if (protocol == "iclamp") {
            auto p = default_iclamp(cell_name);
            p.amplitude_pA = arg_double(argc, argv, "--amp-pa", p.amplitude_pA);
            p.dt_ms = arg_double(argc, argv, "--dt-ms", p.dt_ms);
            p.tstop_ms = arg_double(argc, argv, "--tstop-ms", p.tstop_ms);
            auto result = cpp_neuron::run_current_clamp_with_diagnostics(neuron, p);
            cpp_neuron::write_trace_csv(output, result.trace);
            if (!diag_output.empty()) {
                cpp_neuron::write_channel_diagnostics_csv(diag_output, result.diagnostics);
            }
        } else if (protocol == "seclamp") {
            auto p = default_seclamp(cell_name);
            p.v_command_mV = arg_double(argc, argv, "--vcmd-mv", p.v_command_mV);
            p.dt_ms = arg_double(argc, argv, "--dt-ms", p.dt_ms);
            p.tstop_ms = arg_double(argc, argv, "--tstop-ms", p.tstop_ms);
            auto result = cpp_neuron::run_seclamp_with_diagnostics(neuron, p);
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
        std::cerr << "ERROR: " << exc.what() << '\n';
        return EXIT_FAILURE;
    }
}
