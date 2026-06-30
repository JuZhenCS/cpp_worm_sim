# Channel实现模块

本目录包含 `Channel` 抽象接口的具体离子通道实现。

## 统一结构

每个channel通常实现：

- 构造函数：接收基础conductance；
- `step(voltage, dt)`：更新门控状态；
- `current_pA(voltage)`：计算当前电流；
- `name()`：提供稳定诊断名称；
- `state()`：可选暴露门控变量；
- `clone()`：支持 `Compartment` 深拷贝。

钙依赖channel还覆写 `set_calcium()`。

## 当前实现

- calcium channels：`egl19`、`cca1`、`unc2`
- potassium/rectifier channels：`irk`、`kqt3`、`egl2`、`shk1`、`kvs1`、`shl1`、`egl36`
- sodium-like channel：`nca`
- calcium-activated channel：`kcnl`
- coupled SLO mechanisms：`slo1_unc2`、`slo_coupled`

`slo_coupled` 通过参数表达SLO1/SLO2及其calcium partner组合，避免为每个组合复制整套类。

## 依赖边界

channel实现只能依赖 `channel.hpp` 和自身数学需要的标准库。它们不应依赖runner、CSV、cell loader、factory或protocol。

## 新增channel

1. 新增头文件和实现；
2. 实现全部纯虚函数；
3. 在 `Compartment` 增加conductance；
4. 在cell loader读取CSV字段；
5. 在 `NeuronMechanismConfig` 增加开关；
6. 在factory和 `MultiCompartmentNeuron` 增加挂载；
7. 更新diagnostics与CSV；
8. 加入CMake source列表和验证命令。
