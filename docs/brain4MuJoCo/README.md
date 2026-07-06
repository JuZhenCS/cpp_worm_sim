# brain4MuJoCo 运行说明

`brain4MuJoCo/` 是给 Python / MuJoCo 使用的 step-by-step brain runtime 路径。它和 `worm_brain_runner` 分开：

```text
worm_brain_runner
    命令行一次性运行 whole-brain simulation

brain4MuJoCo
    初始化一次 BrainRuntime
    多次 simulate(step)
    每次返回 96 路 body wall muscle output
```

C API 不调用 `run_brain()`，而是长期持有 `BrainRuntime` 和 `BrainNetwork`。

## 代码入口

| 文件 | 作用 |
| --- | --- |
| `include/brain4MuJoCo/brain_runtime.hpp` | C++ runtime API，长期持有 brain network。 |
| `src/brain4MuJoCo/brain_runtime.cpp` | `reset()` / `simulate_steps()` 实现。 |
| `include/brain4MuJoCo/muscle/muscle_output.hpp` | 96 路 muscle output API 和 projection edge 配置。 |
| `src/brain4MuJoCo/muscle/muscle_output.cpp` | 读取 projection CSV，并从 neuron voltage 聚合 muscle output。 |
| `include/brain4MuJoCo/brain_c_api.h` | 给 Python ctypes 使用的 C ABI。 |
| `src/brain4MuJoCo/brain_c_api.cpp` | DLL 导出函数和全局 runtime 生命周期。 |
| `examples/python_ctypes_worm_brain.py` | Python ctypes 调用示例。 |

## 构建

如果 `build/` 已经配置过：

```powershell
cmake --build build --target worm_brain_shared --config Release
```

如果还没有配置：

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target worm_brain_shared --config Release
```

同时确认原命令行 runner 仍能构建：

```powershell
cmake --build build --target worm_brain_runner --config Release
```

当前 MinGW/Ninja 构建下 DLL 产物通常是：

```text
build/libworm_brain_shared.dll
```

## Python ctypes 运行

```powershell
D:\anaconda3\envs\SCA\python.exe examples\python_ctypes_worm_brain.py
```

示例会做：

```text
1. 查找 build/libworm_brain_shared.dll
2. 把 DLL 目录和 CMake compiler bin 目录加入 Windows DLL 搜索路径
3. worm_brain_init(project_root)
4. 连续调用 simulate(10) 三次
5. 验证每次返回 96 个 finite float
6. worm_brain_shutdown()
```

典型输出：

```text
call=0 count=96 min=0 max=0.511907 first8=[...]
call=1 count=96 min=0 max=0.505811 first8=[...]
call=2 count=96 min=0 max=0.502125 first8=[...]
```

数值随连续调用变化，说明 runtime 没有每次 rebuild network，而是在同一个 brain state 上推进。

## C API

```c
int32_t worm_brain_init(const char* source_dir);
void worm_brain_reset(void);
float* simulate(int32_t step);
int32_t worm_brain_muscle_count(void);
const char* worm_brain_last_error(void);
void worm_brain_shutdown(void);
```

调用顺序：

```text
worm_brain_init(source_dir)
  -> simulate(step)
  -> simulate(step)
  -> ...
  -> worm_brain_shutdown()
```

`source_dir` 应该指向项目根目录。如果传 `nullptr` 或空字符串，DLL 会使用编译时的 `CPP_WORM_SIM_SOURCE_DIR`。

## simulate(step)

`simulate(step)` 的语义：

```text
推进已有 brain network 状态 step 个内部仿真步
每个内部步使用 dt_ms = 0.1
更新 96 路 muscle output buffer
返回 float* 指向内部 buffer
```

约束：

```text
必须先调用 worm_brain_init
step 不能为负数
返回指针由 DLL 持有，不要在 Python/C 侧释放
下一次 simulate/reset/shutdown 后旧指针内容可能改变或失效
```

## 96 路 muscle output

顺序固定：

```text
0  - 23 : DL01 - DL24
24 - 47 : DR01 - DR24
48 - 71 : VL01 - VL24
72 - 95 : VR01 - VR24
```

来源 CSV：

```text
data/muscle/motor_to_muscle_projection_v0.csv
```

这个 CSV 来自 Cook 2019 SI5 `hermaphrodite chemical` matrix。详细来源见：

```text
docs/data/motor_to_muscle_projection_v0.md
```

## muscle_output v0 计算

当前是 motor-to-muscle projection v0，不是真实 NMJ 模型。

对每条 projection edge：

```text
activation = clamp((pre_neuron_voltage - v_rest_mV) / v_scale_mV, 0, 1)
contribution = sign * weight * activation
```

对每个 muscle channel：

```text
output = clamp(sum(contribution) / sum(weight), 0, 1)
```

默认：

```text
v_rest_mV = -60.0
v_scale_mV = 40.0
```

如果某个 channel 没有 projection edge，则输出 `0`。当前 Cook SI5 缺失：

```text
vBWML24 = VL24 = muscle_index 71
```

所以 index 71 默认没有输入。

## 常见问题

### Python 找不到 DLL

先确认 shared library 已构建：

```powershell
cmake --build build --target worm_brain_shared --config Release
```

如果报依赖 DLL 找不到，通常是 MinGW runtime DLL 不在搜索路径。`examples/python_ctypes_worm_brain.py` 会自动读取 `build/CMakeCache.txt` 里的 `CMAKE_CXX_COMPILER`，并把 compiler 所在目录加入 `os.add_dll_directory()`。

### simulate 返回空指针

读取错误信息：

```python
lib.worm_brain_last_error().decode("utf-8")
```

常见原因：

```text
没有先 worm_brain_init
source_dir 不对，找不到 data/synapse_v0 或 data/muscle
step 是负数
```

### 和 worm_brain_runner 的关系

`worm_brain_runner` 仍然是命令行一次性 runner。`brain4MuJoCo` 是长期 runtime，不输出 summary，不做 diagnostics，不写文件。