# src/brain/main.cpp

`src/brain/main.cpp` 是 `worm_brain_runner` 的薄入口。它只负责解析 CLI、调用 `run_brain()`、在存在 diagnostics result 时委托 recorder 输出。

## 主流程

```text
parse_config(argc, argv, CPP_WORM_SIM_SOURCE_DIR)
run_brain(config)
if error: print error and return failure
if diagnostics result exists: write_brain_summary(result.diagnostics)
return exit code
```

`main.cpp` 不计算 summary，不遍历 network，不统计 voltage，也不排序 top current edges。

## diagnostics CLI

默认 diagnostics 关闭：

```powershell
.\build\worm_brain_runner.exe
```

显式开启 diagnostics summary：

```powershell
.\build\worm_brain_runner.exe --diagnostics summary
```

## 参数

| 参数 | 写入字段 |
| --- | --- |
| `--data-dir` | synapse v0 数据目录。 |
| `--chemical-csv` | `config.network.chemical_csv`。 |
| `--gap-csv` | `config.network.gap_csv`。 |
| `--neuron-reference-csv` | `config.network.neuron_reference_csv`。 |
| `--template-data-dir` | `config.network.template_data_dir`。 |
| `--dt-ms` | `config.simulation.dt_ms`。 |
| `--tstop-ms` | `config.tstop_ms`。 |
| `--diagnostics summary` | `config.diagnostics.enabled = true`。 |
| `--explode-voltage-mv` | `config.diagnostics.explode_voltage_mV`。 |
| `--top-current-edges` | `config.diagnostics.top_current_edges`。 |

## 调试建议

| 断点 | 看什么 |
| --- | --- |
| `parse_config()` 返回前 | CLI 是否解析到预期路径和 flags。 |
| `brain::run_brain(config)` | 进入 runner 调度。 |
| `result.diagnostics.has_value()` | 默认模式是否真的不输出 summary。 |
| `write_brain_summary()` | summary 输出是否只在显式开启时发生。 |


