下面这段对应 [src/neuron/cable_solver.cpp](/E:/1 PhD work/C.elegans simulation/code/neural network/annotate-and-test-main/src/neuron/cable_solver.cpp:26) 的 `CableSolver::advance_voltages()`，核心数学目标是：

```text
给定当前电压 V^k，求下一步电压 V^{k+1}

A V^{k+1} = b
```
**函数参数**

这三个参数都是长度等于 compartment 数量的 `std::vector<double>`，即：

```cpp
vector[i] 对应第 i 个 compartment
```

* `injected_current_pA[i]`：注入到第 `i` 个 compartment 的外部电流，单位 `pA`，数学上对应 $I_{\mathrm{inj},i}$，进入右端项：
  $$
  b_i += I_{\mathrm{inj},i}
  $$
  👉 **对应 iclamp（电流钳）**：直接向系统注入电流，不改变系统矩阵，只影响右端项。

* `extra_conductance_nS[i]`：第 `i` 个 compartment 上额外施加的电导，单位 `nS`，主要用于 voltage clamp。数学上对应 $g_{\mathrm{extra},i}$，进入矩阵左边：
  $$
  A_{ii} += g_{\mathrm{extra},i}
  $$

* `extra_reversal_mV[i]`：额外电导对应的目标电压/反转电位，单位 `mV`，与 `extra_conductance_nS` 配套使用。数学上：
  $$
  I_{\mathrm{extra},i} = g_{\mathrm{extra},i}(E_{\mathrm{extra},i} - V_i)
  $$
  因此：
  $$
  A_{ii} += g_{\mathrm{extra},i}, \qquad
  b_i += g_{\mathrm{extra},i} E_{\mathrm{extra},i}
  $$
  👉 **对应 seclamp / voltage clamp（电压钳）**：通过增加一个“导向目标电压”的电导，把膜电位拉向 $E_{\mathrm{extra},i}$，同时修改矩阵（左边）和右端项。

---

### iclamp vs seclamp 在这里的区别

* **iclamp（电流钳）**

    * 只使用 `injected_current_pA`
    * 只影响右端项 $b$
    * 不改变系统矩阵 $A$
    * 物理意义：给定电流，让电压自由响应

* **seclamp（电压钳）**

    * 使用 `extra_conductance_nS` + `extra_reversal_mV`
    * 同时修改矩阵 $A$ 和右端项 $b$
    * 物理意义：通过一个很大的电导，把电压“拉”到目标值

---

在当前 `iclamp` 情况下，只使用 `injected_current_pA`，另外两个 vector 全是 0。刺激期间：

```cpp
injected_current_pA = [5, 5, 0, 0, ...]
extra_conductance_nS = [0, 0, 0, 0, ...]
extra_reversal_mV = [0, 0, 0, 0, ...]
```

非刺激期间三者全为 0。

**变量定义**

对第 `i` 个 compartment：

```text
V_i^k        当前膜电位
V_i^{k+1}    下一步膜电位
C_i          membrane capacitance
gL_i         leak conductance
EL_i         leak reversal potential
I_inj,i      外部注入电流
I_ion,i      active channels 总电流
g_ij         i 和 j 之间的轴向电导
dt           时间步长
```

核心一句话：

$\boxed{ \text{它把当前 } V^n \text{、通道电流、注入电流、漏电和轴向耦合，组装成 } A V^{n+1}=b }$

然后求解：

$\boxed{ V^{n+1}=A^{-1}b }$

* * *

# 0\. 输入和目标

对应代码：**第 26–31 行**

函数输入是：

```
Cell& cell,
double dt_ms,
const std::vector<double>& injected_current_pA,
const std::vector<double>& extra_conductance_nS,
const std::vector<double>& extra_reversal_mV
```

数学上，给定当前时刻 $t_n$：

$V^n = \begin{bmatrix} V_0^n\\ V_1^n\\ \vdots\\ V_{N-1}^n \end{bmatrix}$

目标是求下一时刻：

$V^{n+1} = \begin{bmatrix} V_0^{n+1}\\ V_1^{n+1}\\ \vdots\\ V_{N-1}^{n+1} \end{bmatrix}$

其中：

$\Delta t = dt_{\mathrm{ms}}$

如果是 AIYL：

$N=16$

但这个函数本身不关心是不是 AIYL，它只看 `cell.compartments.size()`。

* * *

# 1\. 读取 compartment 数量并检查输入合法性

对应代码：**第 32–41 行**

```
const std::size_t n = cell.compartments.size();
```

数学上：

$N = |\mathrm{compartments}|$

然后检查三个条件：

$|I^{\mathrm{inj}}| = N$ $|g^{\mathrm{extra}}| = N$ $|E^{\mathrm{extra}}| = N$ $\Delta t > 0$

否则直接报错。

这里的含义是：

```
每个 compartment 必须都有一个 injected current
每个 compartment 必须都有一个 extra conductance
每个 compartment 必须都有一个 extra reversal
时间步长必须为正
```

* * *

# 2\. 准备矩阵和右端项

对应代码：**第 43–48 行**

```
resize_if_needed(n);
...
std::fill(matrix_.begin(), matrix_.end(), 0.0);
std::fill(rhs_.begin(), rhs_.end(), 0.0);
```

数学上准备一个线性系统：

$A V^{n+1}=b$

其中：

$A\in \mathbb{R}^{N\times N}$ $b\in \mathbb{R}^{N}$

初始时：

$A=0$ $b=0$

* * *

# 3\. 判断矩阵是否可以缓存

对应代码：**第 44–46 行**

```
const bool cacheable_matrix = std::all_of(extra_conductance_nS.begin(), extra_conductance_nS.end(), [](double g) {
    return g == 0.0;
});
```

数学上检查：

$g_i^{\mathrm{extra}}=0,\quad \forall i$

如果所有 extra conductance 都是 0，则：

$\mathrm{cacheable}=true$

否则：

$\mathrm{cacheable}=false$

含义是：

```
如果没有额外电导，矩阵 A 只由 C、g_L、g_C、dt 决定。
同一个 dt 下，A 基本不变，可以缓存 A^{-1}。
```

如果有 extra conductance，例如 voltage clamp 加进来的电导项：

$g_i^{\mathrm{extra}}\neq 0$

那矩阵每步可能变，不能复用旧逆矩阵。

* * *

# 4\. 遍历每个 compartment

对应代码：**第 50 行**

```
for (std::size_t i = 0; i < n; ++i)
```

数学上，对每个 compartment：

$i=0,1,\dots,N-1$

依次组装矩阵 $A$ 和右端项 $b$。

* * *

# 5\. 读取当前 compartment 参数

对应代码：**第 51–56 行**

```
auto& c = cell.compartments[i];
...
const double c_over_dt = c.capacitance_pF / dt_ms;
const double extra_g = extra_conductance_nS[i];
```

对第 $i$ 个 compartment，定义：

$C_i = c.\mathrm{capacitance\_pF}$ $g_{L,i}=c.\mathrm{leak\_conductance\_nS}$ $E_{L,i}=c.\mathrm{leak\_reversal\_mV}$ $V_i^n=c.\mathrm{voltage\_mV}$ $g_i^{\mathrm{extra}}=\mathrm{extra\_conductance\_nS}[i]$ $E_i^{\mathrm{extra}}=\mathrm{extra\_reversal\_mV}[i]$

然后定义：

$\frac{C_i}{\Delta t}$

代码中是：

```
c_over_dt = c.capacitance_pF / dt_ms;
```

单位上：

$\frac{\mathrm{pF}}{\mathrm{ms}} = \mathrm{nS}$

因为：

$\mathrm{nS}\cdot \mathrm{mV} = \mathrm{pA}$

所以这里单位是能对上的。

* * *

# 6\. 检查电容必须为正

对应代码：**第 52–54 行**

```
if (c.capacitance_pF <= 0.0) {
    throw std::runtime_error("Compartment capacitance must be positive");
}
```

数学要求：

$C_i>0$

否则：

$\frac{C_i}{\Delta t}$

没有物理意义，线性系统也可能坏掉。

* * *

# 7\. 加入膜电容、漏电导和额外电导到矩阵对角线

对应代码：**第 55–57 行**

```
at(i, i, n) += c_over_dt + c.leak_conductance_nS + extra_g;
```

数学上，对矩阵 $A$ 的第 $i$ 个对角线元素加入：

$A_{ii} \leftarrow A_{ii} + \frac{C_i}{\Delta t} + g_{L,i} + g_i^{\mathrm{extra}}$

这一步来自隐式离散化：

$C_i\frac{V_i^{n+1}-V_i^n}{\Delta t} = -g_{L,i}(V_i^{n+1}-E_{L,i}) -g_i^{\mathrm{extra}}(V_i^{n+1}-E_i^{\mathrm{extra}}) +\cdots$

把含 $V_i^{n+1}$ 的项移到左边，就得到：

$\left( \frac{C_i}{\Delta t} + g_{L,i} + g_i^{\mathrm{extra}} \right) V_i^{n+1}$

所以这些项进入 $A_{ii}$。

* * *

# 8\. 计算当前 compartment 的所有通道电流

对应代码：**第 59–64 行**

```
double ion_current_pA = 0.0;
for (const auto& channel : c.channels) {
    channel->set_calcium(c.cai_uM_per_um2);
    channel->step(c.voltage_mV, dt_ms);
    ion_current_pA += channel->current_pA(c.voltage_mV);
}
```

数学上，对 compartment $i$ 的所有通道 $k\in\mathcal{M}_i$：

先把钙浓度传给通道：

$Ca_i^n \rightarrow \mathrm{channel}_k$

然后通道内部状态更新：

$x_{k,i}^{n+1} = F_k(x_{k,i}^{n}, V_i^n, Ca_i^n, \Delta t)$

然后计算通道电流：

$I_{k,i}^{n} = I_k(V_i^n, x_{k,i}^{n+1}, Ca_i^n)$

代码这里把所有通道电流加起来：

$I_i^{\mathrm{ion}} = \sum_{k\in\mathcal{M}_i} I_{k,i}^{n}$

也就是：

$I_i^{\mathrm{ion}} = I_{\mathrm{NCA},i} + I_{\mathrm{IRK},i} + I_{\mathrm{KQT3},i} + \cdots$

注意一个关键点：

```
通道电流用的是当前电压 V_i^n，不是未知的 V_i^{n+1}
```

所以这里的通道电流是显式处理。

* * *

# 9\. 组装右端项 b

对应代码：**第 66–67 行**

```
rhs_[i] += c_over_dt * c.voltage_mV + c.leak_conductance_nS * c.leak_reversal_mV
           + extra_g * extra_reversal_mV[i] + injected_current_pA[i] - ion_current_pA;
```

数学上：

$b_i \leftarrow b_i + \frac{C_i}{\Delta t}V_i^n + g_{L,i}E_{L,i} + g_i^{\mathrm{extra}}E_i^{\mathrm{extra}} + I_i^{\mathrm{inj}} - I_i^{\mathrm{ion}}$

也就是：

$\boxed{ b_i = \frac{C_i}{\Delta t}V_i^n + g_{L,i}E_{L,i} + g_i^{\mathrm{extra}}E_i^{\mathrm{extra}} + I_i^{\mathrm{inj}} - I_i^{\mathrm{ion}} }$

其中：

| 项   | 含义  |
| --- | --- |
| $\frac{C_i}{\Delta t}V_i^n$ | 上一时刻膜电压带来的电容项 |
| $g_{L,i}E_{L,i}$ | 漏电反转电位项 |
| $g_i^{\mathrm{extra}}E_i^{\mathrm{extra}}$ | 额外电导的反转电位项 |
| $I_i^{\mathrm{inj}}$ | 外部注入电流，比如 IClamp 或 synapse 输入 |
| $-I_i^{\mathrm{ion}}$ | 离子通道电流作为外流项被减掉 |

这一行非常核心。它是右端项 $b$ 的主要来源。

* * *

# 10\. 处理轴向连接：如果当前 compartment 有 parent

对应代码：**第 69–79 行**

```
if (c.parent_index >= 0) {
    const auto parent = static_cast<std::size_t>(c.parent_index);
    ...
    const double g = c.axial_conductance_to_parent_nS;
    at(i, i, n) += g;
    at(parent, parent, n) += g;
    at(i, parent, n) -= g;
    at(parent, i, n) -= g;
}
```

如果 compartment $i$ 有父节点 $p$，则它们之间有轴向电导：

$g_{ip}=g_i^{\mathrm{axial}}$

轴向电流为：

$I_{i\rightarrow p} = g_{ip}(V_i^{n+1}-V_p^{n+1})$

对节点 $i$，这个项进入方程左边：

$+g_{ip}V_i^{n+1} -g_{ip}V_p^{n+1}$

对节点 $p$，对应为：

$-g_{ip}V_i^{n+1} +g_{ip}V_p^{n+1}$

所以矩阵更新是：

$A_{ii} \leftarrow A_{ii}+g_{ip}$ $A_{pp} \leftarrow A_{pp}+g_{ip}$ $A_{ip} \leftarrow A_{ip}-g_{ip}$ $A_{pi} \leftarrow A_{pi}-g_{ip}$

这正好对应第 75–78 行。

这一段的物理意义：

```
两个 compartment 被一个电导连接。
如果二者电压不同，就产生轴向电流。
矩阵里表现为对角线加 g，非对角线减 g。
```

* * *

# 11\. 循环结束后得到完整线性系统

对应代码：**第 50–80 行整体**

经过所有 compartment 后，得到：

$A V^{n+1}=b$

其中矩阵 $A$ 的一般形式是：

$A_{ii} = \frac{C_i}{\Delta t} + g_{L,i} + g_i^{\mathrm{extra}} + \sum_{j\in\mathcal{N}(i)}g_{ij}$

如果 $i$ 和 $j$ 之间有轴向连接：

$A_{ij}=-g_{ij}$

否则：

$A_{ij}=0$

右端项是：

$b_i = \frac{C_i}{\Delta t}V_i^n + g_{L,i}E_{L,i} + g_i^{\mathrm{extra}}E_i^{\mathrm{extra}} + I_i^{\mathrm{inj}} - I_i^{\mathrm{ion}}$

所以完整的 compartment 方程是：

$\boxed{ \left( \frac{C_i}{\Delta t} + g_{L,i} + g_i^{\mathrm{extra}} + \sum_{j\in\mathcal{N}(i)}g_{ij} \right) V_i^{n+1} - \sum_{j\in\mathcal{N}(i)} g_{ij}V_j^{n+1} = \frac{C_i}{\Delta t}V_i^n + g_{L,i}E_{L,i} + g_i^{\mathrm{extra}}E_i^{\mathrm{extra}} + I_i^{\mathrm{inj}} - I_i^{\mathrm{ion}} }$

这就是 `advance_voltages()` 的数学核心。

* * *

# 12\. 选择求解方式

对应代码：**第 82–134 行**

代码分三种情况。

* * *

## 12.1 情况一：矩阵可缓存，而且已有缓存

对应代码：**第 82–83 行**

```
if (cacheable_matrix && cached_inverse_valid_ && cached_inverse_dt_ms_ == dt_ms) {
    solve_with_cached_inverse(n, dt_ms, true);
}
```

数学条件：

$g_i^{\mathrm{extra}}=0,\quad \forall i$ $A^{-1}\ \text{已经缓存}$ $\Delta t=\Delta t_{\mathrm{cached}}$

那么直接使用：

$V^{n+1}=A^{-1}_{\mathrm{cached}}b$

对应真正乘法在 **第 141–149 行**：

$V_i^{n+1} = \sum_{j=0}^{N-1} (A^{-1})_{ij}b_j$

代码：

```
voltage += inverse_matrix_[i * n + j] * rhs_[j];
solution_[i] = voltage;
```

* * *

## 12.2 情况二：矩阵可缓存，但还没有缓存

对应代码：**第 84–131 行**

条件：

$g_i^{\mathrm{extra}}=0,\quad \forall i$

但还没有可用的 $A^{-1}$。

于是代码先构造单位矩阵：

对应代码：**第 85–88 行**

$A^{-1}_{\mathrm{work}} = I$

然后对矩阵 $A$ 做 Gauss-Jordan 消元。

* * *

### 12.2.1 找主元

对应代码：**第 90–102 行**

对第 `col` 列，找绝对值最大的主元：

$pivot = \arg\max_{row\ge col} |A_{row,col}|$

如果：

$|A_{pivot,col}| < 10^{-18}$

说明矩阵近似奇异，报错。

* * *

### 12.2.2 交换行

对应代码：**第 103–108 行**

如果：

$pivot\neq col$

交换 $A$ 的两行，同时交换逆矩阵工作区的两行：

$A_{col,*}\leftrightarrow A_{pivot,*}$ $B_{col,*}\leftrightarrow B_{pivot,*}$

这里 $B$ 最开始是单位矩阵，最后会变成：

$B=A^{-1}$

* * *

### 12.2.3 主元行归一化

对应代码：**第 109–113 行**

令：

$d=A_{col,col}$

然后：

$A_{col,k}\leftarrow \frac{A_{col,k}}{d}$ $B_{col,k}\leftarrow \frac{B_{col,k}}{d}$

这样主元变成：

$A_{col,col}=1$

* * *

### 12.2.4 消去其他行

对应代码：**第 115–127 行**

对所有其他行：

$row\neq col$

令：

$factor=A_{row,col}$

然后：

$A_{row,k} \leftarrow A_{row,k} - factor\cdot A_{col,k}$ $B_{row,k} \leftarrow B_{row,k} - factor\cdot B_{col,k}$

最终把整列除了主元以外都消成 0。

* * *

### 12.2.5 得到并缓存逆矩阵

对应代码：**第 129–130 行**

消元完成后：

$A\rightarrow I$ $B\rightarrow A^{-1}$

所以：

$\mathrm{inverse\_matrix\_}=A^{-1}$

然后标记：

$\mathrm{cached\_inverse\_valid}=true$ $\mathrm{cached\_inverse\_dt}=\Delta t$

* * *

### 12.2.6 用缓存逆矩阵求解

对应代码：**第 131 行**  
具体乘法：**第 141–149 行**

$V^{n+1}=A^{-1}b$

即：

$V_i^{n+1} = \sum_{j=0}^{N-1} (A^{-1})_{ij}b_j$

* * *

## 12.3 情况三：矩阵不可缓存，直接求解

对应代码：**第 132–134 行**

```
else {
    solve_direct(n);
}
```

当存在：

$g_i^{\mathrm{extra}}\neq 0$

则直接解：

$A V^{n+1}=b$

对应实现：**第 151–195 行**

* * *

### 12.3.1 逐列消元

对应代码：**第 152–190 行**

这也是 Gauss-Jordan 消元，不过这次不是求 $A^{-1}$，而是直接对增广系统：

$[A\mid b]$

做消元。

* * *

### 12.3.2 找主元

对应代码：**第 153–164 行**

$pivot = \arg\max_{row\ge col} |A_{row,col}|$

如果：

$|A_{pivot,col}|<10^{-18}$

报错：

$A\ \text{singular}$

* * *

### 12.3.3 交换行

对应代码：**第 165–170 行**

$A_{col,*}\leftrightarrow A_{pivot,*}$ $b_{col}\leftrightarrow b_{pivot}$

* * *

### 12.3.4 主元行归一化

对应代码：**第 171–175 行**

令：

$d=A_{col,col}$

然后：

$A_{col,k}\leftarrow \frac{A_{col,k}}{d}$ $b_{col}\leftarrow \frac{b_{col}}{d}$

* * *

### 12.3.5 消去其他行

对应代码：**第 177–189 行**

对所有：

$row\neq col$

令：

$factor=A_{row,col}$

然后：

$A_{row,k} \leftarrow A_{row,k} - factor A_{col,k}$ $b_{row} \leftarrow b_{row} - factor b_{col}$

消元完成后：

$A\rightarrow I$ $b\rightarrow V^{n+1}$

* * *

### 12.3.6 保存解

对应代码：**第 192–194 行**

```
solution_[i] = rhs_[i];
```

数学上：

$solution_i = b_i$

因为此时：

$b=V^{n+1}$

* * *

# 13\. 把解写回 cell

对应代码：**第 136–138 行**

```
cell.compartments[i].voltage_mV = solution_[i];
```

数学上：

$V_i^{n+1} \leftarrow solution_i$

也就是：

$cell.compartments[i].voltage\_mV \leftarrow V_i^{n+1}$

从这一刻开始，`cell` 内部保存的电压就从旧的：

$V^n$

变成新的：

$V^{n+1}$

* * *

# 14\. 总公式版

把上面全部压缩成一个完整数学流程：

对每个 compartment $i$：

$C_i\frac{V_i^{n+1}-V_i^n}{\Delta t} = -g_{L,i}(V_i^{n+1}-E_{L,i}) -g_i^{\mathrm{extra}}(V_i^{n+1}-E_i^{\mathrm{extra}}) -\sum_{j\in\mathcal{N}(i)}g_{ij}(V_i^{n+1}-V_j^{n+1}) -I_i^{\mathrm{ion}}(V_i^n) + I_i^{\mathrm{inj}}$

其中：

$I_i^{\mathrm{ion}}(V_i^n) = \sum_{k\in\mathcal{M}_i} I_{k,i}(V_i^n, x_{k,i}, Ca_i)$

整理得到：

$\left( \frac{C_i}{\Delta t} + g_{L,i} + g_i^{\mathrm{extra}} + \sum_{j\in\mathcal{N}(i)}g_{ij} \right) V_i^{n+1} - \sum_{j\in\mathcal{N}(i)} g_{ij}V_j^{n+1} = \frac{C_i}{\Delta t}V_i^n + g_{L,i}E_{L,i} + g_i^{\mathrm{extra}}E_i^{\mathrm{extra}} + I_i^{\mathrm{inj}} - I_i^{\mathrm{ion}}$

然后组成：

$A V^{n+1}=b$

最后解：

$V^{n+1}=A^{-1}b$

* * *

# 15\. 行号总表

| 步骤  | 数学含义 | 对应代码行 |
| --- | --- | --- |
| 1   | 输入 `cell, dt, Iinj, extra_g, extra_E` | 26–31 |
| 2   | 得到 compartment 数量 $N$ | 32  |
| 3   | 检查输入维度 | 33–38 |
| 4   | 检查 $\Delta t>0$ | 39–41 |
| 5   | 准备矩阵和 RHS | 43, 47–48 |
| 6   | 判断是否可缓存 $A^{-1}$ | 44–46 |
| 7   | 遍历 compartment | 50  |
| 8   | 读取 compartment 参数 | 51, 55–56 |
| 9   | 检查 $C_i>0$ | 52–54 |
| 10  | 加入 $\frac{C_i}{dt}+g_L+g_{extra}$ 到 $A_{ii}$ | 57  |
| 11  | 更新通道状态并求 $I_i^{ion}$ | 59–64 |
| 12  | 组装 $b_i$ | 66–67 |
| 13  | 加入 parent 轴向连接 | 69–79 |
| 14  | 若可复用缓存，直接 $V=A^{-1}b$ | 82–83, 141–149 |
| 15  | 若需要新建缓存，求 $A^{-1}$ | 84–131 |
| 16  | 若不可缓存，直接解 $AV=b$ | 132–134, 151–195 |
| 17  | 写回新电压 $V^{n+1}$ | 136–138 |

* * *

# 16\. 最关键的一句话

这段代码不是在“手动更新一个 compartment 的电压”，而是在做：

$\boxed{ \text{把所有 compartment 的耦合关系一次性组装成矩阵，然后整体求解下一步电压} }$

所以它不是简单的：

$V_i^{n+1}=V_i^n+\Delta t f(V_i)$

而是：

$\boxed{ A V^{n+1}=b }$

这就是多隔室模型的核心。漏电和轴向连接是隐式的，通道电流是显式的。这样比纯显式欧拉稳一些，不然多隔室耦合一强，电压很容易直接飞天。