# cell_loader.cpp 开发说明

对应 [cell_loader.cpp](../../src/neuron/cell_loader.cpp) 和 [cell_loader.hpp](../../include/neuron/neuron/cell_loader.hpp)。

## 职责

把单个cell CSV转换成 `Cell`。公开接口只有：

```cpp
Cell load_cell_csv(const std::string& path, const std::string& cell_name);
```

## 解析流程

1. 打开文件；
2. 第一行建立“列名到索引”映射；
3. 逐行创建 `Compartment`；
4. 读取必填被动参数；
5. 读取可选面积和机制conductance；
6. 初始电压设为leak reversal；
7. 返回包含全部compartments的 `Cell`。

## 字段策略

必填字段通过 `require_field()` 读取，缺失时报错：

- `idx`
- `parent_idx`
- `label`
- `C_m_pF`
- `g_L_nS`
- `e_L_mV`
- `g_C_nS`

机制conductance通过 `optional_double()` 读取，缺失或空值默认为0。

## 限制

当前 `split_csv_line()` 是简单逗号切分，不支持带逗号的quoted field。当前cell表是数值/简单标签，因此满足运行需求；若CSV格式扩展，应换成结构化CSV parser，而不是继续增加字符串特例。

## 错误

文件打不开、必填列缺失、行列数不足、数字转换失败或没有compartment时会抛出 `std::exception`，最终由入口层报告。
