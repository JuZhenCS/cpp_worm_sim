cmake_minimum_required(VERSION 3.16)

project(cpp_worm_sim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(cpp_neuron_core
    src/types.cpp
    src/neuron.cpp
    src/neuron_factory.cpp
    src/protocol.cpp
    src/recorder.cpp
    src/cell_loader.cpp
    src/calcium_internal.cpp
    src/synapse.cpp
    src/synapse_loader.cpp
    src/channels/nca.cpp
    src/channels/irk.cpp
    src/channels/kqt3.cpp
    src/channels/egl2.cpp
    src/channels/shk1.cpp
    src/channels/kvs1.cpp
    src/channels/shl1.cpp
    src/channels/egl36.cpp
    src/channels/egl19.cpp
    src/channels/cca1.cpp
    src/channels/unc2.cpp
    src/channels/kcnl.cpp
    src/channels/slo1_unc2.cpp
    src/channels/slo_coupled.cpp
)

target_include_directories(cpp_neuron_core PUBLIC include)

add_executable(cpp_neuron_runner src/main.cpp)
target_link_libraries(cpp_neuron_runner PRIVATE cpp_neuron_core)

add_executable(synapse_tests tests/test_synapse.cpp)
target_link_libraries(synapse_tests PRIVATE cpp_neuron_core)
target_compile_definitions(synapse_tests PRIVATE CPP_WORM_SIM_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(synapse_smoke tests/synapse_smoke.cpp)
target_link_libraries(synapse_smoke PRIVATE cpp_neuron_core)
target_compile_definitions(synapse_smoke PRIVATE CPP_WORM_SIM_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
