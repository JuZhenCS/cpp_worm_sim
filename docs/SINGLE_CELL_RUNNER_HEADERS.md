# 单细胞Runner头文件架构

本文档总览 `neuron_runner.exe` 直接或间接依赖的头文件。目标是让每个头文件只表达一种稳定概念，避免应用入口、数值模型和文件I/O互相渗透。

## 分层

```text
应用入口
  src/neuron_runner/neuron_runner.hpp
       |
用例层
  clamp_protocol.hpp
  neuron_factory.hpp
       |
领域模型
  neuron_model.hpp
  multi_compartment_neuron.hpp
  channel.hpp
  calcium_internal.hpp
       |
领域数据
  cell.hpp
  recording.hpp
       |
I/O边界
  cell_loader.hpp
  csv_writer.hpp
```

依赖方向只能从上向下。领域模型不应依赖 CLI、runner 或具体输出路径。

## 头文件职责

### src/neuron_runner/neuron_runner.hpp

应用层接口，不属于 `neuron` 公共领域接口。它定义 `ProtocolKind`、`RunnerConfig`、`parse_config()` 和 `run()`，把命令行边界与核心库连接起来。

### channel.hpp

所有active ion channel的抽象接口，负责更新门控状态、接收可选钙浓度、计算电流、暴露诊断状态并通过 `clone()` 支持多态深拷贝。具体channel类只依赖该接口。

### neuron/core/cell.hpp

定义 `Compartment` 和 `Cell`。它们是多隔室细胞的领域数据，不再混放trace或CSV类型。`Compartment` 拥有 `unique_ptr<Channel>`，因此实现了通道对象的深拷贝。

### neuron/core/neuron_model.hpp

网络和突触层使用的最小神经元抽象，只暴露名称、compartment数量、电压、电流注入和时间推进。synapse层不依赖具体多隔室实现。

### neuron/core/multi_compartment_neuron.hpp

`NeuronModel` 的具体实现，负责持有 `Cell`、多隔室隐式求解、外部电流、额外conductance、通道挂载、内部钙更新和诊断电流。原文件名 `neuron.hpp` 过于宽泛，现使用具体类名。

### neuron/core/neuron_factory.hpp

神经元装配接口，定义 `NeuronMechanismConfig`、`NeuronBuildConfig` 以及具体/抽象创建函数。配置不叫channel config，因为其中还包含内部钙动力学开关。

### recording/recording.hpp

纯仿真记录数据，定义 `TracePoint`、`ChannelDiagnosticPoint` 和 `ProtocolResult`。它不负责文件输出。

### protocol/clamp_protocol.hpp

单细胞clamp用例接口，定义 `IClampProtocol`、`SEClampProtocol` 及两种运行函数。原文件名 `protocol.hpp` 范围大于实际功能。

### neuron/core/cell_loader.hpp

从CSV加载 `Cell` 的I/O边界。CSV表头、必填字段和默认值等细节都留在实现文件。

### recording/csv_writer.hpp

把 `recording.hpp` 中的数据写为trace CSV和diagnostics CSV。它不参与仿真，也不定义记录类型。

### neuron/core/calcium_internal.hpp

内部钙浓度的一阶动力学计算器。它不拥有compartment，由 `MultiCompartmentNeuron` 在时间步中调用。

## 本次拆分

| 原设计 | 问题 | 当前设计 |
| --- | --- | --- |
| `types.hpp` | cell与trace混合 | `cell.hpp` + `recording.hpp` |
| `recorder.hpp` | 数据结构与CSV输出混合 | `recording.hpp` + `csv_writer.hpp` |
| `neuron.hpp` | 文件名无法说明具体实现 | `multi_compartment_neuron.hpp` |
| `protocol.hpp` | 名称范围大于功能 | `clamp_protocol.hpp` |
| `NeuronChannelConfig` | 实际还控制内部钙 | `NeuronMechanismConfig` |

## Include规则

1. 使用某个类型时直接包含定义该类型的头文件。
2. 不依赖另一个头文件碰巧产生的间接include。
3. 数据头文件不包含I/O实现。
4. 抽象接口不包含具体channel类。
5. runner专用接口留在 `src`，不伪装成核心库公共API。
6. 不重新引入 `types.hpp`、`utils.hpp` 之类职责模糊的容器文件。

## 扩展检查

新增channel时检查：channel实现、`Compartment` conductance、cell loader字段、mechanism config、factory挂载以及diagnostics列。

新增protocol时先判断是否属于clamp。非clamp用例应建立独立模块，而不是继续扩大 `clamp_protocol.hpp`。
