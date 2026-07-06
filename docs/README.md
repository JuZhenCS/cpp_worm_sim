# 文档目录

文档目录按代码模块组织：

```text
include/brain/      src/brain/      docs/brain/
include/brain4MuJoCo/ src/brain4MuJoCo/ docs/brain4MuJoCo/
include/neuron/     src/neuron/     docs/neuron/
include/synapse/    src/synapse*.cpp docs/synapse/
data/             scripts/         docs/data/
```

## 模块入口

- [brain/worm_brain_runner.md](brain/worm_brain_runner.md)
  302-neuron 全脑网络模拟入口、网络构建、仿真循环和 summary 输出。

- [neuron/README.md](neuron/README.md)
  单神经元模型、channels、protocol、recording 和 `neuron_runner`。

- [synapse/README.md](synapse/README.md)
  chemical synapse、gap junction 和 synapse CSV loader。

- [data/README.md](data/README.md)
  运行时 CSV / JSON 数据、原始来源和重新导出脚本。

- [NEXT_STEPS.md](NEXT_STEPS.md)
  当前工程后续事项。

## 维护规则

新增代码文档时，优先放到与代码目录同名的位置。例如：

- `src/brain/foo.cpp` -> `docs/brain/foo.md`
- `src/neuron/core-ish file` / `include/neuron/core/foo.hpp` -> `docs/neuron/core/foo.md`
- `src/neuron/channels/foo.cpp` -> `docs/neuron/channels/foo.md`
- `src/neuron/protocol/foo.cpp` -> `docs/neuron/protocol/foo.md`
- `src/neuron/recording/foo.cpp` -> `docs/neuron/recording/foo.md`
- `src/synapse.cpp` / `src/synapse_loader.cpp` -> `docs/synapse/`
- `data/...` / `scripts/export_*.py` -> `docs/data/`


