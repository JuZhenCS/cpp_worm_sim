# src/brain/brain_network_builder.cpp

`brain_network_builder.cpp` 负责把 CSV 数据装配成一个可运行的 `BrainNetwork`：读取 302 neuron 模板映射，创建 neuron model，建立 neuron name 到 model 的索引，然后加载 chemical synapse 和 gap junction。

## 对应接口

```text
include/brain/brain_network_builder.hpp
```

核心导出函数：

```cpp
BrainNetwork build_brain_network(const BrainNetworkConfig& config);
```

## 输入

`BrainNetworkConfig` 提供四个路径：

| 字段 | 内容 |
| --- | --- |
| `chemical_csv` | neuron-to-neuron chemical component CSV，默认是 `data/synapse_v0/chemical_components_neuron_neuron_v0.csv`。 |
| `gap_csv` | gap junction CSV，默认是 `data/synapse_v0/gap_junctions_v0.csv`。 |
| `neuron_reference_csv` | 302 neuron 到代表模板的映射，默认是 `data/synapse_v0/neuron_parameter_reference_v0.csv`。 |
| `template_data_dir` | 五个代表 cell CSV 的根目录，默认是 `data/`。 |

## 输出

`BrainNetwork` 包含：

| 字段 | 内容 |
| --- | --- |
| `index` | `neuron::NeuronIndex`，从 neuron name 映射到 `NeuronModel`。 |
| `neurons` | 所有 neuron model 的数组。 |
| `neuron_names` | 与 `neurons` 同顺序的 neuron 名称。 |
| `synapses` | `neuron::SynapseNetwork`，包含 chemical synapses 和 gap junctions。 |

## 构建流程

```text
build_brain_network(config)
  -> collect_chemical_names(config.chemical_csv, names)
  -> collect_gap_names(config.gap_csv, names)
  -> load_neuron_references(config.neuron_reference_csv)
  -> for each sorted name:
       find neuron reference
       representative_neuron_template(reference.parameter_reference, template_data_dir)
       neuron::create_neuron(build_config)
       set initial voltage with small deterministic offset
       fill index / neurons / neuron_names
  -> neuron::load_synapse_network_csv(chemical_csv, gap_csv, network.index)
  -> return network
```

## 局部 helper

| 函数 | 作用 |
| --- | --- |
| `split_csv_line()` | 简单 CSV split，支持双引号内逗号。 |
| `header_index()` | 建立 header name 到 column index 的映射。 |
| `field()` | 按字段名读取当前 row，缺字段时抛错。 |
| `collect_chemical_names()` | 从 chemical CSV 的 `pre` / `post` 收集 neuron 名称。 |
| `collect_gap_names()` | 从 gap CSV 的 `cell_a` / `cell_b` 收集 neuron 名称。 |
| `load_neuron_references()` | 读取 `neuron_parameter_reference_v0.csv`，生成 neuron 到 parameter reference 的映射。builder 只需要 `parameter_reference` 来选择代表模板；`functional_group` 只属于 diagnostics 统计。 |

## 初始电压规则

每个 neuron 的基础初始电压来自 `representative_neuron_template()`：

```text
AWC -> -65.0 mV
AIY -> -45.0 mV
AVA -> -30.0 mV
RIM -> -39.3 mV
VD5 -> -75.0 mV
```

builder 还会加一个小的 deterministic offset：

```cpp
0.1 * (static_cast<double>(idx % 7) - 3.0)
```

作用是避免所有同模板 neuron 完全同初值。这个 offset 是当前 runner 行为的一部分。

## 失败条件

常见抛错位置：

| 条件 | 报错来源 |
| --- | --- |
| chemical CSV 打不开 | `collect_chemical_names()` |
| gap CSV 打不开 | `collect_gap_names()` |
| neuron reference CSV 打不开或为空 | `load_neuron_references()` |
| CSV 缺少必需字段 | `field()` |
| synapse CSV 中出现的 neuron 没有 reference | `build_brain_network()` |
| `parameter_reference` 不是 AWC / AIY / AVA / RIM / VD5 | `representative_neuron_template()` |
| cell CSV 打不开或格式错误 | `neuron::create_neuron()` / `load_cell_csv()` |
| synapse endpoint 不在 `network.index` 中 | `load_synapse_network_csv()` |

## 调试建议

| 断点 | 看什么 |
| --- | --- |
| `collect_chemical_names()` 结束后 | chemical CSV 中实际参与建模的 neuron 名称集合。 |
| `collect_gap_names()` 结束后 | gap CSV 是否引入额外 neuron。 |
| `load_neuron_references()` 返回前 | 是否解析到 302 条 neuron reference。 |
| `representative_neuron_template()` 调用处 | 每个 neuron 用哪个代表 cell CSV 和机制开关。 |
| `neuron::load_synapse_network_csv()` 前后 | neuron 已建好后，突触网络加载数量是否正确。 |

## 当前边界

这个模块不做数值仿真，不决定 synapse 方程，也不改变 cell CSV 内容。它只把数据文件装配成运行时对象。summary/diagnostics 需要的模板计数和 functional group 计数不保存在 `BrainNetwork` 中，而是在 diagnostics 开启时由 diagnostics 模块按需读取。
