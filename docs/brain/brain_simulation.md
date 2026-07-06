# src/brain/brain_simulation.cpp

`brain_simulation.cpp` 现在只负责纯状态推进，不收集 summary，不做 NaN / min-max / exploding 检测，也不打印任何内容。

## 对应接口

```text
include/brain/brain_simulation.hpp
```

## API

```cpp
neuron::SynapseNetwork::StepStats step_brain_network(
    BrainNetwork& network,
    double dt_ms);

void run_brain_steps(
    BrainNetwork& network,
    double dt_ms,
    std::size_t steps);
```

## 单步顺序

`step_brain_network()` 保持固定更新顺序：

```text
1. clear_currents() for every neuron
2. network.synapses.apply(dt_ms)
3. neuron.step(dt_ms) for every neuron
```

这是 brain 核心模拟的最小状态推进单元。未来 C API 例如 `simulate(step)` 应该直接调用这一层，而不是 diagnostics 层。

## 返回值

`step_brain_network()` 返回 `neuron::SynapseNetwork::StepStats`：

| 字段 | 含义 |
| --- | --- |
| `max_abs_chemical_current_pA` | 当前 step 内 chemical synapse 最大绝对电流。 |
| `max_abs_gap_current_pA` | 当前 step 内 gap junction 最大绝对电流。 |

这些 stats 只是 synapse apply 的直接返回值；`brain_simulation.cpp` 不解释、不累计、不排序。

## OpenMP

当前有两个并行区域：

| 区域 | 内容 |
| --- | --- |
| clear currents | 并行清空所有 neuron 的 current buffer。 |
| neuron step | 并行推进所有 neuron 一个 dt。 |

`synapses.apply(dt_ms)` 仍在两者之间执行，保持原来的电流注入顺序。

## 不属于本模块的内容

这些逻辑已经移到 diagnostics：

```text
min/max voltage 统计
NaN 计数
exploding neuron 检测
top current edge 排序
summary result 生成
summary 打印
CSV 输出
```

## 调试建议

| 断点 | 看什么 |
| --- | --- |
| `step_brain_network()` 入口 | `dt_ms` 和 `network.neurons.size()`。 |
| `clear_currents()` 循环 | 每步开始前 current buffer 是否清空。 |
| `network.synapses.apply(dt_ms)` 后 | 当前 step 的 synapse stats。 |
| `neuron->step(dt_ms)` | 单 neuron 积分是否异常。 |
