#pragma once

#include <cstddef>
#include <string>

namespace cpp_neuron {

class NeuronModel {
public:
    virtual ~NeuronModel() = default;

    virtual const std::string& name() const = 0;
    virtual std::size_t size() const = 0;
    virtual double voltage_mV(std::size_t compartment_index) const = 0;
    virtual double soma_voltage_mV() const = 0;

    virtual void set_all_voltages(double voltage_mV) = 0;
    virtual void add_current_pA(std::size_t compartment_index, double current_pA) = 0;
    virtual void clear_currents() = 0;
    virtual void step(double dt_ms) = 0;
};

}  // namespace cpp_neuron
