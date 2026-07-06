# brain network 数学流程

下面是 `step_brain_network()` 这一层的纯数学流程。它对应的主要 C++ 落点是：

- `src/brain/brain_simulation.cpp`
  负责一个纯全脑时间步：清空输入电流、应用突触/电连接电流、推进每个 neuron。
- `src/synapse.cpp`
  负责单个 graded chemical synapse 和 gap junction 的电流公式。
- `src/synapse_loader.cpp`
  负责 `SynapseNetwork::apply(dt)`，也就是遍历所有 chemical / gap edge 并把电流注入 neuron。
- `src/neuron/cable_solver.cpp`
  负责每个 neuron 内部 multi-compartment 电压方程；这部分数学已经在 `docs/neuron/core/cable_solver_math.md`。

brain 层本身不再组装一个 302-neuron 的全局隐式矩阵；它把网络连接产生的电流作为外部注入项，交给每个 neuron 自己的 cable solver。

* * *

# 1. 网络状态定义

假设全脑网络有 $M$ 个 neuron：

$r=0,1,\dots,M-1$

第 $r$ 个 neuron 有 $N_r$ 个 compartment：

$i=0,1,\dots,N_r-1$

当前时间为：

$t_n$

时间步长为：

$\Delta t$

第 $r$ 个 neuron 的第 $i$ 个 compartment 当前电压为：

$V_{r,i}^n$

每个 compartment 在当前时间步累计到的网络输入电流为：

$I_{r,i}^{\mathrm{net},n}$

在 `step_brain_network()` 的每一步开始，所有 neuron 都执行：

$I_{r,i}^{\mathrm{net},n} \leftarrow 0$

对应代码是：

```text
neuron->clear_currents()
```

* * *

# 2. 化学突触变量

对一条 graded chemical synapse，设它从 presynaptic neuron 的 compartment $(a,p)$ 指向 postsynaptic neuron 的 compartment $(b,q)$。

当前 presynaptic 电压为：

$V_{\mathrm{pre}}^n = V_{a,p}^n$

当前 postsynaptic 电压为：

$V_{\mathrm{post}}^n = V_{b,q}^n$

这条 synapse 有参数：

$g$

表示最大电导，单位是 $\mu S$；

$E_{\mathrm{rev}}$

表示反转电位；

$\tau$

表示门控时间常数；

$V_{1/2}$

表示半激活电压；

$k_s$

表示 sigmoid 斜率参数。

synapse 的稳态开放比例为：

$s_\infty(V_{\mathrm{pre}}^n)=\frac{1}{1+\exp\left(-\frac{V_{\mathrm{pre}}^n-V_{1/2}}{k_s}\right)}$

代码位置是：

```text
GradedChemicalSynapse::s_inf()
```

* * *

# 3. 化学突触门控更新

每条 chemical synapse 保存一个动态门控变量：

$s^n$

它的连续形式可以理解为：

$\frac{ds}{dt}=\frac{s_\infty(V_{\mathrm{pre}})-s}{\tau}$

代码里用显式 Euler 更新：

$s^{n+1}=s^n+\Delta t\frac{s_\infty(V_{\mathrm{pre}}^n)-s^n}{\tau}$

然后把它限制在 $[0,1]$：

$s^{n+1}\leftarrow \mathrm{clamp}(s^{n+1},0,1)$

注意，这里的 presynaptic 电压使用的是当前时间步开始时的电压：

$V_{\mathrm{pre}}^n$

而不是 neuron 更新后的电压。

* * *

# 4. 化学突触电流

chemical synapse 注入 postsynaptic compartment 的电流为：

$I_{\mathrm{chem}}^{n+1}=1000 \cdot g \cdot s^{n+1}\cdot (E_{\mathrm{rev}}-V_{\mathrm{post}}^n)$

其中：

- $g$ 的单位是 $\mu S$；
- 电压单位是 $mV$；
- $\mu S \cdot mV = nA$；
- 乘以 $1000$ 后转成 $pA$。

电流符号约定是：

$I_{\mathrm{chem}}>0$

表示向 postsynaptic compartment 注入正电流。

所以对目标 compartment 的网络输入电流做累加：

$I_{b,q}^{\mathrm{net},n} \leftarrow I_{b,q}^{\mathrm{net},n}+I_{\mathrm{chem}}^{n+1}$

对应代码是：

```text
GradedChemicalSynapse::apply_and_current_pA()
post_->add_current_pA(post_compartment_, current)
```

* * *

# 5. gap junction 电流

对一条 gap junction，设它连接 neuron compartment $(a,p)$ 和 $(b,q)$。

当前两端电压为：

$V_a^n=V_{a,p}^n$

$V_b^n=V_{b,q}^n$

gap junction 电导为：

$g_{\mathrm{gap}}$

流入 $a$ 端的电流定义为：

$I_{b\to a}^{n}=1000 \cdot g_{\mathrm{gap}}\cdot (V_b^n-V_a^n)$

于是两端输入电流分别累加：

$I_{a,p}^{\mathrm{net},n}\leftarrow I_{a,p}^{\mathrm{net},n}+I_{b\to a}^{n}$

$I_{b,q}^{\mathrm{net},n}\leftarrow I_{b,q}^{\mathrm{net},n}-I_{b\to a}^{n}$

这保证 gap junction 对两端 neuron 注入的总电流为 0：

$I_{a,p}^{\mathrm{gap}}+I_{b,q}^{\mathrm{gap}}=0$

对应代码是：

```text
GapJunction::apply_and_current_to_a_pA()
a_->add_current_pA(comp_a_, i_to_a)
b_->add_current_pA(comp_b_, -i_to_a)
```

* * *

# 6. 全网络突触应用

`SynapseNetwork::apply(dt)` 做的是两个求和：

第一，对所有 active chemical synapse：

$I_{r,i}^{\mathrm{net},n} \mathrel{+}= I_{\mathrm{chem},e}^{n+1}$

第二，对所有 active gap junction：

$I_{r,i}^{\mathrm{net},n} \mathrel{+}= I_{\mathrm{gap},e}^{n}$

所以对任意 neuron compartment $(r,i)$，进入 neuron solver 前的总网络输入电流是：

$\boxed{I_{r,i}^{\mathrm{net},n}=\sum_{e\in \mathcal{C}_{\to(r,i)}} 1000 g_e s_e^{n+1}(E_{e}-V_{r,i}^n)+\sum_{e\in \mathcal{G}_{(r,i)}} 1000 g_e(V_{\mathrm{other}(e,r,i)}^n-V_{r,i}^n)}$

其中：

$\mathcal{C}_{\to(r,i)}$

表示所有投射到 compartment $(r,i)$ 的 chemical synapse；

$\mathcal{G}_{(r,i)}$

表示所有连接到 compartment $(r,i)$ 的 gap junction；

$V_{\mathrm{other}(e,r,i)}^n$

表示 gap junction 另一端 compartment 的当前电压。

* * *

# 7. neuron 内部电压更新

突触和 gap current 都累加完成后，`step_brain_network()` 对每个 neuron 调用：

```text
neuron->step(dt_ms)
```

对第 $r$ 个 neuron，这等价于把它的网络输入电流向量：

$$
\mathbf{I}_r^{\mathrm{net},n}
=
\begin{bmatrix}
I_{r,0}^{\mathrm{net},n}\\
I_{r,1}^{\mathrm{net},n}\\
\vdots\\
I_{r,N_r-1}^{\mathrm{net},n}
\end{bmatrix}
$$

传给 `MultiCompartmentNeuron::step()`，再传给：

```text
CableSolver::advance_voltages()
```

于是每个 neuron 独立求解自己的 cable 方程：

$A_r\mathbf{V}_r^{n+1}=\mathbf{b}_r$

和单神经元文档相比，区别只是右端项多了 network 输入电流：

$$
b_{r,i}
=
\frac{C_{r,i}}{\Delta t}V_{r,i}^n
+g_{L,r,i}E_{L,r,i}
+I_{r,i}^{\mathrm{net},n}
-I_{r,i}^{\mathrm{ion},n}
$$

如果没有 voltage clamp 额外电导，brain runner 下的 extra conductance 项为 0。

求解后得到：

$\mathbf{V}_r^{n+1}=A_r^{-1}\mathbf{b}_r$

也就是每个 neuron 的 compartment 电压都从：

$\mathbf{V}_r^n$

更新到：

$\mathbf{V}_r^{n+1}$

* * *

# 8. brain 单步压缩版

一个全脑时间步可以压缩为：

$\boxed{
\mathbf{V}^n
\Rightarrow
\left[
\begin{array}{c}
s_e^{n+1}=s_e^n+\Delta t(s_{\infty,e}(V_{\mathrm{pre}}^n)-s_e^n)/\tau_e\\
I_{\mathrm{chem}}=1000 g_e s_e^{n+1}(E_e-V_{\mathrm{post}}^n)\\
I_{\mathrm{gap}}=1000 g_e(V_b^n-V_a^n)
\end{array}
\right]
\Rightarrow
\mathbf{I}^{\mathrm{net},n}
\Rightarrow
A_r\mathbf{V}_r^{n+1}=\mathbf{b}_r
}$

其中 $r=0,\dots,M-1$，每个 neuron 的 $A_r$ 都由已有 multi-compartment cable solver 构造。

* * *

# 9. 和 `cable_solver_math.md` 的关系

`cable_solver_math.md` 描述的是一个 neuron 内部的 multi-compartment 电压求解：

$A\mathbf{V}^{n+1}=\mathbf{b}$

本文件描述的是 brain 层怎样产生每个 neuron 的外部输入项：

$\mathbf{I}^{\mathrm{net},n}$

并把它放进每个 neuron 的 $\mathbf{b}$ 里。

所以两份文档的关系是：

$\boxed{
\text{brain synapse/gap current}
\Rightarrow
I^{\mathrm{net}}
\Rightarrow
\text{per-neuron cable solver}
}$

当前 C++ 没有一个单独的 `brain_math.cpp`。真正的数学分布在：

- 网络时间步：`src/brain/brain_simulation.cpp`
- chemical / gap 电流：`src/synapse.cpp`
- synapse 网络遍历与注入：`src/synapse_loader.cpp`
- neuron 内部电压求解：`src/neuron/cable_solver.cpp`


