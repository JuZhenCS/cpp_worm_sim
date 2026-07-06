# data 目录

运行时数据目前分两类：

```text
data/synapse_v0/
    302-neuron 全脑网络连接、gap junction、synapse 参数和 neuron 模板映射

data/aiyl/ data/aval/ data/awcl/ data/riml/ data/vd05/
    五个代表 neuron 模板的 multi-compartment cell CSV
```

## 文档

- [synapse_v0.md](synapse_v0.md)
  说明 `data/synapse_v0/` 每个文件的用途、生成链路、字段、原始来源和重新导出命令。

- [representative_cells.md](representative_cells.md)
  说明五个代表 cell CSV 的来源、字段和重新导出命令。

## 代码消费路径

- `src/brain/main.cpp` 默认读取 `data/synapse_v0` 和 `data`。
- `src/brain/brain_network_builder.cpp` 读取 `neuron_parameter_reference_v0.csv`，再通过 `representative_neuron_catalog` 映射到五个 cell CSV。
- `src/synapse_loader.cpp` 读取 chemical component CSV 和 gap junction CSV。
- `src/neuron/cell_loader.cpp` 读取各代表 cell CSV。
- 后续 `brain4MuJoCo/muscle_output` 读取 `data/muscle/motor_to_muscle_projection_v0.csv`。

