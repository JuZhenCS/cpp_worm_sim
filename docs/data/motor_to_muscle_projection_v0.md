# motor_to_muscle_projection_v0.csv

`data/muscle/motor_to_muscle_projection_v0.csv` 是 brain-to-MuJoCo muscle coupling 的第一版 direct chemical projection 表。

它不是 placeholder 映射，也不是按 neuron 排序硬填 96 路；它直接从 Cook 2019 SI5 的 `hermaphrodite chemical` matrix 中提取：

```text
rows = presynaptic cell
columns = postsynaptic cell
value = chemical connection weight
```

本表只保留 postsynaptic cell 为 body wall muscle 的 direct chemical connection。

## 来源

| 项目 | 内容 |
| --- | --- |
| 原始文件 | `E:/1 PhD work/C.elegans simulation/code/draft/C.elegans.network/cook 2019 SI/41586_2019_1352_MOESM9_ESM SI5.xlsx` |
| sheet | `hermaphrodite chemical` |
| 输出脚本 | `scripts/export_motor_to_muscle_projection_v0.py` |
| source 字段 | `Cook2019_SI5` |

## 输出文件

```text
data/muscle/motor_to_muscle_projection_v0.csv
data/muscle/motor_to_muscle_projection_v0_summary.json
```

当前导出结果：

| 指标 | 值 |
| --- | ---: |
| projection rows | 956 |
| unique presynaptic neurons | 162 |
| SI5 body wall muscle columns | 95 |
| MuJoCo channels with projection edge | 95 |
| missing / no-edge channel | `vBWML24` = `VL24` = `muscle_index 71` |

`vBWML24` 在 SI5 `hermaphrodite chemical` matrix 中没有对应 body wall muscle column，因此没有伪造连接。后续 `muscle_output` 对这个 channel 应该默认输出 0，或由独立平滑规则处理。

## 字段

| 字段 | 含义 |
| --- | --- |
| `pre_neuron` | Presynaptic neuron name，来自 SI5 row name。AS/DA/DB/DD/VA/VB/VC/VD 编号统一补零，例如 `DA1 -> DA01`。 |
| `muscle_name` | Cook SI5 body wall muscle 名称，例如 `dBWML1`。 |
| `muscle_index` | MuJoCo 96 路 muscle channel index，范围 `0..95`。 |
| `quadrant` | `DL` / `DR` / `VL` / `VR`。 |
| `segment` | body segment，范围 `1..24`。 |
| `weight` | SI5 chemical matrix 中的 direct connection weight，只输出正数。 |
| `sign` | v0 近似符号：`+1` excitatory/default，`-1` inhibitory。 |
| `source` | 固定为 `Cook2019_SI5`。 |

## 96 路 muscle index 映射

| Cook SI5 muscle | MuJoCo quadrant | muscle_index |
| --- | --- | --- |
| `dBWMLk` | `DLk` | `k - 1` |
| `dBWMRk` | `DRk` | `24 + k - 1` |
| `vBWMLk` | `VLk` | `48 + k - 1` |
| `vBWMRk` | `VRk` | `72 + k - 1` |

## sign v0 规则

这是 motor-to-muscle projection v0 的粗略符号规则，不是最终 NMJ 模型：

| pre neuron class | sign | 说明 |
| --- | ---: | --- |
| `DD`, `VD` | `-1` | 已知 GABAergic inhibitory body motor neurons。 |
| `DA`, `DB`, `VA`, `VB`, `AS`, `VC`, `PDA`, `PDB`, `SAB*` | `+1` | v0 中按 cholinergic/excitatory motor output 处理。 |
| 其他 direct presynaptic neuron | `+1` | SI5 中确实有 direct body wall muscle edge，但本版本没有更细的 NMJ sign，先保留连接并默认 `+1`。 |

未知默认 `+1` 的 class 列表写入：

```text
data/muscle/motor_to_muscle_projection_v0_summary.json
```

## 重新生成

```powershell
D:\anaconda3\envs\SCA\python.exe scripts\export_motor_to_muscle_projection_v0.py --cook-si5 "E:\1 PhD work\C.elegans simulation\code\draft\C.elegans.network\cook 2019 SI\41586_2019_1352_MOESM9_ESM SI5.xlsx"
```

也可以通过环境变量指定 source root：

```powershell
$env:CELEGANS_NETWORK_SOURCE_ROOT = "E:\1 PhD work\C.elegans simulation\code\draft\C.elegans.network"
D:\anaconda3\envs\SCA\python.exe scripts\export_motor_to_muscle_projection_v0.py
```

## 校验条件

脚本生成前会校验：

```text
columns exactly: pre_neuron,muscle_name,muscle_index,quadrant,segment,weight,sign,source
0 <= muscle_index <= 95
quadrant in DL/DR/VL/VR
1 <= segment <= 24
weight > 0
sign in {-1, +1}
source == Cook2019_SI5
```