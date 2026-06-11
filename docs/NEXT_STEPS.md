# Next Steps

The current runtime has the neuron layer and the first synapse layer connected through `NeuronModel`.

Completed for SynapseModel-v0:

1. Five multi-compartment representative neuron templates are available: `AWCL`, `AIYL`, `AVAL`, `RIML`, `VD05`.
2. BAAIWorm Table 4 mapping is exported as `neuron_parameter_reference_v0.csv`.
3. Fenyves chemical sign prediction is exported as excitatory/inhibitory graded components.
4. Cook SI5 gap junctions are exported as ohmic coupling rows.
5. C++ loaders build a `SynapseNetwork` from the exported CSV files.
6. Full 302-neuron smoke test maps every neuron to one of the five multi-compartment templates.
7. Full-network stability gates have been run at 1 s, 10 s, and 60 s with `dt=0.5 ms`.
8. Stability output includes min/max voltage, peak chemical/gap currents, NaN count, exploding neuron list, and top current edges.

Immediate next engineering steps:

1. Replace the fixed compartment choice `pre_compartment=0, post_compartment=0` with an explicit v0 placement policy.
   For now all synapses target soma compartment 0. The next version should either keep that as a named policy or add a deterministic representative-compartment mapping.

2. Add a small named circuit regression.
   Use a small subset such as AVA/RIM/AIY/VD-related nodes and verify loaded component counts, voltage ranges, and gap current conservation over a longer interval.

3. Add a stricter stability regression target.
   The current 1 s/10 s/60 s gates are executable checks. A useful next gate is a committed expected-range test for min/max voltages and top current edge identities.

4. Move shared CSV parsing helpers out of test code if another executable needs them.
   The production C++ loaders already parse synapse CSVs; `synapse_smoke.cpp` also parses the BAAIWorm reference CSV for test construction.

5. Keep v0 scope constrained.
   Do not add Wang transmitter mechanics, monoamine modulation, neuropeptides, STDP, receptor-specific kinetics, or body/environment coupling until the Cook/Fenyves synapse layer is stable.

Current validation commands:

```powershell
cmake -S . -B build_synapse_tests -DCMAKE_BUILD_TYPE=Release
cmake --build build_synapse_tests --target synapse_tests synapse_smoke
.\build_synapse_tests\synapse_tests.exe
.\build_synapse_tests\synapse_smoke.exe --tstop-ms 10 --dt-ms 0.1
.\build_synapse_tests\synapse_smoke.exe --tstop-ms 1000 --dt-ms 0.5 --top-current-edges 10
.\build_synapse_tests\synapse_smoke.exe --tstop-ms 10000 --dt-ms 0.5 --top-current-edges 10
.\build_synapse_tests\synapse_smoke.exe --tstop-ms 60000 --dt-ms 0.5 --top-current-edges 10
```
