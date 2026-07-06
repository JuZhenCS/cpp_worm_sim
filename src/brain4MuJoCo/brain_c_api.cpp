#include "brain4MuJoCo/brain_c_api.h"

#include "brain4MuJoCo/brain_runtime.hpp"

#include <exception>
#include <memory>
#include <string>

#ifndef CPP_WORM_SIM_SOURCE_DIR
#define CPP_WORM_SIM_SOURCE_DIR "."
#endif

namespace {

std::unique_ptr<brain4MuJoCo::BrainRuntime> g_runtime;
std::string g_last_error;

std::string normalize_source_dir(const char* source_dir) {
    if (source_dir != nullptr && source_dir[0] != '\0') {
        return std::string(source_dir);
    }
    return std::string(CPP_WORM_SIM_SOURCE_DIR);
}

brain4MuJoCo::BrainRuntimeConfig make_runtime_config(const std::string& source_dir) {
    brain4MuJoCo::BrainRuntimeConfig config;
    const std::string synapse_dir = source_dir + "/data/synapse_v0";
    config.network.chemical_csv = synapse_dir + "/chemical_components_neuron_neuron_v0.csv";
    config.network.gap_csv = synapse_dir + "/gap_junctions_v0.csv";
    config.network.neuron_reference_csv = synapse_dir + "/neuron_parameter_reference_v0.csv";
    config.network.template_data_dir = source_dir + "/data";
    config.simulation.dt_ms = 0.1;
    config.muscle.projection_csv = source_dir + "/data/muscle/motor_to_muscle_projection_v0.csv";
    return config;
}

void set_last_error(const std::string& message) {
    g_last_error = message;
}

void clear_last_error() {
    g_last_error.clear();
}

}  // namespace

extern "C" {

WORM_BRAIN_API int32_t worm_brain_init(const char* source_dir) {
    try {
        const auto root = normalize_source_dir(source_dir);
        g_runtime = std::make_unique<brain4MuJoCo::BrainRuntime>(make_runtime_config(root));
        clear_last_error();
        return 0;
    } catch (const std::exception& exc) {
        g_runtime.reset();
        set_last_error(exc.what());
        return -1;
    } catch (...) {
        g_runtime.reset();
        set_last_error("Unknown error in worm_brain_init");
        return -1;
    }
}

WORM_BRAIN_API void worm_brain_reset(void) {
    try {
        if (!g_runtime) {
            set_last_error("worm_brain_reset called before worm_brain_init");
            return;
        }
        g_runtime->reset();
        clear_last_error();
    } catch (const std::exception& exc) {
        set_last_error(exc.what());
    } catch (...) {
        set_last_error("Unknown error in worm_brain_reset");
    }
}

WORM_BRAIN_API float* simulate(int32_t step) {
    try {
        if (!g_runtime) {
            set_last_error("simulate called before worm_brain_init");
            return nullptr;
        }
        const auto& output = g_runtime->simulate_steps(step);
        clear_last_error();
        return const_cast<float*>(output.data());
    } catch (const std::exception& exc) {
        set_last_error(exc.what());
        return nullptr;
    } catch (...) {
        set_last_error("Unknown error in simulate");
        return nullptr;
    }
}

WORM_BRAIN_API int32_t worm_brain_muscle_count(void) {
    return static_cast<int32_t>(brain4MuJoCo::kBodyMuscleCount);
}

WORM_BRAIN_API const char* worm_brain_last_error(void) {
    return g_last_error.c_str();
}

WORM_BRAIN_API void worm_brain_shutdown(void) {
    g_runtime.reset();
    clear_last_error();
}

}  // extern "C"