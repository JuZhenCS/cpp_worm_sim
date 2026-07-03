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
        src/protocol/clamp_protocol.cpp
        src/recording/csv_writer.cpp
        src/neuron/cell_loader.cpp
        src/neuron/calcium_internal.cpp
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

# -------------------------------
# 5. 头文件路径（对外暴露接口）
# -------------------------------
target_include_directories(neuron_core PUBLIC include)

# -------------------------------
# 6. 单神经元runner支持库（供主程序和测试复用）
# -------------------------------
add_library(neuron_runner_support STATIC
        src/neuron_runner/runner.cpp
)
target_include_directories(neuron_runner_support PUBLIC src)
target_link_libraries(neuron_runner_support PUBLIC neuron_core)

# -------------------------------
# 7. 主运行程序（可执行文件）
# -------------------------------
add_executable(neuron_runner src/main.cpp)
target_link_libraries(neuron_runner PRIVATE neuron_runner_support)

# -------------------------------
# 8. 测试程序（单元测试）
# -------------------------------
add_executable(runner_tests tests/test_runner.cpp)
target_link_libraries(runner_tests PRIVATE neuron_runner_support)

add_executable(synapse_tests tests/test_synapse.cpp)
target_link_libraries(synapse_tests PRIVATE neuron_core)

# 给测试程序定义一个宏（告诉代码工程路径）
target_compile_definitions(synapse_tests PRIVATE
        CPP_WORM_SIM_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

enable_testing()
add_test(NAME runner_tests COMMAND runner_tests)
add_test(NAME synapse_tests COMMAND synapse_tests)

# -------------------------------
# 9. smoke测试（快速验证功能）
# -------------------------------
add_executable(synapse_smoke tests/synapse_smoke.cpp)
target_link_libraries(synapse_smoke PRIVATE neuron_core)

target_compile_definitions(synapse_smoke PRIVATE
        CPP_WORM_SIM_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

# -------------------------------
# 10. OpenMP并行支持（如果系统支持）
# -------------------------------
find_package(OpenMP QUIET)

if(OpenMP_CXX_FOUND)

    # 链接OpenMP库（开启多线程）
    target_link_libraries(synapse_smoke PRIVATE OpenMP::OpenMP_CXX)

    # 编译 & 链接参数
    if(OpenMP_CXX_FLAGS)
        separate_arguments(OPENMP_CXX_FLAGS_LIST NATIVE_COMMAND "${OpenMP_CXX_FLAGS}")

        target_compile_options(synapse_smoke PRIVATE ${OPENMP_CXX_FLAGS_LIST})
        target_link_options(synapse_smoke PRIVATE ${OPENMP_CXX_FLAGS_LIST})
    endif()

endif()
