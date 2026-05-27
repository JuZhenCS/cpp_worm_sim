# Next Steps

目标是从单神经元运行时包扩展到 C++ 版线虫网络模拟。

建议顺序：

1. 增加 `src/synapse/`，实现 BAAIWorm/eWorm 形式的分级化学突触：

```text
s_inf(V_pre) = 1 / (1 + exp((Vth - V_pre) / delta))
ds/dt = (s_inf - s) / tau
I_syn = g_syn * s * (E_rev - V_post)
```

2. 增加最小 two-cell scheduler：

```text
pre neuron step
synapse step
post neuron receives I_syn
record V_pre, s, I_syn, V_post
```

3. 扩展到 chemical-only microcircuit。

4. 单独处理 gap junction backend，不能用 open-loop replay 代替闭环电耦合。

5. 最后接 connectome、body 和 environment。

当前不要把旧 NESTML 或验证脚本重新混入这个目录。
