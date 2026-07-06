# src/brain/brain_runner.cpp

`brain_runner.cpp` 是全脑模拟的高层调度层。它负责 build network、计算 step 数、决定是否启用 diagnostics，并调用纯 simulation API。

## 对应接口

```text
include/brain/brain_runner.hpp
```

## API

```cpp
BrainRunnerResult run_brain(const BrainRunnerConfig& config);
```

## 配置

| 字段 | 含义 |
| --- | --- |
| `network` | `BrainNetworkConfig`，CSV 和模板数据路径。 |
| `simulation` | `BrainSimulationConfig`，当前主要是 `dt_ms`。 |
| `diagnostics` | `BrainDiagnosticsConfig`，控制 summary 是否启用。 |
| `tstop_ms` | 总仿真时长。 |

## 行为

### diagnostics 关闭

```text
build_brain_network(config.network)
run_brain_steps(network, dt_ms, steps)
return success
```

不会读取 diagnostics metadata，不会创建 `BrainDiagnosticsCollector`，不会收集 summary，也不会打印 summary。

### diagnostics 开启

```text
build_brain_network(config.network)
load_brain_diagnostics_metadata_csv(config.network.neuron_reference_csv)
create BrainDiagnosticsCollector
for each step:
    stats = step_brain_network(network, dt_ms)
    diagnostics.observe_after_step(network, stats, time_ms)
result.diagnostics = diagnostics.finish(network, tstop_ms, dt_ms)
return success
```

## 错误处理

`run_brain()` 捕获 `std::exception`，写入：

```text
BrainRunnerResult.ok = false
BrainRunnerResult.error_message = exc.what()
```

simulation 正常完成时，`ok` 表示运行调度成功；diagnostics 自身是否发现 NaN / exploding neuron 由 `BrainDiagnosticsResult::ok()` 表示。

## 调试建议

| 断点 | 看什么 |
| --- | --- |
| `build_brain_network(config.network)` | network 是否成功构建。 |
| `simulation_steps()` | `tstop_ms / dt_ms` 算出的 step 数。 |
| `if (!config.diagnostics.enabled)` | 当前是否走默认无 summary 路径。 |
| `step_brain_network()` 调用处 | 每步纯推进是否正常。 |
| `load_brain_diagnostics_metadata_csv()` | metadata 统计是否只在 enabled 时读取。 |
| `diagnostics.observe_after_step()` | summary 统计是否只在 enabled 时发生。 |
