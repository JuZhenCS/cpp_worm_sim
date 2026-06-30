#include "runner/runner.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

// 详细的函数职责、调用链和调试说明见 docs/runner/main.md。
int main(int argc, char** argv) {
    try {
        const auto config = cpp_neuron::runner::parse_config(argc, argv);
        cpp_neuron::runner::run(config);
        std::cout << "Wrote " << config.output << '\n';
        return EXIT_SUCCESS;
    } catch (const std::exception& exc) {
        std::cerr << "ERROR: " << exc.what() << '\n';
        return EXIT_FAILURE;
    }
}
