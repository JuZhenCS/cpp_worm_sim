# calcium_internal.cpp 开发说明

对应 [calcium_internal.cpp](../../src/neuron/calcium_internal.cpp) 和 [calcium_internal.hpp](../../include/neuron/core/calcium_internal.hpp)。

## 职责

`CalciumInternalState` 根据钙电流密度，用一阶模型更新内部钙浓度。该类保存模型参数，但不拥有neuron或compartment状态。

## 参数

- `vcell_um3`：有效细胞体积；
- `free_fraction`：自由钙比例；
- `removal_tau_ms`：清除时间常数；
- `caeq`：平衡钙浓度。

## step

当voltage不高于60 mV时，钙电流形成source；随后计算带source的目标浓度，并用指数形式精确推进一阶清除过程：

```text
target = caeq + source * tau
cai_new = target + (cai_old - target) * exp(-dt / tau)
```

`MultiCompartmentNeuron` 汇总egl19、cca1和unc2电流密度后调用该函数。

如果增加新的钙通道，必须同步多隔室neuron中的钙电流汇总。
