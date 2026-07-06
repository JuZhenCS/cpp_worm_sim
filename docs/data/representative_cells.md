# representative cell CSV 数据

`data/aiyl/`、`data/aval/`、`data/awcl/`、`data/riml/`、`data/vd05/` 保存五个代表 neuron 模板：

```text
AIYL_cell.csv
AVAL_cell.csv
AWCL_cell.csv
RIML_cell.csv
VD05_cell.csv
```

`worm_brain_runner` 不为 302 个 neuron 各自读取独立 morphology 参数；它先读取 `data/synapse_v0/neuron_parameter_reference_v0.csv`，再通过 `representative_neuron_catalog` 将每个 neuron 映射到上面五个代表 cell CSV 之一。

## 导出脚本

当前仓库内脚本：

```text
scripts/export_cell_from_baaiworm.py
```

它从已经验证过的 BAAIWorm / NEST diagnostics 结果中合并两类文件：

```text
<diagnostics-dir>/<cell>_compartment_table.csv
<diagnostics-dir>/<cell>_nest_params_preview.json
```

默认 diagnostics 目录来自环境变量 `CPP_WORM_VALIDATED_NEURON_DIAGNOSTICS`。如果没有设置，脚本会按顺序尝试当前机器的两个历史路径：

```text
E:\1 PhD work\C.elegans simulation\code\0522_sysTest\validated_neuron_layer\build\diagnostics
E:\1 PhD work\C.elegans simulation\code\draft\0522_sysTest\validated_neuron_layer\build\diagnostics
```

## 重新导出

导出全部五个代表 cell：

```powershell
python scripts\export_cell_from_baaiworm.py `
  --cell all `
  --diagnostics-dir "E:\1 PhD work\C.elegans simulation\code\0522_sysTest\validated_neuron_layer\build\diagnostics" `
  --output-dir data
```

只导出一个 cell：

```powershell
python scripts\export_cell_from_baaiworm.py --cell AIYL --output-dir data
```

## 字段来源

- `idx`、`parent_idx`、`label`、`section`、`seg`、`nseg`、`area_um2` 来自 compartment table。
- `C_m_pF`、`g_L_nS`、`e_L_mV`、`g_C_nS` 和 active mechanism conductance 来自 `*_nest_params_preview.json`。
- active mechanism 字段包括 `gbnca`、`gbirk`、`gbkqt3`、`gbegl2`、`gbshk1`、`gbkvs1`、`gbshl1`、`gbegl36`、`gbegl19`、`gbcca1`、`gbunc2`、`gbkcnl`、`gbslo1_egl19`、`gbslo1_unc2`、`gbslo2_egl19`、`gbslo2_unc2`。

这些 CSV 是 C++ `load_cell_csv()` 的直接输入。字段名变更必须同步修改 `src/neuron/cell_loader.cpp`。

