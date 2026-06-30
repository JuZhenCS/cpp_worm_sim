# Neuron模块阅读顺序

文件名不使用 `1_`、`2_` 前缀。调用关系不是固定直线，推荐阅读顺序记录在这里。

## 目录对应

```text
include/neuron/neuron/   公共头文件
src/neuron/                       实现文件
docs/neuron/                      开发说明
```

## 推荐阅读顺序

1. [neuron_model.hpp](../../include/neuron/neuron/neuron_model.hpp)
   先理解网络和突触层依赖的最小抽象。

2. [cell.hpp](../../include/neuron/neuron/cell.hpp) 和 [cell.md](cell.md)
   理解compartment、cell、通道所有权和深拷贝。

3. [calcium_internal.hpp](../../include/neuron/neuron/calcium_internal.hpp) 和 [calcium_internal.md](calcium_internal.md)
   理解被多隔室模型调用的内部钙子模型。

4. [multi_compartment_neuron.hpp](../../include/neuron/neuron/multi_compartment_neuron.hpp) 和 [multi_compartment_neuron.md](multi_compartment_neuron.md)
   理解具体neuron状态、step入口和矩阵求解。

5. [cell_loader.hpp](../../include/neuron/neuron/cell_loader.hpp) 和 [cell_loader.md](cell_loader.md)
   理解外部CSV如何变成 `Cell`。

6. [neuron_factory.hpp](../../include/neuron/neuron/neuron_factory.hpp) 和 [neuron_factory.md](neuron_factory.md)
   最后理解loader、具体模型和mechanism配置如何装配。

## 运行调用关系

```text
runner
  -> neuron_factory
       -> cell_loader
            -> Cell
       -> MultiCompartmentNeuron
       -> enable mechanisms

clamp_protocol
  -> MultiCompartmentNeuron

synapse/network
  -> NeuronModel
```

## 修改边界

- 模型积分改 `multi_compartment_neuron`；
- CSV cell格式改 `cell_loader`；
- 创建和机制选择改 `neuron_factory`；
- 网络通用契约改 `neuron_model`；
- 不要让neuron模块依赖runner、CLI或CSV输出。
