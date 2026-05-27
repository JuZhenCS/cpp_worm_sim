# cpp_worm_sim

这是 C++ 版线虫模拟的干净运行时起点。

当前目录只保留已经验证过的五类多室神经元核心：`AIYL`、`AVAL`、`RIML`、`AWCL`、`VD05`。来源是 `cpp_neuron_core_v2` 的 `five-cell-neuron-core-v1` 神经元层，不包含 NEURON reference、历史输出、趋势图、对齐脚本、NESTML、synapse、network、body 或 environment。

后续扩展方向是在这个目录上继续加：

```text
src/synapse/       自定义分级化学突触
src/network/       connectome 和网络调度
src/body/          身体模型接口
src/environment/   环境接口
```

## 构建

推荐用 Ninja：

```powershell
cd "E:\1 PhD work\C.elegans simulation\code\cpp_worm_sim"
cmake -S . -B build -G Ninja
cmake --build build
```

如果本机没有 Ninja：

```powershell
cd "E:\1 PhD work\C.elegans simulation\code\cpp_worm_sim"
cmake -S . -B build
cmake --build build --config Release
```

## 运行 current clamp

AVAL：

```powershell
.\build\cpp_neuron_runner.exe --cell AVAL --protocol iclamp --cell-file data\aval\AVAL_cell.csv --amp-pa 10 --enable-nca --enable-shk1 --enable-shl1 --enable-egl19 --enable-cca1 --enable-unc2 --enable-calcium-internal --enable-kcnl --enable-slo1-unc2 --output output\aval_10pA.csv --diag-output output\aval_10pA_diag.csv
```

AIYL：

```powershell
.\build\cpp_neuron_runner.exe --cell AIYL --protocol iclamp --cell-file data\aiyl\AIYL_cell.csv --amp-pa 10 --enable-nca --enable-irk --enable-kqt3 --enable-egl2 --enable-shk1 --enable-kvs1 --enable-shl1 --enable-egl36 --enable-egl19 --enable-cca1 --enable-calcium-internal --enable-kcnl --enable-slo1-egl19 --enable-slo1-unc2 --enable-slo2-egl19 --enable-slo2-unc2 --output output\aiyl_10pA.csv --diag-output output\aiyl_10pA_diag.csv
```

RIML：

```powershell
.\build\cpp_neuron_runner.exe --cell RIML --protocol iclamp --cell-file data\riml\RIML_cell.csv --amp-pa 10 --enable-nca --enable-irk --enable-kqt3 --enable-egl2 --enable-shk1 --enable-kvs1 --enable-shl1 --enable-egl36 --enable-slo1-egl19 --enable-slo1-unc2 --enable-slo2-egl19 --output output\riml_10pA.csv --diag-output output\riml_10pA_diag.csv
```

## 运行 SEClamp

AWCL：

```powershell
.\build\cpp_neuron_runner.exe --cell AWCL --protocol seclamp --cell-file data\awcl\AWCL_cell.csv --vcmd-mv 70 --enable-kqt3 --enable-shl1 --enable-egl19 --enable-unc2 --output output\awcl_v70.csv --diag-output output\awcl_v70_diag.csv
```

VD05：

```powershell
.\build\cpp_neuron_runner.exe --cell VD05 --protocol seclamp --cell-file data\vd05\VD05_cell.csv --vcmd-mv 20 --enable-nca --enable-kqt3 --enable-egl2 --enable-shk1 --enable-shl1 --enable-egl36 --enable-egl19 --enable-cca1 --enable-slo1-unc2 --enable-slo2-egl19 --enable-slo2-unc2 --output output\vd05_v20.csv --diag-output output\vd05_v20_diag.csv
```

SEClamp 输出的是 comparable current，也就是去掉命令跳变边缘电容瞬态后的电流口径。后续网络模拟不会使用 SEClamp。

## 当前边界

当前已经可以作为神经元层使用；还没有实现突触、gap junction、connectome、网络调度、身体或环境。

下一步建议先实现 `src/synapse/graded_chemical_synapse.*`，从 BAAIWorm/eWorm 的分级化学突触公式开始；再做最小 two-cell network scheduler。
