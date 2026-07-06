# worm_brain_runner 说明

`worm_brain_runner` 是当前 C++ 运行时的 302-neuron 线虫脑网络模拟入口，对应：

```text
include/brain/
src/brain/
```

## 职责

```text
加载 302 个神经元
加载 chemical synapse / gap junction
构建线虫脑网络
运行一段时间
按需输出 diagnostics summary
```

默认运行时 diagnostics 关闭：不收集 summary，不打印 summary，成功时安静返回 0。

## 模块边界

| 模块 | 职责 |
| --- | --- |
| [`representative_neuron_catalog`](representative_neuron_catalog.md) | AWC / AIY / AVA / RIM / VD5 代表模板、机制开关和初始电压 |
| [`brain_network_builder`](brain_network_builder.md) | 读取 `neuron_parameter_reference_v0.csv`，创建 neuron，调用 `load_synapse_network_csv()` |
| [`brain_simulation`](brain_simulation.md) | 纯状态推进：`clear_currents -> synapses.apply -> neuron.step` |
| [`brain_runner`](brain_runner.md) | build network、计算 steps、按 config 决定是否启用 diagnostics |
| [`diagnostics/brain_diagnostics_observe`](diagnostics/brain_diagnostics_observe.md) | 可选 summary/statistics 收集 |
| [`diagnostics/brain_diagnostics_recorder`](diagnostics/brain_diagnostics_recorder.md) | 写出已经收集好的 diagnostics summary |
| [`src/brain/main.cpp`](main.md) | CLI 参数解析、调用 runner、按需委托 recorder 输出 |

## 代码说明文档

| C++ 文件 | 说明文档 |
| --- | --- |
| `src/brain/main.cpp` | [main.md](main.md) |
| `src/brain/brain_network_builder.cpp` | [brain_network_builder.md](brain_network_builder.md) |
| `src/brain/brain_simulation.cpp` | [brain_simulation.md](brain_simulation.md) |
| `src/brain/brain_runner.cpp` | [brain_runner.md](brain_runner.md) |
| `src/brain/diagnostics/brain_diagnostics_observe.cpp` | [diagnostics/brain_diagnostics_observe.md](diagnostics/brain_diagnostics_observe.md) |
| `src/brain/diagnostics/brain_diagnostics_recorder.cpp` | [diagnostics/brain_diagnostics_recorder.md](diagnostics/brain_diagnostics_recorder.md) |
| `src/brain/representative_neuron_catalog.cpp` | [representative_neuron_catalog.md](representative_neuron_catalog.md) |

## 调试配置

CLion Run/Debug Configuration 填写见 [clion_debug.md](clion_debug.md)。

## 运行命令

构建：

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target worm_brain_runner --config Release
```

默认无 diagnostics：

```powershell
.\build\worm_brain_runner.exe
```

显式开启 diagnostics summary：

```powershell
.\build\worm_brain_runner.exe --diagnostics summary
```

## 数据来源

运行时默认数据来自 `data/synapse_v0/` 和 `data/{aiyl,aval,awcl,riml,vd05}/`。这些 CSV / JSON 的原始来源和重新导出命令见 [../data/README.md](../data/README.md)。

## 参数

| 参数 | 默认值 | 说明 |
| --- | --- | --- |
| `--data-dir` | `<source>/data/synapse_v0` | synapse v0 数据目录 |
| `--chemical-csv` | `chemical_components_neuron_neuron_v0.csv` | neuron-to-neuron chemical component CSV |
| `--gap-csv` | `gap_junctions_v0.csv` | gap junction CSV |
| `--neuron-reference-csv` | `neuron_parameter_reference_v0.csv` | neuron 到代表模板的映射 |
| `--template-data-dir` | `<source>/data` | 五个代表性 cell CSV 的根目录 |
| `--dt-ms` | `0.1` | 积分步长 |
| `--tstop-ms` | `10.0` | 模拟时长 |
| `--diagnostics summary` | off | 启用 diagnostics summary 收集和输出 |
| `--explode-voltage-mv` | `100.0` | diagnostics summary 启用时，电压绝对值超过该阈值记为 exploding |
| `--top-current-edges` | `10` | diagnostics summary 启用时，输出 peak current 最大的边数 |

## 当前边界

本阶段只把 diagnostics/summary 从核心 simulation 中拆出，不改变 neuron、channel、synapse 或数值积分逻辑。全 compartment 电压 trace 和更复杂的 CSV 记录器还没有加入。





