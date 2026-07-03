#pragma once

namespace neuron {

// 构建神经元时启用的动力学机制，包括active channels和内部钙动力学。
struct NeuronMechanismConfig {
    bool enable_nca = false;
    bool enable_irk = false;
    bool enable_kqt3 = false;
    bool enable_egl2 = false;
    bool enable_shk1 = false;
    bool enable_kvs1 = false;
    bool enable_shl1 = false;
    bool enable_egl36 = false;
    bool enable_egl19 = false;
    bool enable_cca1 = false;
    bool enable_unc2 = false;
    bool enable_calcium_internal = false;
    bool enable_kcnl = false;
    bool enable_slo1_egl19 = false;
    bool enable_slo1_unc2 = false;
    bool enable_slo2_egl19 = false;
    bool enable_slo2_unc2 = false;
};

}  // namespace neuron
