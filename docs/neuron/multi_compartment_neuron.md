# multi_compartment_neuron.cpp 开发说明

对应 [multi_compartment_neuron.cpp](../../src/neuron/multi_compartment_neuron.cpp) 和 [multi_compartment_neuron.hpp](../../include/neuron/core/multi_compartment_neuron.hpp)。

## 职责

`MultiCompartmentNeuron` 是当前多隔室active neuron具体实现，负责：

- 保存 `Cell` 参数和动态状态；
- 累积外部注入电流；
- 更新channel门控状态和电流；
- 求解漏电、轴向耦合、外部conductance共同决定的电压；
- 更新内部钙；
- 提供诊断电流。

## 三个step入口

`step(dt)` 使用由 `add_current_pA()` 累积的电流，完成后清零。

`step(dt, injected)` 直接接收每个compartment的电流，供current clamp使用。

`step_with_conductance(...)` 额外接收conductance和reversal，供voltage clamp模拟series resistance。

## 数值流程

每个时间步：

1. 校验vector大小和dt；
2. 按当前电压更新channel状态；
3. 汇总channel电流；
4. 构造隐式电压线性系统；
5. 加入leak、parent-child轴向连接和外部conductance；
6. 求解所有compartment新电压；
7. 汇总钙通道电流并更新内部钙。

无额外conductance且dt不变时，矩阵不变，因此缓存逆矩阵；seclamp的conductance参与矩阵，不能使用该缓存。

## soma定义

单compartment时直接返回第0个电压；多compartment时返回前两个compartment电压平均值。这是当前代表细胞的约定，不是通用形态学soma检测。

## mechanism挂载

`attach_*_channels()` 遍历compartments，只在对应conductance大于0时创建channel对象。`enable_calcium_internal()` 设置内部钙开关和初值。

## 维护风险

- solver工作区vector尺寸必须与compartment数一致；
- 修改矩阵系数时必须同时检查缓存条件；
- 新增钙通道时需更新钙电流汇总名单；
- diagnostics使用的电流符号约定要与protocol和CSV保持一致。
