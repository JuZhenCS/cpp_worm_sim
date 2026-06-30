#pragma once

#include "neuron/recording/recording.hpp"

#include <string>
#include <vector>

namespace neuron {

void write_trace_csv(const std::string& path, const std::vector<TracePoint>& trace);

void write_channel_diagnostics_csv(
    const std::string& path,
    const std::vector<ChannelDiagnosticPoint>& diagnostics);

}  // namespace neuron
