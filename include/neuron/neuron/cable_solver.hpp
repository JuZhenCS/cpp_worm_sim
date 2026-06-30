#pragma once

#include "neuron/neuron/cell.hpp"

#include <cstddef>
#include <vector>

namespace neuron {

// Builds and solves the implicit multi-compartment cable equation for one time step.
// MultiCompartmentNeuron owns biological state; CableSolver owns numerical work buffers.
class CableSolver {
public:
    explicit CableSolver(std::size_t compartment_count);

    void advance_voltages(
        Cell& cell,
        double dt_ms,
        const std::vector<double>& injected_current_pA,
        const std::vector<double>& extra_conductance_nS,
        const std::vector<double>& extra_reversal_mV);

private:
    void resize_if_needed(std::size_t compartment_count);
    void solve_with_cached_inverse(std::size_t n, double dt_ms, bool cacheable_matrix);
    void solve_direct(std::size_t n);

    double& at(std::size_t row, std::size_t col, std::size_t n) {
        return matrix_[row * n + col];
    }

    double& inv_at(std::size_t row, std::size_t col, std::size_t n) {
        return inverse_matrix_[row * n + col];
    }

    std::vector<double> matrix_;
    std::vector<double> inverse_matrix_;
    std::vector<double> rhs_;
    std::vector<double> solution_;

    // Without extra clamp conductance, the matrix only depends on dt; cache its inverse.
    double cached_inverse_dt_ms_ = 0.0;
    bool cached_inverse_valid_ = false;
};

}  // namespace neuron
