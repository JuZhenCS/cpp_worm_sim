#include "neuron/core/cable_solver.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace neuron {

CableSolver::CableSolver(std::size_t compartment_count) {
    resize_if_needed(compartment_count);
}

void CableSolver::resize_if_needed(std::size_t compartment_count) {
    const auto matrix_size = compartment_count * compartment_count;
    if (rhs_.size() == compartment_count && matrix_.size() == matrix_size) {
        return;
    }
    matrix_.assign(matrix_size, 0.0);
    inverse_matrix_.assign(matrix_size, 0.0);
    rhs_.assign(compartment_count, 0.0);
    solution_.assign(compartment_count, 0.0);
    cached_inverse_valid_ = false;
    cached_inverse_dt_ms_ = 0.0;
}

void CableSolver::advance_voltages(
    Cell& cell,
    double dt_ms,
    const std::vector<double>& injected_current_pA,
    const std::vector<double>& extra_conductance_nS,
    const std::vector<double>& extra_reversal_mV) {
    const std::size_t n = cell.compartments.size();
    if (injected_current_pA.size() != n) {
        throw std::runtime_error("Injected current vector size does not match compartment count");
    }
    if (extra_conductance_nS.size() != n || extra_reversal_mV.size() != n) {
        throw std::runtime_error("Extra conductance vectors do not match compartment count");
    }
    if (dt_ms <= 0.0) {
        throw std::runtime_error("Time step must be positive");
    }

    resize_if_needed(n);
    const bool cacheable_matrix = std::all_of(extra_conductance_nS.begin(), extra_conductance_nS.end(), [](double g) {
        return g == 0.0;
    });
    std::fill(matrix_.begin(), matrix_.end(), 0.0);
    std::fill(rhs_.begin(), rhs_.end(), 0.0);

    for (std::size_t i = 0; i < n; ++i) {
        auto& c = cell.compartments[i];
        if (c.capacitance_pF <= 0.0) {
            throw std::runtime_error("Compartment capacitance must be positive");
        }
        const double c_over_dt = c.capacitance_pF / dt_ms;
        const double extra_g = extra_conductance_nS[i];
        at(i, i, n) += c_over_dt + c.leak_conductance_nS + extra_g;

        double ion_current_pA = 0.0;
        for (const auto& channel : c.channels) {
            channel->set_calcium(c.cai_uM_per_um2);
            channel->step(c.voltage_mV, dt_ms);
            ion_current_pA += channel->current_pA(c.voltage_mV);
        }

        rhs_[i] += c_over_dt * c.voltage_mV + c.leak_conductance_nS * c.leak_reversal_mV
                   + extra_g * extra_reversal_mV[i] + injected_current_pA[i] - ion_current_pA;

        if (c.parent_index >= 0) {
            const auto parent = static_cast<std::size_t>(c.parent_index);
            if (parent >= n) {
                throw std::runtime_error("Parent compartment index out of range");
            }
            const double g = c.axial_conductance_to_parent_nS;
            at(i, i, n) += g;
            at(parent, parent, n) += g;
            at(i, parent, n) -= g;
            at(parent, i, n) -= g;
        }
    }

    if (cacheable_matrix && cached_inverse_valid_ && cached_inverse_dt_ms_ == dt_ms) {
        solve_with_cached_inverse(n, dt_ms, true);
    } else if (cacheable_matrix) {
        std::fill(inverse_matrix_.begin(), inverse_matrix_.end(), 0.0);
        for (std::size_t i = 0; i < n; ++i) {
            inv_at(i, i, n) = 1.0;
        }

        for (std::size_t col = 0; col < n; ++col) {
            std::size_t pivot = col;
            double pivot_abs = std::abs(at(col, col, n));
            for (std::size_t row = col + 1; row < n; ++row) {
                const double candidate = std::abs(at(row, col, n));
                if (candidate > pivot_abs) {
                    pivot = row;
                    pivot_abs = candidate;
                }
            }
            if (pivot_abs < 1e-18) {
                throw std::runtime_error("Passive solve failed: singular matrix");
            }
            if (pivot != col) {
                for (std::size_t k = 0; k < n; ++k) {
                    std::swap(at(col, k, n), at(pivot, k, n));
                    std::swap(inv_at(col, k, n), inv_at(pivot, k, n));
                }
            }
            const double diag = at(col, col, n);
            for (std::size_t k = 0; k < n; ++k) {
                at(col, k, n) /= diag;
                inv_at(col, k, n) /= diag;
            }

            for (std::size_t row = 0; row < n; ++row) {
                if (row == col) {
                    continue;
                }
                const double factor = at(row, col, n);
                if (factor == 0.0) {
                    continue;
                }
                for (std::size_t k = 0; k < n; ++k) {
                    at(row, k, n) -= factor * at(col, k, n);
                    inv_at(row, k, n) -= factor * inv_at(col, k, n);
                }
            }
        }
        cached_inverse_dt_ms_ = dt_ms;
        cached_inverse_valid_ = true;
        solve_with_cached_inverse(n, dt_ms, true);
    } else {
        solve_direct(n);
    }

    for (std::size_t i = 0; i < n; ++i) {
        cell.compartments[i].voltage_mV = solution_[i];
    }
}

void CableSolver::solve_with_cached_inverse(std::size_t n, double, bool) {
    for (std::size_t i = 0; i < n; ++i) {
        double voltage = 0.0;
        for (std::size_t j = 0; j < n; ++j) {
            voltage += inverse_matrix_[i * n + j] * rhs_[j];
        }
        solution_[i] = voltage;
    }
}

void CableSolver::solve_direct(std::size_t n) {
    for (std::size_t col = 0; col < n; ++col) {
        std::size_t pivot = col;
        double pivot_abs = std::abs(at(col, col, n));
        for (std::size_t row = col + 1; row < n; ++row) {
            const double candidate = std::abs(at(row, col, n));
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
                std::swap(at(col, k, n), at(pivot, k, n));
            }
            std::swap(rhs_[col], rhs_[pivot]);
        }
        const double diag = at(col, col, n);
        for (std::size_t k = col; k < n; ++k) {
            at(col, k, n) /= diag;
        }
        rhs_[col] /= diag;

        for (std::size_t row = 0; row < n; ++row) {
            if (row == col) {
                continue;
            }
            const double factor = at(row, col, n);
            if (factor == 0.0) {
                continue;
            }
            for (std::size_t k = col; k < n; ++k) {
                at(row, k, n) -= factor * at(col, k, n);
            }
            rhs_[row] -= factor * rhs_[col];
        }
    }

    for (std::size_t i = 0; i < n; ++i) {
        solution_[i] = rhs_[i];
    }
}

}  // namespace neuron
