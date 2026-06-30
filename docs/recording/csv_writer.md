# csv_writer.cpp 开发说明

对应 [csv_writer.cpp](../../src/recording/csv_writer.cpp)、[csv_writer.hpp](../../include/cpp_neuron_core/recording/csv_writer.hpp) 和 [recording.hpp](../../include/cpp_neuron_core/recording/recording.hpp)。

## 职责

只负责把内存中的记录数据序列化为CSV：

- `write_trace_csv()`
- `write_channel_diagnostics_csv()`

它不创建目录、不运行仿真，也不决定输出路径。

## trace格式

```text
time_ms,soma_v_mV,stimulus,clamp_current_pA
```

`stimulus` 在iclamp中是注入电流，在seclamp中是command voltage。

## diagnostics格式

每行对应一个采样时间，包含voltage、leak、axial、总离子电流、各channel电流、基础conductance、reversal常量和内部钙浓度。

表头顺序必须与写值顺序严格一致。新增字段时应同时修改：

1. `ChannelDiagnosticPoint`；
2. clamp protocol采样；
3. CSV表头；
4. CSV值写出；
5. 下游分析脚本。

## 错误

输出文件无法打开时抛出 `runtime_error`。目录创建属于调用方职责。
