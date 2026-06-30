#pragma once

#include <memory>
#include <string>
#include <vector>

namespace cpp_neuron {

// struct 是一种聚合数据类型；这里用来打包一个状态名和一个数值。
// 诊断或调试时暴露的单个通道内部状态，例如 gating variable。
struct ChannelStateValue {
    std::string name;
    double value = 0.0;
};

// 所有 active ion channel 的统一抽象接口。
// MultiCompartmentNeuron 只依赖这个接口，因此可以把 nca/irk/kqt3/egl19 等通道统一放进同一个 vector。
// class 默认成员是 private；这里用 public: 明确声明外部可调用的接口。
class Channel {
public:
    // virtual 析构函数保证通过 Channel* 删除派生类对象时，会正确调用派生类析构函数。
    // = default 表示使用编译器生成的默认析构实现。
    virtual ~Channel() = default;

    // 根据当前电压和时间步长更新通道内部状态，例如门控变量。
    // virtual 表示派生类可以提供自己的实现；= 0 表示这是纯虚函数，Channel 本身不能直接实例化。
    virtual void step(double voltage_mV, double dt_ms) = 0;

    // 钙依赖通道可覆写这个函数；非钙依赖通道保持默认空实现。
    // 注释掉参数名 /*...*/ 是为了说明参数含义，同时避免默认空实现里出现未使用参数警告。
    virtual void set_calcium(double /*cai_uM_per_um2*/) {}

    // 在给定电压下计算该通道贡献的电流，单位 pA。
    // const 放在函数末尾，表示这个函数不会修改通道对象的成员状态。
    virtual double current_pA(double voltage_mV) const = 0;

    // 返回通道名称，用于诊断输出和按名称汇总电流。
    // 返回 std::string 是按值返回；调用方拿到的是名字的一份拷贝。
    virtual std::string name() const = 0;

    // 可选：返回内部状态；不需要暴露状态的通道返回空 vector。
    // 这里不是纯虚函数，因为基类提供了一个默认实现：返回 {}，也就是空 vector。
    virtual std::vector<ChannelStateValue> state() const { return {}; }

    // 复制通道对象。MultiCompartmentNeuron 被按值复制时，需要复制每个多态 Channel。
    // std::unique_ptr 表示独占所有权；clone 返回一个新分配出来的 Channel 对象。
    virtual std::unique_ptr<Channel> clone() const = 0;
};

}  // namespace cpp_neuron
