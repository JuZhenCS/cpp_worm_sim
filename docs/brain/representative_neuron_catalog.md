# src/brain/representative_neuron_catalog.cpp

`representative_neuron_catalog.cpp` 负责把 BAAIWorm 的五类 `parameter_reference` 映射到当前 C++ runner 可用的代表 neuron 模板。它决定三件事：cell CSV 文件、启用哪些机制、初始电压。

## 对应接口

```text
include/brain/representative_neuron_catalog.hpp
```

核心导出函数：

```cpp
RepresentativeNeuronTemplate representative_neuron_template(
    const std::string& reference,
    const std::string& template_data_dir);
```

## 输入输出

输入：

| 参数 | 含义 |
| --- | --- |
| `reference` | `neuron_parameter_reference_v0.csv` 中的 `parameter_reference`，必须是 AWC / AIY / AVA / RIM / VD5。 |
| `template_data_dir` | 五个代表 cell CSV 的根目录，默认是 `data/`。 |

输出 `RepresentativeNeuronTemplate`：

| 字段 | 含义 |
| --- | --- |
| `cell_file` | 代表 cell CSV 路径。 |
| `mechanisms` | `neuron::NeuronMechanismConfig`，决定启用哪些 active mechanisms。 |
| `initial_voltage_mV` | 该代表模板的基础初始电压。 |

## reference 到 cell CSV

| reference | cell CSV | 初始电压 |
| --- | --- | --- |
| `AWC` | `<template_data_dir>/awcl/AWCL_cell.csv` | `-65.0 mV` |
| `AIY` | `<template_data_dir>/aiyl/AIYL_cell.csv` | `-45.0 mV` |
| `AVA` | `<template_data_dir>/aval/AVAL_cell.csv` | `-30.0 mV` |
| `RIM` | `<template_data_dir>/riml/RIML_cell.csv` | `-39.3 mV` |
| `VD5` | `<template_data_dir>/vd05/VD05_cell.csv` | `-75.0 mV` |

## mechanism 开关

| reference | 启用机制 |
| --- | --- |
| `AWC` | `kqt3`, `shl1`, `egl19`, `unc2` |
| `AIY` | `nca`, `irk`, `kqt3`, `egl2`, `shk1`, `kvs1`, `shl1`, `egl36`, `egl19`, `cca1`, `calcium_internal`, `kcnl`, `slo1_egl19`, `slo1_unc2`, `slo2_egl19`, `slo2_unc2` |
| `AVA` | `nca`, `shk1`, `shl1`, `egl19`, `cca1`, `unc2`, `calcium_internal`, `kcnl`, `slo1_unc2` |
| `RIM` | `nca`, `irk`, `kqt3`, `egl2`, `shk1`, `kvs1`, `shl1`, `egl36`, `slo1_egl19`, `slo1_unc2`, `slo2_egl19` |
| `VD5` | `nca`, `kqt3`, `egl2`, `shk1`, `shl1`, `egl36`, `egl19`, `cca1`, `slo1_unc2`, `slo2_egl19`, `slo2_unc2` |

## 调用位置

`brain_network_builder.cpp` 在创建每个 neuron 时调用：

```text
representative_neuron_template(reference.parameter_reference, config.template_data_dir)
```

然后把返回的 `cell_file` 和 `mechanisms` 放进 `neuron::NeuronBuildConfig`，交给 `neuron::create_neuron()`。

## 失败条件

如果 `reference` 不是 AWC / AIY / AVA / RIM / VD5，会抛出：

```text
Unknown neuron parameter reference: <reference>
```

这通常表示 `neuron_parameter_reference_v0.csv` 内容和当前 catalog 不匹配。

## 调试建议

| 断点 | 看什么 |
| --- | --- |
| `mechanisms_for_reference()` | 某个 reference 是否启用了预期机制。 |
| `representative_neuron_template()` 返回前 | cell CSV 路径和初始电压是否正确。 |
| 抛错分支 | CSV 中是否出现了未知 `parameter_reference`。 |

## 当前边界

这个模块只是 v0 模板映射表。它不解析 BAAIWorm 原始数据，也不生成 cell CSV；这些来源见 `docs/data/representative_cells.md` 和 `docs/data/synapse_v0.md`。

如果以后要把 302 个 neuron 做成各自独立参数，这个模块应该被替换或降级为 fallback，而不是继续硬塞更多特殊规则。
