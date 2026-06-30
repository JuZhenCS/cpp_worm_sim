# cell.cpp 开发说明

对应 [cell.cpp](../../src/neuron/cell.cpp) 和 [cell.hpp](../../include/cpp_neuron_core/neuron/cell.hpp)。

## 职责

`cell.hpp` 定义多隔室领域数据：

- `Compartment`：电容、漏电、轴向连接、面积、电压、钙状态、各机制conductance和通道对象；
- `Cell`：细胞名称与compartment序列。

`cell.cpp` 只实现 `Compartment` 的深拷贝。

## 为什么需要自定义拷贝

`Compartment::channels` 是：

```cpp
std::vector<std::unique_ptr<Channel>>
```

`unique_ptr` 不能直接复制，因此编译器无法生成拷贝构造和拷贝赋值。实现通过每个通道的 `clone()` 创建独立对象：

```text
原Compartment
  -> channel->clone()
  -> 新Compartment拥有独立channel状态
```

这使protocol可以按值接收 `MultiCompartmentNeuron`，仿真副本不会修改原对象。

## 维护规则

给 `Compartment` 新增字段时，必须同步：

1. 成员声明和默认值；
2. 拷贝构造初始化列表；
3. 拷贝赋值；
4. cell loader；
5. 必要时更新diagnostics。

本模块不应包含trace、protocol或文件写出类型。
