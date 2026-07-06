# neuron_factory.cpp 开发说明

对应 [neuron_factory.cpp](../../../src/neuron/neuron_factory.cpp) 和 [neuron_factory.hpp](../../../include/neuron/core/neuron_factory.hpp)。

## 职责

factory负责“装配”，不负责数值积分：

```text
NeuronBuildConfig
  -> load_cell_csv()
  -> MultiCompartmentNeuron
  -> enable_mechanisms()
  -> 返回对象
```

## 配置

`NeuronMechanismConfig` 包含active channels和内部钙动力学开关。`NeuronBuildConfig` 包含：

- 运行时名称；
- cell CSV路径；
- mechanism配置。

## enable_mechanisms

该私有函数把配置开关转换成具体动作：

- channel开关调用对应 `attach_*_channels()`；
- calcium internal开关调用 `enable_calcium_internal()`。

每个attach函数仍会检查compartment conductance是否大于0，因此“启用机制”不代表每个compartment都创建通道。

## 两个创建接口

`create_multi_compartment_neuron()` 返回具体类型，供单细胞clamp和diagnostics使用。

`create_neuron()` 返回 `unique_ptr<NeuronModel>`，供synapse/network层只依赖抽象接口。

新增机制时必须同步config字段、runner CLI映射、factory挂载和具体neuron attach方法。
