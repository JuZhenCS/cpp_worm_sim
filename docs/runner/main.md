# cpp_neuron_runner 开发说明

本文档说明 `cpp_neuron_runner.exe` 的入口架构。入口相关代码被拆为三个文件：

| 文件 | 职责 |
| --- | --- |
| [main.cpp](../../src/main.cpp) | 进程入口和异常边界 |
| [runner.hpp](../../src/runner/runner.hpp) | runner 配置模型与公开函数 |
| [runner.cpp](../../src/runner/runner.cpp) | CLI 解析、protocol 配置、执行和输出 |

核心原则是：`main.cpp` 只负责启动应用，不包含业务细节。

## 总体调用链

```text
操作系统启动 cpp_neuron_runner.exe
  -> main(argc, argv)
  -> runner::parse_config(argc, argv)
       -> 读取 cell、protocol、路径和数值参数
       -> 解析 --enable-* 通道开关
       -> 合并细胞默认 protocol 参数与 CLI 覆盖
       -> 返回 RunnerConfig
  -> runner::run(config)
       -> create_multi_compartment_neuron(config.neuron)
            -> load_cell_csv()
            -> 构造 MultiCompartmentNeuron
            -> 启用已配置的 mechanisms
       -> 运行 iclamp 或 seclamp
       -> 写 trace CSV
       -> 可选写 diagnostics CSV
  -> main() 输出成功消息或错误消息
```

## 设计边界

### main.cpp：进程入口

`main.cpp` 是 composition root，只做四件事：

1. 把 `argc/argv` 交给 `parse_config()`。
2. 把解析结果交给 `run()`。
3. 成功时返回 `EXIT_SUCCESS`。
4. 捕获 `std::exception`，打印错误并返回 `EXIT_FAILURE`。

它不需要知道：

- 有哪些 channel；
- 不同细胞的 protocol 默认值；
- 如何读取 cell CSV；
- 如何创建 neuron；
- 如何运行 clamp；
- 输出 CSV 有哪些列。

这样新增 channel 或修改 protocol 时，一般不需要改 `main.cpp`。

### runner.hpp：稳定接口

`runner.hpp` 定义入口层内部使用的配置模型。

#### ProtocolKind

```cpp
enum class ProtocolKind {
    IClamp,
    SEClamp,
};
```

使用枚举代替在执行层反复比较字符串。字符串 `"iclamp"` 和 `"seclamp"` 只在 CLI 边界解析一次；进入执行阶段后使用类型安全的枚举。

#### RunnerConfig

```cpp
struct RunnerConfig {
    NeuronBuildConfig neuron;
    ProtocolKind protocol;
    IClampProtocol iclamp;
    SEClampProtocol seclamp;
    std::string output;
    std::string diagnostic_output;
};
```

它是一次运行所需信息的完整快照：

- `neuron`：细胞名称、cell CSV、启用的 mechanisms；
- `protocol`：本次运行选择哪种 clamp；
- `iclamp/seclamp`：解析后的 protocol 参数；
- `output`：主 trace 路径；
- `diagnostic_output`：可选诊断路径。

`RunnerConfig` 同时保存两种 protocol 结构体，但执行时只使用 `protocol` 指定的那一个。这比在入口层引入继承或 `std::variant` 更直接。

#### 公开函数

```cpp
RunnerConfig parse_config(int argc, char** argv);
void run(const RunnerConfig& config);
```

`parse_config()` 负责把外部字符串输入变成内部强类型配置；`run()` 只接受已解析配置，不再处理命令行字符串。

### runner.cpp：应用编排

`runner.cpp` 分为两个部分。

匿名命名空间中的函数属于实现细节：

- `arg_value()`
- `arg_double()`
- `has_arg()`
- `parse_mechanism_config()`
- `default_iclamp()`
- `default_seclamp()`

`cpp_neuron::runner` 命名空间中的公开实现：

- `parse_config()`
- `run()`

匿名命名空间保证 CLI 辅助函数不会成为其他模块可以依赖的公共 API。

## CLI 解析

### arg_value

读取 `--name value` 形式参数：

```text
cpp_neuron_runner.exe --cell AIYL --protocol iclamp
```

对应的 `argv` 大致是：

```text
argv[0] = "cpp_neuron_runner.exe"
argv[1] = "--cell"
argv[2] = "AIYL"
argv[3] = "--protocol"
argv[4] = "iclamp"
```

找到参数名后返回下一个元素；找不到则返回 fallback。如果参数名位于命令末尾、没有对应 value，则立即报错。

### arg_double

通过 `std::stod` 把参数字符串转换为 `double`。非法数字会抛出异常，最后由 `main()` 统一报告。

### has_arg

检查不带 value 的布尔开关，例如：

```text
--enable-nca
--enable-egl19
--enable-calcium-internal
```

### MechanismFlagBinding

机制映射表把 CLI flag 与 `NeuronMechanismConfig` 的布尔成员关联：

```cpp
struct MechanismFlagBinding {
    const char* flag;
    bool NeuronMechanismConfig::* enabled;
};
```

`bool NeuronMechanismConfig::*` 是成员指针。应用到具体对象时：

```cpp
mechanisms.*(binding.enabled) = has_arg(argc, argv, binding.flag);
```

这样 `parse_mechanism_config()` 不需要为每个机制重复写完整赋值语句。机制既包括 active channels，也包括内部钙动力学。

## 参数解析结果

基础参数：

| CLI 参数 | 默认值 | 写入位置 |
| --- | --- | --- |
| `--cell` | `AIYL` | `config.neuron.name` |
| `--protocol` | `iclamp` | `config.protocol` |
| `--data-dir` | `../data/aiyl` | 用于构造默认 cell 路径 |
| `--cell-file` | 目录 + 细胞名 | `config.neuron.cell_file` |
| `--output` | `trace.csv` | `config.output` |
| `--diag-output` | 空 | `config.diagnostic_output` |

相对路径依据进程的 working directory 解析，不依据 exe 所在目录。README 示例显式传入 `--cell-file`，可以避免 IDE 和终端工作目录不同造成的问题。

### iclamp 默认值

| Cell | 初始电压 | 默认电流 | 其他覆盖 |
| --- | ---: | ---: | --- |
| AIYL | -45.0 mV | 10.0 pA | 无 |
| AVAL | -30.0 mV | 10.0 pA | duration = 5015 ms |
| RIML | -39.3 mV | 10.0 pA | 无 |

CLI 可覆盖：

- `--amp-pa`
- `--dt-ms`
- `--tstop-ms`

### seclamp 默认值

| Cell | tstop | duration | command voltage |
| --- | ---: | ---: | ---: |
| AWCL | 1600 ms | 100 ms | 30 mV |
| VD05 | 2600 ms | 1210 ms | 20 mV |

CLI 可覆盖：

- `--vcmd-mv`
- `--dt-ms`
- `--tstop-ms`

未知 protocol 会在 `parse_config()` 阶段立即报错，不会进入 neuron 创建或仿真。
`dt` 和 `tstop` 也必须大于 0，否则在配置解析阶段报错。

## 执行阶段

`run()` 首先调用：

```cpp
auto neuron = create_multi_compartment_neuron(config.neuron);
```

然后通过 `switch (config.protocol)` 选择：

```cpp
run_current_clamp_with_diagnostics(...)
run_seclamp_with_diagnostics(...)
```

两个函数都返回 `ProtocolResult`，所以后续输出逻辑只写一次：

```cpp
write_trace_csv(config.output, result.trace);

if (!config.diagnostic_output.empty()) {
    write_channel_diagnostics_csv(
        config.diagnostic_output,
        result.diagnostics);
}
```

这消除了原来 iclamp 和 seclamp 分支中重复的 CSV 写出代码。

## 依赖方向

```text
main.cpp
  -> runner.hpp

runner.cpp
  -> runner.hpp
  -> neuron_factory.hpp
  -> clamp_protocol.hpp
  -> csv_writer.hpp

neuron factory
  -> cell loader
  -> MultiCompartmentNeuron

clamp protocol
  -> MultiCompartmentNeuron
  -> recording data types
```

入口层依赖核心层，核心层不依赖 runner。这样核心 neuron/synapse 模型可以被测试程序或其他可执行文件复用。

## 修改指南

### 新增 channel

至少检查：

1. `NeuronMechanismConfig` 增加开关；
2. `runner.cpp` 的 channel flag 映射表增加 CLI 名称；
3. `neuron_factory.cpp` 增加挂载判断；
4. `MultiCompartmentNeuron` 增加 attach 方法；
5. cell CSV loader 和 `Compartment` 增加 conductance 字段；
6. diagnostics 是否需要增加输出列。

### 新增 protocol

至少检查：

1. `ProtocolKind` 增加枚举值；
2. `RunnerConfig` 增加 protocol 配置；
3. `parse_config()` 增加字符串解析和参数覆盖；
4. `run()` 的 switch 增加执行分支；
5. `clamp_protocol.hpp/.cpp` 增加具体算法。

### 修改 CLI

CLI 细节应留在 `runner.cpp`。不要把参数扫描重新放回 `main.cpp`，也不要让 neuron/core 层依赖 `argc/argv`。

## Debug 建议

Debug 构建：

```powershell
cmake -S . -B build_debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build_debug
```

建议参数：

```text
--cell AIYL
--protocol iclamp
--cell-file data/aiyl/AIYL_cell.csv
--tstop-ms 2
--dt-ms 0.2
--enable-nca
```

建议断点：

1. `main()`
2. `runner::parse_config()`
3. `parse_mechanism_config()`
4. `runner::run()`
5. `create_multi_compartment_neuron()`
6. `load_cell_csv()`
7. `run_current_clamp_with_diagnostics()`
8. `MultiCompartmentNeuron::step_with_conductance()`

重点观察：

- `argc`、`argv`
- `config`
- `config.neuron.mechanisms`
- `neuron->cell().compartments`
- `result.trace`
- 仿真循环中的 `t_ms`、注入电流和 soma voltage

## 输出

主 trace：

```text
time_ms,soma_v_mV,stimulus,clamp_current_pA
```

诊断 CSV 包含指定 compartment 的 leak、axial、总离子电流、各通道电流、conductance 和内部钙浓度。

所有异常最终由 `main()` 输出为：

```text
ERROR: <具体错误>
```
