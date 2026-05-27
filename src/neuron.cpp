#include "cpp_neuron_core/neuron.hpp"

#include "cpp_neuron_core/calcium_internal.hpp"
#include "cpp_neuron_core/channels/cca1.hpp"
#include "cpp_neuron_core/channels/egl36.hpp"
#include "cpp_neuron_core/channels/egl19.hpp"
#include "cpp_neuron_core/channels/egl2.hpp"
#include "cpp_neuron_core/channels/irk.hpp"
#include "cpp_neuron_core/channels/kqt3.hpp"
#include "cpp_neuron_core/channels/kcnl.hpp"
#include "cpp_neuron_core/channels/kvs1.hpp"
#include "cpp_neuron_core/channels/nca.hpp"
#include "cpp_neuron_core/channels/shk1.hpp"
#include "cpp_neuron_core/channels/shl1.hpp"
#include "cpp_neuron_core/channels/slo_coupled.hpp"
#include "cpp_neuron_core/channels/slo1_unc2.hpp"
#include "cpp_neuron_core/channels/unc2.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>

namespace cpp_neuron {

PassiveNeuron::PassiveNeuron(Cell cell) : cell_(std::move(cell)) {
    if (cell_.compartments.empty()) {
        throw std::runtime_error("PassiveNeuron requires at least one compartment");
    }
}

void PassiveNeuron::set_all_voltages(double voltage_mV) {
    for (auto& compartment : cell_.compartments) {
        compartment.voltage_mV = voltage_mV;
    }
}

void PassiveNeuron::step(double dt_ms, const std::vector<double>& injected_current_pA) {
    step_with_conductance(
        dt_ms,
        injected_current_pA,
        std::vector<double>(cell_.compartments.size(), 0.0),
        std::vector<double>(cell_.compartments.size(), 0.0));
}

void PassiveNeuron::step_with_conductance(
    double dt_ms,
    const std::vector<double>& injected_current_pA,
    const std::vector<double>& extra_conductance_nS,
    const std::vector<double>& extra_reversal_mV) {
    if (injected_current_pA.size() != cell_.compartments.size()) {
        throw std::runtime_error("Injected current vector size does not match compartment count");
    }
    if (extra_conductance_nS.size() != cell_.compartments.size()
        || extra_reversal_mV.size() != cell_.compartments.size()) {
        throw std::runtime_error("Extra conductance vectors do not match compartment count");
    }
    if (dt_ms <= 0.0) {
        throw std::runtime_error("Time step must be positive");
    }

    const std::size_t n = cell_.compartments.size();
    std::vector<double> matrix(n * n, 0.0);
    std::vector<double> rhs(n, 0.0);

    auto at = [&matrix, n](std::size_t row, std::size_t col) -> double& {
        return matrix[row * n + col];
    };

    for (std::size_t i = 0; i < n; ++i) {
        const auto& c = cell_.compartments[i];
        if (c.capacitance_pF <= 0.0) {
            throw std::runtime_error("Compartment capacitance must be positive");
        }
        const double c_over_dt = c.capacitance_pF / dt_ms;
        const double extra_g = extra_conductance_nS[i];
        at(i, i) += c_over_dt + c.leak_conductance_nS + extra_g;
        double ion_current_pA = 0.0;
        for (const auto& channel : c.channels) {
            channel->set_calcium(c.cai_uM_per_um2);
            channel->step(c.voltage_mV, dt_ms);
            ion_current_pA += channel->current_pA(c.voltage_mV);
        }
        rhs[i] += c_over_dt * c.voltage_mV + c.leak_conductance_nS * c.leak_reversal_mV
                  + extra_g * extra_reversal_mV[i] + injected_current_pA[i] - ion_current_pA;

        if (c.parent_index >= 0) {
            const auto parent = static_cast<std::size_t>(c.parent_index);
            if (parent >= n) {
                throw std::runtime_error("Parent compartment index out of range");
            }
            const double g = c.axial_conductance_to_parent_nS;
            at(i, i) += g;
            at(parent, parent) += g;
            at(i, parent) -= g;
            at(parent, i) -= g;
        }
    }

    for (std::size_t col = 0; col < n; ++col) {
        std::size_t pivot = col;
        double pivot_abs = std::abs(at(col, col));
        for (std::size_t row = col + 1; row < n; ++row) {
            const double candidate = std::abs(at(row, col));
            if (candidate > pivot_abs) {
                pivot = row;
                pivot_abs = candidate;
            }
        }
        if (pivot_abs < 1e-18) {
            throw std::runtime_error("Passive solve failed: singular matrix");
        }
        if (pivot != col) {
            for (std::size_t k = col; k < n; ++k) {
                std::swap(at(col, k), at(pivot, k));
            }
            std::swap(rhs[col], rhs[pivot]);
        }
        const double diag = at(col, col);
        for (std::size_t k = col; k < n; ++k) {
            at(col, k) /= diag;
        }
        rhs[col] /= diag;

        for (std::size_t row = 0; row < n; ++row) {
            if (row == col) {
                continue;
            }
            const double factor = at(row, col);
            if (factor == 0.0) {
                continue;
            }
            for (std::size_t k = col; k < n; ++k) {
                at(row, k) -= factor * at(col, k);
            }
            rhs[row] -= factor * rhs[col];
        }
    }

    for (std::size_t i = 0; i < n; ++i) {
        cell_.compartments[i].voltage_mV = rhs[i];
    }

    CalciumInternalState calcium;
    for (auto& c : cell_.compartments) {
        if (!c.calcium_internal_enabled || c.area_um2 <= 0.0) {
            continue;
        }
        double calcium_current_total_pA = 0.0;
        for (const auto& channel : c.channels) {
            const auto channel_name = channel->name();
            if (channel_name == "egl19" || channel_name == "cca1" || channel_name == "unc2") {
                calcium_current_total_pA += channel->current_pA(c.voltage_mV);
            }
        }
        const double calcium_current_density = calcium_current_total_pA / (c.area_um2 * 10.0);
        c.cai_uM_per_um2 = calcium.step(c.cai_uM_per_um2, calcium_current_density, c.voltage_mV, dt_ms);
    }
}

double PassiveNeuron::soma_voltage_mV() const {
    if (cell_.compartments.size() == 1) {
        return cell_.compartments.front().voltage_mV;
    }
    return 0.5 * (cell_.compartments[0].voltage_mV + cell_.compartments[1].voltage_mV);
}

void PassiveNeuron::attach_nca_channels() {
    for (auto& compartment : cell_.compartments) {
        if (compartment.nca_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<NcaChannel>(compartment.nca_conductance_nS));
        }
    }
}

void PassiveNeuron::attach_irk_channels() {
    for (auto& compartment : cell_.compartments) {
        if (compartment.irk_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<IrkChannel>(compartment.irk_conductance_nS));
        }
    }
}

void PassiveNeuron::attach_kqt3_channels() {
    for (auto& compartment : cell_.compartments) {
        if (compartment.kqt3_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<Kqt3Channel>(compartment.kqt3_conductance_nS));
        }
    }
}

void PassiveNeuron::attach_egl2_channels() {
    for (auto& compartment : cell_.compartments) {
        if (compartment.egl2_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<Egl2Channel>(compartment.egl2_conductance_nS));
        }
    }
}

void PassiveNeuron::attach_shk1_channels() {
    for (auto& compartment : cell_.compartments) {
        if (compartment.shk1_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<Shk1Channel>(compartment.shk1_conductance_nS));
        }
    }
}

void PassiveNeuron::attach_kvs1_channels() {
    for (auto& compartment : cell_.compartments) {
        if (compartment.kvs1_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<Kvs1Channel>(compartment.kvs1_conductance_nS));
        }
    }
}

void PassiveNeuron::attach_shl1_channels() {
    for (auto& compartment : cell_.compartments) {
        if (compartment.shl1_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<Shl1Channel>(compartment.shl1_conductance_nS));
        }
    }
}

void PassiveNeuron::attach_egl36_channels() {
    for (auto& compartment : cell_.compartments) {
        if (compartment.egl36_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<Egl36Channel>(compartment.egl36_conductance_nS));
        }
    }
}

void PassiveNeuron::attach_egl19_channels() {
    for (auto& compartment : cell_.compartments) {
        if (compartment.egl19_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<Egl19Channel>(compartment.egl19_conductance_nS));
        }
    }
}

void PassiveNeuron::attach_cca1_channels() {
    for (auto& compartment : cell_.compartments) {
        if (compartment.cca1_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<Cca1Channel>(compartment.cca1_conductance_nS));
        }
    }
}

void PassiveNeuron::attach_unc2_channels() {
    for (auto& compartment : cell_.compartments) {
        if (compartment.unc2_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<Unc2Channel>(compartment.unc2_conductance_nS));
        }
    }
}

void PassiveNeuron::attach_kcnl_channels() {
    for (auto& compartment : cell_.compartments) {
        if (compartment.kcnl_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<KcnlChannel>(compartment.kcnl_conductance_nS));
        }
    }
}

void PassiveNeuron::attach_slo1_egl19_channels() {
    for (auto& compartment : cell_.compartments) {
        if (compartment.slo1_egl19_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<SloCoupledChannel>(
                "slo1_egl19", compartment.slo1_egl19_conductance_nS, SloKind::Slo1, CalciumPartner::Egl19));
        }
    }
}

void PassiveNeuron::attach_slo1_unc2_channels() {
    for (auto& compartment : cell_.compartments) {
        if (compartment.slo1_unc2_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<Slo1Unc2Channel>(compartment.slo1_unc2_conductance_nS));
        }
    }
}

void PassiveNeuron::attach_slo2_egl19_channels() {
    for (auto& compartment : cell_.compartments) {
        if (compartment.slo2_egl19_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<SloCoupledChannel>(
                "slo2_egl19", compartment.slo2_egl19_conductance_nS, SloKind::Slo2, CalciumPartner::Egl19));
        }
    }
}

void PassiveNeuron::attach_slo2_unc2_channels() {
    for (auto& compartment : cell_.compartments) {
        if (compartment.slo2_unc2_conductance_nS > 0.0) {
            compartment.channels.push_back(std::make_unique<SloCoupledChannel>(
                "slo2_unc2", compartment.slo2_unc2_conductance_nS, SloKind::Slo2, CalciumPartner::Unc2));
        }
    }
}

void PassiveNeuron::enable_calcium_internal() {
    for (auto& compartment : cell_.compartments) {
        compartment.calcium_internal_enabled = true;
        compartment.cai_uM_per_um2 = 0.05;
    }
}

double PassiveNeuron::leak_current_pA(std::size_t compartment_index) const {
    const auto& c = cell_.compartments.at(compartment_index);
    return c.leak_conductance_nS * (c.voltage_mV - c.leak_reversal_mV);
}

double PassiveNeuron::axial_current_pA(std::size_t compartment_index) const {
    const std::size_t n = cell_.compartments.size();
    const auto& c = cell_.compartments.at(compartment_index);
    double current = 0.0;
    if (c.parent_index >= 0) {
        const auto parent = static_cast<std::size_t>(c.parent_index);
        if (parent >= n) {
            throw std::runtime_error("Parent compartment index out of range");
        }
        current += c.axial_conductance_to_parent_nS * (c.voltage_mV - cell_.compartments[parent].voltage_mV);
    }
    for (const auto& child : cell_.compartments) {
        if (child.parent_index == static_cast<int>(compartment_index)) {
            current += child.axial_conductance_to_parent_nS * (c.voltage_mV - child.voltage_mV);
        }
    }
    return current;
}

double PassiveNeuron::ion_current_pA(std::size_t compartment_index) const {
    const auto& c = cell_.compartments.at(compartment_index);
    double current = 0.0;
    for (const auto& channel : c.channels) {
        current += channel->current_pA(c.voltage_mV);
    }
    return current;
}

double PassiveNeuron::channel_current_pA(std::size_t compartment_index, const std::string& channel_name) const {
    const auto& c = cell_.compartments.at(compartment_index);
    double current = 0.0;
    for (const auto& channel : c.channels) {
        if (channel->name() == channel_name) {
            current += channel->current_pA(c.voltage_mV);
        }
    }
    return current;
}

}  // namespace cpp_neuron
