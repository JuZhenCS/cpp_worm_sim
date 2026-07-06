# clamp_protocol.cpp 开发说明

对应 [clamp_protocol.cpp](../../../src/neuron/protocol/clamp_protocol.cpp) 和 [clamp_protocol.hpp](../../../include/neuron/protocol/clamp_protocol.hpp)。

## 职责

实现单细胞current clamp和voltage clamp用例，并生成 `ProtocolResult`。它负责实验时序，不负责CLI或文件输出。

## current clamp

流程：

1. 所有compartment设为 `v_init_mV`；
2. 根据dt和tstop计算步数；
3. 在delay/duration窗口内产生 `amplitude_pA`；
4. 多compartment时把刺激平均分到前两个soma compartment；
5. 调用neuron step；
6. 记录soma voltage、stimulus和diagnostics。

记录时间是完成一步后的 `t + dt`。

## voltage clamp

series resistance转换为总conductance：

```text
g_total_nS = 1000 / series_resistance_MOhm
```

conductance分配到前两个soma compartments，并通过 `step_with_conductance()` 加入矩阵。

输出的comparable current为：

```text
electrode current - capacitive current
```

这是AWCL/VD05与参考实现比较时采用的口径。

## diagnostics

`sample_diagnostics()` 读取指定compartment的：

- voltage；
- leak和axial current；
- ion current总和；
- 每个channel current和基础conductance；
- 内部钙浓度。

新增channel时必须同步diagnostic结构、采样和CSV writer列。

## 复制语义

protocol函数按值接收 `MultiCompartmentNeuron`，因此仿真会操作深拷贝，不修改调用方持有的原始neuron状态。
