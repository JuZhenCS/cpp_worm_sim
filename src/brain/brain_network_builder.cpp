#include "brain/brain_network_builder.hpp"

#include "brain/representative_neuron_catalog.hpp"
#include "neuron/core/neuron_factory.hpp"

#include <fstream>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace brain {
namespace {

struct NeuronReference {
    std::string parameter_reference;
};

std::vector<std::string> split_csv_line(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool in_quotes = false;
    for (char ch : line) {
        if (ch == '"') {
            in_quotes = !in_quotes;
        } else if (ch == ',' && !in_quotes) {
            fields.push_back(field);
            field.clear();
        } else {
            field.push_back(ch);
        }
    }
    fields.push_back(field);
    return fields;
}

std::unordered_map<std::string, std::size_t> header_index(const std::vector<std::string>& header) {
    std::unordered_map<std::string, std::size_t> index;
    for (std::size_t i = 0; i < header.size(); ++i) {
        index.emplace(header[i], i);
    }
    return index;
}

std::string field(
    const std::vector<std::string>& row,
    const std::unordered_map<std::string, std::size_t>& header,
    const std::string& name) {
    const auto found = header.find(name);
    if (found == header.end() || found->second >= row.size()) {
        throw std::runtime_error("Missing CSV field: " + name);
    }
    return row[found->second];
}

void collect_chemical_names(const std::string& path, std::set<std::string>& names) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Could not open chemical CSV: " + path);
    }
    std::string line;
    std::getline(input, line);
    const auto header = header_index(split_csv_line(line));
    while (std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }
        const auto row = split_csv_line(line);
        names.insert(field(row, header, "pre"));
        names.insert(field(row, header, "post"));
    }
}

void collect_gap_names(const std::string& path, std::set<std::string>& names) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Could not open gap CSV: " + path);
    }
    std::string line;
    std::getline(input, line);
    const auto header = header_index(split_csv_line(line));
    while (std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }
        const auto row = split_csv_line(line);
        names.insert(field(row, header, "cell_a"));
        names.insert(field(row, header, "cell_b"));
    }
}

std::unordered_map<std::string, NeuronReference> load_neuron_references(const std::string& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Could not open neuron reference CSV: " + path);
    }

    std::string line;
    if (!std::getline(input, line)) {
        throw std::runtime_error("Empty neuron reference CSV: " + path);
    }
    const auto header = header_index(split_csv_line(line));

    std::unordered_map<std::string, NeuronReference> references;
    while (std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }
        const auto row = split_csv_line(line);
        NeuronReference reference;
        reference.parameter_reference = field(row, header, "parameter_reference");
        references.emplace(field(row, header, "neuron"), reference);
    }
    return references;
}

}  // namespace

BrainNetwork build_brain_network(const BrainNetworkConfig& config) {
    std::set<std::string> names; // 一开始为空，下两句填充，且std::set 会自动去重和排序
    collect_chemical_names(config.chemical_csv, names); // 有 &，表示传引用。也就是说，函数里面操作的不是副本，而是外面的那个 names 本体。这些 insert() 会直接把 CSV 里的 neuron 名字插入到 build_brain_network() 里的那个 names 变量。
    collect_gap_names(config.gap_csv, names); // 从 chemical CSV / gap CSV 加入 pre/post neuron 名字。
    const auto neuron_references = load_neuron_references(config.neuron_reference_csv); // 读取“每个神经元应该参考哪个模板/参数”的映射表

    BrainNetwork network; // 创建一个空的 BrainNetwork 对象，随后这个对象会被逐步填满，最后返回给调用者。
    network.neurons.reserve(names.size());
    network.neuron_names.reserve(names.size()); // 先给神经元数组预留空间。
    std::size_t idx = 0;
    for (const auto& name : names) { // “依次取出 names 集合里的每一个元素，把当前元素临时叫做 name，然后执行 { ... } 里面的代码。” &表示用引用，省内存和时间
        const auto reference_it = neuron_references.find(name); // 在 neuron_references 映射表里查找当前神经元的参数参考信息
        if (reference_it == neuron_references.end()) { // 如果没有找到当前神经元对应的参数参考信息
            throw std::runtime_error("Missing neuron parameter reference for neuron: " + name); // 抛出错误，提示哪个神经元缺少参考参数
        }
        const auto& reference = reference_it->second; // 取出查找到的参数参考记录
        const auto representative = representative_neuron_template(reference.parameter_reference, config.template_data_dir); // 根据参数参考名加载代表性神经元模板

        neuron::NeuronBuildConfig build_config; // 创建一个神经元构建配置对象
        build_config.name = name; // 设置要创建的神经元名字
        build_config.cell_file = representative.cell_file; // 设置该神经元使用的细胞形态文件
        build_config.mechanisms = representative.mechanisms; // 设置该神经元需要加载的机制

        std::shared_ptr<neuron::NeuronModel> model(neuron::create_neuron(build_config)); // 根据配置创建神经元模型，并用 shared_ptr 管理
        model->set_all_voltages(representative.initial_voltage_mV + 0.1 * (static_cast<double>(idx % 7) - 3.0)); // 设置初始电压，并给不同神经元加一个小扰动
        network.index.emplace(name, model); // 把神经元名字和模型指针放进索引，方便后续按名字查找
        network.neurons.push_back(std::move(model)); // 把神经元模型加入网络的神经元列表
        network.neuron_names.push_back(name); // 保存神经元名字，顺序与 neurons 列表对齐
        ++idx; // 更新神经元序号，供下一轮计算初始电压扰动
    }
    // 循环体的整体作用是：对 names 里的每个 neuron 名字，找到它对应的参数模板，创建一个 NeuronModel，设置初始电压，然后加入 network。
    network.synapses = neuron::load_synapse_network_csv(config.chemical_csv, config.gap_csv, network.index); // 读取突触 CSV 文件，并把读出来的突触连接保存到 network.synapses 里
    return network;
}

}  // namespace brain

