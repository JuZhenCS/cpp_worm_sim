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

Synapses do not depend on `MultiCompartmentNeuron` internals, channel details, or a specific representative class.

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

Developer documentation for the single-cell runner:

- [entrypoint and runner flow](docs/neuron_runner/main.md)
- [header architecture](docs/SINGLE_CELL_RUNNER_HEADERS.md)
- [neuron module reading order](docs/neuron/README.md)
- implementation notes including
  [runner](docs/neuron_runner/neuron_runner.md),
  [cell loader](docs/neuron/cell_loader.md),
  [factory](docs/neuron/neuron_factory.md),
  [multi-compartment solver](docs/neuron/multi_compartment_neuron.md),
  [clamp protocols](docs/protocol/clamp_protocol.md), and
  [CSV output](docs/recording/csv_writer.md)

AVAL current clamp:

```powershell
.\build\neuron_runner.exe --cell AVAL --protocol iclamp --cell-file data\aval\AVAL_cell.csv --amp-pa 10 --enable-nca --enable-shk1 --enable-shl1 --enable-egl19 --enable-cca1 --enable-unc2 --enable-calcium-internal --enable-kcnl --enable-slo1-unc2 --output output\aval_10pA.csv --diag-output output\aval_10pA_diag.csv
```

AIYL current clamp:

```powershell
.\build\neuron_runner.exe --cell AIYL --protocol iclamp --cell-file data\aiyl\AIYL_cell.csv --amp-pa 10 --enable-nca --enable-irk --enable-kqt3 --enable-egl2 --enable-shk1 --enable-kvs1 --enable-shl1 --enable-egl36 --enable-egl19 --enable-cca1 --enable-calcium-internal --enable-kcnl --enable-slo1-egl19 --enable-slo1-unc2 --enable-slo2-egl19 --enable-slo2-unc2 --output output\aiyl_10pA.csv --diag-output output\aiyl_10pA_diag.csv
```

RIML current clamp:

```powershell
.\build\neuron_runner.exe --cell RIML --protocol iclamp --cell-file data\riml\RIML_cell.csv --amp-pa 10 --enable-nca --enable-irk --enable-kqt3 --enable-egl2 --enable-shk1 --enable-kvs1 --enable-shl1 --enable-egl36 --enable-slo1-egl19 --enable-slo1-unc2 --enable-slo2-egl19 --output output\riml_10pA.csv --diag-output output\riml_10pA_diag.csv
```

AWCL voltage clamp:

```powershell
.\build\neuron_runner.exe --cell AWCL --protocol seclamp --cell-file data\awcl\AWCL_cell.csv --vcmd-mv 70 --enable-kqt3 --enable-shl1 --enable-egl19 --enable-unc2 --output output\awcl_v70.csv --diag-output output\awcl_v70_diag.csv
```

VD05 voltage clamp:

```powershell
.\build\neuron_runner.exe --cell VD05 --protocol seclamp --cell-file data\vd05\VD05_cell.csv --vcmd-mv 20 --enable-nca --enable-kqt3 --enable-egl2 --enable-shk1 --enable-shl1 --enable-egl36 --enable-egl19 --enable-cca1 --enable-slo1-unc2 --enable-slo2-egl19 --enable-slo2-unc2 --output output\vd05_v20.csv --diag-output output\vd05_v20_diag.csv
```

## Synapse v0

Implemented files:

- `include/neuron/synapse.hpp`
- `src/synapse.cpp`
- `include/neuron/synapse_loader.hpp`
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

Conductances are already materialized in the exported CSVs. Cook 2019 SI5 is the primary chemical structure source. Fenyves 2020 supplies the sign. `complex` chemical connections are split into one excitatory and one inhibitory component with `rho=0.5` each. Fenyves `no pred` and missing Fenyves signs default to inhibitory in v0.

## Synapse Data

The required synapse v0 runtime tables are included in this repository:

```text
data\synapse_v0
```

Main runtime files:

- `chemical_components_neuron_neuron_v0.csv`: active neuron-to-neuron excitatory/inhibitory components used by the current C++ 302-neuron runtime
- `gap_junctions_v0.csv`: Cook SI5 gap junctions
- `neuron_parameter_reference_v0.csv`: BAAIWorm Table 4 mapping from each neuron to one of `AWC/AIY/AVA/RIM/VD5`
- `synapse_config_v0.json`: v0 global parameters and disabled features

The full Cook/Fenyves chemical export is generated outside this repository. It includes:

- `chemical_connections_v0.csv`: Cook SI5 chemical structure plus Fenyves polarity annotation
- `chemical_components_v0.csv`: full active component table, including neuron-to-non-neuron or muscle/NMJ targets

The full-network smoke test maps all 302 neurons to the five existing multi-compartment templates. It does not create single-compartment stand-ins.

## Verification

Build and run the synapse checks:

```powershell
cmake -S . -B build_synapse_tests -DCMAKE_BUILD_TYPE=Release
cmake --build build_synapse_tests --target synapse_tests synapse_smoke
.\build_synapse_tests\synapse_tests.exe
.\build_synapse_tests\synapse_smoke.exe --tstop-ms 10 --dt-ms 0.1
```

The smoke/stability executable reports min/max soma voltage, peak chemical/gap current, NaN count, exploding neurons, and top current edges. `exploding_neurons` uses `--explode-voltage-mv` with default threshold `100 mV`.

Longer full-network stability gates:

```powershell
.\build_synapse_tests\synapse_smoke.exe --tstop-ms 1000 --dt-ms 0.5 --top-current-edges 10
.\build_synapse_tests\synapse_smoke.exe --tstop-ms 10000 --dt-ms 0.5 --top-current-edges 10
.\build_synapse_tests\synapse_smoke.exe --tstop-ms 60000 --dt-ms 0.5 --top-current-edges 10
```

Current stability result, Release build with OpenMP enabled:

```text
neurons=302
chemical_components=4308
gap_junctions=1093
min_voltage_mV=-73.90989443
max_voltage_mV=13.46289298
max_chemical_current_pA=18.57103727
max_gap_current_pA=2.567083344
nan_count=0
exploding_neurons=0
top_current_edge rank=1 type=chemical label=SAAVR__IL1DL__exc peak_abs_current_pA=18.57103727
top_current_edge rank=2 type=chemical label=RIML__AIBR__exc peak_abs_current_pA=14.50091966
top_current_edge rank=3 type=chemical label=IL1L__RMDDL__exc peak_abs_current_pA=14.44405289
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
