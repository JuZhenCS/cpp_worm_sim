# cpp_worm_sim

This directory is the C++ runtime for the current C. elegans neuron and synapse work.

The neuron layer keeps the five validated multi-compartment representative cells:

- `AWCL`: sensory neuron representative, BAAIWorm reference `AWC`
- `AIYL`: interneuron representative, BAAIWorm reference `AIY`
- `AVAL`: command neuron representative, BAAIWorm reference `AVA`
- `RIML`: head motor neuron representative, BAAIWorm reference `RIM`
- `VD05`: body motor neuron representative, BAAIWorm reference `VD5`

The synapse layer depends only on the unified `NeuronModel` interface:

```cpp
name()
size()
voltage_mV(compartment_id)
soma_voltage_mV()
add_current_pA(compartment_id, current_pA)
clear_currents()
step(dt_ms)
```

Synapses do not depend on `PassiveNeuron` internals, channel details, or a specific representative class.

## Build

```powershell
cd "E:\1 PhD work\C.elegans simulation\code\cpp_worm_sim"
cmake -S . -B build -G Ninja
cmake --build build
```

If Ninja is unavailable:

```powershell
cd "E:\1 PhD work\C.elegans simulation\code\cpp_worm_sim"
cmake -S . -B build
cmake --build build --config Release
```

## Single-Cell Runs

AVAL current clamp:

```powershell
.\build\cpp_neuron_runner.exe --cell AVAL --protocol iclamp --cell-file data\aval\AVAL_cell.csv --amp-pa 10 --enable-nca --enable-shk1 --enable-shl1 --enable-egl19 --enable-cca1 --enable-unc2 --enable-calcium-internal --enable-kcnl --enable-slo1-unc2 --output output\aval_10pA.csv --diag-output output\aval_10pA_diag.csv
```

AIYL current clamp:

```powershell
.\build\cpp_neuron_runner.exe --cell AIYL --protocol iclamp --cell-file data\aiyl\AIYL_cell.csv --amp-pa 10 --enable-nca --enable-irk --enable-kqt3 --enable-egl2 --enable-shk1 --enable-kvs1 --enable-shl1 --enable-egl36 --enable-egl19 --enable-cca1 --enable-calcium-internal --enable-kcnl --enable-slo1-egl19 --enable-slo1-unc2 --enable-slo2-egl19 --enable-slo2-unc2 --output output\aiyl_10pA.csv --diag-output output\aiyl_10pA_diag.csv
```

RIML current clamp:

```powershell
.\build\cpp_neuron_runner.exe --cell RIML --protocol iclamp --cell-file data\riml\RIML_cell.csv --amp-pa 10 --enable-nca --enable-irk --enable-kqt3 --enable-egl2 --enable-shk1 --enable-kvs1 --enable-shl1 --enable-egl36 --enable-slo1-egl19 --enable-slo1-unc2 --enable-slo2-egl19 --output output\riml_10pA.csv --diag-output output\riml_10pA_diag.csv
```

AWCL voltage clamp:

```powershell
.\build\cpp_neuron_runner.exe --cell AWCL --protocol seclamp --cell-file data\awcl\AWCL_cell.csv --vcmd-mv 70 --enable-kqt3 --enable-shl1 --enable-egl19 --enable-unc2 --output output\awcl_v70.csv --diag-output output\awcl_v70_diag.csv
```

VD05 voltage clamp:

```powershell
.\build\cpp_neuron_runner.exe --cell VD05 --protocol seclamp --cell-file data\vd05\VD05_cell.csv --vcmd-mv 20 --enable-nca --enable-kqt3 --enable-egl2 --enable-shk1 --enable-shl1 --enable-egl36 --enable-egl19 --enable-cca1 --enable-slo1-unc2 --enable-slo2-egl19 --enable-slo2-unc2 --output output\vd05_v20.csv --diag-output output\vd05_v20_diag.csv
```

## Synapse v0

Implemented files:

- `include/cpp_neuron_core/synapse.hpp`
- `src/synapse.cpp`
- `include/cpp_neuron_core/synapse_loader.hpp`
- `src/synapse_loader.cpp`

The v0 chemical model uses graded excitatory/inhibitory components from Fenyves sign prediction:

```text
I_pA = 1000 * g_uS * s * (E_rev_mV - V_post_mV)
ds/dt = (s_inf(V_pre) - s) / tau
s_inf(V_pre) = 1 / (1 + exp(-(V_pre - V_half) / k_s))
```

The v0 gap junction model is ohmic and current-conserving:

```text
I_to_a_pA = 1000 * g_uS * (V_b_mV - V_a_mV)
I_to_b_pA = -I_to_a_pA
```

Conductances are already materialized in the exported CSVs. `complex` chemical connections are split into one excitatory and one inhibitory component with `rho=0.5` each. `no pred` chemical connections are retained in the anatomical table but disabled for v0 current.

## Synapse Data

The synapse tables are generated in:

```text
E:\1 PhD work\C.elegans simulation\code\C.elegans.network\synapse_v0
```

Main files:

- `chemical_connections_v0.csv`: Fenyves/Cook chemical connection table, including inactive `no pred` rows
- `chemical_components_v0.csv`: active excitatory/inhibitory components used by C++
- `gap_junctions_v0.csv`: Cook SI5 gap junctions
- `neuron_parameter_reference_v0.csv`: BAAIWorm Table 4 mapping from each neuron to one of `AWC/AIY/AVA/RIM/VD5`
- `synapse_config_v0.json`: v0 global parameters and disabled features

The full-network smoke test maps all 302 neurons to the five existing multi-compartment templates. It does not create single-compartment stand-ins.

## Verification

Build and run the synapse checks:

```powershell
cmake -S . -B build_synapse_tests
cmake --build build_synapse_tests --target synapse_tests synapse_smoke
.\build_synapse_tests\synapse_tests.exe
.\build_synapse_tests\synapse_smoke.exe --data-dir "E:\1 PhD work\C.elegans simulation\code\C.elegans.network\synapse_v0" --template-data-dir "E:\1 PhD work\C.elegans simulation\code\cpp_worm_sim\data" --tstop-ms 10 --dt-ms 0.1
```

Expected smoke-test scale:

```text
neurons=302
chemical_components=2693
gap_junctions=1093
nan_count=0
```

## Current Boundary

Implemented now:

- five representative multi-compartment neurons
- Fenyves/Cook chemical component export
- Cook gap junction export
- BAAIWorm Table 4 neuron-to-representative mapping
- graded chemical synapse current
- ohmic gap junction current
- CSV loaders and 302-neuron multi-compartment smoke test

Not implemented in v0:

- Wang transmitter/modulator mechanics
- monoamine modulation
- neuropeptide wireless layer
- STDP
- individual synapse-level receptor kinetics
- body/environment closed loop
