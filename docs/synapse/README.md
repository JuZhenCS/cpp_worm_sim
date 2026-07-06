# Synapse模块阅读顺序

突触层现在只保留核心模型和 CSV loader。全脑网络装配与运行已经提升到 `brain` 模块。

## 目录对应

```text
include/synapse/      突触公共头文件
src/synapse.cpp       chemical synapse 与 gap junction 实现
src/synapse_loader.cpp      chemical/gap CSV 到 SynapseNetwork 的 loader
docs/synapse/         突触层说明
```

## 推荐阅读顺序

1. [synapse.hpp](../../include/synapse/synapse.hpp)
   理解 `GradedChemicalSynapse`、`GapJunction` 和电流符号约定。

2. [synapse_loader.hpp](../../include/synapse/synapse_loader.hpp)
   理解 `NeuronIndex`、`SynapseNetwork` 和 CSV loader 对外接口。

3. [worm_brain_runner](../brain/worm_brain_runner.md)
   看突触 loader 如何被 302-neuron 全脑网络入口调用。

## 修改边界

- 单个突触公式改 `src/synapse.cpp`；
- CSV schema 和对象装配改 `src/synapse_loader.cpp`；
- 全脑网络构建、代表 neuron 模板选择和仿真循环改 `src/brain/`。