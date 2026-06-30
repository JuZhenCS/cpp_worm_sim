#include "runner/runner.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void require(bool condition) {
    if (!condition) {
        throw std::runtime_error("runner test requirement failed");
    }
}

cpp_neuron::runner::RunnerConfig parse(std::initializer_list<const char*> raw_args) {
    std::vector<std::string> storage;
    storage.reserve(raw_args.size());
    for (const char* arg : raw_args) {
        storage.emplace_back(arg);
    }

    std::vector<char*> argv;
    argv.reserve(storage.size());
    for (auto& arg : storage) {
        argv.push_back(arg.data());
    }

    return cpp_neuron::runner::parse_config(static_cast<int>(argv.size()), argv.data());
}

void test_defaults() {
    const auto config = parse({"cpp_neuron_runner.exe"});

    require(config.neuron.name == "AIYL");
    require(config.neuron.cell_file == "../data/aiyl/AIYL_cell.csv");
    require(config.protocol == cpp_neuron::runner::ProtocolKind::IClamp);
    require(config.iclamp.v_init_mV == -45.0);
    require(config.iclamp.amplitude_pA == 10.0);
    require(config.output == "trace.csv");
    require(config.diagnostic_output.empty());
}

void test_iclamp_overrides_and_mechanisms() {
    const auto config = parse({
        "cpp_neuron_runner.exe",
        "--cell",
        "AVAL",
        "--protocol",
        "iclamp",
        "--cell-file",
        "aval.csv",
        "--amp-pa",
        "12.5",
        "--dt-ms",
        "0.1",
        "--tstop-ms",
        "3",
        "--enable-nca",
        "--enable-egl19",
        "--output",
        "trace-out.csv",
        "--diag-output",
        "diag-out.csv",
    });

    require(config.neuron.name == "AVAL");
    require(config.neuron.cell_file == "aval.csv");
    require(config.neuron.mechanisms.enable_nca);
    require(config.neuron.mechanisms.enable_egl19);
    require(!config.neuron.mechanisms.enable_irk);
    require(config.iclamp.amplitude_pA == 12.5);
    require(config.iclamp.dt_ms == 0.1);
    require(config.iclamp.tstop_ms == 3.0);
    require(config.output == "trace-out.csv");
    require(config.diagnostic_output == "diag-out.csv");
}

void test_seclamp_overrides() {
    const auto config = parse({
        "cpp_neuron_runner.exe",
        "--cell",
        "AWCL",
        "--protocol",
        "seclamp",
        "--vcmd-mv",
        "70",
        "--dt-ms",
        "0.05",
        "--tstop-ms",
        "5",
    });

    require(config.protocol == cpp_neuron::runner::ProtocolKind::SEClamp);
    require(config.seclamp.v_command_mV == 70.0);
    require(config.seclamp.dt_ms == 0.05);
    require(config.seclamp.tstop_ms == 5.0);
}

void test_unknown_protocol_rejected() {
    bool threw = false;
    try {
        (void)parse({"cpp_neuron_runner.exe", "--protocol", "unknown"});
    } catch (const std::runtime_error&) {
        threw = true;
    }
    require(threw);
}

void test_missing_value_rejected() {
    bool threw = false;
    try {
        (void)parse({"cpp_neuron_runner.exe", "--cell"});
    } catch (const std::runtime_error&) {
        threw = true;
    }
    require(threw);
}

void test_nonpositive_timing_rejected() {
    bool threw = false;
    try {
        (void)parse({"cpp_neuron_runner.exe", "--dt-ms", "0"});
    } catch (const std::runtime_error&) {
        threw = true;
    }
    require(threw);
}

}  // namespace

int main() {
    test_defaults();
    test_iclamp_overrides_and_mechanisms();
    test_seclamp_overrides();
    test_unknown_protocol_rejected();
    test_missing_value_rejected();
    test_nonpositive_timing_rejected();
    std::cout << "runner tests passed\n";
    return 0;
}
