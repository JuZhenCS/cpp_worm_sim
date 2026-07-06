#include "neuron/neuron_runner/runner.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

int main(int argc, char** argv) {
    try {
        const auto config = neuron::runner::parse_config(argc, argv);
        neuron::runner::run(config);
        std::cout << "Wrote " << config.output << '\n';
        return EXIT_SUCCESS;
    } catch (const std::exception& exc) {
        std::cerr << "ERROR: " << exc.what() << '\n';
        return EXIT_FAILURE;
    }
}
 