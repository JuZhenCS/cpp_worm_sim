下面是 `CableSolver::advance_voltages()` 的**纯数学流程**，完全不带代码。

* * *

# 1\. 问题定义

假设一个神经元有 $N$ 个 compartment：

$i=0,1,\dots,N-1$

当前时刻为：

$t_n$

当前电压向量为：

$\mathbf{V}^n= \begin{bmatrix} V_0^n\\ V_1^n\\ \vdots\\ V_{N-1}^n \end{bmatrix}$

目标是求下一时刻：

$\mathbf{V}^{n+1}= \begin{bmatrix} V_0^{n+1}\\ V_1^{n+1}\\ \vdots\\ V_{N-1}^{n+1} \end{bmatrix}$

时间步长为：

$\Delta t$

* * *

# 2\. 每个 compartment 的参数

对第 $i$ 个 compartment，有：

$C_i$

表示膜电容；

$g_{L,i}$

表示漏电导；

$E_{L,i}$

表示漏电反转电位；

$I_i^{\mathrm{inj}}$

表示外部注入电流；

$g_i^{\mathrm{extra}}$

表示额外电导，例如 clamp 电导；

$E_i^{\mathrm{extra}}$

表示额外电导对应的反转电位；

$\mathcal{N}(i)$

表示与 compartment $i$ 相连的相邻 compartment 集合；

$g_{ij}$

表示 compartment $i$ 和 $j$ 之间的轴向电导。

* * *

# 3\. 通道电流

第 $i$ 个 compartment 上可能有多个离子通道。设通道集合为：

$\mathcal{M}_i$

每个通道 $k$ 产生电流：

$I_{k,i}^{\mathrm{ion}}$

总离子通道电流为：

$I_i^{\mathrm{ion}} = \sum_{k\in\mathcal{M}_i} I_{k,i}^{\mathrm{ion}}$

一般形式可以写成：

$I_{k,i}^{\mathrm{ion}} = g_{k,i} G_{k,i} \left( V_i^n-E_{k} \right)$

其中：

$g_{k,i}$

是该通道最大电导；

$G_{k,i}$

是门控变量、钙浓度、电压共同决定的有效开放比例；

$E_k$

是该通道反转电位。

所以：

$I_i^{\mathrm{ion}} = \sum_{k\in\mathcal{M}_i} g_{k,i} G_{k,i} \left( V_i^n-E_k \right)$

注意，这里的通道电流使用的是当前电压：

$V_i^n$

而不是未知的新电压：

$V_i^{n+1}$

因此通道电流是显式项。

* * *

# 4\. 原始膜电位方程

对每个 compartment $i$，电压更新满足：

$C_i \frac{ V_i^{n+1}-V_i^n }{ \Delta t } = -I_{L,i}^{n+1} -I_{\mathrm{extra},i}^{n+1} -I_{\mathrm{axial},i}^{n+1} -I_i^{\mathrm{ion}} + I_i^{\mathrm{inj}}$

其中：

$I_{L,i}^{n+1} = g_{L,i} \left( V_i^{n+1}-E_{L,i} \right)$ $I_{\mathrm{extra},i}^{n+1} = g_i^{\mathrm{extra}} \left( V_i^{n+1}-E_i^{\mathrm{extra}} \right)$ $I_{\mathrm{axial},i}^{n+1} = \sum_{j\in\mathcal{N}(i)} g_{ij} \left( V_i^{n+1}-V_j^{n+1} \right)$

代入后：

$C_i \frac{ V_i^{n+1}-V_i^n }{ \Delta t } = -g_{L,i} \left( V_i^{n+1}-E_{L,i} \right) -g_i^{\mathrm{extra}} \left( V_i^{n+1}-E_i^{\mathrm{extra}} \right) -\sum_{j\in\mathcal{N}(i)} g_{ij} \left( V_i^{n+1}-V_j^{n+1} \right) -I_i^{\mathrm{ion}} + I_i^{\mathrm{inj}}$

* * *

# 5\. 整理未知量

把所有含 $V^{n+1}$ 的项放到左边，把已知项放到右边。

左边：

$\left( \frac{C_i}{\Delta t} + g_{L,i} + g_i^{\mathrm{extra}} + \sum_{j\in\mathcal{N}(i)}g_{ij} \right) V_i^{n+1} - \sum_{j\in\mathcal{N}(i)} g_{ij}V_j^{n+1}$

右边：

$\frac{C_i}{\Delta t}V_i^n + g_{L,i}E_{L,i} + g_i^{\mathrm{extra}}E_i^{\mathrm{extra}} + I_i^{\mathrm{inj}} - I_i^{\mathrm{ion}}$

所以每个 compartment 的离散方程是：

$\boxed{ \left( \frac{C_i}{\Delta t} + g_{L,i} + g_i^{\mathrm{extra}} + \sum_{j\in\mathcal{N}(i)}g_{ij} \right) V_i^{n+1} - \sum_{j\in\mathcal{N}(i)} g_{ij}V_j^{n+1} = \frac{C_i}{\Delta t}V_i^n + g_{L,i}E_{L,i} + g_i^{\mathrm{extra}}E_i^{\mathrm{extra}} + I_i^{\mathrm{inj}} - I_i^{\mathrm{ion}} }$

* * *

# 6\. 写成矩阵形式

把所有 compartment 的方程合起来，得到线性系统：

$\boxed{ A\mathbf{V}^{n+1}=\mathbf{b} }$

其中矩阵 $A$ 的元素为：

$A_{ii} = \frac{C_i}{\Delta t} + g_{L,i} + g_i^{\mathrm{extra}} + \sum_{j\in\mathcal{N}(i)}g_{ij}$

如果 $i$ 和 $j$ 相连：

$A_{ij}=-g_{ij}$

如果 $i$ 和 $j$ 不相连：

$A_{ij}=0$

右端向量为：

$b_i = \frac{C_i}{\Delta t}V_i^n + g_{L,i}E_{L,i} + g_i^{\mathrm{extra}}E_i^{\mathrm{extra}} + I_i^{\mathrm{inj}} - I_i^{\mathrm{ion}}$

* * *

# 7\. 求解下一步电压

线性系统为：

$A\mathbf{V}^{n+1}=\mathbf{b}$

如果 $A$ 可逆，则：

$\boxed{ \mathbf{V}^{n+1}=A^{-1}\mathbf{b} }$

也就是：

$V_i^{n+1} = \sum_{j=0}^{N-1} (A^{-1})_{ij}b_j$

* * *

# 8\. 更新神经元状态

求出：

$\mathbf{V}^{n+1}$

之后，将每个 compartment 的电压更新为：

$V_i \leftarrow V_i^{n+1}$

也就是：

$\mathbf{V}^n \rightarrow \mathbf{V}^{n+1}$

然后进入下一个时间步。

* * *

# 9\. 最终压缩版

整个数学流程可以压缩成：

$\boxed{ \mathbf{V}^n,\ I^{\mathrm{inj}},\ I^{\mathrm{ion}},\ C,\ g_L,\ E_L,\ g_{\mathrm{axial}} \Rightarrow A\mathbf{V}^{n+1}=\mathbf{b} \Rightarrow \mathbf{V}^{n+1} }$

其中最核心的单 compartment 方程是：

$\boxed{ C_i \frac{ V_i^{n+1}-V_i^n }{ \Delta t } = -g_{L,i} \left( V_i^{n+1}-E_{L,i} \right) -g_i^{\mathrm{extra}} \left( V_i^{n+1}-E_i^{\mathrm{extra}} \right) -\sum_{j\in\mathcal{N}(i)} g_{ij} \left( V_i^{n+1}-V_j^{n+1} \right) -I_i^{\mathrm{ion}} + I_i^{\mathrm{inj}} }$

整理后就是：

$\boxed{ A\mathbf{V}^{n+1}=\mathbf{b} }$

这就是这段函数的纯数学含义。