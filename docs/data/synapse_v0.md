# synapse_v0 数据

`data/synapse_v0/` 是 `worm_brain_runner` 的全脑网络输入。它不是手写数据，应该由 `scripts/export_synapse_v0_components.py` 从原始 Cook / Fenyves / BAAIWorm 文件重新导出。

## 文件数量说明

这个目录当前有 7 个文件。按用途分成三类：

| 类别 | 文件 | 说明 |
| --- | --- | --- |
| 运行时直接读取 | `chemical_components_neuron_neuron_v0.csv` | chemical synapse component 表，过滤到 neuron-to-neuron。 |
| 运行时直接读取 | `gap_junctions_v0.csv` | gap junction 表。 |
| 运行时直接读取 | `neuron_parameter_reference_v0.csv` | 302 个 neuron 到五个代表 cell 模板的映射。 |
| 导出中间表 | `chemical_connections_v0.csv` | connection 级 chemical 表，合并 Cook structure 和 Fenyves polarity / weight。 |
| 导出中间表 | `chemical_components_v0.csv` | component 级 chemical 表，包含 neuron-to-non-neuron / muscle。 |
| 导出配置 | `synapse_config_v0.json` | 记录导出参数和 v0 policy。 |
| 导出摘要 | `summary.json` | 记录导出计数、来源和分类统计。 |

所以之前说“5 个核心文件”不准确。更准确地说，结构化等价验证覆盖的是 **5 个 CSV 数据表 + 1 个 JSON 配置文件**；`summary.json` 是统计摘要，也由脚本生成，但不是运行时输入。

## 生成链路

`scripts/export_synapse_v0_components.py` 的核心步骤是：

```text
load_cook_chemical_connections()
  读取 Cook SI5 hermaphrodite chemical 矩阵

load_fenyves_connections()
  读取 Fenyves S1 sheet 5 Sign prediction

load_scored_contact_counts()
  读取 Cook SI2 herm chem synapse adjacency，用作 scored contact metadata

build_chemical_connections()
  合并 Cook / Fenyves，生成 chemical_connections_v0.csv

build_components()
  把 connection 按 polarity 转成 exc / inh component，生成 chemical_components_v0.csv
  再过滤 neuron-to-neuron，生成 chemical_components_neuron_neuron_v0.csv

build_gap_junctions()
  读取 Cook SI5 herm gap jn symmetric，生成 gap_junctions_v0.csv

parse_baaiworm_table4()
  从 BAAIWorm Supplementary Table 4 文本解析 302 neuron 模板映射，生成 neuron_parameter_reference_v0.csv

synapse_config()
  写出 synapse_config_v0.json

summary block
  写出 summary.json
```

## 原始来源

导出脚本需要这些原始文件：

| 来源 | 默认路径 |
| --- | --- |
| Cook 2019 SI2 | `<source-root>/cook 2019 SI/41586_2019_1352_MOESM6_ESM SI2.xlsx` |
| Cook 2019 SI5 | `<source-root>/cook 2019 SI/41586_2019_1352_MOESM9_ESM SI5.xlsx` |
| Fenyves 2020 S1 | `<source-root>/fenyves 2020 S Data/S1_Data.xlsx` |
| BAAIWorm Table 4 text | `<source-root>/../multi-compartment neuron 0416/baaiworm_pdf_text/pdf_1.txt` |

脚本默认先读环境变量 `CELEGANS_NETWORK_SOURCE_ROOT`。如果没设置，会在当前机器已知的历史路径中找：

```text
E:\1 PhD work\C.elegans simulation\code\C.elegans.network
E:\1 PhD work\C.elegans simulation\code\draft\C.elegans.network
```

## chemical_connections_v0.csv

来源：`build_chemical_connections()`。

粒度：一行是一条 directed chemical connection，即 `pre -> post`。

| 字段 | 含义 |
| --- | --- |
| `connection_id` | `pre__post`。 |
| `pre` | presynaptic cell 名称。 |
| `post` | postsynaptic cell 名称。 |
| `connection_class` | `neuron_to_neuron`、`neuron_to_non_neuron_or_muscle` 或 `non_neuron_source`。 |
| `connection_source` | connection 来自 `cook_si5_only`、`fenyves_only` 或 `cook_si5_and_fenyves`。 |
| `anatomical_weight` | 用于 v0 的 anatomical weight，优先 Cook weight，缺失时用 Fenyves weight。 |
| `cook_weight` | Cook SI5 chemical matrix 中的 weight。 |
| `fenyves_weight` | Fenyves S1 中的 weight。 |
| `weight_source` | 当前 `anatomical_weight` 的来源。 |
| `fenyves_polarity_raw` | Fenyves 原始 polarity 归一化后的值：`+`、`-`、`complex`、`no pred` 或缺失。 |
| `effective_polarity` | v0 实际使用 polarity。缺失和 `no pred` 当前按 `-`。 |
| `polarity_policy` | polarity 如何得到，例如 `fenyves_sign_prediction`、`missing_fenyves_default_inhibitory`。 |
| `active_in_v0` | 当前 connection 是否纳入 v0 导出。 |
| `pre_is_neuron` | pre 是否被判定为 neuron。 |
| `post_is_neuron` | post 是否被判定为 neuron。 |
| `has_cook_si5_connection` | Cook SI5 是否存在该 connection。 |
| `has_fenyves_sign_row` | Fenyves 是否存在该 connection 的 sign row。 |
| `scored_contact_count` | Cook SI2 scored contact count metadata。 |
| `has_scored_contact` | `scored_contact_count > 0`。 |

## chemical_components_v0.csv

来源：`build_components()`。

粒度：一行是一个 active chemical synapse component。`complex` polarity 会拆成两行：一个 `exc`，一个 `inh`。

| 字段 | 含义 |
| --- | --- |
| `component_id` | `connection_id__component_type`。 |
| `connection_id` | 对应 `chemical_connections_v0.csv` 的 connection。 |
| `pre` | presynaptic cell 名称。 |
| `post` | postsynaptic cell 名称。 |
| `pre_compartment` | presynaptic placement compartment。v0 固定为 0。 |
| `post_compartment` | postsynaptic placement compartment。v0 固定为 0。 |
| `placement_policy` | placement 规则。当前为 `soma_compartment_0_v0`。 |
| `component_type` | `exc` 或 `inh`。 |
| `fenyves_polarity_raw` | Fenyves polarity 原值归一化结果。 |
| `effective_polarity` | v0 实际 polarity。 |
| `polarity_policy` | polarity policy。 |
| `connection_class` | 从 connection 表继承。 |
| `connection_source` | 从 connection 表继承。 |
| `anatomical_weight` | 从 connection 表继承。 |
| `cook_weight` | 从 connection 表继承。 |
| `fenyves_weight` | 从 connection 表继承。 |
| `weight_source` | 从 connection 表继承。 |
| `normalized_weight` | 归一化后的 weight，当前默认 `log1p_max`。 |
| `rho` | component 在该 connection 内的占比。普通 `+` / `-` 为 1，`complex` 拆分后为 0.5。 |
| `g0_uS` | 该 component type 的 base conductance，单位 uS。 |
| `g_uS` | 实际 conductance，`g0_uS * normalized_weight * rho`。 |
| `e_rev_mV` | reversal potential。当前 exc 为 30 mV，inh 为 -70 mV。 |
| `tau_ms` | synapse time constant。 |
| `v_half_mV` | presynaptic activation sigmoid 的 half voltage。 |
| `k_s_mV` | presynaptic activation sigmoid slope。 |
| `active_in_v0` | 当前 component 是否纳入 v0。 |
| `evidence` | 数据证据说明。 |

## chemical_components_neuron_neuron_v0.csv

来源：从 `chemical_components_v0.csv` 过滤得到。

过滤条件：`pre` 和 `post` 都在 `neuron_parameter_reference_v0.csv` 的 302 neuron 名单中。

字段：和 `chemical_components_v0.csv` 完全相同。

运行时：`worm_brain_runner` 默认读取这个文件，而不是完整的 `chemical_components_v0.csv`。当前对应 summary 输出 `chemical_components=4308`。

## gap_junctions_v0.csv

来源：`build_gap_junctions()`。

粒度：一行是一条 undirected gap junction edge。

| 字段 | 含义 |
| --- | --- |
| `gap_id` | `cell_a__cell_b`。 |
| `cell_a` | gap junction 一端 cell。 |
| `cell_b` | gap junction 另一端 cell。 |
| `compartment_a` | `cell_a` placement compartment。v0 固定为 0。 |
| `compartment_b` | `cell_b` placement compartment。v0 固定为 0。 |
| `placement_policy` | placement 规则。当前为 `soma_compartment_0_v0`。 |
| `anatomical_weight` | Cook SI5 symmetric gap junction weight。 |
| `normalized_weight` | 归一化后的 weight，当前默认 `log1p_max`。 |
| `g0_uS` | gap junction base conductance，单位 uS。 |
| `g_uS` | 实际 conductance，`g0_uS * normalized_weight`。 |
| `active_in_v0` | 当前 gap junction 是否纳入 v0。 |
| `evidence` | 数据证据说明。 |

运行时：`worm_brain_runner` 默认读取这个文件。当前对应 summary 输出 `gap_junctions=1093`。

## neuron_parameter_reference_v0.csv

来源：`parse_baaiworm_table4()`。

粒度：一行是一个 BAAIWorm 302-neuron 列表中的 neuron。

| 字段 | 含义 |
| --- | --- |
| `baai_index` | BAAIWorm Supplementary Table 4 中的 index。 |
| `neuron` | neuron 名称。 |
| `functional_group` | functional group，例如 sensory neuron、interneuron、body motor neuron。 |
| `parameter_reference` | 五类参数模板之一：AWC / AIY / AVA / RIM / VD5。 |
| `reference_cell` | 当前 C++ 使用的代表 cell 名称：AWCL / AIYL / AVAL / RIML / VD05。 |
| `evidence` | 数据证据说明。 |

运行时：`brain_network_builder` 读这个文件创建 302 个 neuron，再由 `representative_neuron_catalog` 把 `reference_cell` 映射到 `data/awcl/AWCL_cell.csv` 等五个代表 cell CSV。

## synapse_config_v0.json

来源：`synapse_config()`。

它不是连接表，而是导出参数记录。主要字段：

| 字段 | 含义 |
| --- | --- |
| `chemical.structure_source` | chemical structure 来源。当前为 Cook SI5 hermaphrodite chemical。 |
| `chemical.polarity_source` | polarity 来源。当前为 Fenyves S1 sign prediction。 |
| `chemical.weight_normalization` | chemical weight 归一化策略。 |
| `chemical.g0_exc_uS` / `chemical.g0_inh_uS` | exc / inh base conductance。 |
| `chemical.tau_ms` | chemical synapse time constant。 |
| `chemical.v_half_mV` / `chemical.k_s_mV` | presynaptic activation sigmoid 参数。 |
| `chemical.E_exc_mV` / `chemical.E_inh_mV` | exc / inh reversal potential。 |
| `chemical.no_pred_policy` | Fenyves `no pred` 的处理策略。当前 default inhibitory。 |
| `chemical.missing_fenyves_policy` | 缺失 Fenyves polarity 的处理策略。当前 default inhibitory。 |
| `gap.weight_normalization` | gap weight 归一化策略。 |
| `gap.g0_gap_uS` | gap junction base conductance。 |
| `neuron_parameter_reference` | 302-neuron 模板映射来源和五个代表模板。 |
| `disabled_in_v0` | 明确记录 v0 暂未启用的机制或数据源。 |

## summary.json

来源：导出脚本最后的 summary block。

它是检查用统计摘要，不是运行时输入。主要内容包括：

| 字段 | 含义 |
| --- | --- |
| `data_sources` | 本次导出使用的 Cook / Fenyves / BAAIWorm 文件路径。 |
| `chemical_connections` | connection 总数。当前为 5001。 |
| `chemical_connections_by_source` | 按 Cook / Fenyves 来源分类计数。 |
| `chemical_connections_by_class` | 按 neuron-to-neuron 等 connection class 计数。 |
| `chemical_connections_by_effective_polarity` | 按 v0 effective polarity 计数。 |
| `chemical_components` | component 总数。当前为 5478。 |
| `chemical_components_neuron_neuron` | neuron-to-neuron component 数。当前为 4308。 |
| `gap_junctions` | gap junction 数。当前为 1093。 |
| `neuron_parameter_references` | AWC / AIY / AVA / RIM / VD5 模板分布。 |
| `neuron_functional_groups` | functional group 分布。 |

## 重新导出

推荐显式指定 source root，避免机器路径变化导致误读：

```powershell
python scripts\export_synapse_v0_components.py `
  --source-root "E:\1 PhD work\C.elegans simulation\code\draft\C.elegans.network" `
  --output-dir data\synapse_v0
```

如果 BAAIWorm 文本不在默认位置，显式传入：

```powershell
python scripts\export_synapse_v0_components.py `
  --source-root "E:\1 PhD work\C.elegans simulation\code\draft\C.elegans.network" `
  --baai-table4-text "E:\1 PhD work\C.elegans simulation\code\draft\multi-compartment neuron 0416\baaiworm_pdf_text\pdf_1.txt" `
  --output-dir data\synapse_v0
```

## 当前策略

- chemical structure 主要来自 Cook 2019 SI5。
- polarity 来自 Fenyves 2020 sign prediction。
- Fenyves `no pred` 和缺失 polarity 当前按 inhibitory 处理。
- `complex` polarity 拆成 exc / inh 两个 component，`rho=0.5/0.5`。
- synapse placement 当前全部落在 soma compartment 0。
- gap junction 来自 Cook 2019 SI5 symmetric gap junction matrix。

这些策略属于 v0 数据导出规则。不要在 C++ loader 中偷偷改这些规则；需要改数据语义时，应先改导出脚本并重新生成 CSV。
