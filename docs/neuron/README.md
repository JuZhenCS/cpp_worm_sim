# Neuron模块文档索引

`docs/neuron/` 对应代码里的 `include/neuron/` 和 `src/neuron/`。子目录尽量和代码目录保持一致。

## 目录对应

```text
include/neuron/core/       src/neuron/*.cpp                 docs/neuron/core/
include/neuron/channels/   src/neuron/channels/             docs/neuron/channels/
include/neuron/protocol/   src/neuron/protocol/             docs/neuron/protocol/
include/neuron/recording/  src/neuron/recording/            docs/neuron/recording/
src/neuron/neuron_runner/  src/neuron/main.cpp              docs/neuron/neuron_runner/
```

## 推荐阅读顺序

1. [status.md](status.md)
   先看当前五个代表细胞的可用状态。

2. [core/neuron_model.hpp](../../include/neuron/core/neuron_model.hpp)
   网络和突触层依赖的最小 neuron 抽象。

3. [core/cell.md](core/cell.md)
   compartment、cell、通道所有权和深拷贝。

4. [core/calcium_internal.md](core/calcium_internal.md)
   多隔室模型调用的内部钙动力学。

5. [core/multi_compartment_neuron.md](core/multi_compartment_neuron.md)
   具体 neuron 状态、step 入口和矩阵求解。

6. [core/cell_loader.md](core/cell_loader.md)
   外部 CSV 如何变成 `Cell`。

7. [core/neuron_factory.md](core/neuron_factory.md)
   loader、具体模型和 mechanism 配置如何装配。

8. [channels/README.md](channels/README.md)
   active channel 实现边界。

9. [protocol/clamp_protocol.md](protocol/clamp_protocol.md)
   单细胞 clamp protocol。

10. [recording/csv_writer.md](recording/csv_writer.md)
    单细胞 trace/diagnostics CSV 输出。

11. [neuron_runner/main.md](neuron_runner/main.md)
    `neuron_runner` 可执行入口和 CLI 编排。

## 修改边界

- neuron 状态和积分改 `src/neuron/multi_compartment_neuron.cpp`；
- cell CSV 格式改 `src/neuron/cell_loader.cpp`；
- active channel 安装改 `src/neuron/channel_installer.cpp` 和 `src/neuron/channels/`；
- 创建和机制选择改 `src/neuron/neuron_factory.cpp`；
- 单细胞 protocol 改 `src/neuron/protocol/`；
- 单细胞输出改 `src/neuron/recording/`；
- 单细胞 CLI 改 `src/neuron/main.cpp` 和 `src/neuron/neuron_runner/`。