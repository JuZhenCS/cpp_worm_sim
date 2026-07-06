# multi_compartment_neuron.cpp 开发说明

对应 [multi_compartment_neuron.cpp](../../../src/neuron/multi_compartment_neuron.cpp) 和 [multi_compartment_neuron.hpp](../../../include/neuron/core/multi_compartment_neuron.hpp)。

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

### 关于Ca+为什么在求完电压之后更新
`update_calcium_internal(cell_, dt_ms)` 的作用是更新内部钙浓度 `cai_uM_per_um2`，主要影响钙依赖通道，例如 `kcnl`、`slo1_egl19`、`slo1_unc2`、`slo2_egl19`、`slo2_unc2`。

它不是主电压更新步骤。主电压更新已经在前一句完成：

```cpp
solver_.advance_voltages(...);
```

当前时间步的顺序可以理解为：

```text
已有 V^k, Ca^k

1. 通道读取当前钙浓度 Ca^k
2. 根据 V^k 和 Ca^k 更新通道状态
3. 计算通道电流
4. 解 cable 方程，得到 V^{k+1}
5. 更新内部钙浓度，得到 Ca^{k+1}
6. 下一步再用 V^{k+1}, Ca^{k+1}
```

所以：

```text
update_calcium_internal 不会修改刚刚已经算出来的 V^{k+1}
它是为下一步准备 Ca^{k+1}
```

数学上可以写成：

$V^k, Ca^k \rightarrow V^{k+1}$

然后：

$Ca^k \rightarrow Ca^{k+1}$

下一步再用：

$V^{k+1}, Ca^{k+1} \rightarrow V^{k+2}$

也就是说，这是一个显式/分裂式更新：

```text
先用旧 Ca 算电压，再更新新 Ca；
新 Ca 从下一步开始反馈影响电压。
```

第 0 步也考虑钙，但用的是初始钙：

```cpp
compartment.cai_uM_per_um2 = 0.05;
```

所以第 0 步是：

```text
V^0, Ca^0 -> V^1
Ca^0 -> Ca^1
```

不是“不考虑 Ca”，而是每一步只使用该步开始时已经存在的 Ca。新产生的 Ca 会滞后一个时间步影响电压。


## soma定义

单compartment时直接返回第0个电压；多compartment时返回前两个compartment电压平均值。这是当前代表细胞的约定，不是通用形态学soma检测。

## mechanism挂载

`attach_*_channels()` 遍历compartments，只在对应conductance大于0时创建channel对象。`enable_calcium_internal()` 设置内部钙开关和初值。

## 维护风险

- solver工作区vector尺寸必须与compartment数一致；
- 修改矩阵系数时必须同时检查缓存条件；
- 新增钙通道时需更新钙电流汇总名单；
- diagnostics使用的电流符号约定要与protocol和CSV保持一致。
