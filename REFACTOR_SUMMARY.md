# Neuron runner/core refactor summary

## Renames

- Executable target: `cpp_neuron_runner` -> `neuron_runner`
- Runner support target: `cpp_neuron_runner_support` -> `neuron_runner_support`
- Core library target: `cpp_neuron_core` -> `neuron_core`
- Public include root: `include/cpp_neuron_core/...` -> `include/neuron/...`
- C++ namespace: `cpp_neuron` -> `neuron`
- Runner namespace: `cpp_neuron::runner` -> `neuron::neuron_runner`
- Runner source folder: `src/runner` -> `src/neuron_runner`
- Runner files: `runner.hpp/cpp` -> `neuron_runner.hpp/cpp`

## Structural cleanup

`MultiCompartmentNeuron` no longer owns every low-level helper directly. Three responsibilities were split out:

- `CableSolver`
  - `include/neuron/neuron/cable_solver.hpp`
  - `src/neuron/cable_solver.cpp`
  - Owns implicit cable-equation matrix buffers, inverse-cache logic, and direct solve fallback.

- Channel installation helpers
  - `include/neuron/neuron/channel_installer.hpp`
  - `src/neuron/channel_installer.cpp`
  - Keeps channel construction out of `MultiCompartmentNeuron`.

- Diagnostics helpers
  - `include/neuron/neuron/diagnostics.hpp`
  - `src/neuron/diagnostics.cpp`
  - Moves protocol diagnostic sampling out of `clamp_protocol.cpp`.

## Validation performed

Built with CMake + Ninja and ran:

```bash
cmake -S . -B build_refactor -G Ninja
cmake --build build_refactor
ctest --test-dir build_refactor --output-on-failure
```

Result: `runner_tests` and `synapse_tests` both passed.

Also ran an AIYL current-clamp smoke command using the new executable:

```bash
./build_refactor/neuron_runner --cell AIYL --protocol iclamp \
  --cell-file data/aiyl/AIYL_cell.csv \
  --amp-pa 10 --enable-nca \
  --output build_refactor/aiyl_iclamp.csv \
  --diag-output build_refactor/aiyl_iclamp_diag.csv
```

The generated trace and diagnostic CSV files were byte-identical to the original `cpp_neuron_runner` output for the same command.
