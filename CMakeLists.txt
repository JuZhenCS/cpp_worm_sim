# -------------------------------
# 1. CMake最低版本要求
# -------------------------------
cmake_minimum_required(VERSION 3.16)

# -------------------------------
# 2. 项目名称 + 使用语言
# -------------------------------
project(cpp_worm_sim LANGUAGES CXX)

# -------------------------------
# 3. C++标准设置
# -------------------------------
set(CMAKE_CXX_STANDARD 17)          # 使用 C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 强制使用该标准
set(CMAKE_CXX_EXTENSIONS OFF)       # 禁用GCC扩展（保证标准兼容）

# -------------------------------
# 4. 核心库：神经系统模块
# -------------------------------
add_library(neuron_core
        src/neuron/cell.cpp
        src/neuron/multi_compartment_neuron.cpp
        src/neuron/cable_solver.cpp
        src/neuron/channel_installer.cpp
        src/neuron/diagnostics.cpp
        src/neuron/mechanism_registry.cpp
        src/neuron/neuron_factory.cpp
        src/neuron/protocol/clamp_protocol.cpp
        src/neuron/recording/csv_writer.cpp
        src/neuron/cell_loader.cpp
        src/neuron/calcium_internal.cpp
        src/synapse.cpp
        src/synapse_loader.cpp
        src/neuron/channels/nca.cpp
        src/neuron/channels/irk.cpp
        src/neuron/channels/kqt3.cpp
        src/neuron/channels/egl2.cpp
        src/neuron/channels/shk1.cpp
        src/neuron/channels/kvs1.cpp
        src/neuron/channels/shl1.cpp
        src/neuron/channels/egl36.cpp
        src/neuron/channels/egl19.cpp
        src/neuron/channels/cca1.cpp
        src/neuron/channels/unc2.cpp
        src/neuron/channels/kcnl.cpp
        src/neuron/channels/slo1_unc2.cpp
        src/neuron/channels/slo_coupled.cpp
)

# -------------------------------
# 5. 头文件路径（对外暴露接口）
# -------------------------------
target_include_directories(neuron_core PUBLIC include)

# -------------------------------
# 6. 单神经元runner支持库（供主程序和测试复用）
# -------------------------------
add_library(neuron_runner_support STATIC
        src/neuron/neuron_runner/runner.cpp
)
target_include_directories(neuron_runner_support PUBLIC src)
target_link_libraries(neuron_runner_support PUBLIC neuron_core)

# -------------------------------
# 7. 主运行程序（可执行文件）
# -------------------------------
add_executable(neuron_runner src/neuron/main.cpp)
target_link_libraries(neuron_runner PRIVATE neuron_runner_support)

# -------------------------------
# 8. 全脑网络模拟支持库
# -------------------------------
add_library(worm_brain STATIC
        src/brain/brain_simulation.cpp
        src/brain/brain_network_builder.cpp
        src/brain/representative_neuron_catalog.cpp
        src/brain/diagnostics/brain_diagnostics_observe.cpp
        src/brain/diagnostics/brain_diagnostics_recorder.cpp
        src/brain/brain_runner.cpp
)
target_include_directories(worm_brain PUBLIC include)
target_link_libraries(worm_brain PUBLIC neuron_core)

# -------------------------------
# 9. MuJoCo/Python step-by-step brain runtime
# -------------------------------
add_library(worm_brain_mujoco STATIC
        src/brain4MuJoCo/brain_runtime.cpp
        src/brain4MuJoCo/muscle/muscle_output.cpp
)
target_include_directories(worm_brain_mujoco PUBLIC include)
target_link_libraries(worm_brain_mujoco PUBLIC worm_brain)

add_library(worm_brain_shared SHARED
        src/brain4MuJoCo/brain_c_api.cpp
)
target_include_directories(worm_brain_shared PUBLIC include)
target_link_libraries(worm_brain_shared PRIVATE
        worm_brain
        worm_brain_mujoco
)
target_compile_definitions(worm_brain_shared PRIVATE
        WORM_BRAIN_BUILD_DLL
        CPP_WORM_SIM_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

set_target_properties(neuron_core worm_brain worm_brain_mujoco PROPERTIES
        POSITION_INDEPENDENT_CODE ON
)

# -------------------------------
# 10. 全脑网络模拟入口
# -------------------------------
add_executable(worm_brain_runner src/brain/main.cpp)
target_link_libraries(worm_brain_runner PRIVATE worm_brain)

target_compile_definitions(worm_brain_runner PRIVATE
        CPP_WORM_SIM_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
# -------------------------------
# 11. OpenMP并行支持（如果系统支持）
# -------------------------------
find_package(OpenMP QUIET)

if(OpenMP_CXX_FOUND)

    # 链接OpenMP库（开启多线程）
    target_link_libraries(worm_brain PUBLIC OpenMP::OpenMP_CXX)

    # 编译 & 链接参数
    if(OpenMP_CXX_FLAGS)
        separate_arguments(OPENMP_CXX_FLAGS_LIST NATIVE_COMMAND "${OpenMP_CXX_FLAGS}")

        target_compile_options(worm_brain PRIVATE ${OPENMP_CXX_FLAGS_LIST})
        target_link_options(worm_brain_runner PRIVATE ${OPENMP_CXX_FLAGS_LIST})
    endif()

endif()

