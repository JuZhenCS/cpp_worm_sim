# src/brain/diagnostics/brain_diagnostics_observe.cpp

`brain_diagnostics_observe.cpp` 是可选 diagnostics collector。它只在 `BrainDiagnosticsConfig.enabled == true` 时由 `brain_runner` 创建。

## 对应接口

```text
include/brain/diagnostics/brain_diagnostics_observe.hpp
```

## 职责

```text
收集 min/max soma voltage
统计 NaN
记录首次 exploding neuron
累计 max chemical/gap current
在 finish 时生成 top current edges
生成 BrainDiagnosticsResult
按需读取 diagnostics metadata
```

它不推进 simulation，不调用 `clear_currents()`，不调用 `synapses.apply()`，不调用 `neuron.step()`。

## 主要结构

| 类型 | 作用 |
| --- | --- |
| `BrainDiagnosticsConfig` | 控制 diagnostics 是否启用、爆电压阈值、top edge 数量。 |
| `BrainDiagnosticsMetadata` | diagnostics 专用 metadata：parameter reference 计数和 functional group 计数。 |
| `ExplodingNeuron` | 记录第一次异常的 neuron 名称、时间、电压。 |
| `TopCurrentEdge` | 记录 peak current 最大的 chemical / gap edge。 |
| `BrainDiagnosticsResult` | summary recorder 使用的完整结果。 |
| `BrainDiagnosticsCollector` | 每步观察 network 状态并累计统计。 |

## 生命周期

```text
load_brain_diagnostics_metadata_csv(neuron_reference_csv)
  -> read parameter_reference / functional_group counts

BrainDiagnosticsCollector(network, config, metadata)
  -> copy diagnostics metadata
  -> initialize min/max voltage
  -> allocate exploding_neuron_seen

observe_after_step(network, synapse_stats, time_ms)
  -> update max current
  -> read soma_voltage_mV() for every neuron
  -> update min/max / NaN / exploding list

finish(network, tstop_ms, dt_ms)
  -> fill tstop/dt
  -> sort top current edges from synapse objects
  -> return BrainDiagnosticsResult
```

## `ok()` 语义

`BrainDiagnosticsResult::ok()` 为 true 的条件：

```text
nan_count == 0
exploding_neurons.empty()
min_voltage_mV is finite
max_voltage_mV is finite
```

## 调试建议

| 断点 | 看什么 |
| --- | --- |
| `load_brain_diagnostics_metadata_csv()` | `neuron_parameter_reference_v0.csv` 是否读到 302 个 neuron 的模板和功能分组统计。 |
| constructor | metadata 是否正确传入，network 是否已有 302 neurons、4308 chemical、1093 gap。 |
| `observe_after_step()` | 每步 voltage / current 统计是否合理。 |
| `exploding_neurons.push_back(...)` | 哪个 neuron 第一次异常。 |
| `finish()` | top current edges 是否排序正确。 |
