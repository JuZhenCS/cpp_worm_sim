# Neuron Layer Status

来源：`neuron_v2`，Git tag `five-cell-neuron-core-v1`。

当前已整理进运行时包的细胞：

| Cell | Protocol | Status |
| --- | --- | --- |
| AIYL | current clamp | usable |
| AVAL | current clamp | usable |
| RIML | current clamp | usable |
| AWCL | SEClamp / voltage clamp | comparable-current usable |
| VD05 | SEClamp / voltage clamp | comparable-current usable |

说明：

- 这里的 `usable` 指 C++ 单神经元 active dynamics 已完成 NEURON 对照验证，可以作为后续 C++ 网络模拟的神经元层起点。
- AWCL/VD05 的 raw clamp transition edge transient 与 NEURON 底层实现不完全相同；已使用 comparable current 口径处理。网络模拟不依赖 SEClamp。
- 当前目录不包含 NEURON reference 和对齐脚本；这些保留在研发仓库 `neuron_v2`。
