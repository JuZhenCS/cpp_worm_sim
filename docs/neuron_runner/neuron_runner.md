# neuron_runner.cpp 开发说明

对应 [neuron_runner.cpp](../../src/neuron_runner/runner.cpp) 和 [neuron_runner.hpp](../../src/neuron_runner/runner.hpp)。

## 职责

`neuron_runner.cpp` 是单细胞可执行程序的应用编排层：

1. 把 `argc/argv` 解析为 `RunnerConfig`；
2. 合并细胞默认protocol与CLI覆盖；
3. 解析动力学机制开关；
4. 创建神经元；
5. 分发iclamp或seclamp；
6. 写主trace和可选diagnostics。

它不实现CSV cell解析、通道方程或多隔室求解。

## 内部辅助函数

- `arg_value()`：读取 `--name value`，缺少value时报错；
- `arg_double()`：使用 `std::stod` 转换数值；
- `has_arg()`：读取不带value的开关；
- `parse_mechanism_config()`：把 `--enable-*` 映射到 `NeuronMechanismConfig`；
- `default_iclamp()` / `default_seclamp()`：提供细胞专用默认值；
- `validate_timing()`：拒绝非正的dt和tstop。

这些函数位于匿名命名空间，不是公共API。

## parse_config

输出一个完全解析的 `RunnerConfig`。字符串protocol只在这里出现一次，并被转换为 `ProtocolKind`。

```text
CLI strings
  -> basic paths
  -> mechanism config
  -> protocol defaults
  -> numeric overrides
  -> validation
  -> RunnerConfig
```

未知protocol、缺失value、非法数字和非正时间参数都会在此阶段失败，不会进入仿真。

## run

`run()` 只接受强类型配置：

1. factory创建 `MultiCompartmentNeuron`；
2. switch分发clamp protocol；
3. 得到统一 `ProtocolResult`；
4. CSV writer输出。

新增CLI参数应改 `parse_config()`；新增执行用例应同时改 `ProtocolKind`、`RunnerConfig` 和 `run()`。不要把这些逻辑放回 `main.cpp`。

## 验证

`tests/test_runner.cpp` 覆盖默认值、参数覆盖、mechanism开关、未知protocol、缺失value和非法时间参数。
