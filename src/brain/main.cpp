#include "brain/brain_runner.hpp"
#include "brain/diagnostics/brain_diagnostics_recorder.hpp"

#include <cstdlib>
#include <exception>
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

std::size_t arg_size(int argc, char** argv, const std::string& name, std::size_t fallback) {
    return static_cast<std::size_t>(std::stoull(arg_value(argc, argv, name, std::to_string(fallback))));
}

brain::BrainRunnerConfig parse_config(int argc, char** argv, const std::string& source_dir) {
    brain::BrainRunnerConfig config;
    const std::string data_dir = arg_value(argc, argv, "--data-dir", source_dir + "/data/synapse_v0");
    config.network.chemical_csv =
        arg_value(argc, argv, "--chemical-csv", data_dir + "/chemical_components_neuron_neuron_v0.csv");
    config.network.gap_csv = arg_value(argc, argv, "--gap-csv", data_dir + "/gap_junctions_v0.csv");
    config.network.neuron_reference_csv =
        arg_value(argc, argv, "--neuron-reference-csv", data_dir + "/neuron_parameter_reference_v0.csv");
    config.network.template_data_dir = arg_value(argc, argv, "--template-data-dir", source_dir + "/data");

    config.simulation.dt_ms = arg_double(argc, argv, "--dt-ms", config.simulation.dt_ms);
    config.tstop_ms = arg_double(argc, argv, "--tstop-ms", config.tstop_ms);

    const std::string diagnostics = arg_value(argc, argv, "--diagnostics", "");
    if (!diagnostics.empty() && diagnostics != "summary") {
        throw std::runtime_error("--diagnostics currently supports only: summary");
    }
    config.diagnostics.enabled = diagnostics == "summary";
    config.diagnostics.explode_voltage_mV =
        arg_double(argc, argv, "--explode-voltage-mv", config.diagnostics.explode_voltage_mV);
    config.diagnostics.top_current_edges =
        arg_size(argc, argv, "--top-current-edges", config.diagnostics.top_current_edges);
    return config;
}

}  // namespace

int main(int argc, char** argv) { // --diagnostics summary --tstop-ms 10 --dt-ms 0.1 --top-current-edges 5 // --tstop-ms 10 --dt-ms 0.1 --top-current-edges 5
    try {
        const auto config = parse_config(argc, argv, CPP_WORM_SIM_SOURCE_DIR);
        const auto result = brain::run_brain(config);
        if (!result.ok) {
            std::cerr << "ERROR: " << result.error_message << '\n';
            return EXIT_FAILURE;
        }
        if (result.diagnostics.has_value()) {
            brain::write_brain_summary(*result.diagnostics, std::cout);
            return result.diagnostics->ok() ? EXIT_SUCCESS : EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    } catch (const std::exception& exc) {
        std::cerr << "ERROR: " << exc.what() << '\n';
        return EXIT_FAILURE;
    }
}

