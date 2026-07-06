# brain 模块文档

`brain/` 是 whole-worm-brain runner 的代码和文档目录。当前结构按职责拆分：

```text
src/brain/main.cpp
src/brain/brain_runner.cpp
src/brain/brain_network_builder.cpp
src/brain/brain_simulation.cpp
src/brain/representative_neuron_catalog.cpp
src/brain/diagnostics/
```

## 阅读顺序

1. [worm_brain_runner.md](worm_brain_runner.md)
   总入口、运行方式、CLI 参数、模块边界。

2. [clion_debug.md](clion_debug.md)
   CLion Run/Debug Configuration 应该怎么填，以及建议断点。

3. [brain_runner.md](brain_runner.md)
   高层调度：build network、run steps、按需启用 diagnostics。

4. [brain_simulation.md](brain_simulation.md)
   纯状态推进：`clear_currents -> synapses.apply -> neuron.step`。

5. [brain_network_builder.md](brain_network_builder.md)
   从 CSV 构建 302-neuron network。

6. [diagnostics/brain_diagnostics_observe.md](diagnostics/brain_diagnostics_observe.md)
   可选 summary/statistics 观察器。

7. [diagnostics/brain_diagnostics_recorder.md](diagnostics/brain_diagnostics_recorder.md)
   summary 输出格式化。

8. [representative_neuron_catalog.md](representative_neuron_catalog.md)
   AWC / AIY / AVA / RIM / VD5 代表模板映射。

9. [brain_simulation_math.md](brain_simulation_math.md)
   brain step 数学流程。

## 当前清理规则

- `brain_simulation` 不放 diagnostics / summary 逻辑。
- `main.cpp` 不计算 summary，只解析 CLI 并委托 runner / recorder。
- summary 统计只放在 `brain/diagnostics/brain_diagnostics_observe`。
- summary 输出只放在 `brain/diagnostics/brain_diagnostics_recorder`。
