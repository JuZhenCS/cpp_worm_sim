# CLion Debug 配置：worm_brain_runner

这个页面对应 CLion 的 `Run/Debug Configurations` 窗口。

## 推荐配置

| 字段 | 填写 |
| --- | --- |
| `Name` | `worm_brain_runner_summary` 或 `worm_brain_runner_core` |
| `Target` | `worm_brain_runner` |
| `Executable` | `worm_brain_runner` |
| `Program arguments` | 见下面两种模式 |
| `Working directory` | `$ProjectFileDir$`，或者绝对路径 `E:\1 PhD work\C.elegans simulation\code\neural network\annotate-and-test-main` |
| `Environment variables` | 留空 |
| `Before launch` | `Build` |

建议使用 CLion 的 Debug CMake profile 构建和调试。如果当前 profile 是 Release，也能运行，但断点、变量查看和单步体验会差很多。

## Program arguments

### 调试核心 simulation，默认无 summary

```text
--tstop-ms 10 --dt-ms 0.1
```

这个模式不会创建 diagnostics collector，也不会打印 summary。适合看 `brain_runner -> step_brain_network` 的纯推进路径。

### 调试 summary / diagnostics

```text
--diagnostics summary --tstop-ms 10 --dt-ms 0.1 --top-current-edges 5
```

如果你想看到 summary 输出，必须使用 `--diagnostics summary`。你截图里的参数如果只有：

```text
--tstop-ms 10 --dt-ms 0.1 --top-current-edges 5
```

现在不会打印 summary，因为 diagnostics 默认关闭。

## 截图里的配置检查

你截图里这些项基本是对的：

```text
Target: worm_brain_runner
Executable: worm_brain_runner
Working directory: 项目根目录
Before launch: Build
```

需要注意两点：

1. `Program arguments` 最前面必须是完整的 `--tstop-ms`，不要漏掉前面的 `--tst`。
2. 如果要看 summary 输出，使用 `--diagnostics summary`。

推荐直接填：

```text
--diagnostics summary --tstop-ms 10 --dt-ms 0.1 --top-current-edges 5
```

## 建议断点

| 文件 | 位置 | 看什么 |
| --- | --- | --- |
| `src/brain/main.cpp` | `parse_config()` 返回前 | CLI 是否解析正确，尤其 `diagnostics.enabled`。 |
| `src/brain/brain_runner.cpp` | `run_brain()` 入口 | network config、dt、tstop。 |
| `src/brain/brain_runner.cpp` | `if (!config.diagnostics.enabled)` | 当前走 core-only 还是 summary 路径。 |
| `src/brain/brain_network_builder.cpp` | `build_brain_network()` | neuron 数、synapse 数、reference mapping。 |
| `src/brain/brain_simulation.cpp` | `step_brain_network()` | 单步顺序是否是 clear/apply/step。 |
| `src/brain/brain_simulation.cpp` | `network.synapses.apply(dt_ms)` 后 | 当前 step 的 chemical/gap current stats。 |
| `src/brain/diagnostics/brain_diagnostics_observe.cpp` | `observe_after_step()` | diagnostics summary 启用时的 min/max voltage、NaN、exploding 检测。 |

## 常用 Watch 表达式

```text
config.diagnostics.enabled
config.simulation.dt_ms
config.tstop_ms
network.neurons.size()
network.synapses.chemical_synapses.size()
network.synapses.gap_junctions.size()
stats.max_abs_chemical_current_pA
stats.max_abs_gap_current_pA
```



