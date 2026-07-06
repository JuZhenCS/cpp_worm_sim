# src/brain/diagnostics/brain_diagnostics_recorder.cpp

`brain_diagnostics_recorder.cpp` 只负责把已经收集好的 `BrainDiagnosticsResult` 写成 text summary。

## 对应接口

```text
include/brain/diagnostics/brain_diagnostics_recorder.hpp
```

## API

```cpp
void write_brain_summary(
    const BrainDiagnosticsResult& result,
    std::ostream& output);
```

## 职责边界

这个模块只格式化输出：

```text
不推进 simulation
不读取 CSV
不遍历 live network 重新统计
不排序 top current edges
```

所有统计值都必须已经在 `BrainDiagnosticsResult` 中。

## 输出格式

当前输出保持原 summary 的 line-oriented key-value 格式：

```text
tstop_ms=10
dt_ms=0.1
openmp_threads=16
neurons=302
chemical_components=4308
...
```

## 调试建议

| 断点 | 看什么 |
| --- | --- |
| 函数入口 | result 是否已经完整。 |
| 输出 `exploding_neurons` | 异常 neuron 是否按 collector 结果输出。 |
| 输出 `top_current_edge` | rank、type、label、peak current 是否正确。 |
