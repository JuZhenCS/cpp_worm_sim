现在 `brain4MuJoCo` 的 96 路肌肉输出，归一化分两层：

**1. 单个 neuron 电压先转成 0-1 activation**

代码在：

[src/brain4MuJoCo/muscle/muscle_output.cpp](/E:/1 PhD work/C.elegans simulation/code/neural network/annotate-and-test-main/src/brain4MuJoCo/muscle/muscle_output.cpp)

当前公式是：

```
activation = clamp01((voltage - v_rest_mV) / v_scale_mV);
```

默认参数：

```
v_rest_mV = -60.0
v_scale_mV = 40.0
```

所以含义是：

```
V <= -60 mV       -> activation = 0
V = -40 mV        -> activation = 0.5
V >= -20 mV       -> activation = 1
```

中间线性插值，最后 clamp 到 `[0, 1]`。

**2. 多个 neuron 投到同一个 muscle 时，做权重加权平均**

每条 projection edge 来自 Cook SI5：

```
pre_neuron -> muscle_index
weight = Cook SI5 chemical matrix weight
sign = +1 或 -1
```

每条边的贡献是：

```
contribution = sign * weight * activation
```

同一个 muscle channel 累加：

```
signed_sum[muscle] += sign * weight * activation
weight_sum[muscle] += weight
```

最后输出：

```
output[muscle] = clamp01(signed_sum[muscle] / weight_sum[muscle])
```

所以不是按 neuron 数量平均，而是按 Cook SI5 的连接权重平均。权重大的一条连接影响更大。

举个例子，同一个肌肉有 3 条输入：

```
Neuron A: activation=0.8, weight=10, sign=+1
Neuron B: activation=0.2, weight=2,  sign=+1
Neuron C: activation=0.5, weight=4,  sign=-1
```

计算：

```
signed_sum = 10*0.8 + 2*0.2 - 4*0.5 = 6.4
weight_sum = 10 + 2 + 4 = 16
raw = 6.4 / 16 = 0.4
output = 0.4
```

如果抑制项很强导致 `raw < 0`，最后会 clamp 成 `0`。如果兴奋很强导致 `raw > 1`，会 clamp 成 `1`。

这套归一化的作用是：不让某个肌肉因为输入边很多就天然输出更大，而是把结果保持在 MuJoCo 容易消费的固定范围 `[0,1]`。但这仍然是 v0 近似，不是最终 NMJ/肌肉动力学模型。
